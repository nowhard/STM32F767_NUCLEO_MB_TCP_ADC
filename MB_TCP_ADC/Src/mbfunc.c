#include "mbfunc.h"
#include "mb.h"
#include "mb_m.h"
#include "mbtcp.h"
#include "cfg_info.h"
#include "adc_dcmi.h"
#include "udp_send.h"
#include "discret_out.h"
#include "mb_master_user.h"
#include "data_converter.h"
#include "adc_pyro_buf.h"
#include "system_reset.h"
#include "mbmasterpyro.h"
#include "utilities.h"
#include "spi_adc.h"
#include "string.h"
#include "jumpers.h"

/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START         1001
#define REG_INPUT_NREGS         64
#define REG_HOLDING_START       2001
#define REG_HOLDING_NREGS       100


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
//---------PYRO SQUIB REGS------------
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
#define	PYRO_SQUIB_PIR_IN_LINE			42

#define PYRO_SQUIB_MB_CONNECT_ERROR	43
//--------FAULT SIGNALS---------------
#define FAULT_OUT_1_SIG							44
#define FAULT_OUT_7_SIG							45

#define FAULT_250A_SIG							46
#define FAULT_150A_SIG							47
#define FAULT_75A_SIG								48
#define FAULT_7_5A_SIG							49
//------------------------------------
#define DEV_SET_OUTPUTS_SEQUENCE_IN_PROGRESS	50


/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];


extern SemaphoreHandle_t	xMBSaveSettingsSemaphore;

/*-----------------------Pyro squib vars-------------------------------------*/
extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];
extern xSemaphoreHandle xStartReadPyroADCSem;
extern xSemaphoreHandle xMBRTUMutex;

uint16_t 				usMRegTempBuf[M_REG_HOLDING_NREGS];
/*---------------------------------------------------------------------------*/

extern stChnCalibrValues 			ChnCalibrValues;

stTCPtoRTURegWrite 						TCPtoRTURegWrite;

extern eMBMasterReqErrCode    MB_Master_ErrorCode;

extern stSetSequenceParams		discrOutSequenceParams;
extern uint8_t 								discrOutSequenceProgress;

/* ----------------------- Time of process ---------------------------------*/
typedef struct
{
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
}stTimeProc;

