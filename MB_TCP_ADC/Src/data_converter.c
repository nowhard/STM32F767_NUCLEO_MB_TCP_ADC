#include "stm32f7xx_hal.h"
#include "data_converter.h"
#include "jumpers.h"
#include "spi_adc.h"
#include "cfg_info.h"
#include "FreeRTOS.h"
#include "task.h"


#define CHANNEL_3_CURR_TRESHOLD		250.0
#define CHANNEL_2_CURR_TRESHOLD		150.0
#define CHANNEL_1_CURR_TRESHOLD		75.0
#define CHANNEL_0_CURR_TRESHOLD		7.5

extern uint8_t *ADC_DCMI_buf_pnt;
extern uint16_t *currentSPI3_ADC_Buf;
extern uint16_t *currentSPI6_ADC_Buf;

#pragma anon_unions
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
float ADC_GetParallelCurrentSensorsValue(void);
float ADC_GetCalibrateValue(uint8_t channel, uint16_t value);



inline float ADC_CurrentChannelsConvolution(void)//������� ������� �������
{
	 if(ChnCalibrValues.val_current[0]>=CHANNEL_0_CURR_TRESHOLD)
	 {
				if(ChnCalibrValues.val_current[1]>=CHANNEL_1_CURR_TRESHOLD)
				{
						return ChnCalibrValues.val_current[2];
				}	
				else
				{
						return ChnCalibrValues.val_current[1];
				}
		}	
		else
		{
				return ChnCalibrValues.val_current[0];
		}
}

inline float ADC_GetParallelCurrentSensorsValue(void)
{
		float current_1=0.0;
		float current_2=0.0;
	
	 if(ChnCalibrValues.val_current[0]>=CHANNEL_0_CURR_TRESHOLD)//shunt 1
	 {
			current_1=ChnCalibrValues.val_current[1];
	 }
	 else
	 {
			current_1=ChnCalibrValues.val_current[0];
	 }
	 
	 if(ChnCalibrValues.val_current[2]>=CHANNEL_0_CURR_TRESHOLD)//shunt 2
	 {
			current_2=ChnCalibrValues.val_current[3];
	 }
	 else
	 {
			current_2=ChnCalibrValues.val_current[2];
	 }
	
		return (current_1+current_2);
}


inline float ADC_GetCalibrateValue(uint8_t channel, uint16_t value)
{
	return (configInfo.ConfigADC.calibrChannel[channel].k*value+configInfo.ConfigADC.calibrChannel[channel].b);		 
}


void ADC_ConvertDCMIAndAssembleUDPBuf(float *resultBuf, uint16_t *resultBufLen)
{
	uint16_t cycleCount;
	uint8_t *dcmiBuf=ADC_DCMI_buf_pnt;
	uint16_t *spiBuf_1=currentSPI6_ADC_Buf;
	uint16_t *spiBuf_2=currentSPI3_ADC_Buf;
	const uint16_t dcmiHalfBufLen=(ADC_DCMI_BUF_LEN>>1);
	union bytefield byte;
	union wordfield out1, out2, out3, out4, out5, out6, out7, out8;
	
	
	for(cycleCount=0;cycleCount<(dcmiHalfBufLen/ADC_DCMI_NUM_BITS);cycleCount++)
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

			dcmiBuf+=ADC_DCMI_NUM_BITS;
		  		
			ChnCalibrValues.val_chn_raw[0]=out1.val;
			ChnCalibrValues.val_chn_raw[1]=out2.val;
			ChnCalibrValues.val_chn_raw[2]=out3.val;
			ChnCalibrValues.val_chn_raw[3]=out4.val;
			ChnCalibrValues.val_chn_raw[4]=spiBuf_1[cycleCount/SPI_ADC_FREQ_DIV]&0xFFFF;
			ChnCalibrValues.val_chn_raw[5]=spiBuf_2[cycleCount/SPI_ADC_FREQ_DIV]&0xFFFF;
			
			if(jumpersDevSectionType==SECTION_TYPE_1234)
			{
					ChnCalibrValues.val_current[0]=ADC_GetCalibrateValue(0,(out1.val&0xFFFF));
					ChnCalibrValues.val_current[1]=ADC_GetCalibrateValue(1,(out2.val&0xFFFF));
					ChnCalibrValues.val_current[2]=ADC_GetCalibrateValue(2,(out3.val&0xFFFF));
					ChnCalibrValues.val_current[3]=0.0;//ADC_GetCalibrateValue(3,(out4.val&0xFFFF));
					ChnCalibrValues.val_current_conv=ADC_CurrentChannelsConvolution();
			}
			else if(jumpersDevSectionType==SECTION_TYPE_56)
			{
					ChnCalibrValues.val_current[0]=ADC_GetCalibrateValue(0,(out1.val&0xFFFF));
					ChnCalibrValues.val_current[1]=ADC_GetCalibrateValue(1,(out2.val&0xFFFF));
					ChnCalibrValues.val_current[2]=ADC_GetCalibrateValue(2,(out3.val&0xFFFF));
					ChnCalibrValues.val_current[3]=ADC_GetCalibrateValue(3,(out4.val&0xFFFF));
					ChnCalibrValues.val_current_conv=ADC_GetParallelCurrentSensorsValue();
			}
			ChnCalibrValues.val_voltage	 = ADC_GetCalibrateValue(4,(spiBuf_1[cycleCount/SPI_ADC_FREQ_DIV]&0xFFFF));
			ChnCalibrValues.val_pressure = ADC_GetCalibrateValue(5,(spiBuf_2[cycleCount/SPI_ADC_FREQ_DIV]&0xFFFF));

			
			*resultBuf=ChnCalibrValues.val_current_conv;
			resultBuf++;
			*resultBuf=ChnCalibrValues.val_voltage;
			resultBuf++;
			*resultBuf=ChnCalibrValues.val_pressure;
			resultBuf++;	
	}
	
	*resultBufLen=cycleCount*ADC_UDP_CHN_NUM;	
}