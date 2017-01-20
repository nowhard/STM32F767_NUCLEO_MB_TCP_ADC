#include "data_converter.h"
#include "main.h"

void ADC_ConvertBuf(uint8_t *dcmiBuf,uint16_t dcmiBufLen, uint16_t *resultBuf)
{
	uint8_t bit_count=0, channel_count=0;
	uint16_t cycle_count=(dcmiBufLen/16);

	while(cycle_count!=0)
	{
			for(bit_count=0;bit_count<16;bit_count++)
			{
				for(channel_count=0;channel_count<ADC_CHN_NUM;channel_count++)
				{
					resultBuf[channel_count]|=((dcmiBuf[bit_count]&0x1)<<(15-bit_count));
					dcmiBuf[bit_count]=dcmiBuf[bit_count]>>1;
				}
			}
			cycle_count--;
			dcmiBuf+=16;
			resultBuf+=8;			
	}
}


void ADC_CalibrateBuf(uint16_t *resultBuf, float *calibrateBuf)
{
	
}