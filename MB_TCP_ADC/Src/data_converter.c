#include "stm32f7xx_hal.h"
#include "data_converter.h"
#include "spi_adc.h"
#include "cfg_info.h"

#pragma anon_unions

// for calibration testing
float a=2.356;
float b=34.397;

//extern sConfigInfo configInfo;	

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
		uint_fast16_t b0:1;
		uint_fast16_t b1:1;
		uint_fast16_t b2:1;
		uint_fast16_t b3:1;
		uint_fast16_t b4:1;
		uint_fast16_t b5:1;
		uint_fast16_t b6:1;
		uint_fast16_t b7:1;
		uint_fast16_t b8:1;
		uint_fast16_t b9:1;
		uint_fast16_t b10:1;
		uint_fast16_t b11:1;
		uint_fast16_t b12:1;
		uint_fast16_t b13:1;
		uint_fast16_t b14:1;
		uint_fast16_t b15:1;
	};
	uint_fast16_t val;
};

union dwordfield{
	struct{
		uint_fast32_t b0:1;
		uint_fast32_t b1:1;
		uint_fast32_t b2:1;
		uint_fast32_t b3:1;
		uint_fast32_t b4:1;
		uint_fast32_t b5:1;
		uint_fast32_t b6:1;
		uint_fast32_t b7:1;
		uint_fast32_t b8:1;
		uint_fast32_t b9:1;
		uint_fast32_t b10:1;
		uint_fast32_t b11:1;
		uint_fast32_t b12:1;
		uint_fast32_t b13:1;
		uint_fast32_t b14:1;
		uint_fast32_t b15:1;
		uint_fast32_t b16:1;
		uint_fast32_t b17:1;
		uint_fast32_t b18:1;
		uint_fast32_t b19:1;
		uint_fast32_t b20:1;
		uint_fast32_t b21:1;
		uint_fast32_t b22:1;
		uint_fast32_t b23:1;
		uint_fast32_t b24:1;
		uint_fast32_t b25:1;
		uint_fast32_t b26:1;
		uint_fast32_t b27:1;
		uint_fast32_t b28:1;
		uint_fast32_t b29:1;
		uint_fast32_t b30:1;
		uint_fast32_t b31:1;
	};
	uint_fast32_t val;
};

