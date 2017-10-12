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
	uint16_t val_chn0_raw;
	uint16_t val_chn1_raw;
	uint16_t val_chn2_raw;
	uint16_t val_chn3_raw;
	uint16_t val_chn4_raw;
	uint16_t val_chn5_raw;
	
	float val_250A;
	float val_150A;
	float val_75A;
	float val_7_5A;
	float val_current;
	
	float val_voltage_1;
	float val_voltage_2;
}stChnCalibrValues;


void ADC_ConvertDCMIAndAssembleUDPBuf(float *resultBuf, uint16_t *resultBufLen);
#endif
