#include "adc_pyro_buf.h"
#include "fifo.h"
#include "string.h"

FIFO(ADC_PYRO_FIFO_LEN) ADCPyroFIFO;
static enADCPyroBufState ADCPyroBufState=ADC_PYRO_BUF_FILL_STOP;
static uint64_t ADC_Pyro_Timestamp=0;

uint16_t ADC_PyroBuf_Copy(void *dst_buf, uint16_t max_size)
{
	uint16_t buf_len;//=FIFO_COUNT(ADCPyroFIFO);
	uint16_t buf_cnt=0;	
	stADCPyroBuf ADCPyroBuf_temp;
	
	if(ADCPyroBufState==ADC_PYRO_BUF_FILL_START)//ждем заполнения буфера до останова
	{
		return 0;
	}
	
	if(FIFO_COUNT(ADCPyroFIFO)*sizeof(stADCPyroBuf)>max_size)
	{
			buf_len=max_size;
	}
	else
	{
			buf_len=FIFO_COUNT(ADCPyroFIFO)*sizeof(stADCPyroBuf);
			memset(dst_buf,0,max_size);
	}

	for(buf_cnt=0;buf_cnt<(buf_len/sizeof(stADCPyroBuf));buf_cnt++)
	{
		ADCPyroBuf_temp=FIFO_FRONT(ADCPyroFIFO);
		memcpy(&((uint8_t *)dst_buf)[buf_cnt*sizeof(stADCPyroBuf)], &ADCPyroBuf_temp, sizeof(stADCPyroBuf));
		FIFO_POP(ADCPyroFIFO);
	}
	
	return buf_len;
}

uint16_t ADC_PyroBuf_GetCurrentLength(void)
{
	return FIFO_COUNT(ADCPyroFIFO);
}

void  ADC_PyroBuf_SetState(enADCPyroBufState state)
{
	 ADCPyroBufState=state;
}

enADCPyroBufState ADC_PyroBuf_GetState(void)
{
	return ADCPyroBufState;
}

void ADC_PyroBuf_SetTimestamp(uint64_t timestamp)
{
		ADC_Pyro_Timestamp=timestamp;
}

uint64_t ADC_PyroBuf_GetTimestamp(void)
{
		return ADC_Pyro_Timestamp;
}

void ADC_PyroBuf_Add(float *mb_regs)
{
	stADCPyroBuf temp_buf;	
 	temp_buf=*(stADCPyroBuf*)mb_regs;
	FIFO_PUSH(ADCPyroFIFO,temp_buf);
}