#include "discret_out.h"

typedef enum
{
  DISCR_OUT_DISABLE = 0,
  DISCR_OUT_ENABLE
}enDiscrOutState;

void DiscretOutputs_Enable(enDiscrOutState DiscrOutState)
{
	if(DiscrOutState==DISCR_OUT_ENABLE)	
	{
		HAL_GPIO_WritePin(OE_PORT, OE_PIN, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(OE_PORT, OE_PIN, GPIO_PIN_RESET);	
	}
}

void DiscretOutputs_Set(uint64_t discrOut)
{
	uint8_t dsc_out_cnt=0;
	HAL_GPIO_WritePin(DATALED_PORT, DATALED_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
	
	for(dsc_out_cnt=0;dsc_out_cnt<DISCRET_OUT_NUM;dsc_out_cnt++)
	{
			HAL_GPIO_WritePin(DATALED_PORT, DATALED_PIN, discrOut&0x1);
			HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
			//delay
			HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
			discrOut>>1;
	}
	
	HAL_GPIO_WritePin(STROB_PORT, STROB_PIN, GPIO_PIN_SET);
			//delay
	HAL_GPIO_WritePin(STROB_PORT, STROB_PIN, GPIO_PIN_RESET);
}