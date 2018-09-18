#include "data_converter.h"
#include "cmsis_os.h"
#include "main.h"
#include "spi_adc.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi6;

static uint8_t spi3_DMA_buf[3]={0};
static uint8_t spi6_DMA_buf[3]={0};

static stSPI_ADC_Buf SPI3_ADC_Buf={{0},0};
static stSPI_ADC_Buf SPI6_ADC_Buf={{0},0};

static uint16_t *currentSPI3_ADC_Buf=&SPI3_ADC_Buf.buf[0];
static uint16_t *currentSPI6_ADC_Buf=&SPI6_ADC_Buf.buf[0];


void SPI_ADC_TimerCallback(void)
{
		HAL_GPIO_WritePin(AIR_CS_GPIO_Port, AIR_CS_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(U_CS_GPIO_Port, U_CS_Pin, GPIO_PIN_RESET);
		HAL_SPI_Receive_DMA(&hspi3, spi3_DMA_buf, 3);
		HAL_SPI_Receive_DMA(&hspi6, spi6_DMA_buf, 3);
}

void SPI_ADC_ResetIndex(void)
{
		SPI3_ADC_Buf.index=0;
		SPI6_ADC_Buf.index=0;
}

void SPI_ADC_GetCurrentBufPtr(SPI_HandleTypeDef *hspi,uint16_t **buf)
{
	if(hspi->Instance==SPI3)
	{
			*buf=currentSPI3_ADC_Buf;
	}
	else if(hspi->Instance==SPI6)
	{
			*buf=currentSPI6_ADC_Buf;
	}
}

uint16_t SPI_ADC_GetCurrentValue(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance==SPI3)
	{
			return SPI3_ADC_Buf.buf[SPI3_ADC_Buf.index];
	}
	else if(hspi->Instance==SPI6)
	{
			return SPI6_ADC_Buf.buf[SPI6_ADC_Buf.index];
	}
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance==SPI3)
	{
			SPI3_ADC_Buf.buf[SPI3_ADC_Buf.index]=(((uint16_t)spi3_DMA_buf[1])<<8)|((uint16_t)spi3_DMA_buf[2]);
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
			
			HAL_GPIO_WritePin(AIR_CS_GPIO_Port, AIR_CS_Pin, GPIO_PIN_SET);
	}
	else if(hspi->Instance==SPI6)
	{
			SPI6_ADC_Buf.buf[SPI6_ADC_Buf.index]=(((uint16_t)spi6_DMA_buf[1])<<8)|((uint16_t)spi6_DMA_buf[2]);
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
			HAL_GPIO_WritePin(U_CS_GPIO_Port, U_CS_Pin, GPIO_PIN_SET);
	}
}


