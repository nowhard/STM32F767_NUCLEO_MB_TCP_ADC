#include "mbfunc.h"
#include "mb.h"
#include "mbtcp.h"
#include "cfg_info.h"
#include "adc_dcmi.h"
#include "udp_send.h"
#include "discret_out.h"
#include "mb_master_user.h"
#include "data_converter.h"
#include "adc_pyro_buf.h"
//#include "main.h"

#pragma anon_unions
typedef struct
{
	union
	{
		float val;
		uint16_t buf[2];
	};
}stFloatToUint16Buf;

typedef struct
{
	union
	{
		uint64_t val;
		uint16_t buf[4];
	};
}stUint64ToUint16Buf;

stFloatToUint16Buf FloatToUint16Buf;
stUint64ToUint16Buf Uint64ToUint16Buf;

#define FLOAT_TO_UINT16_BUF(fl,uint16buf) 	FloatToUint16Buf.val=fl;\
																						(uint16buf)[0]=FloatToUint16Buf.buf[0];\
																						(uint16buf)[1]=FloatToUint16Buf.buf[1];

#define UINT16_BUF_TO_FLOAT(uint16buf,fl) 	FloatToUint16Buf.buf[0]=(uint16buf)[0];\
																						FloatToUint16Buf.buf[1]=(uint16buf)[1];\
																						fl=FloatToUint16Buf.val;

#define UINT64_TO_UINT16_BUF(uint64_val,uint16buf) 	Uint64ToUint16Buf.val=uint64_val;\
																						(uint16buf)[0]=Uint64ToUint16Buf.buf[0];\
																						(uint16buf)[1]=Uint64ToUint16Buf.buf[1];\
																						(uint16buf)[2]=Uint64ToUint16Buf.buf[2];\
																						(uint16buf)[3]=Uint64ToUint16Buf.buf[3];

#define UINT16_BUF_TO_UINT64(uint16buf,uint64_val) 	Uint64ToUint16Buf.buf[0]=(uint16buf)[0];\
																						Uint64ToUint16Buf.buf[1]=(uint16buf)[1];\
																						Uint64ToUint16Buf.buf[2]=(uint16buf)[2];\
																						Uint64ToUint16Buf.buf[3]=(uint16buf)[3];\
																						uint64_val=Uint64ToUint16Buf.val;
																		

/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START         1001
#define REG_INPUT_NREGS         64
#define REG_HOLDING_START       2001
#define REG_HOLDING_NREGS       64


//-------ADC RESULT REGS--------
#define ADC_CHANNEL_0_RAW						0
#define ADC_CHANNEL_1_RAW						1
#define ADC_CHANNEL_2_RAW						2
#define ADC_CHANNEL_3_RAW						3
#define ADC_CHANNEL_4_RAW						4
#define ADC_CHANNEL_5_RAW						5

#define ADC_CHANNEL_0_RESULT				6
#define ADC_CHANNEL_1_RESULT				8
#define ADC_CHANNEL_2_RESULT				10
#define ADC_CHANNEL_3_RESULT				12
#define ADC_CHANNEL_4_RESULT				14
#define ADC_CHANNEL_5_RESULT				16

#define	ADC_CHANNEL_CONV						18

#define TIMESTAMP_CURRENT						20

#define ADC_PYRO_SQUIB_0						24
#define ADC_PYRO_SQUIB_1						26
#define ADC_PYRO_SQUIB_2						28
#define ADC_PYRO_SQUIB_3						30
#define ADC_PYRO_SQUIB_4						32
#define ADC_PYRO_SQUIB_5						34
#define ADC_PYRO_SQUIB_6						36
#define ADC_PYRO_SQUIB_7						38

#define PYRO_SQUIB_PIR_STATE				40
#define PYRO_SQUIB_PIR_ERROR				41

//--------FAULT SIGNALS-----------
#define FAULT_OUT_1_SIG							42
#define FAULT_OUT_7_SIG							43

#define FAULT_250A_SIG							44
#define FAULT_150A_SIG							45
#define FAULT_75A_SIG								46
#define FAULT_7_5A_SIG							47

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

