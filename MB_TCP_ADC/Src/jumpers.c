//#include "jumpers.h"
#include "main.h"
#include "stm32f7xx_hal.h"

uint8_t jumpers_dev_addr=0;

void Jumpers_ReadSettings(void)
{
	jumpers_dev_addr|=HAL_GPIO_ReadPin(ADR0_GPIO_Port, ADR0_Pin);
	jumpers_dev_addr|=(HAL_GPIO_ReadPin(ADR1_GPIO_Port, ADR1_Pin)<<1);
	jumpers_dev_addr|=(HAL_GPIO_ReadPin(ADR2_GPIO_Port, ADR2_Pin)<<2);
	
	jumpers_dev_addr&=0x7;	

	
//	if(HAL_GPIO_ReadPin(ADR3_GPIO_Port, ADR3_Pin))
//	{
//			HAL_GPIO_WritePin(BUSE_SYNC_GPIO_Port, BUSE_SYNC_Pin, GPIO_PIN_SET);
//	}
//	else
//	{
//			HAL_GPIO_WritePin(BUSE_SYNC_GPIO_Port, BUSE_SYNC_Pin, GPIO_PIN_RESET);
//	}
	
	HAL_GPIO_WritePin(BUSE_SYNC_GPIO_Port, BUSE_SYNC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BUSE_SIG_GPIO_Port, BUSE_SIG_Pin, GPIO_PIN_RESET);
}