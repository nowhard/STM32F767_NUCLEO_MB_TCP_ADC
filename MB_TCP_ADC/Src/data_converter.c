#include "stm32f7xx_hal.h"
#include "data_converter.h"
#include "spi_adc.h"
#include "cfg_info.h"
#include "FreeRTOS.h"
#include "task.h"

#pragma anon_unions

#define CHANNEL_250A_MAX_VAL	250.0
#define CHANNEL_150A_MAX_VAL	150.0
#define CHANNEL_75A_MAX_VAL		75.0
#define CHANNEL_7_5A_MAX_VAL	7.5

union bytefield{
	struct{
	uint_fast8_t b0:1;
	uint_fast8_t b1:1;
	uint_fast8_t b2:1;
	uint_fast8_t b3:1;
	uint_fast8_t b4:1;
	uint_fast8_t b5:1;
	uint_fast8_t b6:1;
	uint_fast8_t b7:1;
	};
	uint_fast8_t val;
};

union wordfield{
	struct{
		uint_fast16_t b15:1;
		uint_fast16_t b14:1;
		uint_fast16_t b13:1;
		uint_fast16_t b12:1;
		uint_fast16_t b11:1;
		uint_fast16_t b10:1;
		uint_fast16_t b9:1;
		uint_fast16_t b8:1;
		uint_fast16_t b7:1;
		uint_fast16_t b6:1;
		uint_fast16_t b5:1;
		uint_fast16_t b4:1;
		uint_fast16_t b3:1;
		uint_fast16_t b2:1;
		uint_fast16_t b1:1;
		uint_fast16_t b0:1;
	};
	uint_fast16_t val;
};

stChnCalibrValues ChnCalibrValues;

float ADC_CurrentChannelsConvolution(void);
float ADC_GetCalibrateValue(uint8_t channel, uint16_t value);