extern float ADC_resultBuf[ADC_RESULT_BUF_LEN];
extern SemaphoreHandle_t	xMBSaveSettingsSemaphore;

extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];
extern xSemaphoreHandle xSendRTURegSem;
extern stChnCalibrValues ChnCalibrValues;

stTCPtoRTURegWrite TCPtoRTURegWrite;

extern uint32_t counter_DMA_half;
extern uint32_t counter_DMA_full;
extern uint32_t udp_send_counter;

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
    uint8_t i=0;


    //xQueueReceive( xADC_MB_Queue, &( usRegInputBuf ), ( TickType_t ) 0 ) ;

	

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
			
			usRegInputBuf[ADC_CHANNEL_0_RAW]=ChnCalibrValues.val_chn0_raw;
			usRegInputBuf[ADC_CHANNEL_1_RAW]=ChnCalibrValues.val_chn1_raw;
			usRegInputBuf[ADC_CHANNEL_2_RAW]=ChnCalibrValues.val_chn2_raw;
			usRegInputBuf[ADC_CHANNEL_3_RAW]=ChnCalibrValues.val_chn3_raw;
			usRegInputBuf[ADC_CHANNEL_4_RAW]=ChnCalibrValues.val_chn4_raw;
			usRegInputBuf[ADC_CHANNEL_5_RAW]=ChnCalibrValues.val_chn5_raw;
			
			
//			*((float*)&usRegInputBuf[ADC_CHANNEL_0_RESULT])=ChnCalibrValues.val_250A;
				FLOAT_TO_UINT16_BUF(ChnCalibrValues.val_250A, &usRegInputBuf[ADC_CHANNEL_0_RESULT]);
//			*((float*)&usRegInputBuf[ADC_CHANNEL_1_RESULT])=ChnCalibrValues.val_150A;
				FLOAT_TO_UINT16_BUF(ChnCalibrValues.val_150A, &usRegInputBuf[ADC_CHANNEL_1_RESULT]);			
//			*((float*)&usRegInputBuf[ADC_CHANNEL_2_RESULT])=ChnCalibrValues.val_75A;
				FLOAT_TO_UINT16_BUF(ChnCalibrValues.val_75A, &usRegInputBuf[ADC_CHANNEL_2_RESULT]);			
//			*((float*)&usRegInputBuf[ADC_CHANNEL_3_RESULT])=ChnCalibrValues.val_7_5A;
				FLOAT_TO_UINT16_BUF(ChnCalibrValues.val_7_5A, &usRegInputBuf[ADC_CHANNEL_3_RESULT]);			
//			*((float*)&usRegInputBuf[ADC_CHANNEL_4_RESULT])=ChnCalibrValues.val_voltage_1;
				FLOAT_TO_UINT16_BUF(ChnCalibrValues.val_voltage_1, &usRegInputBuf[ADC_CHANNEL_4_RESULT]);			
//			*((float*)&usRegInputBuf[ADC_CHANNEL_5_RESULT])=ChnCalibrValues.val_voltage_2;
				FLOAT_TO_UINT16_BUF(ChnCalibrValues.val_voltage_2, &usRegInputBuf[ADC_CHANNEL_5_RESULT]);
			
			*((float*)&usRegInputBuf[ADC_CHANNEL_CONV])=ChnCalibrValues.val_current;
			
			uint64_t temp=DCMI_ADC_GetLastTimestamp();
			UINT64_TO_UINT16_BUF(temp,&usRegInputBuf[TIMESTAMP_CURRENT]);
			
			//*((uint64_t*)&usRegInputBuf[TIMESTAMP_CURRENT])=DCMI_ADC_GetLastTimestamp();
			
			memcpy((void *)&usRegInputBuf[ADC_PYRO_SQUIB_0],(const void*)&usMRegInBuf[0][0],M_REG_INPUT_NREGS*sizeof(uint16_t));
			
			usRegInputBuf[FAULT_OUT_1_SIG]=HAL_GPIO_ReadPin(FAULT_OUT_1_GPIO_Port,FAULT_OUT_1_Pin);
			usRegInputBuf[FAULT_OUT_7_SIG]=HAL_GPIO_ReadPin(FAULT_OUT_7_GPIO_Port,FAULT_OUT_7_Pin);
			
			usRegInputBuf[FAULT_250A_SIG]=HAL_GPIO_ReadPin(FAULT_250A_GPIO_Port,FAULT_250A_Pin);
			usRegInputBuf[FAULT_150A_SIG]=HAL_GPIO_ReadPin(FAULT_150A_GPIO_Port,FAULT_150A_Pin);
			usRegInputBuf[FAULT_75A_SIG]=HAL_GPIO_ReadPin(FAULT_75A_GPIO_Port,FAULT_75A_Pin);
			usRegInputBuf[FAULT_7_5A_SIG]=HAL_GPIO_ReadPin(FAULT_7_5A_GPIO_Port,FAULT_7_5A_Pin);
			
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}


