#include "discret_out.h"
#include "main.h"
#include "utilities.h"

#include "FreeRTOS.h"
#include "task.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

#define OUTPUTS_REG_ALL_RELAY_OFF		0xFFFFFFFFFFFFFFFF
#define SPI_OUT_REG_NUM							8

typedef enum
{
  DISCR_OUT_DISABLE = 0,
  DISCR_OUT_ENABLE
}enDiscrOutState;

//typedef enum 
//{
//	DISCR_OUT_TRANSFER_BUSY=0,
//	DISCR_OUT_TRANSFER_COMPLETE,
//}enDiscrOutTransferState;




uint8_t 	discrOutSequenceProgress=0;
uint64_t	discrOutTempReg=OUTPUTS_REG_ALL_RELAY_OFF; 
stSetSequenceParams	discrOutSequenceParams={OUTPUTS_REG_ALL_RELAY_OFF,OUTPUTS_REG_ALL_RELAY_OFF,OUTPUTS_REG_ALL_RELAY_OFF,10,1};


extern SPI_HandleTypeDef hspi5;
extern DMA_HandleTypeDef hdma_spi5_tx;

//enDiscrOutTransferState DiscrOutTransferState=DISCR_OUT_TRANSFER_COMPLETE;

void SPI5_DMA_TransferCallback(void);
void DiscretOutputs_Enable(enDiscrOutState DiscrOutState);


#define SEQUENCE_SET_TASK_STACK_SIZE	128
#define SEQUENCE_SET_TASK_PRIO				5

void DiscretOutputs_SetSequence_Task( void *pvParameters );

void DiscretOutputs_Init(void)
{
		DiscretOutputs_Enable(DISCR_OUT_ENABLE);
		HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);
		
		DiscretOutputs_Set(OUTPUTS_REG_ALL_RELAY_OFF);
		HAL_GPIO_WritePin(ENABLE_OUT_1_GPIO_Port, ENABLE_OUT_1_Pin, GPIO_PIN_RESET);//включаем оптроны реле
		HAL_GPIO_WritePin(ENABLE_OUT_7_GPIO_Port, ENABLE_OUT_7_Pin, GPIO_PIN_RESET);
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
	static  uint64_t temp_out;
	temp_out=ReverseBytes_UINT64(discrOut);
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);	
	HAL_SPI_Transmit_DMA(&hspi5, (uint8_t*)&temp_out, SPI_OUT_REG_NUM);
}


//static stSetSequenceParams SetSequenceParams;

uint8_t DiscretOutputs_StartSequence(void)
{
	if(!discrOutSequenceProgress)
	{
			xTaskCreate( DiscretOutputs_SetSequence_Task, "Set Sequence  Task", SEQUENCE_SET_TASK_STACK_SIZE, (void *)&discrOutSequenceParams, SEQUENCE_SET_TASK_PRIO, NULL );
	}
}


void DiscretOutputs_SetSequence_Task( void *pvParameters )
{
	uint16_t i=0;
	stSetSequenceParams *taskParams;

	
	discrOutSequenceProgress=1;
	taskParams=(stSetSequenceParams*)pvParameters;

	if(IS_DISCR_OUT_TIME(taskParams->time) && IS_DISCR_OUT_NUM_CYCLES(taskParams->num_cycles))
	{
			for(i=0;i<taskParams->num_cycles;i++)
			{
				discrOutTempReg=taskParams->state_1;
				DiscretOutputs_Set(taskParams->state_1);
				vTaskDelay(taskParams->time);
				discrOutTempReg=taskParams->state_2;
				DiscretOutputs_Set(taskParams->state_2);
				vTaskDelay(taskParams->time);
			}
			discrOutTempReg=taskParams->state_end;
			DiscretOutputs_Set(taskParams->state_end);
	}
	discrOutSequenceProgress=0;
	vTaskDelete(NULL);
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_SET);
}