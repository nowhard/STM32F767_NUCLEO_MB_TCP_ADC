#include "discret_out.h"
#include "main.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef enum
{
  DISCR_OUT_DISABLE = 0,
  DISCR_OUT_ENABLE
}enDiscrOutState;

typedef enum 
{
	DISCR_OUT_TRANSFER_BUSY=0,
	DISCR_OUT_TRANSFER_COMPLETE,
}enDiscrOutTransferState;

//uint16_t outputs_temp_reg_0=0xFFFF;
//uint16_t outputs_temp_reg_1=0xFFFF;
//uint16_t outputs_temp_reg_2=0xFFFF;
//uint16_t outputs_temp_reg_3=0xFFFF;
	
uint64_t	outputs_temp_reg=0xFFFFFFFFFFFFFFFF; 

#define SPI_OUT_REG_NUM	8

extern SPI_HandleTypeDef hspi5;
extern DMA_HandleTypeDef hdma_spi5_tx;

enDiscrOutTransferState DiscrOutTransferState=DISCR_OUT_TRANSFER_COMPLETE;

void SPI5_DMA_TransferCallback(void);
void DiscretOutputs_Enable(enDiscrOutState DiscrOutState);

void Discr_RelayTest_Task(void *pvParameters);

void DiscretOutputs_Init(void)
{
		DiscretOutputs_Enable(DISCR_OUT_ENABLE);
		HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);
	
		xTaskCreate( Discr_RelayTest_Task, "Relay Test Task", 256, NULL, 3, NULL );
//		HAL_DMA_RegisterCallback(&hdma_spi5_tx,HAL_DMA_XFER_CPLT_CB_ID,SPI5_DMA_TransferCallback);
}

void DiscretOutputs_Enable(enDiscrOutState DiscrOutState)
{
	if(DiscrOutState==DISCR_OUT_ENABLE)	
	{
		HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_RESET);	
	}
}


void DiscretOutputs_Set(uint64_t discrOut)
{
  static uint64_t temp_out;
	temp_out=discrOut;
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);	
	HAL_SPI_Transmit_DMA(&hspi5, (uint8_t*)&temp_out, SPI_OUT_REG_NUM);
}

//void SPI5_DMA_TransferCallback(void)
//{
//	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_SET);
//	//dly??
//	//HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);	
//}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_SET);
}

uint64_t test_relay_state=0xFFFFFFFFFFFFFFFF;
void Discr_RelayTest_Task(void *pvParameters)
{
	uint8_t i=0;
	uint64_t mask;
	while(1)
	{
			vTaskDelay(1000);
			
			mask=(uint64_t)1<<(i&0x3F);
			
			test_relay_state^=mask;
			
			DiscretOutputs_Set(test_relay_state);
			i++;
	}
}