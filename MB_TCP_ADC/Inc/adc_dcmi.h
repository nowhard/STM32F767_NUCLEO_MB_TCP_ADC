#ifndef ADC_DCMI_H
#define ADC_DCMI_H

#include "stm32f7xx.h"
#define ADC_RESULT_BUF_LEN (ADC_BUF_LEN/4)

typedef enum
{
	ADC_SAMPLERATE_10KHz=0,
	ADC_SAMPLERATE_20KHz=1,
	ADC_SAMPLERATE_50KHz=2,
	ADC_SAMPLERATE_100KHz=3
}enADCSamplerate;

void DCMI_ADC_Init(void);
uint64_t DCMI_ADC_GetLastTimestamp(void);
void DCMI_ADC_ResetTimestamp(void);
void DCMI_ADC_SetSamplerate(enADCSamplerate sampleRate);
void DCMI_ADC_Clock_Start(void);
void DCMI_ADC_Clock_Stop(void);

#endif
