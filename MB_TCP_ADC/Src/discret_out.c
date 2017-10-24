#include "discret_out.h"
#include "main.h"
#include "utilities.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

#define OUTPUTS_REG_ALL_RELAY_OFF		0xFFFFFFFFFFFFFFFF
#define SPI_OUT_BYTES_NUM						8

typedef enum
{
  DISCR_OUT_DISABLE = 0,
  DISCR_OUT_ENABLE
}enDiscrOutState;


static uint8_t 	discrOutSequenceProgress=FALSE;
static uint64_t	discrOutReg=OUTPUTS_REG_ALL_RELAY_OFF; 
stSequenceParams	discrOutSequenceParams={OUTPUTS_REG_ALL_RELAY_OFF,OUTPUTS_REG_ALL_RELAY_OFF,OUTPUTS_REG_ALL_RELAY_OFF,10,1};


extern SPI_HandleTypeDef hspi5;
extern DMA_HandleTypeDef hdma_spi5_tx;

xSemaphoreHandle xOutSequenceSem;

void SPI5_DMA_TransferCallback(void);
void DiscretOutputs_Enable(enDiscrOutState DiscrOutState);


#define SEQUENCE_SET_TASK_STACK_SIZE	128
#define SEQUENCE_SET_TASK_PRIO				5

void DiscretOutputs_SetSequence_Task( void *pvParameters );

void DiscretOutputs_Init(void)
{
		xOutSequenceSem=xSemaphoreCreateBinary();
	
		DiscretOutputs_Enable(DISCR_OUT_ENABLE);
		HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);
		
		DiscretOutputs_Set(OUTPUTS_REG_ALL_RELAY_OFF);
		HAL_GPIO_WritePin(ENABLE_OUT_1_GPIO_Port, ENABLE_OUT_1_Pin, GPIO_PIN_SET);//включаем оптроны реле
		HAL_GPIO_WritePin(ENABLE_OUT_7_GPIO_Port, ENABLE_OUT_7_Pin, GPIO_PIN_RESET);

		xTaskCreate( DiscretOutputs_SetSequence_Task, "Set Sequence  Task", SEQUENCE_SET_TASK_STACK_SIZE, (void *)&discrOutSequenceParams, SEQUENCE_SET_TASK_PRIO, NULL );
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
	discrOutReg=discrOut;
	temp_out=ReverseBytes_UINT64(discrOut);
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_RESET);	
	HAL_SPI_Transmit_DMA(&hspi5, (uint8_t*)&temp_out, SPI_OUT_BYTES_NUM);
}

uint64_t DiscretOutputs_Get(void)
{
		return discrOutReg;
}

void DiscretOutputs_StartSequence(void)
{
	if(!discrOutSequenceProgress)
	{
			xSemaphoreGive(xOutSequenceSem);
	}
}


uint8_t DiscretOutputs_SequenceInProgress(void)
{
		return discrOutSequenceProgress;
}

void DiscretOutputs_SetSequenceState(enSequenceState state, uint64_t discrOut)
{
		switch(state)
		{
			case DISCR_OUT_SEQ_STATE_1:
			{
					discrOutSequenceParams.state_1=state;
			}
			break;
			
			case DISCR_OUT_SEQ_STATE_2:
			{
					discrOutSequenceParams.state_2=state;
			}
			break;
			
			case DISCR_OUT_SEQ_STATE_END:
			{
					discrOutSequenceParams.state_end=state;
			}
			break;

			default:
			{
			}
			break;
		}
}

uint8_t DiscretOutputs_SetSequenceImpulseTime(uint16_t time)
{
	if(!IS_DISCR_OUT_TIME(time))
	{
			return FALSE;
	}
	
	discrOutSequenceParams.time=time;
	return TRUE;
}

uint8_t DiscretOutputs_SetSequenceNumCycles(uint16_t numCycles)
{
	if(!IS_DISCR_OUT_NUM_CYCLES(numCycles))
	{
			return FALSE;
	}
	
	discrOutSequenceParams.num_cycles=numCycles;
	return TRUE;
}

void DiscretOutputs_GetSequenceParams(stSequenceParams *sequenceParams)
{
		*sequenceParams=discrOutSequenceParams;
}

void DiscretOutputs_SetSequence_Task( void *pvParameters )
{
	uint16_t cyclesCnt=0;
	stSequenceParams *sequenceParams;

	while(1)
	{
			xSemaphoreTake(xOutSequenceSem,portMAX_DELAY);
			
			discrOutSequenceProgress=TRUE;
			sequenceParams=(stSequenceParams*)pvParameters;

			if(IS_DISCR_OUT_TIME(sequenceParams->time) && IS_DISCR_OUT_NUM_CYCLES(sequenceParams->num_cycles))
			{
					for(cyclesCnt=0;cyclesCnt<sequenceParams->num_cycles;cyclesCnt++)
					{
							DiscretOutputs_Set(sequenceParams->state_1);
							vTaskDelay(sequenceParams->time);
							DiscretOutputs_Set(sequenceParams->state_2);
							vTaskDelay(sequenceParams->time);
					}
					DiscretOutputs_Set(sequenceParams->state_end);
			}
			discrOutSequenceProgress=FALSE;
	}
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	HAL_GPIO_WritePin(STROB_GPIO_Port, STROB_Pin, GPIO_PIN_SET);
}
