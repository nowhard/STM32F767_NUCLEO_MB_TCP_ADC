#ifndef SPI_ADC_H
#define SPI_ADC_H

#define SPI_ADC_FREQ_DIV	4
#define SPI_ADC_BUF_LEN	500
typedef struct 
{
	uint16_t buf[SPI_ADC_BUF_LEN];
	uint16_t  index;
}stSPI_ADC_Buf;



void SPI_ADC_Init(void);

#endif