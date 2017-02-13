#include "discret_out.h"
#include "main.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

typedef enum
{
  DISCR_OUT_DISABLE = 0,
  DISCR_OUT_ENABLE
}enDiscrOutState;

extern SPI_HandleTypeDef hspi5;
extern DMA_HandleTypeDef hdma_spi5_tx;

void SPI5_DMA_TransferCallback(void);

void DiscretOutputs_Init(void)
{
		HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);
		HAL_DMA_RegisterCallback(&hdma_spi5_tx,HAL_DMA_XFER_CPLT_CB_ID,(void*)SPI5_DMA_TransferCallback);
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
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);	
	HAL_SPI_Transmit_DMA(&hspi5, (uint8_t*)&discrOut, 6);
}

void SPI5_DMA_TransferCallback(void)
{
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_SET);
	//dly??
	//HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);	
}