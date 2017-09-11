#include "discret_out.h"
#include "main.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

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

//#define MASK_RES_FINE_GROUP_1		0x0020202020202000
//#define MASK_RES_FINE_GROUP_2		0x0040404040404000
//#define MASK_RES_FINE_GROUP_3		0x0080808080808000
//#define MASK_RES_FINE_GROUP_4		0x0101010101010000
//#define MASK_RES_FINE_GROUP_5		0x0202020202020000
//#define MASK_RES_FINE_GROUP_6		0x0404040404040000
//#define MASK_RES_FINE_GROUP_7		0x0808080808080000
//#define MASK_RES_FINE_GROUP_8		0x1010101010100000

#define RES_FINE_REL_NUM		8

const uint64_t mask_res_fine_group[RES_FINE_REL_NUM]={0x0020202020202000,
																											0x0040404040404000,
																											0x0080808080808000,
																											0x0101010101010000,
																											0x0202020202020000,
																											0x0404040404040000,
																											0x0808080808080000,
																											0x1010101010100000};

#define OUTPUTS_REG_ALL_RELAY_OFF		0xFFFFFFFFFFFFFFFF
																									
	
uint64_t	outputs_temp_reg=OUTPUTS_REG_ALL_RELAY_OFF; 

#define SPI_OUT_REG_NUM	8

extern SPI_HandleTypeDef hspi5;
extern DMA_HandleTypeDef hdma_spi5_tx;

enDiscrOutTransferState DiscrOutTransferState=DISCR_OUT_TRANSFER_COMPLETE;

void SPI5_DMA_TransferCallback(void);
void DiscretOutputs_Enable(enDiscrOutState DiscrOutState);

void DiscretOutputs_Init(void)
{
		DiscretOutputs_Enable(DISCR_OUT_ENABLE);
		HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);
		outputs_temp_reg=OUTPUTS_REG_ALL_RELAY_OFF;
		DiscretOutputs_Set(&outputs_temp_reg);
		HAL_GPIO_WritePin(ENABLE_OUT_1_GPIO_Port, ENABLE_OUT_1_Pin, GPIO_PIN_RESET);//включаем оптроны реле
		HAL_GPIO_WritePin(ENABLE_OUT_1_GPIO_Port, ENABLE_OUT_7_Pin, GPIO_PIN_RESET);
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


void DiscretOutputs_Set(uint64_t *discrOut)
{
	uint8_t rel_bit_cnt=0;
	
	for(rel_bit_cnt=0;rel_bit_cnt<RES_FINE_REL_NUM;rel_bit_cnt++)
	{
			if((*discrOut)&((uint64_t)1<<rel_bit_cnt))
			{
					*discrOut|=mask_res_fine_group[rel_bit_cnt];
			}
	}
	
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);	
	HAL_SPI_Transmit_DMA(&hspi5, (uint8_t*)discrOut, SPI_OUT_REG_NUM);
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