//------ADC UDP NET SETTINGS REGS----------
#define SERVER_IP_REG_0		0
#define SERVER_IP_REG_1		1
#define SERVER_IP_REG_2		2
#define SERVER_IP_REG_3		3

#define SERVER_PORT_REG_0	4


//-------ADC UDP DATA SETTINGS REGS--------

#define ADC_CHANNEL_0_K				5
#define ADC_CHANNEL_0_B				7
#define ADC_CHANNEL_1_K				9
#define ADC_CHANNEL_1_B				11
#define ADC_CHANNEL_2_K				13
#define ADC_CHANNEL_2_B				15
#define ADC_CHANNEL_3_K				17
#define ADC_CHANNEL_3_B				19
#define ADC_CHANNEL_4_K				21
#define ADC_CHANNEL_4_B				23
#define ADC_CHANNEL_5_K				25
#define ADC_CHANNEL_5_B				27

#define ADC_SAMPLERATE				29 
#define ADC_STARTED						30

//--------BITFIELDS------------------------
#define DEV_SET_OUTPUTS_0			31
#define DEV_SET_OUTPUTS_1			32
#define DEV_SET_OUTPUTS_2			33
#define DEV_SET_OUTPUTS_3			34

#define DEV_SET_OUTPUTS_ALL		35

#define DEV_ENABLE_OUT_1			39
#define DEV_ENABLE_OUT_7			40

#define DEV_EN_VCC_250				41
#define DEV_EN_VCC_150				42
#define DEV_EN_VCC_75					43
#define DEV_EN_VCC_7_5				44

#define DEV_ENABLE_AIR				45
//--------SYNC DEV REGS--------------------
#define DEV_RESET_TIMESTAMP		46
#define DEV_RESET_CONTROLLER	47
//--------REGS STM32F100DISCOVERY----------

#define PYRO_SQUIB_PIR_SET_TIME					48
#define PYRO_SQUIB_PIR_1_SET_CURRENT		49
#define PYRO_SQUIB_PIR_2_SET_CURRENT		51
#define PYRO_SQUIB_PIR_3_SET_CURRENT		53
#define PYRO_SQUIB_PIR_4_SET_CURRENT		55
#define PYRO_SQUIB_PIR_SET_MASK					57
#define PYRO_SQUIB_PIR_START						58


extern uint64_t	outputs_temp_reg;
extern stADCPyroBuf ADCPyroBuf;

