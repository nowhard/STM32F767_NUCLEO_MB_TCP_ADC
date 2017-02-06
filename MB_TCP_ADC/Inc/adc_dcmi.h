#ifndef ADC_DCMI_H
#define ADC_DCMI_H

#include "stm32f7xx.h"
#define ADC_RESULT_BUF_LEN (ADC_BUF_LEN/4)


void DCMI_ADC_Init(void);
uint64_t DCMI_ADC_GetLastTimestamp(void);
void DCMI_ADC_ResetTimestamp(void);

#endif
