#ifndef ADC_PYRO_BUF_H
#define ADC_PYRO_BUF_H

#include "stm32f7xx_hal.h"
#include "main.h"


typedef enum
{
	ADC_PYRO_BUF_FILL_STOP=0,
	ADC_PYRO_BUF_FILL_START,
}enADCPyroBufState;

typedef struct
{
	float buf[ADC_PYRO_CHN_NUM];
}stADCPyroBuf;

uint16_t 					ADC_PyroBuf_Copy(void *dst_buf, uint16_t max_size);
uint16_t 					ADC_PyroBuf_GetCurrentLength(void);
void  						ADC_PyroBuf_SetState(enADCPyroBufState state);
enADCPyroBufState ADC_PyroBuf_GetState(void);
void 							ADC_PyroBuf_SetTimestamp(uint64_t timestamp);
uint64_t					ADC_PyroBuf_GetTimestamp(void);
void 							ADC_PyroBuf_Add(float *mb_regs);

#endif