static stTimeProc TimeProc={0,0,0};
/*--------------------------------------------------------------------------*/

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
		uint64_t				timestampTemp=0;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
			
				usRegInputBuf[ADC_CHANNEL_0_RAW]=ChnCalibrValues.val_chn_raw[0];
				usRegInputBuf[ADC_CHANNEL_1_RAW]=ChnCalibrValues.val_chn_raw[1];
				usRegInputBuf[ADC_CHANNEL_2_RAW]=ChnCalibrValues.val_chn_raw[2];
				usRegInputBuf[ADC_CHANNEL_3_RAW]=ChnCalibrValues.val_chn_raw[3];
				usRegInputBuf[ADC_CHANNEL_4_RAW]=ChnCalibrValues.val_chn_raw[4];
				usRegInputBuf[ADC_CHANNEL_5_RAW]=ChnCalibrValues.val_chn_raw[5];
			
			
				Float_To_UINT16_Buf(ChnCalibrValues.val_current[0], &usRegInputBuf[ADC_CHANNEL_0_RESULT]);
				Float_To_UINT16_Buf(ChnCalibrValues.val_current[1], &usRegInputBuf[ADC_CHANNEL_1_RESULT]);			
				Float_To_UINT16_Buf(ChnCalibrValues.val_current[2], &usRegInputBuf[ADC_CHANNEL_2_RESULT]);			
				Float_To_UINT16_Buf(ChnCalibrValues.val_current[3], &usRegInputBuf[ADC_CHANNEL_3_RESULT]);			
				Float_To_UINT16_Buf(ChnCalibrValues.val_voltage, &usRegInputBuf[ADC_CHANNEL_4_RESULT]);			
				Float_To_UINT16_Buf(ChnCalibrValues.val_pressure, &usRegInputBuf[ADC_CHANNEL_5_RESULT]);
				
				Float_To_UINT16_Buf(ChnCalibrValues.val_current_conv, &usRegInputBuf[ADC_CHANNEL_CONV]);
			
				timestampTemp=DCMI_ADC_GetLastTimestamp();
				UINT64_To_UINT16_Buf(timestampTemp,&usRegInputBuf[TIMESTAMP_CURRENT]);
			
			
				memcpy((void *)&usRegInputBuf[ADC_PYRO_SQUIB_0],(const void*)&usMRegInBuf[0][0],M_REG_INPUT_NREGS*sizeof(uint16_t));
				
				usRegInputBuf[FAULT_OUT_1_SIG]=HAL_GPIO_ReadPin(FAULT_OUT_1_GPIO_Port,FAULT_OUT_1_Pin);
				usRegInputBuf[FAULT_OUT_7_SIG]=HAL_GPIO_ReadPin(FAULT_OUT_7_GPIO_Port,FAULT_OUT_7_Pin);
				
				usRegInputBuf[FAULT_250A_SIG]=HAL_GPIO_ReadPin(FAULT_250A_GPIO_Port,FAULT_250A_Pin);
				usRegInputBuf[FAULT_150A_SIG]=HAL_GPIO_ReadPin(FAULT_150A_GPIO_Port,FAULT_150A_Pin);
				usRegInputBuf[FAULT_75A_SIG]=HAL_GPIO_ReadPin(FAULT_75A_GPIO_Port,FAULT_75A_Pin);
				usRegInputBuf[FAULT_7_5A_SIG]=HAL_GPIO_ReadPin(FAULT_7_5A_GPIO_Port,FAULT_7_5A_Pin);
				usRegInputBuf[PYRO_SQUIB_MB_CONNECT_ERROR]=MB_Master_ErrorCode;
				
				usRegInputBuf[DEV_SET_OUTPUTS_SEQUENCE_IN_PROGRESS]=discrOutSequenceProgress;
				

						
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
#define ADC_SAMPLERATE_FREQ_CORRECTION_FACTOR		30
#define ADC_STARTED						32

//--------BITFIELDS------------------------
#define DEV_SET_OUTPUTS_0			33
#define DEV_SET_OUTPUTS_1			34
#define DEV_SET_OUTPUTS_2			35
#define DEV_SET_OUTPUTS_3			36

#define DEV_SET_OUTPUTS_ALL		37

#define DEV_SET_OUTPUTS_SEQUENCE_STATE_1		41
#define DEV_SET_OUTPUTS_SEQUENCE_STATE_2		45
#define DEV_SET_OUTPUTS_SEQUENCE_STATE_END	49
#define DEV_SET_OUTPUTS_SEQUENCE_TIME				53
#define DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES	54
#define DEV_SET_OUTPUTS_SEQUENCE_START			55
//------------------------------------------
#define DEV_ENABLE_OUT_1			56
#define DEV_ENABLE_OUT_7			57

#define DEV_EN_VCC_250				58
#define DEV_EN_VCC_150				59
#define DEV_EN_VCC_75					60
#define DEV_EN_VCC_7_5				61

#define DEV_ENABLE_AIR				62
//--------SYNC DEV REGS--------------------
#define DEV_RESET_TIMESTAMP		63
#define DEV_RESET_CONTROLLER	64
//--------REGS STM32F100DISCOVERY----------

#define PYRO_SQUIB_PIR_SET_TIME					65
#define PYRO_SQUIB_PIR_1_SET_CURRENT		66
#define PYRO_SQUIB_PIR_2_SET_CURRENT		68
#define PYRO_SQUIB_PIR_3_SET_CURRENT		70
#define PYRO_SQUIB_PIR_4_SET_CURRENT		72
#define PYRO_SQUIB_PIR_SET_MASK					74
#define PYRO_SQUIB_PIR_START						75

