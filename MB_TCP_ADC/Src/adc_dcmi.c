#include "adc_dcmi.h"
#include "cmsis_os.h"
#include "main.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

extern DMA_HandleTypeDef hdma_dcmi;



#define RX_BUFF_SIZE	ADC_BUF_LEN
__IO uint8_t DCMIAdcRxBuff[RX_BUFF_SIZE];


uint8_t *ADC_buf_pnt;
uint16_t ADC_last_data[ADC_CHN_NUM];
uint64_t timestamp=0;

SemaphoreHandle_t xAdcBuf_Send_Semaphore=NULL;
QueueHandle_t xADC_MB_Queue;

void Timestamp_Init(void);
void DCMI_DMA_HalfTransferCallback(void);
void DCMI_DMA_TransferCallback(void);


void DCMI_ADC_Init(void)
{
	vSemaphoreCreateBinary( xAdcBuf_Send_Semaphore );
	HAL_DMA_RegisterCallback(&hdma_dcmi,HAL_DMA_XFER_HALFCPLT_CB_ID,DCMI_DMA_HalfTransferCallback);
	HAL_DMA_RegisterCallback(&hdma_dcmi,HAL_DMA_XFER_CPLT_CB_ID,DCMI_DMA_TransferCallback);
}


uint64_t DCMI_ADC_GetLastTimestamp(void)
{
	return timestamp;
}

void DCMI_DMA_HalfTransferCallback(void)
{
		timestamp=((((uint64_t)(TIM5->CNT))<<16)|TIM4->CNT);
		ADC_buf_pnt=&DCMIAdcRxBuff[0];
		xSemaphoreGiveFromISR( xAdcBuf_Send_Semaphore, NULL);
}

void DCMI_DMA_TransferCallback(void)
{
		timestamp=((((uint64_t)(TIM5->CNT))<<16)|TIM4->CNT);
	  ADC_buf_pnt=&DCMIAdcRxBuff[ADC_BUF_LEN>>1];
		xSemaphoreGiveFromISR( xAdcBuf_Send_Semaphore, NULL);
}


