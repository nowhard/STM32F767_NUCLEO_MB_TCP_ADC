#ifndef SPI_ADC_H
#define SPI_ADC_H

#include "main.h"


typedef struct 
{
	uint16_t buf[SPI_ADC_BUF_LEN];
	uint16_t  index;
}stSPI_ADC_Buf;

void SPI_ADC_TimerCallback(void);

//void SPI_ADC_Init(void);
void 		 SPI_ADC_ResetIndex(void);
void 		 SPI_ADC_GetCurrentBufPtr(SPI_HandleTypeDef *hspi,uint16_t **buf);
uint16_t SPI_ADC_GetCurrentValue(SPI_HandleTypeDef *hspi);

#endif