uint16_t BaseADC_Started_Flag=0;

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_HOLDING_START ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
       
				iRegIndex = ( int )( usAddress - usRegHoldingStart );
			
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
					
					//-----------case READ-----------------------------------------------------
							case MB_REG_READ:
							{
								usRegHoldingBuf[SERVER_IP_REG_0]=configInfo.IPAdress_Server.ip_addr_0;
								usRegHoldingBuf[SERVER_IP_REG_1]=configInfo.IPAdress_Server.ip_addr_1;
								usRegHoldingBuf[SERVER_IP_REG_2]=configInfo.IPAdress_Server.ip_addr_2;
								usRegHoldingBuf[SERVER_IP_REG_3]=configInfo.IPAdress_Server.ip_addr_3;

								usRegHoldingBuf[SERVER_PORT_REG_0]=configInfo.IPAdress_Server.port;


								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[0].k,&usRegHoldingBuf[ADC_CHANNEL_0_K]);
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[0].b,&usRegHoldingBuf[ADC_CHANNEL_0_B]);		
							
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[1].k,&usRegHoldingBuf[ADC_CHANNEL_1_K]);
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[1].b,&usRegHoldingBuf[ADC_CHANNEL_1_B]);
							
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[2].k,&usRegHoldingBuf[ADC_CHANNEL_2_K]);
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[2].b,&usRegHoldingBuf[ADC_CHANNEL_2_B]);
							
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[3].k,&usRegHoldingBuf[ADC_CHANNEL_3_K]);
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[3].b,&usRegHoldingBuf[ADC_CHANNEL_3_B]);
							
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[4].k,&usRegHoldingBuf[ADC_CHANNEL_4_K]);
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[4].b,&usRegHoldingBuf[ADC_CHANNEL_4_B]);
								
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[5].k,&usRegHoldingBuf[ADC_CHANNEL_5_K]);
								FLOAT_TO_UINT16_BUF(configInfo.ConfigADC.calibrChannel[5].b,&usRegHoldingBuf[ADC_CHANNEL_5_B]);									
								
								usRegHoldingBuf[ADC_SAMPLERATE]=(uint16_t)configInfo.ConfigADC.sampleRate;
								
								usRegHoldingBuf[ADC_STARTED]=BaseADC_Started_Flag;
								usRegHoldingBuf[DEV_SET_OUTPUTS_0] = (uint16_t)((outputs_temp_reg)&0xFFFF);
								usRegHoldingBuf[DEV_SET_OUTPUTS_1] = (uint16_t)((outputs_temp_reg>>16)&0xFFFF);
								usRegHoldingBuf[DEV_SET_OUTPUTS_2] = (uint16_t)((outputs_temp_reg>>32)&0xFFFF);
								usRegHoldingBuf[DEV_SET_OUTPUTS_3] = (uint16_t)((outputs_temp_reg>>48)&0xFFFF);
								
