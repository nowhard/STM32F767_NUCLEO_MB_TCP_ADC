#ifndef MBMASTERINIT_H
#define MBMASTERINIT_H
#include "mb.h"
#include "mb_m.h"

//input regs 
#define REG_PIR_ADC_0						0
#define REG_PIR_ADC_1						2
#define REG_PIR_ADC_2						4
#define REG_PIR_ADC_3						6
#define REG_PIR_ADC_4						8
#define REG_PIR_ADC_5						10
#define REG_PIR_ADC_6						12
#define REG_PIR_ADC_7						14


#define REG_PIR_STATE						16
#define REG_PIR_ERROR						17
#define REG_PIR_IN_LINE					18

//holding regs
#define REG_PIR_SET_TIME				0
#define REG_PIR_1_SET_CURRENT		1
#define REG_PIR_2_SET_CURRENT		3
#define REG_PIR_3_SET_CURRENT		5
#define REG_PIR_4_SET_CURRENT		7
#define REG_PIR_SET_MASK				9
#define REG_PIR_START						10

#define REG_PIR_1_CALIBR_CURRENT_K	11
#define REG_PIR_1_CALIBR_CURRENT_B	13
#define REG_PIR_2_CALIBR_CURRENT_K	15
#define REG_PIR_2_CALIBR_CURRENT_B	17
#define REG_PIR_3_CALIBR_CURRENT_K	19
#define REG_PIR_3_CALIBR_CURRENT_B	21
#define REG_PIR_4_CALIBR_CURRENT_K	23
#define REG_PIR_4_CALIBR_CURRENT_B	25

typedef struct 
{
		uint16_t *regBuf;
		uint16_t regAddr;
		uint16_t nRegs;
} stTCPtoRTURegWrite;

void MBMaster_RTU_Init(void);
eMBMasterReqErrCode MBMaster_RTU_WriteRegs(stTCPtoRTURegWrite *regs);
eMBMasterReqErrCode MBMaster_RTU_GetErrorCode(void);
#endif
