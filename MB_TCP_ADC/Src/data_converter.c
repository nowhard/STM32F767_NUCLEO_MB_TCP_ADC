#include "stm32f7xx_hal.h"
#include "data_converter.h"
#include "jumpers.h"
#include "spi_adc.h"
#include "cfg_info.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "udp_send.h"
#include "adc_dcmi.h"



extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi6;
extern SemaphoreHandle_t xAdcBuf_Send_Semaphore;

static uint16_t rawADCVal[ADC_CHN_NUM];
static float 		calibrADCVal[7];
static uint8_t 	udpTransferEnabled=FALSE;
static float 		ADC_resultBuf[ADC_DCMI_RESULT_BUF_LEN];
static uint8_t 	samplingState=FALSE;

#define ADC_SEM_WAIT_PERIOD				2000





float ADC_CurrentChannelsConvolution(float *calibrVal);
float ADC_GetParallelCurrentSensorsValue(float *calibrVal);
float ADC_GetCalibrateValue(uint8_t channel, uint16_t value);
void ADC_DataConverter_Init(void);

#define ADC_CONVERT_TASK_STACK_SIZE	1024
#define ADC_CONVERT_TASK_PRIO				4
void ADC_Converter_Task( void *pvParameters );

void ADC_DataConverter_Init(void)
{
  xTaskCreate( ADC_Converter_Task, "ADC Converter Task", ADC_CONVERT_TASK_STACK_SIZE, NULL, ADC_CONVERT_TASK_PRIO, NULL );
}


inline float ADC_CurrentChannelsConvolution(float *calibrVal)//свертка токовых каналов
{
	 if(calibrVal[ADC_CHN_CURRENT_1]>=CHANNEL_0_CURR_TRESHOLD)
	 {
				if(calibrVal[ADC_CHN_CURRENT_2]>=CHANNEL_1_CURR_TRESHOLD)
				{
						return calibrVal[ADC_CHN_CURRENT_3];
				}	
				else
				{
						return calibrVal[ADC_CHN_CURRENT_2];
				}
		}	
		else
		{
				return calibrVal[ADC_CHN_CURRENT_1];
		}
}

inline float ADC_GetParallelCurrentSensorsValue(float *calibrVal)
{
		float current_1=0.0;
		float current_2=0.0;
	
	 if(calibrVal[ADC_CHN_CURRENT_1]>=CHANNEL_0_CURR_TRESHOLD)//shunt 1
	 {
			current_1=calibrVal[ADC_CHN_CURRENT_2];
	 }
	 else
	 {
			current_1=calibrVal[ADC_CHN_CURRENT_1];
	 }
	 
	 if(calibrVal[ADC_CHN_CURRENT_3]>=CHANNEL_0_CURR_TRESHOLD)//shunt 2
	 {
			current_2=calibrVal[ADC_CHN_CURRENT_4];
	 }
	 else
	 {
			current_2=calibrVal[ADC_CHN_CURRENT_3];
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
	uint8_t *dcmiBuf;
	
	uint16_t *spiBuf_1;
	uint16_t *spiBuf_2;
	
	const uint16_t dcmiHalfBufLen=(ADC_DCMI_BUF_LEN>>1);
	bytefield byte;
	wordfield out1, out2, out3, out4, out5, out6, out7, out8;
	
	SPI_ADC_GetCurrentBufPtr(&hspi3,&spiBuf_2);
	SPI_ADC_GetCurrentBufPtr(&hspi6,&spiBuf_1);
	DCMI_ADC_GetCurrentBufPtr(&dcmiBuf);
	
	
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
		  		
			rawADCVal[0]=out1.val;
			rawADCVal[1]=out2.val;
			rawADCVal[2]=out3.val;
			rawADCVal[3]=out4.val;
			rawADCVal[4]=spiBuf_1[cycleCount/SPI_ADC_FREQ_DIV]&0xFFFF;
			rawADCVal[5]=spiBuf_2[cycleCount/SPI_ADC_FREQ_DIV]&0xFFFF;
			
			uint8_t sectionType=Jumpers_GetDevSectionType();
			if(sectionType==SECTION_TYPE_1234)
			{
					calibrADCVal[ADC_CHN_CURRENT_1]=ADC_GetCalibrateValue(0,(out1.val&0xFFFF));
					calibrADCVal[ADC_CHN_CURRENT_2]=ADC_GetCalibrateValue(1,(out2.val&0xFFFF));
					calibrADCVal[ADC_CHN_CURRENT_3]=ADC_GetCalibrateValue(2,(out3.val&0xFFFF));
					calibrADCVal[ADC_CHN_CURRENT_4]=0.0;//ADC_GetCalibrateValue(3,(out4.val&0xFFFF));
					calibrADCVal[ADC_CHN_CURRENT_CONV]=ADC_CurrentChannelsConvolution(calibrADCVal);
			}
			else if(sectionType==SECTION_TYPE_56)
			{
					calibrADCVal[ADC_CHN_CURRENT_1]=ADC_GetCalibrateValue(0,(out1.val&0xFFFF))/2;
					calibrADCVal[ADC_CHN_CURRENT_2]=ADC_GetCalibrateValue(1,(out2.val&0xFFFF))/2;
					calibrADCVal[ADC_CHN_CURRENT_3]=ADC_GetCalibrateValue(2,(out3.val&0xFFFF))/2;
					calibrADCVal[ADC_CHN_CURRENT_4]=ADC_GetCalibrateValue(3,(out4.val&0xFFFF))/2;
					calibrADCVal[ADC_CHN_CURRENT_CONV]=ADC_GetParallelCurrentSensorsValue(calibrADCVal);
			}
			calibrADCVal[ADC_CHN_VOLTAGE]	 = ADC_GetCalibrateValue(4,(spiBuf_1[cycleCount/SPI_ADC_FREQ_DIV]&0xFFFF));
			calibrADCVal[ADC_CHN_PRESSURE] = ADC_GetCalibrateValue(5,(spiBuf_2[cycleCount/SPI_ADC_FREQ_DIV]&0xFFFF));

			
			*resultBuf=calibrADCVal[ADC_CHN_CURRENT_CONV];
			resultBuf++;
			*resultBuf=calibrADCVal[ADC_CHN_VOLTAGE];
			resultBuf++;
			*resultBuf=calibrADCVal[ADC_CHN_PRESSURE];
			resultBuf++;	
	}
	
	*resultBufLen=cycleCount*ADC_UDP_CHN_NUM;	
}

