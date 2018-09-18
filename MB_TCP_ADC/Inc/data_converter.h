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

#define CHANNEL_3_CURR_TRESHOLD		1000.0
#define CHANNEL_2_CURR_TRESHOLD		400.0
#define CHANNEL_1_CURR_TRESHOLD		200.0
#define CHANNEL_0_CURR_TRESHOLD		100.0

void 			ADC_DataConverter_Init(void);
void 			ADC_ConvertDCMIAndAssembleUDPBuf(float *resultBuf, uint16_t *resultBufLen);
uint16_t 	ADC_GetRawChannelValue(uint8_t channel);
float    	ADC_GetCalibratedChannelValue(enADCCalibrChannels channel);
float     ADC_GetCalibratedChannelInstantValue(enADCCalibrChannels channel);
void 			ADC_SetUDPTransferEnabled(uint8_t state);
uint8_t 	ADC_GetUDPTransferState(void);
uint8_t 	ADC_GetSamplingState(void);
#endif
