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

#pragma anon_unions
typedef union 
{
	struct
	{
		uint_fast8_t b0:1;
		uint_fast8_t b1:1;
		uint_fast8_t b2:1;
		uint_fast8_t b3:1;
		uint_fast8_t b4:1;
		uint_fast8_t b5:1;
		uint_fast8_t b6:1;
		uint_fast8_t b7:1;
	};
	uint_fast8_t val;
}bytefield;

typedef union 
{
	struct
	{
		uint_fast16_t b15:1;
		uint_fast16_t b14:1;
		uint_fast16_t b13:1;
		uint_fast16_t b12:1;
		uint_fast16_t b11:1;
		uint_fast16_t b10:1;
		uint_fast16_t b9:1;
		uint_fast16_t b8:1;
		uint_fast16_t b7:1;
		uint_fast16_t b6:1;
		uint_fast16_t b5:1;
		uint_fast16_t b4:1;
		uint_fast16_t b3:1;
		uint_fast16_t b2:1;
		uint_fast16_t b1:1;
		uint_fast16_t b0:1;
	};
	uint_fast16_t val;
}wordfield;


void DCMI_ADC_Init(void);
uint64_t DCMI_ADC_GetLastTimestamp(void);
uint64_t DCMI_ADC_GetCurrentTimestamp(void);
void DCMI_ADC_ResetTimestamp(void);
void DCMI_ADC_SetSamplerate(enADCSamplerate sampleRate);
void DCMI_ADC_Clock_Start(void);
void DCMI_ADC_Clock_Stop(void);
void DCMI_ADC_GetCurrentBufPtr(uint8_t **buf);
uint8_t DCMI_ADC_Started(void);
void DCMI_ADC_ConvertSample(uint8_t *inputBuf ,uint16_t *outputBuf);
uint8_t *DCMI_ADC_GetLastSample(void);

#endif