//void ADC_ConvertBuf(uint8_t *dcmiBuf,uint16_t dcmiBufLen, uint16_t *resultBuf)
void ADC_ConvertBuf(uint8_t *dcmiBuf,uint16_t dcmiBufLen, uint16_t *spiBuf_1, uint16_t *spiBuf_2, uint16_t spiBufLen,float *resultBuf,uint16_t *resultBufLen)
{

	uint16_t cycle_count;//=(dcmiBufLen/16);

	union bytefield byte;
	union wordfield out1, out2, out3, out4, out5, out6, out7, out8;
	
	for(cycle_count=0;cycle_count<(dcmiBufLen>>4);cycle_count++)
	{
			byte.val=dcmiBuf[0];
			out1.b0=byte.b0;
			out2.b0=byte.b1;
			out3.b0=byte.b2;
			out4.b0=byte.b3;
//			out5.b0=byte.b4;
//			out6.b0=byte.b5;
//			out7.b0=byte.b6;
//			out8.b0=byte.b7;

			byte.val=dcmiBuf[1];
			out1.b1=byte.b0;
			out2.b1=byte.b1;
			out3.b1=byte.b2;
			out4.b1=byte.b3;
//			out5.b1=byte.b4;
//			out6.b1=byte.b5;
//			out7.b1=byte.b6;
//			out8.b1=byte.b7;
			
			byte.val=dcmiBuf[2];
			out1.b2=byte.b0;
			out2.b2=byte.b1;
			out3.b2=byte.b2;
			out4.b2=byte.b3;
//			out5.b2=byte.b4;
//			out6.b2=byte.b5;
//			out7.b2=byte.b6;
//			out8.b2=byte.b7;

			byte.val=dcmiBuf[3];
			out1.b3=byte.b0;
			out2.b3=byte.b1;
			out3.b3=byte.b2;
			out4.b3=byte.b3;
//			out5.b3=byte.b4;
//			out6.b3=byte.b5;
//			out7.b3=byte.b6;
//			out8.b3=byte.b7;

			byte.val=dcmiBuf[4];
			out1.b4=byte.b0;
			out2.b4=byte.b1;
			out3.b4=byte.b2;
			out4.b4=byte.b3;
//			out5.b4=byte.b4;
//			out6.b4=byte.b5;
//			out7.b4=byte.b6;
//			out8.b4=byte.b7;

			byte.val=dcmiBuf[5];
			out1.b5=byte.b0;
			out2.b5=byte.b1;
			out3.b5=byte.b2;
			out4.b5=byte.b3;
//			out5.b5=byte.b4;
//			out6.b5=byte.b5;
//			out7.b5=byte.b6;
//			out8.b5=byte.b7;

			byte.val=dcmiBuf[6];
			out1.b6=byte.b0;
			out2.b6=byte.b1;
			out3.b6=byte.b2;
			out4.b6=byte.b3;
//			out5.b6=byte.b4;
//			out6.b6=byte.b5;
//			out7.b6=byte.b6;
//			out8.b6=byte.b7;

			byte.val=dcmiBuf[7];
			out1.b7=byte.b0;
			out2.b7=byte.b1;
			out3.b7=byte.b2;
			out4.b7=byte.b3;
//			out5.b7=byte.b4;
//			out6.b7=byte.b5;
//			out7.b7=byte.b6;
//			out8.b7=byte.b7;

			byte.val=dcmiBuf[8];
			out1.b8=byte.b0;
			out2.b8=byte.b1;
			out3.b8=byte.b2;
			out4.b8=byte.b3;
//			out5.b8=byte.b4;
//			out6.b8=byte.b5;
//			out7.b8=byte.b6;
//			out8.b8=byte.b7;

			byte.val=dcmiBuf[9];
			out1.b9=byte.b0;
			out2.b9=byte.b1;
			out3.b9=byte.b2;
			out4.b9=byte.b3;
//			out5.b9=byte.b4;
//			out6.b9=byte.b5;
//			out7.b9=byte.b6;
//			out8.b9=byte.b7;

			byte.val=dcmiBuf[10];
			out1.b10=byte.b0;
			out2.b10=byte.b1;
			out3.b10=byte.b2;
			out4.b10=byte.b3;
//			out5.b10=byte.b4;
//			out6.b10=byte.b5;
//			out7.b10=byte.b6;
//			out8.b10=byte.b7;

			byte.val=dcmiBuf[11];
			out1.b11=byte.b0;
			out2.b11=byte.b1;
			out3.b11=byte.b2;
			out4.b11=byte.b3;
//			out5.b11=byte.b4;
//			out6.b11=byte.b5;
//			out7.b11=byte.b6;
//			out8.b11=byte.b7;

			byte.val=dcmiBuf[12];
			out1.b12=byte.b0;
			out2.b12=byte.b1;
			out3.b12=byte.b2;
			out4.b12=byte.b3;
//			out5.b12=byte.b4;
//			out6.b12=byte.b5;
//			out7.b12=byte.b6;
//			out8.b12=byte.b7;

			byte.val=dcmiBuf[13];
			out1.b13=byte.b0;
			out2.b13=byte.b1;
			out3.b13=byte.b2;
			out4.b13=byte.b3;
//			out5.b13=byte.b4;
//			out6.b13=byte.b5;
//			out7.b13=byte.b6;
//			out8.b13=byte.b7;

			byte.val=dcmiBuf[14];
			out1.b14=byte.b0;
			out2.b14=byte.b1;
			out3.b14=byte.b2;
			out4.b14=byte.b3;
//			out5.b14=byte.b4;
//			out6.b14=byte.b5;
//			out7.b14=byte.b6;
//			out8.b14=byte.b7;

			byte.val=dcmiBuf[15];
			out1.b15=byte.b0;
			out2.b15=byte.b1;
			out3.b15=byte.b2;
			out4.b15=byte.b3;
//			out5.b15=byte.b4;
//			out6.b15=byte.b5;
//			out7.b15=byte.b6;
//			out8.b15=byte.b7;

			*(resultBuf+0)=a*out1.val+b;
			*(resultBuf+1)=a*out2.val+b;
			*(resultBuf+2)=a*out3.val+b;
			*(resultBuf+3)=a*out4.val+b;
			*(resultBuf+4)=a*(spiBuf_1[cycle_count/SPI_ADC_FREQ_DIV])+b;
			*(resultBuf+5)=a*(spiBuf_2[cycle_count/SPI_ADC_FREQ_DIV])+b;
			
			configInfo.ConfigADC.calibrChannel[0].k;
			
			uint8_t i=0;
			
			for(i=0;i<ADC_CHN_NUM;i++)
			{
					if(configInfo.ConfigADC.channelMask&0x1)
					{
					}
			}
			
//			*(resultBuf+4)=a*out5.val+b;
//			*(resultBuf+5)=a*out6.val+b;
//			*(resultBuf+6)=a*out7.val+b;
//			*(resultBuf+7)=a*out8.val+b;
			
//			cycle_count--;
			dcmiBuf+=16;
			//spiBuf_1+=(cycle_count/SPI_ADC_FREQ_DIV);
			//spiBuf_2+=(cycle_count/SPI_ADC_FREQ_DIV);
			resultBuf+=ADC_CHN_NUM;			
	}
	
	*resultBufLen=cycle_count*6;
}

