#ifndef DISCRET_OUT_H
#define DISCRET_OUT_H
#include "stm32f7xx.h"


#define DISCRET_OUT_NUM	48

void DiscretOutputs_Init(void);
void DiscretOutputs_Set(uint64_t discrOut);

#endif