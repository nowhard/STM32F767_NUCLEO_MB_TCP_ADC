#ifndef DATA_CONVERTER_H
#define DATA_CONVERTER_H

#include "stm32f7xx_hal.h"

typedef struct
{
		float k;
		float b;
}stCalibrChannel;

typedef enum
{
	ADC_CHN_CURRENT_1=0,
	ADC_CHN_CURRENT_2=1,
	ADC_CHN_CURRENT_3=2,
	ADC_CHN_CURRENT_4=3,
	ADC_CHN_CURRENT_CONV=4,
	ADC_CHN_VOLTAGE=5,
	ADC_CHN_PRESSURE=6,
}enADCCalibrChannels;

#define CHANNEL_3_CURR_TRESHOLD		250.0
#define CHANNEL_2_CURR_TRESHOLD		150.0
#define CHANNEL_1_CURR_TRESHOLD		75.0
#define CHANNEL_0_CURR_TRESHOLD		7.5


void ADC_ConvertDCMIAndAssembleUDPBuf(float *resultBuf, uint16_t *resultBufLen);
uint16_t ADC_GetRawChannelValue(uint8_t channel);
float    ADC_GetCalibratedChannelValue(enADCCalibrChannels channel);

#endif