uint16_t ADC_GetRawChannelValue(uint8_t channel)
{
		if(channel<ADC_CHN_NUM)
		{
				return rawADCVal[channel];
		}
		else
		{
				return 0;
		}
}


float    ADC_GetCalibratedChannelValue(enADCCalibrChannels channel)
{
		return calibrADCVal[channel];
}

void ADC_GetDCMICalibratedValue(float *val)
{
		uint8_t *dcmiBuf;
		uint16_t dcmiADCVal[4];
		uint8_t sectionType;
		dcmiBuf = DCMI_ADC_GetLastSample();
		DCMI_ADC_ConvertSample(dcmiBuf, dcmiADCVal);
		sectionType = Jumpers_GetDevSectionType();

		if(sectionType==SECTION_TYPE_1234)
		{
				val[ADC_CHN_CURRENT_1] = ADC_GetCalibrateValue(0,dcmiADCVal[ADC_CHN_CURRENT_1]);
				val[ADC_CHN_CURRENT_2] = ADC_GetCalibrateValue(1,dcmiADCVal[ADC_CHN_CURRENT_2]);
				val[ADC_CHN_CURRENT_3] = ADC_GetCalibrateValue(2,dcmiADCVal[ADC_CHN_CURRENT_3]);
				val[ADC_CHN_CURRENT_4] = 0.0;
				val[ADC_CHN_CURRENT_CONV]=ADC_CurrentChannelsConvolution(val);
		}
		else if(sectionType==SECTION_TYPE_56)
		{
				calibrADCVal[ADC_CHN_CURRENT_1] = ADC_GetCalibrateValue(0,dcmiADCVal[ADC_CHN_CURRENT_1])/2;
				calibrADCVal[ADC_CHN_CURRENT_2] = ADC_GetCalibrateValue(1,dcmiADCVal[ADC_CHN_CURRENT_2])/2;
				calibrADCVal[ADC_CHN_CURRENT_3] = ADC_GetCalibrateValue(2,dcmiADCVal[ADC_CHN_CURRENT_3])/2;
				calibrADCVal[ADC_CHN_CURRENT_4] = ADC_GetCalibrateValue(3,dcmiADCVal[ADC_CHN_CURRENT_4])/2;
				calibrADCVal[ADC_CHN_CURRENT_CONV] = ADC_GetParallelCurrentSensorsValue(val);
		}			
}

float    ADC_GetCalibratedChannelInstantValue(enADCCalibrChannels channel)
{
		switch(channel)
		{
			case ADC_CHN_CURRENT_1:
			{
					return calibrADCVal[channel];
			}
			break;
			
			case ADC_CHN_CURRENT_2:
			{
					return calibrADCVal[channel];
			}
			break;

			case ADC_CHN_CURRENT_3:
			{
					return calibrADCVal[channel];
			}
			break;

			case ADC_CHN_CURRENT_4:
			{
					return calibrADCVal[channel];
			}
			break;			
			
			case ADC_CHN_CURRENT_CONV:
			{
					return calibrADCVal[channel];
			}
			break;	
			
			case ADC_CHN_VOLTAGE:
			{
					return ADC_GetCalibrateValue(4,SPI_ADC_GetCurrentValue(&hspi3));
			}
			break;	

			case ADC_CHN_PRESSURE:
			{
					return ADC_GetCalibrateValue(5,SPI_ADC_GetCurrentValue(&hspi6));
			}
			break;				

			default:
			{
					return 0;
			}
			break;			
		}
}


void ADC_SetUDPTransferEnabled(uint8_t state)
{
		if(state)
		{
				udpTransferEnabled=TRUE;
		}
		else
		{
				udpTransferEnabled=FALSE;
		}
}

uint8_t ADC_GetUDPTransferState(void)
{
		return udpTransferEnabled;
}


uint8_t ADC_GetSamplingState(void)
{
		return samplingState;
}


void ADC_Converter_Task( void *pvParameters )
{
	static uint16_t resultBufLen=0;

	while(1)
	{
		if(xSemaphoreTake( xAdcBuf_Send_Semaphore, ADC_SEM_WAIT_PERIOD ))
		{
				samplingState=TRUE;
				ADC_ConvertDCMIAndAssembleUDPBuf(ADC_resultBuf, &resultBufLen);
				if(udpTransferEnabled)
				{
					UDP_SendBaseBuf(ADC_resultBuf,resultBufLen);
					UDP_SendPyroBuf();
				}
		}
		else
		{
				samplingState=FALSE;
		}
	}
}