void ADC_ConvertBuf(uint8_t *dcmiBuf,uint16_t dcmiBufLen, uint16_t *spiBuf_1, uint16_t *spiBuf_2, uint16_t spiBufLen,float *resultBuf,uint16_t *resultBufLen)
{

	uint16_t cycle_count;

	union bytefield byte;
	union wordfield out1, out2, out3, out4, out5, out6, out7, out8;
	
	
	for(cycle_count=0;cycle_count<(dcmiBufLen>>4);cycle_count++)
	{
			byte.val=dcmiBuf[0];
			out1.b0=byte.b0;
			out2.b0=byte.b1;
			out3.b0=byte.b2;
			out4.b0=byte.b3;


			byte.val=dcmiBuf[1];
			out1.b1=byte.b0;
			out2.b1=byte.b1;
			out3.b1=byte.b2;
			out4.b1=byte.b3;

			
			byte.val=dcmiBuf[2];
			out1.b2=byte.b0;
			out2.b2=byte.b1;
			out3.b2=byte.b2;
			out4.b2=byte.b3;


			byte.val=dcmiBuf[3];
			out1.b3=byte.b0;
			out2.b3=byte.b1;
			out3.b3=byte.b2;
			out4.b3=byte.b3;


			byte.val=dcmiBuf[4];
			out1.b4=byte.b0;
			out2.b4=byte.b1;
			out3.b4=byte.b2;
			out4.b4=byte.b3;


			byte.val=dcmiBuf[5];
			out1.b5=byte.b0;
			out2.b5=byte.b1;
			out3.b5=byte.b2;
			out4.b5=byte.b3;


			byte.val=dcmiBuf[6];
			out1.b6=byte.b0;
			out2.b6=byte.b1;
			out3.b6=byte.b2;
			out4.b6=byte.b3;


			byte.val=dcmiBuf[7];
			out1.b7=byte.b0;
			out2.b7=byte.b1;
			out3.b7=byte.b2;
			out4.b7=byte.b3;


			byte.val=dcmiBuf[8];
			out1.b8=byte.b0;
			out2.b8=byte.b1;
			out3.b8=byte.b2;
			out4.b8=byte.b3;


			byte.val=dcmiBuf[9];
			out1.b9=byte.b0;
			out2.b9=byte.b1;
			out3.b9=byte.b2;
			out4.b9=byte.b3;


			byte.val=dcmiBuf[10];
			out1.b10=byte.b0;
			out2.b10=byte.b1;
			out3.b10=byte.b2;
			out4.b10=byte.b3;


			byte.val=dcmiBuf[11];
			out1.b11=byte.b0;
			out2.b11=byte.b1;
			out3.b11=byte.b2;
			out4.b11=byte.b3;


			byte.val=dcmiBuf[12];
			out1.b12=byte.b0;
			out2.b12=byte.b1;
			out3.b12=byte.b2;
			out4.b12=byte.b3;


			byte.val=dcmiBuf[13];
			out1.b13=byte.b0;
			out2.b13=byte.b1;
			out3.b13=byte.b2;
			out4.b13=byte.b3;


			byte.val=dcmiBuf[14];
			out1.b14=byte.b0;
			out2.b14=byte.b1;
			out3.b14=byte.b2;
			out4.b14=byte.b3;


			byte.val=dcmiBuf[15];
			out1.b15=byte.b0;
			out2.b15=byte.b1;
			out3.b15=byte.b2;
			out4.b15=byte.b3;


		  uint8_t offset=0;
			
			ChnCalibrValues.val_chn0_raw=out1.val;
			ChnCalibrValues.val_chn1_raw=out2.val;
			ChnCalibrValues.val_chn2_raw=out3.val;
			ChnCalibrValues.val_chn3_raw=out4.val;
			
			ChnCalibrValues.val_7_5A=ADC_GetCalibrateValue(0,(out1.val&0xFFFF));
			ChnCalibrValues.val_75A	=ADC_GetCalibrateValue(1,(out2.val&0xFFFF));
			ChnCalibrValues.val_150A=ADC_GetCalibrateValue(2,(out3.val&0xFFFF));
			ChnCalibrValues.val_250A=ADC_GetCalibrateValue(3,(out4.val&0xFFFF));
		
			ChnCalibrValues.val_current=ADC_CurrentChannelsConvolution();

			
			*(resultBuf+offset)=ChnCalibrValues.val_current;
			offset++;
						
			ChnCalibrValues.val_chn4_raw=spiBuf_1[cycle_count/SPI_ADC_FREQ_DIV]&0xFFFF;
			ChnCalibrValues.val_chn5_raw=spiBuf_2[cycle_count/SPI_ADC_FREQ_DIV]&0xFFFF;
			
			ChnCalibrValues.val_voltage_1=*(resultBuf+offset)= ADC_GetCalibrateValue(4,(spiBuf_1[cycle_count/SPI_ADC_FREQ_DIV]&0xFFFF));
			offset++;
			ChnCalibrValues.val_voltage_2=*(resultBuf+offset)= ADC_GetCalibrateValue(5,(spiBuf_2[cycle_count/SPI_ADC_FREQ_DIV]&0xFFFF));
			offset++;
		
			dcmiBuf+=16;

			resultBuf+=ADC_UDP_CHN_NUM;	
	}
	
	*resultBufLen=cycle_count*ADC_UDP_CHN_NUM;
}

inline float ADC_CurrentChannelsConvolution(void)//свертка токовых каналов
{
	 if(ChnCalibrValues.val_7_5A>=CHANNEL_7_5A_MAX_VAL)
	 {
				if(ChnCalibrValues.val_75A>=CHANNEL_75A_MAX_VAL)
				{
						if(ChnCalibrValues.val_150A>=CHANNEL_150A_MAX_VAL)
						{
								return ChnCalibrValues.val_250A;
						}	
						else
						{
								return ChnCalibrValues.val_75A;
						}
				}	
				else
				{
						return ChnCalibrValues.val_75A;
				}
		}	
		else
		{
				return ChnCalibrValues.val_7_5A;
		}
}

inline float ADC_GetCalibrateValue(uint8_t channel, uint16_t value)
{
	return (configInfo.ConfigADC.calibrChannel[channel].k*value+configInfo.ConfigADC.calibrChannel[channel].b);		 
}