//								*(uint64_t*)&usRegHoldingBuf[DEV_SET_OUTPUTS_ALL]=outputs_temp_reg;

								UINT64_TO_UINT16_BUF(outputs_temp_reg, &usRegHoldingBuf[DEV_SET_OUTPUTS_ALL]);
								
								usRegHoldingBuf[DEV_ENABLE_OUT_1]=HAL_GPIO_ReadPin(ENABLE_OUT_1_GPIO_Port,ENABLE_OUT_1_Pin);
								usRegHoldingBuf[DEV_ENABLE_OUT_7]=HAL_GPIO_ReadPin(ENABLE_OUT_7_GPIO_Port,ENABLE_OUT_7_Pin);
								
								usRegHoldingBuf[DEV_EN_VCC_250]=HAL_GPIO_ReadPin(EN_VCC_250_GPIO_Port,EN_VCC_250_Pin);
								usRegHoldingBuf[DEV_EN_VCC_150]=HAL_GPIO_ReadPin(EN_VCC_150_GPIO_Port,EN_VCC_150_Pin);
								usRegHoldingBuf[DEV_EN_VCC_75]=HAL_GPIO_ReadPin(EN_VCC_75_GPIO_Port,EN_VCC_75_Pin);
								usRegHoldingBuf[DEV_EN_VCC_7_5]=HAL_GPIO_ReadPin(EN_VCC_7_5_GPIO_Port,EN_VCC_7_5_Pin);
								
								usRegHoldingBuf[DEV_ENABLE_AIR]=HAL_GPIO_ReadPin(ENABLE_AIR_GPIO_Port,ENABLE_AIR_Pin);
								

								memcpy((void *)&usRegHoldingBuf[PYRO_SQUIB_PIR_SET_TIME],(const void*)&usMRegHoldBuf[0][0],M_REG_HOLDING_NREGS*sizeof(uint16_t));

								while( usNRegs > 0 )
								{
										*pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] >> 8 );
										*pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] & 0xFF );
										iRegIndex++;
										usNRegs--;
								}            
							}
							break;
							//-----------case READ end-----------------------------------------------------
            /* Update current register values with new values from the
             * protocol stack. */
							
							//-----------case WRITE-----------------------------------------------------
							case MB_REG_WRITE:
							{
									uint8_t settings_need_write=0;
									while( usNRegs > 0 )
									{
											usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
											usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
											
											switch(iRegIndex)
											{
												case SERVER_IP_REG_0:
												{
														configInfo.IPAdress_Server.ip_addr_0=usRegHoldingBuf[SERVER_IP_REG_0];
														settings_need_write=1;
												}
												break;
												
												case SERVER_IP_REG_1:
												{
														configInfo.IPAdress_Server.ip_addr_1=usRegHoldingBuf[SERVER_IP_REG_1];
														settings_need_write=1;
												}
												break;
												
												case SERVER_IP_REG_2:
												{
														configInfo.IPAdress_Server.ip_addr_2=usRegHoldingBuf[SERVER_IP_REG_2];
														settings_need_write=1;
												}
												break;	

												case SERVER_IP_REG_3:
												{
														configInfo.IPAdress_Server.ip_addr_3=usRegHoldingBuf[SERVER_IP_REG_3];
														settings_need_write=1;
												}
												break;

												case SERVER_PORT_REG_0:
												{
														configInfo.IPAdress_Server.port=usRegHoldingBuf[SERVER_PORT_REG_0];
														settings_need_write=1;
												}
												break;

												case ADC_CHANNEL_0_K+(1):
												{
														//configInfo.ConfigADC.calibrChannel[0].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_0_K]);
													
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_0_K], configInfo.ConfigADC.calibrChannel[0].k);							
														settings_need_write=1;
												}
												break;

												case ADC_CHANNEL_0_B+(1):
												{
														//configInfo.ConfigADC.calibrChannel[0].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_0_B]);	
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_0_B], configInfo.ConfigADC.calibrChannel[0].b);														
														settings_need_write=1;
												}
												break;

												case ADC_CHANNEL_1_K+(1):
												{
														//configInfo.ConfigADC.calibrChannel[1].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_1_K]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_1_K], configInfo.ConfigADC.calibrChannel[1].k);
														settings_need_write=1;										
												}
												break;

												case ADC_CHANNEL_1_B+(1):
												{
														//configInfo.ConfigADC.calibrChannel[1].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_1_B]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_1_B], configInfo.ConfigADC.calibrChannel[1].b);													
														settings_need_write=1;										
												}
												break;	

												case ADC_CHANNEL_2_K+(1):
												{
														//configInfo.ConfigADC.calibrChannel[2].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_2_K]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_2_K], configInfo.ConfigADC.calibrChannel[2].k);													
														settings_need_write=1;										
												}
												break;

												case ADC_CHANNEL_2_B+(1):
												{
														//configInfo.ConfigADC.calibrChannel[2].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_2_B]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_2_B], configInfo.ConfigADC.calibrChannel[2].b);													
														settings_need_write=1;										
												}
												break;	

												case ADC_CHANNEL_3_K+(1):
												{
														//configInfo.ConfigADC.calibrChannel[3].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_3_K]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_3_K], configInfo.ConfigADC.calibrChannel[3].k);
														settings_need_write=1;										
												}
												break;

												case ADC_CHANNEL_3_B+(1):
												{
														//configInfo.ConfigADC.calibrChannel[3].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_3_B]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_3_B], configInfo.ConfigADC.calibrChannel[3].b);													
														settings_need_write=1;										
												}
												break;	

												case ADC_CHANNEL_4_K+(1):
												{
														//configInfo.ConfigADC.calibrChannel[4].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_4_K]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_4_K], configInfo.ConfigADC.calibrChannel[4].k);													
														settings_need_write=1;										
												}
												break;

												case ADC_CHANNEL_4_B+(1):
												{
														//configInfo.ConfigADC.calibrChannel[4].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_4_B]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_4_B], configInfo.ConfigADC.calibrChannel[4].b);													
														settings_need_write=1;										
												}
												break;	

												case ADC_CHANNEL_5_K+(1):
												{
														//configInfo.ConfigADC.calibrChannel[5].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_5_K]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_5_K], configInfo.ConfigADC.calibrChannel[5].k);													
														settings_need_write=1;										
												}
												break;

												case ADC_CHANNEL_5_B+(1):
												{
														//configInfo.ConfigADC.calibrChannel[5].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_5_B]);
														UINT16_BUF_TO_FLOAT(&usRegHoldingBuf[ADC_CHANNEL_5_B], configInfo.ConfigADC.calibrChannel[5].b);													
														settings_need_write=1;
												}
												break;

												case ADC_SAMPLERATE:
												{
														DCMI_ADC_SetSamplerate(usRegHoldingBuf[ADC_SAMPLERATE]);
														settings_need_write=1;										
												}
												break;	
												
												case ADC_STARTED:
												{
														if(usRegHoldingBuf[ADC_STARTED])
														{
																DCMI_ADC_Clock_Start();
																BaseADC_Started_Flag=1;
														}
														else
														{
																DCMI_ADC_Clock_Stop();
																BaseADC_Started_Flag=0;
														}
														usRegHoldingBuf[ADC_STARTED]=0;
												}
												break;	
												
													
												case DEV_SET_OUTPUTS_0:
												{											
														outputs_temp_reg&=(~((uint64_t)0xFFFF));
														outputs_temp_reg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_0];
														DiscretOutputs_Set(outputs_temp_reg);
												}
												break;

												case DEV_SET_OUTPUTS_1:
												{											
														outputs_temp_reg&=(~((uint64_t)0xFFFF<<16));
														outputs_temp_reg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_1]<<16;
														DiscretOutputs_Set(outputs_temp_reg);
												}
												break;	

												case DEV_SET_OUTPUTS_2:
												{											
														outputs_temp_reg&=(~((uint64_t)0xFFFF<<32));
														outputs_temp_reg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_2]<<32;
														DiscretOutputs_Set(outputs_temp_reg);
												}
												break;		

												case DEV_SET_OUTPUTS_3:
												{											
														outputs_temp_reg&=(~((uint64_t)0xFFFF<<48));
														outputs_temp_reg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_3]<<48;
														DiscretOutputs_Set(outputs_temp_reg);
												}
												break;		
												
												case DEV_SET_OUTPUTS_ALL+3:
												{
														UINT16_BUF_TO_UINT64(&usRegHoldingBuf[DEV_SET_OUTPUTS_ALL],outputs_temp_reg);
														DiscretOutputs_Set(outputs_temp_reg);	
												}
												break;													

												case DEV_ENABLE_OUT_1:
												{
														HAL_GPIO_WritePin(ENABLE_OUT_1_GPIO_Port,ENABLE_OUT_1_Pin,usRegHoldingBuf[DEV_ENABLE_OUT_1]);
												}
												break;
												
												case DEV_ENABLE_OUT_7:
												{
														HAL_GPIO_WritePin(ENABLE_OUT_7_GPIO_Port,ENABLE_OUT_7_Pin,usRegHoldingBuf[DEV_ENABLE_OUT_7]);
												}
												break;				

												case DEV_EN_VCC_250:
												{
														HAL_GPIO_WritePin(EN_VCC_250_GPIO_Port,EN_VCC_250_Pin,usRegHoldingBuf[DEV_EN_VCC_250]);
												}
												break;
												
												case DEV_EN_VCC_150:
												{
														HAL_GPIO_WritePin(EN_VCC_150_GPIO_Port,EN_VCC_150_Pin,usRegHoldingBuf[DEV_EN_VCC_150]);
												}
												break;		

												case DEV_EN_VCC_75:
												{
														HAL_GPIO_WritePin(EN_VCC_75_GPIO_Port,EN_VCC_75_Pin,usRegHoldingBuf[DEV_EN_VCC_75]);
												}
												break;	

												case DEV_EN_VCC_7_5:
												{
														HAL_GPIO_WritePin(EN_VCC_7_5_GPIO_Port,EN_VCC_7_5_Pin,usRegHoldingBuf[DEV_EN_VCC_7_5]);
												}
												break;	

												case DEV_ENABLE_AIR:
												{
														HAL_GPIO_WritePin(ENABLE_AIR_GPIO_Port,ENABLE_AIR_Pin,usRegHoldingBuf[DEV_ENABLE_AIR]);
												}
												break;										

												case DEV_RESET_TIMESTAMP:
												{
														if(usRegHoldingBuf[DEV_RESET_TIMESTAMP])//reset timestamp
														{
																DCMI_ADC_ResetTimestamp();
																usRegHoldingBuf[DEV_RESET_TIMESTAMP]=0;
														}	
												}
												break;	
												
												case DEV_RESET_CONTROLLER:
												{
														if(usRegHoldingBuf[DEV_RESET_CONTROLLER])//reset timestamp
														{
																//DCMI_ADC_ResetTimestamp();
																usRegHoldingBuf[DEV_RESET_CONTROLLER]=0;
																NVIC_SystemReset();
														}	
												}
												break;	

												case PYRO_SQUIB_PIR_SET_TIME:
												{
														TCPtoRTURegWrite.nRegs=1;
														TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+0;
														TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PYRO_SQUIB_PIR_SET_TIME];
														xSemaphoreGive(xSendRTURegSem);
												}
												break;
												
												case PYRO_SQUIB_PIR_1_SET_CURRENT +(1) :
												{
														TCPtoRTURegWrite.nRegs=2;
														TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+1;
														TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PYRO_SQUIB_PIR_1_SET_CURRENT];
														xSemaphoreGive(xSendRTURegSem);	
												}
												break;
												
												case PYRO_SQUIB_PIR_2_SET_CURRENT +(1):
												{
														TCPtoRTURegWrite.nRegs=2;
														TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+3;
														TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PYRO_SQUIB_PIR_2_SET_CURRENT];
														xSemaphoreGive(xSendRTURegSem);	
												}
												break;
												
												case PYRO_SQUIB_PIR_3_SET_CURRENT +(1):
												{
														TCPtoRTURegWrite.nRegs=2;
														TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+5;
														TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PYRO_SQUIB_PIR_3_SET_CURRENT];
														xSemaphoreGive(xSendRTURegSem);	
												}
												break;

												case PYRO_SQUIB_PIR_4_SET_CURRENT +(1):
												{
														TCPtoRTURegWrite.nRegs=2;
														TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+7;
														TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PYRO_SQUIB_PIR_4_SET_CURRENT];
														xSemaphoreGive(xSendRTURegSem);	
												}
												break;
												
												case PYRO_SQUIB_PIR_SET_MASK:
												{
														TCPtoRTURegWrite.nRegs=1;
														TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+9;
														TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PYRO_SQUIB_PIR_SET_MASK];
														xSemaphoreGive(xSendRTURegSem);											
												}
												break;
												
												case PYRO_SQUIB_PIR_START:
												{
														TCPtoRTURegWrite.nRegs=1;
														TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+10;
														TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PYRO_SQUIB_PIR_START];
														xSemaphoreGive(xSendRTURegSem);		

												}
												break;
												
												default:
												{
												}
												
						
									}//switch(iRegIndex)
									iRegIndex++;
									usNRegs--;	
								}//while( usNRegs > 0 )
									
								if(settings_need_write)
								{
										StartConfigInfoWrite();
								}
							}
							//-----------case WRITE end-----------------------------------------------------							
						

							
						}//switch ( eMode )

		}// if( ( usAddress >= REG_HOLDING_START ) &&.....
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}
