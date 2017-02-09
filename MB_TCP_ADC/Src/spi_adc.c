#include "data_converter.h"
#include "cmsis_os.h"
#include "main.h"
#include "spi_adc.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi6;

static uint8_t spi3_buf[3];
static uint8_t spi6_buf[3];



stSPI_ADC_Buf SPI3_ADC_Buf;
stSPI_ADC_Buf SPI6_ADC_Buf;

uint16_t *currentSPI3_ADC_Buf;
uint16_t *currentSPI6_ADC_Buf;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM9)
	{
		HAL_SPI_Receive_DMA(&hspi3, spi3_buf, 3);
		HAL_SPI_Receive_DMA(&hspi6, spi6_buf, 3);
	} 
}


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance==SPI3)
	{
			//osMessagePut( ADC_SPI3_QueueHandle, (uint32_t)&spi3_buf[1], osWaitForever );
			SPI3_ADC_Buf.buf[SPI3_ADC_Buf.index]=*(uint16_t *)&spi3_buf[1];
			SPI3_ADC_Buf.index++;
			if(SPI3_ADC_Buf.index==SPI_ADC_BUF_LEN)
			{
					SPI3_ADC_Buf.index=0;
					currentSPI3_ADC_Buf=&SPI3_ADC_Buf.buf[SPI_ADC_BUF_LEN>>1];
			}
			else if(SPI3_ADC_Buf.index==(SPI_ADC_BUF_LEN>>1))
			{
					currentSPI3_ADC_Buf=&SPI3_ADC_Buf.buf[0];
			}
	}
	else if(hspi->Instance==SPI6)
	{
			//osMessagePut( ADC_SPI6_QueueHandle, (uint32_t)&spi6_buf[1], osWaitForever );
			SPI6_ADC_Buf.buf[SPI6_ADC_Buf.index]=*(uint16_t *)&spi6_buf[1];
			SPI6_ADC_Buf.index++;
			if(SPI6_ADC_Buf.index==SPI_ADC_BUF_LEN)
			{
					SPI6_ADC_Buf.index=0;
					currentSPI6_ADC_Buf=&SPI6_ADC_Buf.buf[SPI_ADC_BUF_LEN>>1];
			}
			else if(SPI6_ADC_Buf.index==(SPI_ADC_BUF_LEN>>1))
			{
					currentSPI6_ADC_Buf=&SPI6_ADC_Buf.buf[0];
			}
	}
}


