#ifndef UTILITIES_H
#define UTILITIES_H
#include "stm32f7xx_hal.h"

uint64_t ReverseBytes_UINT64(uint64_t val);

void Float_To_UINT16_Buf(float val, uint16_t *buf);
void UINT16_Buf_To_Float(uint16_t * buf, float *val);

void UINT64_To_UINT16_Buf(uint64_t val, uint16_t *buf);
void UINT16_Buf_To_UINT64(uint16_t * buf, uint64_t *val);

#endif