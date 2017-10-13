#ifndef DATA_CONVERTER_H
#define DATA_CONVERTER_H

#include "stm32f7xx_hal.h"

typedef struct
{
		float k;
		float b;
}stCalibrChannel;

typedef struct
{
	uint16_t val_chn_raw[6];
	float val_current[4];
	float val_current_conv;
	
	float val_voltage;
	float val_pressure;
}stChnCalibrValues;


void ADC_ConvertDCMIAndAssembleUDPBuf(float *resultBuf, uint16_t *resultBufLen);
#endif
