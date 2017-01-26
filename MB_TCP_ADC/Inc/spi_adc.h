#ifndef SPI_ADC_H
#define SPI_ADC_H

typedef struct 
{
	uint64_t 	timestamp;
	union
	{
		float 		value_calibr;
		uint16_t  value
	}
		
}stSPI_ADC;



void SPI_ADC_Init(void);

#endif