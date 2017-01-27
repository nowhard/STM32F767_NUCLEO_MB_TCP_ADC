#ifndef ADC_DCMI_H
#define ADC_DCMI_H

#include "stm32f7xx.h"

void DCMI_ADC_Init(void);
uint64_t DCMI_ADC_GetLastTimestamp(void);

#endif
