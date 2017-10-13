#include "utilities.h"
#include "math.h"

typedef 		union 
{
	float 		val;
	uint16_t  buf[2];
}unConvFloatToBufUINT16;

typedef 		union 
{
	uint64_t 		val;
	uint16_t  buf[4];
}unConvUINT64ToBufUINT16;

uint64_t ReverseBytes_UINT64(uint64_t val)
{
		uint64_t out_val;
		uint8_t i=0;
		
		for(i=0;i<sizeof(uint64_t);i++)
		{
			((uint8_t*)&out_val)[i]=((uint8_t*)&val)[(sizeof(uint64_t)-1)-i];	
		}
		
		return out_val;
}

void Float_To_UINT16_Buf(float val, uint16_t *buf)
{
		unConvFloatToBufUINT16 conv;
		
		conv.val=val;
		buf[0]=conv.buf[0];
		buf[1]=conv.buf[1];
}

void UINT16_Buf_To_Float(uint16_t * buf, float *val)
{
		unConvFloatToBufUINT16 conv;
	
		conv.buf[0]=buf[0];
		conv.buf[1]=buf[1];		
		*val=conv.val;
}

void UINT64_To_UINT16_Buf(uint64_t val, uint16_t *buf)
{
		unConvUINT64ToBufUINT16 conv;
		
		conv.val=val;
		buf[0]=conv.buf[0];
		buf[1]=conv.buf[1];
		buf[2]=conv.buf[2];
		buf[3]=conv.buf[3];
}


void UINT16_Buf_To_UINT64(uint16_t * buf, uint64_t *val)
{
		unConvUINT64ToBufUINT16 conv;
		
		conv.buf[0]=buf[0];
		conv.buf[1]=buf[1];	
		conv.buf[2]=buf[2];
		conv.buf[3]=buf[3];			
		*val=conv.val;
}	

uint8_t FloatCheckEquality(float a, float b, float epsilon)
{
		if(fabs(a - b) <= epsilon * fabs(a))
		{
				return TRUE;
		}
		else
		{
				return FALSE;
		}
}