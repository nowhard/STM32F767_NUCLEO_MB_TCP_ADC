#ifndef DATA_CONVERTER_H
#define DATA_CONVERTER_H

#include "stm32f7xx_hal.h"

void ADC_ConvertBuf(uint8_t *dcmiBuf,uint16_t dcmiBufLen, uint16_t *resultBuf);
void ADC_CalibrateBuf(uint16_t *resultBuf, float *calibrateBuf);

#endif