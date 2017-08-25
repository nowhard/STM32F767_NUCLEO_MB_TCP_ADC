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


extern DCMI_HandleTypeDef hdcmi;
extern DMA_HandleTypeDef hdma_dcmi;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;


#define RX_BUFF_SIZE	ADC_BUF_LEN
__IO uint8_t DCMIAdcRxBuff[RX_BUFF_SIZE];//__attribute__((at(0x20008000)));


uint8_t *ADC_buf_pnt;
uint64_t timestamp=0;

SemaphoreHandle_t xAdcBuf_Send_Semaphore=NULL;
QueueHandle_t xADC_MB_Queue;

extern sConfigInfo configInfo;

uint32_t counter_DMA_half=0;
uint32_t counter_DMA_full=0;


void DCMI_DMA_HalfTransferCallback(void);
void DCMI_DMA_TransferCallback(void);


void DCMI_ADC_Init(void)
{
	//Set mode and format of data
	//Mode -HI SPEED MODE=00
	 HAL_GPIO_WritePin(GPIO0_ADC_GPIO_Port, GPIO0_ADC_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(GPIO1_ADC_GPIO_Port, GPIO1_ADC_Pin, GPIO_PIN_RESET);
	
	//Format-Frame-Sync Discrete FORMAT=101
	 HAL_GPIO_WritePin(GPIO2_ADC_GPIO_Port, GPIO2_ADC_Pin, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(GPIO3_ADC_GPIO_Port, GPIO3_ADC_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(GPIO4_ADC_GPIO_Port, GPIO4_ADC_Pin, GPIO_PIN_SET);
	
	//SYNC ADC to HIGH
	
	HAL_GPIO_WritePin(SYNC_ADC_GPIO_Port, SYNC_ADC_Pin, GPIO_PIN_SET);
	
	vSemaphoreCreateBinary( xAdcBuf_Send_Semaphore );
	DCMI_ADC_SetSamplerate(configInfo.ConfigADC.sampleRate);
	

	HAL_DMA_RegisterCallback(&hdma_dcmi,HAL_DMA_XFER_HALFCPLT_CB_ID,DCMI_DMA_HalfTransferCallback);
	HAL_DMA_RegisterCallback(&hdma_dcmi,HAL_DMA_XFER_CPLT_CB_ID,DCMI_DMA_TransferCallback);
	
	HAL_TIM_Base_Start(&htim4);
	HAL_TIM_Base_Start(&htim5);
	
	HAL_DCMI_Start_DMA(&hdcmi,DCMI_MODE_CONTINUOUS,(uint32_t)DCMIAdcRxBuff,RX_BUFF_SIZE>>2);
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
				htim2.Init.Prescaler = 10;
				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_1KHz;
				period=(uint16_t)((108000000/10000)-1);
		}
		break;
		
		case ADC_SAMPLERATE_10KHz:
		{
				htim2.Init.Prescaler = 0;
				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_10KHz;
				period=(uint16_t)((108000000/10000)-1);
		}
		break;
		
		case ADC_SAMPLERATE_20KHz:
		{
			htim2.Init.Prescaler = 0;
			configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_20KHz;
			period=(uint16_t)((108000000/20000)-1);
		}
		break;

		case ADC_SAMPLERATE_50KHz:
		{
				htim2.Init.Prescaler = 0;
				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_50KHz;
				period=(uint16_t)((108000000/50000)-1);
		}
		break;

		case ADC_SAMPLERATE_100KHz:
		{
				htim2.Init.Prescaler = 0;
				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_100KHz;
				period=(uint16_t)((108000000/100000)-1);
		}
		break;		
		
		default:
		{
				htim2.Init.Prescaler = 0;
				configInfo.ConfigADC.sampleRate=ADC_SAMPLERATE_100KHz;
				period=(uint16_t)((108000000/100000)-1);
		}
	}
	
  htim2.Instance = TIM2;
//  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = period;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);
}

uint32_t DCMI_ADC_GetSamplerate(void)
{
		return configInfo.ConfigADC.sampleRate;
}

uint64_t DCMI_ADC_GetLastTimestamp(void)
{
	return timestamp;
}

uint64_t DCMI_ADC_GetCurrentTimestamp(void)
{
	return ((((uint64_t)(TIM5->CNT))<<16)|TIM4->CNT);
}


void DCMI_ADC_ResetTimestamp(void)
{
		__HAL_DMA_DISABLE(&hdma_dcmi);  
		TIM5->CNT=0;
		TIM4->CNT=0;
		timestamp=0;
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
	
		timestamp=((((uint64_t)(TIM5->CNT))<<16)|TIM4->CNT);
		ADC_buf_pnt=&DCMIAdcRxBuff[0];
		counter_DMA_half++;
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
	
		timestamp=((((uint64_t)(TIM5->CNT))<<16)|TIM4->CNT);
	  ADC_buf_pnt=&DCMIAdcRxBuff[ADC_BUF_LEN>>1];
		counter_DMA_full++;
		xSemaphoreGiveFromISR( xAdcBuf_Send_Semaphore, &xHigherPriorityTaskWoken);
	
		if( xHigherPriorityTaskWoken == pdTRUE )
		{
			portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		}
}


