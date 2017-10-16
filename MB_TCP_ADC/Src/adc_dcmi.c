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


extern DCMI_HandleTypeDef hdcmi;
extern DMA_HandleTypeDef hdma_dcmi;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim9;


#define RX_BUFF_SIZE	ADC_DCMI_BUF_LEN
__IO uint8_t DCMIAdcRxBuff[RX_BUFF_SIZE]={0};


uint8_t *ADC_DCMI_buf_pnt;
uint64_t lastDCMITimestamp=0;

SemaphoreHandle_t xAdcBuf_Send_Semaphore=NULL;
QueueHandle_t xADC_MB_Queue;

extern sConfigInfo configInfo;




void DCMI_DMA_HalfTransferCallback(void);
void DCMI_DMA_TransferCallback(void);


void DCMI_ADC_Init(void)
{
	ADC_DCMI_buf_pnt=&DCMIAdcRxBuff[0];
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
	DCMI_ADC_SetSamplerate(configInfo.ConfigADC.sampleRate);
	

	HAL_DMA_RegisterCallback(&hdma_dcmi,HAL_DMA_XFER_HALFCPLT_CB_ID,DCMI_DMA_HalfTransferCallback);
	HAL_DMA_RegisterCallback(&hdma_dcmi,HAL_DMA_XFER_CPLT_CB_ID,DCMI_DMA_TransferCallback);
	
	HAL_TIM_Base_Start(&htim4);
	HAL_TIM_Base_Start(&htim5);
	
	HAL_DCMI_Start_DMA(&hdcmi,DCMI_MODE_CONTINUOUS,(uint32_t)DCMIAdcRxBuff,RX_BUFF_SIZE>>2);
	
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
	HAL_TIM_Base_Start_IT(&htim9);
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
				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_1KHz;
				period=(uint16_t)((108000000/10000)-1);
		}
		break;
		
//		case ADC_SAMPLERATE_10KHz:
//		{
//				htim2.Init.Prescaler = 0;
//				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_10KHz;
//				period=(uint16_t)((108000000/10000)-1);
//		}
//		break;
//		
//		case ADC_SAMPLERATE_20KHz:
//		{
//			htim2.Init.Prescaler = 0;
//			configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_20KHz;
//			period=(uint16_t)((108000000/20000)-1);
//		}
//		break;

//		case ADC_SAMPLERATE_50KHz:
//		{
//				htim2.Init.Prescaler = 0;
//				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_50KHz;
//				period=(uint16_t)((108000000/50000)-1);
//		}
//		break;

//		case ADC_SAMPLERATE_100KHz:
//		{
//				htim2.Init.Prescaler = 0;
//				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_100KHz;
//				period=(uint16_t)((108000000/100000)-1);
//		}
//		break;		
		
		default:
		{
				htim2.Init.Prescaler = 10-1;
				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_1KHz;
				period=(uint16_t)((108000000/10000)-1);
		}
	}
	
  htim2.Instance = TIM2;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = (uint16_t)(period*configInfo.ConfigADC.freqCorrectionFactor);
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);
}

inline uint32_t DCMI_ADC_GetSamplerate(void)
{
		return configInfo.ConfigADC.sampleRate;
}

inline uint64_t DCMI_ADC_GetLastTimestamp(void)
{
	return lastDCMITimestamp;
}

inline uint64_t DCMI_ADC_GetCurrentTimestamp(void)
{
	return ((((uint64_t)(TIM5->CNT))<<16)|TIM4->CNT);
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
}

void DCMI_ADC_Clock_Stop(void)
{
		HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
}

void DCMI_DMA_HalfTransferCallback(void)
{
		static portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
	
		lastDCMITimestamp=DCMI_ADC_GetCurrentTimestamp();
		ADC_DCMI_buf_pnt=&DCMIAdcRxBuff[0];
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
	  ADC_DCMI_buf_pnt=&DCMIAdcRxBuff[ADC_DCMI_BUF_LEN>>1];
		SPI_ADC_ResetIndex();
		xSemaphoreGiveFromISR( xAdcBuf_Send_Semaphore, &xHigherPriorityTaskWoken);
	
		if( xHigherPriorityTaskWoken == pdTRUE )
		{
			portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		}
}


