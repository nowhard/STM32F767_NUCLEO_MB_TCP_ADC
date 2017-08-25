#include "adc_pyro_buf.h"
#include "fifo.h"
#include "string.h"

FIFO(ADC_PYRO_FIFO_LEN) ADCPyroFIFO;
enADCPyroBufState ADCPyroBufState=ADC_PYRO_BUF_FILL_STOP;

uint16_t ADC_PyroBuf_Copy(void *dst_buf, uint16_t max_size)
{
	uint16_t buf_len;//=FIFO_COUNT(ADCPyroFIFO);
	uint16_t buf_cnt=0;	
	
	if(ADCPyroBufState==ADC_PYRO_BUF_FILL_START)//ждем заполнения буфера до останова
	{
		return 0;
	}
	
	if(FIFO_COUNT(ADCPyroFIFO)>max_size)
	{
			buf_len=max_size;
	}
	else
	{
			buf_len=FIFO_COUNT(ADCPyroFIFO)*sizeof(stADCPyroBuf);
	}

	memcpy(dst_buf, (const void*)&FIFO_FRONT(ADCPyroFIFO), buf_len);
	//POP???

	return buf_len;
}

uint16_t ADC_PyroBuf_GetCurrentLength(void)
{
	return FIFO_COUNT(ADCPyroFIFO);
}

void ADC_PyroBuf_Add(float *mb_regs)
{
	stADCPyroBuf temp_buf;	
	memcpy(&temp_buf.buf, mb_regs, ADC_PYRO_CHN_NUM);
	FIFO_PUSH(ADCPyroFIFO,temp_buf);
}