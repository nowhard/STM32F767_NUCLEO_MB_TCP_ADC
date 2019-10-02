#include "adc_dcmi.h"
#include "cmsis_os.h"
#include "main.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "cfg_info.h"
#include "spi_adc.h"
#include "utilities.h"
#include "data_converter.h"
#include "jumpers.h"


extern DCMI_HandleTypeDef hdcmi;
extern DMA_HandleTypeDef hdma_dcmi;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim9;


#define RX_BUFF_SIZE	ADC_DCMI_BUF_LEN
static __IO uint8_t DCMIAdcRxBuff[RX_BUFF_SIZE]={0};
static uint8_t *ADC_DCMI_buf_ptr;
static uint64_t lastDCMITimestamp=0;
static uint8_t DCMIADCStarted=FALSE;

SemaphoreHandle_t xAdcBuf_Send_Semaphore=NULL;

extern sConfigInfo configInfo;

void DCMI_DMA_HalfTransferCallback(void);
void DCMI_DMA_TransferCallback(void);


void DCMI_ADC_Init(void)
{
	ADC_DCMI_buf_ptr=&DCMIAdcRxBuff[0];
	//Set mode and format of data ADC
	//Mode -HI SPEED MODE=00
	 HAL_GPIO_WritePin(GPIO0_ADC_GPIO_Port, GPIO0_ADC_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(GPIO1_ADC_GPIO_Port, GPIO1_ADC_Pin, GPIO_PIN_RESET);
	
	//Format-Frame-Sync Discrete FORMAT=101
	 HAL_GPIO_WritePin(GPIO2_ADC_GPIO_Port, GPIO2_ADC_Pin, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(GPIO3_ADC_GPIO_Port, GPIO3_ADC_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(GPIO4_ADC_GPIO_Port, GPIO4_ADC_Pin, GPIO_PIN_SET);
	
	//SYNC ADC to HIGH
	
	HAL_GPIO_WritePin(SYNC_ADC_GPIO_Port, SYNC_ADC_Pin, GPIO_PIN_SET);
	
	xAdcBuf_Send_Semaphore=xSemaphoreCreateBinary();
	DCMI_ADC_SetSamplerate(ADC_SAMPLERATE_1KHz);
	

	HAL_DMA_RegisterCallback(&hdma_dcmi,HAL_DMA_XFER_HALFCPLT_CB_ID,DCMI_DMA_HalfTransferCallback);
	HAL_DMA_RegisterCallback(&hdma_dcmi,HAL_DMA_XFER_CPLT_CB_ID,DCMI_DMA_TransferCallback);
	
	HAL_TIM_Base_Start(&htim4);
	HAL_TIM_Base_Start(&htim5);
	
	HAL_DCMI_Start_DMA(&hdcmi,DCMI_MODE_CONTINUOUS,(uint32_t)DCMIAdcRxBuff,RX_BUFF_SIZE>>2);
	
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
	HAL_TIM_Base_Start_IT(&htim9);
	
	if(Jumpers_DevIsMaster())
	{
		DCMI_ADC_Clock_Start();	
	}
}

void DCMI_ADC_SetSamplerate(enADCSamplerate sampleRate)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
	uint16_t period;
	
	switch(sampleRate)
	{
		case ADC_SAMPLERATE_1KHz:
		{
				htim2.Init.Prescaler = 10-1;
				period=(uint16_t)((108000000/10000)-1);
		}
		break;
		
		case ADC_SAMPLERATE_10KHz:
		{
				htim2.Init.Prescaler = 0;
				period=(uint16_t)((108000000/10000)-1);
		}
		break;
		
		case ADC_SAMPLERATE_20KHz:
		{
			htim2.Init.Prescaler = 0;
			period=(uint16_t)((108000000/20000)-1);
		}
		break;

		case ADC_SAMPLERATE_50KHz:
		{
				htim2.Init.Prescaler = 0;
				period=(uint16_t)((108000000/50000)-1);
		}
		break;

		case ADC_SAMPLERATE_100KHz:
		{
				htim2.Init.Prescaler = 0;
				period=(uint16_t)((108000000/100000)-1);
		}
		break;		
		
		default:
		{
				htim2.Init.Prescaler = 10-1;
				period=(uint16_t)((108000000/10000)-1);
		}
	}
	
	
	
  htim2.Instance = TIM2;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = (uint16_t)(period*configInfo.ConfigADC.freqCorrectionFactor);
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	
	HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);	
  HAL_TIM_Base_Init(&htim2);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
}

inline uint32_t DCMI_ADC_GetSamplerate(void)
{
		return ADC_SAMPLERATE_1KHz;
}

inline uint64_t DCMI_ADC_GetLastTimestamp(void)
{
	return lastDCMITimestamp;
}

inline uint64_t DCMI_ADC_GetCurrentTimestamp(void)
{
	return ((((uint64_t)(TIM5->CNT))<<16)|TIM4->CNT);
}

void DCMI_ADC_GetCurrentBufPtr(uint8_t **buf)
{
		*buf=ADC_DCMI_buf_ptr;
}

void DCMI_ADC_ResetTimestamp(void)
{
		__HAL_DMA_DISABLE(&hdma_dcmi);  
		TIM5->CNT=0;
		TIM4->CNT=0;
		lastDCMITimestamp=0;
	  HAL_DCMI_Start_DMA(&hdcmi,DCMI_MODE_CONTINUOUS,(uint32_t)DCMIAdcRxBuff,RX_BUFF_SIZE>>2);	
		__HAL_DMA_ENABLE(&hdma_dcmi); 
}

void DCMI_ADC_Clock_Start(void)
{
		HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
		DCMIADCStarted=TRUE;
}

void DCMI_ADC_ConvertSample(uint8_t *inputBuf ,uint16_t *outputBuf)
{
		bytefield byte;
		wordfield out1, out2, out3, out4, out5, out6, out7, out8;
	
			byte.val=inputBuf[0];
			out1.b0=byte.b0;
			out2.b0=byte.b1;
			out3.b0=byte.b2;
			out4.b0=byte.b3;


			byte.val=inputBuf[1];
			out1.b1=byte.b0;
			out2.b1=byte.b1;
			out3.b1=byte.b2;
			out4.b1=byte.b3;

			
			byte.val=inputBuf[2];
			out1.b2=byte.b0;
			out2.b2=byte.b1;
			out3.b2=byte.b2;
			out4.b2=byte.b3;


			byte.val=inputBuf[3];
			out1.b3=byte.b0;
			out2.b3=byte.b1;
			out3.b3=byte.b2;
			out4.b3=byte.b3;


			byte.val=inputBuf[4];
			out1.b4=byte.b0;
			out2.b4=byte.b1;
			out3.b4=byte.b2;
			out4.b4=byte.b3;


			byte.val=inputBuf[5];
			out1.b5=byte.b0;
			out2.b5=byte.b1;
			out3.b5=byte.b2;
			out4.b5=byte.b3;


			byte.val=inputBuf[6];
			out1.b6=byte.b0;
			out2.b6=byte.b1;
			out3.b6=byte.b2;
			out4.b6=byte.b3;


			byte.val=inputBuf[7];
			out1.b7=byte.b0;
			out2.b7=byte.b1;
			out3.b7=byte.b2;
			out4.b7=byte.b3;


			byte.val=inputBuf[8];
			out1.b8=byte.b0;
			out2.b8=byte.b1;
			out3.b8=byte.b2;
			out4.b8=byte.b3;


			byte.val=inputBuf[9];
			out1.b9=byte.b0;
			out2.b9=byte.b1;
			out3.b9=byte.b2;
			out4.b9=byte.b3;


			byte.val=inputBuf[10];
			out1.b10=byte.b0;
			out2.b10=byte.b1;
			out3.b10=byte.b2;
			out4.b10=byte.b3;


			byte.val=inputBuf[11];
			out1.b11=byte.b0;
			out2.b11=byte.b1;
			out3.b11=byte.b2;
			out4.b11=byte.b3;


			byte.val=inputBuf[12];
			out1.b12=byte.b0;
			out2.b12=byte.b1;
			out3.b12=byte.b2;
			out4.b12=byte.b3;


			byte.val=inputBuf[13];
			out1.b13=byte.b0;
			out2.b13=byte.b1;
			out3.b13=byte.b2;
			out4.b13=byte.b3;


			byte.val=inputBuf[14];
			out1.b14=byte.b0;
			out2.b14=byte.b1;
			out3.b14=byte.b2;
			out4.b14=byte.b3;


			byte.val=inputBuf[15];
			out1.b15=byte.b0;
			out2.b15=byte.b1;
			out3.b15=byte.b2;
			out4.b15=byte.b3;
			
			
			outputBuf[0]=out1.val;
			outputBuf[1]=out2.val;
			outputBuf[2]=out3.val;
			outputBuf[3]=out4.val;
}

uint8_t *DCMI_ADC_GetLastSample(void)
{
		int32_t 	bufIndex=0;
		bufIndex=((sizeof(DCMIAdcRxBuff)-hdma_dcmi.Instance->NDTR)/ADC_DCMI_NUM_BITS)*ADC_DCMI_NUM_BITS;
		bufIndex -= ADC_DCMI_NUM_BITS;
		if(bufIndex < 0)
		{
				bufIndex = sizeof(DCMIAdcRxBuff) - ADC_DCMI_NUM_BITS;
		}
		return &DCMIAdcRxBuff[bufIndex];
}

void DCMI_ADC_Clock_Stop(void)
{
		HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
		DCMIADCStarted=FALSE;
}

uint8_t DCMI_ADC_Started(void)
{
		return DCMIADCStarted;
}

void DCMI_DMA_HalfTransferCallback(void)
{
		static portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
	
		lastDCMITimestamp=DCMI_ADC_GetCurrentTimestamp();
		ADC_DCMI_buf_ptr=&DCMIAdcRxBuff[0];
		xSemaphoreGiveFromISR( xAdcBuf_Send_Semaphore, &xHigherPriorityTaskWoken );
	
	  if( xHigherPriorityTaskWoken == pdTRUE )
		{
			portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		}
}

void DCMI_DMA_TransferCallback(void)
{
		static portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
	
		lastDCMITimestamp=DCMI_ADC_GetCurrentTimestamp();
	  ADC_DCMI_buf_ptr=&DCMIAdcRxBuff[ADC_DCMI_BUF_LEN>>1];
		SPI_ADC_ResetIndex();
		xSemaphoreGiveFromISR( xAdcBuf_Send_Semaphore, &xHigherPriorityTaskWoken);
	
		if( xHigherPriorityTaskWoken == pdTRUE )
		{
			portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		}
}