#define PYRO_SQUIB_PIR_1_CALIBR_CURRENT_K	76
#define PYRO_SQUIB_PIR_1_CALIBR_CURRENT_B	78
#define PYRO_SQUIB_PIR_2_CALIBR_CURRENT_K	80
#define PYRO_SQUIB_PIR_2_CALIBR_CURRENT_B	82
#define PYRO_SQUIB_PIR_3_CALIBR_CURRENT_K	84
#define PYRO_SQUIB_PIR_3_CALIBR_CURRENT_B	86
#define PYRO_SQUIB_PIR_4_CALIBR_CURRENT_K	88
#define PYRO_SQUIB_PIR_4_CALIBR_CURRENT_B	90
//------------------------------------------
#define DEV_PROC_TIME_HOUR							92
#define DEV_PROC_TIME_MINUTE						93
#define DEV_PROC_TIME_SECOND						94

extern uint64_t	discrOutTempReg;
uint16_t baseADCStarted=FALSE;

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


								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[0].k,&usRegHoldingBuf[ADC_CHANNEL_0_K]);
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[0].b,&usRegHoldingBuf[ADC_CHANNEL_0_B]);		
							
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[1].k,&usRegHoldingBuf[ADC_CHANNEL_1_K]);
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[1].b,&usRegHoldingBuf[ADC_CHANNEL_1_B]);
							
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[2].k,&usRegHoldingBuf[ADC_CHANNEL_2_K]);
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[2].b,&usRegHoldingBuf[ADC_CHANNEL_2_B]);
							
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[3].k,&usRegHoldingBuf[ADC_CHANNEL_3_K]);
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[3].b,&usRegHoldingBuf[ADC_CHANNEL_3_B]);
							
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[4].k,&usRegHoldingBuf[ADC_CHANNEL_4_K]);
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[4].b,&usRegHoldingBuf[ADC_CHANNEL_4_B]);
								
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[5].k,&usRegHoldingBuf[ADC_CHANNEL_5_K]);
								Float_To_UINT16_Buf(configInfo.ConfigADC.calibrChannel[5].b,&usRegHoldingBuf[ADC_CHANNEL_5_B]);									
								
								usRegHoldingBuf[ADC_SAMPLERATE]=(uint16_t)configInfo.ConfigADC.sampleRate;
											
								Float_To_UINT16_Buf(configInfo.ConfigADC.freqCorrectionFactor,&usRegHoldingBuf[ADC_SAMPLERATE_FREQ_CORRECTION_FACTOR]);									
								
								usRegHoldingBuf[ADC_STARTED]=baseADCStarted;
								usRegHoldingBuf[DEV_SET_OUTPUTS_0] = (uint16_t)((discrOutTempReg)&0xFFFF);
								usRegHoldingBuf[DEV_SET_OUTPUTS_1] = (uint16_t)((discrOutTempReg>>16)&0xFFFF);
								usRegHoldingBuf[DEV_SET_OUTPUTS_2] = (uint16_t)((discrOutTempReg>>32)&0xFFFF);
								usRegHoldingBuf[DEV_SET_OUTPUTS_3] = (uint16_t)((discrOutTempReg>>48)&0xFFFF);
								
								UINT64_To_UINT16_Buf(discrOutTempReg, &usRegHoldingBuf[DEV_SET_OUTPUTS_ALL]);
								
								usRegHoldingBuf[DEV_ENABLE_OUT_1]=HAL_GPIO_ReadPin(ENABLE_OUT_1_GPIO_Port,ENABLE_OUT_1_Pin);
								usRegHoldingBuf[DEV_ENABLE_OUT_7]=HAL_GPIO_ReadPin(ENABLE_OUT_7_GPIO_Port,ENABLE_OUT_7_Pin);
								
								usRegHoldingBuf[DEV_EN_VCC_250]=HAL_GPIO_ReadPin(EN_VCC_250_GPIO_Port,EN_VCC_250_Pin);
								usRegHoldingBuf[DEV_EN_VCC_150]=HAL_GPIO_ReadPin(EN_VCC_150_GPIO_Port,EN_VCC_150_Pin);
								usRegHoldingBuf[DEV_EN_VCC_75]=HAL_GPIO_ReadPin(EN_VCC_75_GPIO_Port,EN_VCC_75_Pin);
								usRegHoldingBuf[DEV_EN_VCC_7_5]=HAL_GPIO_ReadPin(EN_VCC_7_5_GPIO_Port,EN_VCC_7_5_Pin);
								
								usRegHoldingBuf[DEV_ENABLE_AIR]=HAL_GPIO_ReadPin(ENABLE_AIR_GPIO_Port,ENABLE_AIR_Pin);
								

								memcpy((void *)&usRegHoldingBuf[PYRO_SQUIB_PIR_SET_TIME],(const void*)&usMRegHoldBuf[0][0],M_REG_HOLDING_NREGS*sizeof(uint16_t));
								
								usRegHoldingBuf[DEV_PROC_TIME_HOUR]=TimeProc.hour;
								usRegHoldingBuf[DEV_PROC_TIME_MINUTE]=TimeProc.minute;
								usRegHoldingBuf[DEV_PROC_TIME_SECOND]=TimeProc.second;
								
								UINT64_To_UINT16_Buf(discrOutSequenceParams.state_1, &usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_1]);
								UINT64_To_UINT16_Buf(discrOutSequenceParams.state_2, &usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_2]);
								UINT64_To_UINT16_Buf(discrOutSequenceParams.state_end, &usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_END]);

								usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_TIME]=discrOutSequenceParams.time;
								usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES]=discrOutSequenceParams.num_cycles;
								usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_START]=0;
	
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
									uint8_t settingsNeedWrite=FALSE;
									float tempCoef;
									TCPtoRTURegWrite.regBuf=usMRegTempBuf;
									TCPtoRTURegWrite.nRegs=0;
									TCPtoRTURegWrite.regAddr=0;

								
									while( usNRegs > 0 )
									{
											usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
											usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
											
											switch(iRegIndex)
											{
												case SERVER_IP_REG_0:
												{
														if(configInfo.IPAdress_Server.ip_addr_0!=usRegHoldingBuf[SERVER_IP_REG_0])
														{
															configInfo.IPAdress_Server.ip_addr_0=usRegHoldingBuf[SERVER_IP_REG_0];
															settingsNeedWrite=TRUE;
														}
												}
												break;
												
												case SERVER_IP_REG_1:
												{
														if(configInfo.IPAdress_Server.ip_addr_1!=usRegHoldingBuf[SERVER_IP_REG_1])
														{
															configInfo.IPAdress_Server.ip_addr_1=usRegHoldingBuf[SERVER_IP_REG_1];
															settingsNeedWrite=TRUE;
														}
												}
												break;
												
												case SERVER_IP_REG_2:
												{
														if(configInfo.IPAdress_Server.ip_addr_2!=usRegHoldingBuf[SERVER_IP_REG_2])
														{
															configInfo.IPAdress_Server.ip_addr_2=usRegHoldingBuf[SERVER_IP_REG_2];
															settingsNeedWrite=TRUE;
														}
												}
												break;	

												case SERVER_IP_REG_3:
												{
														if(configInfo.IPAdress_Server.ip_addr_3!=usRegHoldingBuf[SERVER_IP_REG_3])
														{
															configInfo.IPAdress_Server.ip_addr_3=usRegHoldingBuf[SERVER_IP_REG_3];
															settingsNeedWrite=TRUE;
														}
												}
												break;

												case SERVER_PORT_REG_0:
												{
														if(configInfo.IPAdress_Server.port!=usRegHoldingBuf[SERVER_PORT_REG_0])
														{
															configInfo.IPAdress_Server.port=usRegHoldingBuf[SERVER_PORT_REG_0];
															settingsNeedWrite=TRUE;
														}
												}
												break;

												case ADC_CHANNEL_0_K+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_0_K],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[0].k,tempCoef,FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[0].k=tempCoef;
																settingsNeedWrite=TRUE;
														}														
												}
												break;

												case ADC_CHANNEL_0_B+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_0_B],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[0].b, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[0].b=tempCoef;
																settingsNeedWrite=TRUE;
														}	
												}
												break;

												case ADC_CHANNEL_1_K+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_1_K],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[1].k, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[1].k=tempCoef;
																settingsNeedWrite=TRUE;
														}											
												}
												break;

												case ADC_CHANNEL_1_B+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_1_B],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[1].b, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[1].b=tempCoef;
																settingsNeedWrite=TRUE;
														}										
												}
												break;	

												case ADC_CHANNEL_2_K+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_2_K],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[2].k, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[2].k=tempCoef;
																settingsNeedWrite=TRUE;
														}											
												}
												break;

												case ADC_CHANNEL_2_B+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_2_B],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[2].b, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[2].b=tempCoef;
																settingsNeedWrite=TRUE;
														}										
												}
												break;	

												case ADC_CHANNEL_3_K+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_3_K],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[3].k, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[3].k=tempCoef;
																settingsNeedWrite=TRUE;
														}										
												}
												break;

												case ADC_CHANNEL_3_B+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_3_B],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[3].b, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[3].b=tempCoef;
																settingsNeedWrite=TRUE;
														}										
												}
												break;	

												case ADC_CHANNEL_4_K+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_4_K],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[4].k, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[4].k=tempCoef;
																settingsNeedWrite=TRUE;
														}										
												}
												break;

												case ADC_CHANNEL_4_B+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_4_B],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[4].b, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[4].b=tempCoef;
																settingsNeedWrite=TRUE;
														}										
												}
												break;	

												case ADC_CHANNEL_5_K+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_5_K],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[5].k, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[5].k=tempCoef;
																settingsNeedWrite=TRUE;
														}										
												}
												break;

												case ADC_CHANNEL_5_B+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_CHANNEL_5_B],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.calibrChannel[5].b, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.calibrChannel[5].b=tempCoef;
																settingsNeedWrite=TRUE;
														}	
												}
												break;

												case ADC_SAMPLERATE:
												{
														if(configInfo.ConfigADC.sampleRate!=usRegHoldingBuf[ADC_SAMPLERATE])
														{
															DCMI_ADC_SetSamplerate(usRegHoldingBuf[ADC_SAMPLERATE]);
															settingsNeedWrite=TRUE;	
														}															
												}
												break;	
												
												case ADC_SAMPLERATE_FREQ_CORRECTION_FACTOR+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[ADC_SAMPLERATE_FREQ_CORRECTION_FACTOR],&tempCoef);
													
														if(FloatCheckEquality(configInfo.ConfigADC.freqCorrectionFactor, tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.ConfigADC.freqCorrectionFactor=tempCoef;
																settingsNeedWrite=TRUE;
														}															
												}
												break;
												
												case ADC_STARTED:
												{
														if(usRegHoldingBuf[ADC_STARTED])
														{
																SPI_ADC_ResetIndex();
															
																if(jumpersDevIsMaster==TRUE)
																{
																		DCMI_ADC_Clock_Start();
																}
																
																baseADCStarted=TRUE;
														}
														else
														{
																if(jumpersDevIsMaster==TRUE)
																{
																		DCMI_ADC_Clock_Stop();
																}
																
																baseADCStarted=FALSE;
														}
														usRegHoldingBuf[ADC_STARTED]=0;
												}
												break;	
												
													
												case DEV_SET_OUTPUTS_0:
												{		
														if(!discrOutSequenceProgress)
														{
															discrOutTempReg&=(~((uint64_t)0xFFFF));
															discrOutTempReg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_0];
															DiscretOutputs_Set(discrOutTempReg);
														}
												}
												break;

												case DEV_SET_OUTPUTS_1:
												{		
														if(!discrOutSequenceProgress)
														{
															discrOutTempReg&=(~((uint64_t)0xFFFF<<16));
															discrOutTempReg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_1]<<16;
															DiscretOutputs_Set(discrOutTempReg);
														}
												}
												break;	

												case DEV_SET_OUTPUTS_2:
												{		
														if(!discrOutSequenceProgress)
														{
															discrOutTempReg&=(~((uint64_t)0xFFFF<<32));
															discrOutTempReg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_2]<<32;
															DiscretOutputs_Set(discrOutTempReg);
														}
												}
												break;		

												case DEV_SET_OUTPUTS_3:
												{	
														if(!discrOutSequenceProgress)
														{
															discrOutTempReg&=(~((uint64_t)0xFFFF<<48));
															discrOutTempReg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_3]<<48;
															DiscretOutputs_Set(discrOutTempReg);
														}
												}
												break;		
												
												case DEV_SET_OUTPUTS_ALL+3:
												{
														if(!discrOutSequenceProgress)
														{
															UINT16_Buf_To_UINT64(&usRegHoldingBuf[DEV_SET_OUTPUTS_ALL],&discrOutTempReg);
															DiscretOutputs_Set(discrOutTempReg);	
														}
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
																SPI_ADC_ResetIndex();
																usRegHoldingBuf[DEV_RESET_TIMESTAMP]=0;
														}	
												}
												break;	
												
												case DEV_RESET_CONTROLLER:
												{
														if(usRegHoldingBuf[DEV_RESET_CONTROLLER])//reset timestamp
														{
																usRegHoldingBuf[DEV_RESET_CONTROLLER]=0;
																SystemReset_Start();
														}	
												}
												break;	

												case PYRO_SQUIB_PIR_SET_TIME:
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_SET_TIME;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_SET_TIME];															
												}
												break;
												
												case PYRO_SQUIB_PIR_1_SET_CURRENT +(1) :
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_1_SET_CURRENT;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_1_SET_CURRENT];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_1_SET_CURRENT+1];	
												}
												break;
												
												case PYRO_SQUIB_PIR_2_SET_CURRENT +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_2_SET_CURRENT;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_2_SET_CURRENT];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_2_SET_CURRENT+1];	
												}
												break;
												
												case PYRO_SQUIB_PIR_3_SET_CURRENT +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_3_SET_CURRENT;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_3_SET_CURRENT];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_3_SET_CURRENT+1];	
												}
												break;

												case PYRO_SQUIB_PIR_4_SET_CURRENT +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_4_SET_CURRENT;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_4_SET_CURRENT];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_4_SET_CURRENT+1];	
												}
												break;
												
												case PYRO_SQUIB_PIR_SET_MASK:
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_SET_MASK;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_SET_MASK];	
												}
												break;
												
												case PYRO_SQUIB_PIR_START:
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_START;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_START];									
												}
												break;
												
												case PYRO_SQUIB_PIR_1_CALIBR_CURRENT_B +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_1_CALIBR_CURRENT_B;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_1_CALIBR_CURRENT_B];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_1_CALIBR_CURRENT_B+1];	
												}
												break;
												
												case PYRO_SQUIB_PIR_1_CALIBR_CURRENT_K +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_1_CALIBR_CURRENT_K;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_1_CALIBR_CURRENT_K];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_1_CALIBR_CURRENT_K+1];	
												}
												break;
												
												case PYRO_SQUIB_PIR_2_CALIBR_CURRENT_B +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_2_CALIBR_CURRENT_B;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_2_CALIBR_CURRENT_B];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_2_CALIBR_CURRENT_B+1];	
												}
												break;
												
												case PYRO_SQUIB_PIR_2_CALIBR_CURRENT_K +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_2_CALIBR_CURRENT_K;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_2_CALIBR_CURRENT_K];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_2_CALIBR_CURRENT_K+1];	
												}
												break;		

												case PYRO_SQUIB_PIR_3_CALIBR_CURRENT_B +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_3_CALIBR_CURRENT_B;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_3_CALIBR_CURRENT_B];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_3_CALIBR_CURRENT_B+1];	
												}
												break;
												
												case PYRO_SQUIB_PIR_3_CALIBR_CURRENT_K +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_3_CALIBR_CURRENT_K;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_3_CALIBR_CURRENT_K];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_3_CALIBR_CURRENT_K+1];	
												}
												break;	

												case PYRO_SQUIB_PIR_4_CALIBR_CURRENT_B +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_4_CALIBR_CURRENT_B;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_4_CALIBR_CURRENT_B];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_4_CALIBR_CURRENT_B+1];	
												}
												break;
												
												case PYRO_SQUIB_PIR_4_CALIBR_CURRENT_K +(1):
												{
															if(TCPtoRTURegWrite.regAddr==0)
															{
																	TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+REG_PIR_4_CALIBR_CURRENT_K;
															}
															
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_4_CALIBR_CURRENT_K];		
															TCPtoRTURegWrite.regBuf[TCPtoRTURegWrite.nRegs++]=usRegHoldingBuf[PYRO_SQUIB_PIR_4_CALIBR_CURRENT_K+1];	
												}
												break;												
												
												case DEV_PROC_TIME_HOUR:
												{
														TimeProc.hour=usRegHoldingBuf[DEV_PROC_TIME_HOUR];
												}
												break;
												
												case DEV_PROC_TIME_MINUTE:
												{
														TimeProc.minute=usRegHoldingBuf[DEV_PROC_TIME_MINUTE];
												}
												break;	
												
												case DEV_PROC_TIME_SECOND:
												{
														TimeProc.second=usRegHoldingBuf[DEV_PROC_TIME_SECOND];
												}
												break;		

												case DEV_SET_OUTPUTS_SEQUENCE_STATE_1+ (3):
												{
														uint64_t tempValue;
														UINT16_Buf_To_UINT64(&usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_1], &tempValue);
														discrOutSequenceParams.state_1=tempValue;
												}
												break;
												
												case DEV_SET_OUTPUTS_SEQUENCE_STATE_2+ (3):
												{
														uint64_t tempValue;
														UINT16_Buf_To_UINT64(&usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_2], &tempValue);
														discrOutSequenceParams.state_2=tempValue;
												}
												break;			

												case DEV_SET_OUTPUTS_SEQUENCE_STATE_END+ (3):
												{
														uint64_t tempValue;
														UINT16_Buf_To_UINT64(&usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_END], &tempValue);
														discrOutSequenceParams.state_end=tempValue;
												}
												break;													
												
												case DEV_SET_OUTPUTS_SEQUENCE_TIME:
												{
														if(IS_DISCR_OUT_TIME(usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_TIME]))
														{
																discrOutSequenceParams.time=usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_TIME];
														}
												}
												break;	

												case DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES:
												{
														if(IS_DISCR_OUT_NUM_CYCLES(usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES]))
														{
																discrOutSequenceParams.num_cycles=usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES];
														}		
												}
												break;

												case DEV_SET_OUTPUTS_SEQUENCE_START:
												{
														DiscretOutputs_StartSequence();
														usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_START]=0;
												}
												break;													
												
												default:
												{
												}
												
						
									}//switch(iRegIndex)
									iRegIndex++;
									usNRegs--;	
								}//while( usNRegs > 0 )
									
								if(settingsNeedWrite)
								{
										StartConfigInfoWrite();
								}
								
								if(TCPtoRTURegWrite.nRegs>0)//����� � MBMaster � ���� ������
								{
										if( xSemaphoreTake( xMBRTUMutex, portMAX_DELAY ) == pdTRUE )
										{
												MB_Master_ErrorCode=MBMaster_RTU_WriteRegs(&TCPtoRTURegWrite);
												xSemaphoreGive( xMBRTUMutex );
										}
										
										if(MB_Master_ErrorCode==MB_MRE_NO_ERR)
										{
												if((TCPtoRTURegWrite.regAddr==(M_REG_HOLDING_START+REG_PIR_START))&& baseADCStarted)//������ �� ������ ������������ ������ �������
												{
														//�������� ���������� ��������� ������������
														xSemaphoreGive(xStartReadPyroADCSem);
												}
										}
										else
										{
												eStatus=MB_EIO;
										}
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
