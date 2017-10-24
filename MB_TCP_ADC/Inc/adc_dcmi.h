#ifndef ADC_DCMI_H
#define ADC_DCMI_H

#include "stm32f7xx.h"
#include "main.h"

typedef enum
{
	ADC_SAMPLERATE_1KHz=0,
	ADC_SAMPLERATE_10KHz=1,
	ADC_SAMPLERATE_20KHz=2,
	ADC_SAMPLERATE_50KHz=3,
	ADC_SAMPLERATE_100KHz=4
}enADCSamplerate;

void DCMI_ADC_Init(void);
uint64_t DCMI_ADC_GetLastTimestamp(void);
uint64_t DCMI_ADC_GetCurrentTimestamp(void);
void DCMI_ADC_ResetTimestamp(void);
void DCMI_ADC_SetSamplerate(enADCSamplerate sampleRate);
void DCMI_ADC_Clock_Start(void);
void DCMI_ADC_Clock_Stop(void);
void DCMI_ADC_GetCurrentBufPtr(uint8_t *buf);
uint8_t DCMI_ADC_Started(void);

#endif
