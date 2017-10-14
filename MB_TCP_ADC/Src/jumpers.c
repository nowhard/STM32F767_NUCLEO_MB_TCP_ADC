#include "jumpers.h"
#include "main.h"
#include "stm32f7xx_hal.h"

uint8_t jumpersDevAddr=0;
uint8_t	jumpersDevIsMaster=FALSE;
uint8_t jumpersDevSectionType=SECTION_TYPE_1234;


void Jumpers_ReadSettings(void)
{
	jumpersDevAddr|=HAL_GPIO_ReadPin(ADR0_GPIO_Port, ADR0_Pin);
	jumpersDevAddr|=(HAL_GPIO_ReadPin(ADR1_GPIO_Port, ADR1_Pin)<<1);
	jumpersDevAddr|=(HAL_GPIO_ReadPin(ADR2_GPIO_Port, ADR2_Pin)<<2);
	
	jumpersDevAddr&=0x7;	
	
	if(HAL_GPIO_ReadPin(ADR3_GPIO_Port, ADR3_Pin)) //master-slave
	{
			jumpersDevIsMaster=TRUE;
	}

	if(HAL_GPIO_ReadPin(ADR4_GPIO_Port, ADR4_Pin)) //section (1-2-3-4)or (5-6)
	{
			jumpersDevSectionType=SECTION_TYPE_56;
	}
	
//	HAL_GPIO_WritePin(BUSE_SYNC_GPIO_Port, BUSE_SYNC_Pin, GPIO_PIN_SET); //Настройка направления дифф. драйверов
//	HAL_GPIO_WritePin(BUSE_SIG_GPIO_Port, BUSE_SIG_Pin, GPIO_PIN_RESET);
}