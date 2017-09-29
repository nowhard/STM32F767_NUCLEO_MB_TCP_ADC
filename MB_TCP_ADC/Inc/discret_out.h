#ifndef DISCRET_OUT_H
#define DISCRET_OUT_H
#include "stm32f7xx.h"

#define DISCR_OUT_TIME_MIN		10		
#define DISCR_OUT_TIME_MAX		10000 
#define IS_DISCR_OUT_TIME(__TIME__) (((__TIME__) >=DISCR_OUT_TIME_MIN) && ((__TIME__) <= DISCR_OUT_TIME_MAX))

#define DISCR_OUT_NUM_CYCLES_MIN		1		
#define DISCR_OUT_NUM_CYCLES_MAX		100 
#define IS_DISCR_OUT_NUM_CYCLES(__NUM_CYCLES__) (((__NUM_CYCLES__) >=DISCR_OUT_NUM_CYCLES_MIN) && ((__NUM_CYCLES__) <= DISCR_OUT_NUM_CYCLES_MAX))

typedef struct
{
	uint64_t state_1;
	uint64_t state_2;
	uint64_t state_end;
	uint16_t time; 
	uint16_t num_cycles;
}stSetSequenceParams;

void DiscretOutputs_Init(void);
void DiscretOutputs_Set(uint64_t discrOut);
uint8_t DiscretOutputs_StartSequence(void);

#endif