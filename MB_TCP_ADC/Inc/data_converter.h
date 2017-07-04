#ifndef DATA_CONVERTER_H
#define DATA_CONVERTER_H

#include "stm32f7xx_hal.h"

#define TEST_TRACEMODE

typedef struct
{
		float k;
		float b;
}stCalibrChannel;

typedef struct
{
	float val_250A;
	float val_150A;
	float val_75A;
	float val_7_5A;
	float val_current;
	
	float val_voltage_1;
	float val_voltage_2;
}stChnCalibrValues;

void ADC_ConvertBuf_Init(void);
void ADC_ConvertBuf(uint8_t *dcmiBuf,uint16_t dcmiBufLen, uint16_t *spiBuf_1, uint16_t *spiBuf_2, uint16_t spiBufLen,float *resultBuf, uint16_t *resultBufLen);
#endif
