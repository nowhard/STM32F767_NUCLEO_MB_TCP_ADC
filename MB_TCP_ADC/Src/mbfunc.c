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
#include "mb_tcp_regs.h"




/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];



/*-----------------------Pyro squib vars-------------------------------------*/
extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];

extern xSemaphoreHandle xStartReadPyroADCSem;
extern xSemaphoreHandle xMBRTUMutex;

uint16_t 				usMRegTempBuf[M_REG_HOLDING_NREGS];
/*---------------------------------------------------------------------------*/


static stTCPtoRTURegWrite 						TCPtoRTURegWrite;

/* ----------------------- Time of process ---------------------------------*/

static stTimeProc TimeProc={0,0,0};

/*--------------------------------------------------------------------------*/

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
		uint64_t				timestampTemp=0;
		float						tempADCVal=0;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
			
				usRegInputBuf[ADC_CHANNEL_0_RAW]=ADC_GetRawChannelValue(0);
				usRegInputBuf[ADC_CHANNEL_1_RAW]=ADC_GetRawChannelValue(1);
				usRegInputBuf[ADC_CHANNEL_2_RAW]=ADC_GetRawChannelValue(2);
				usRegInputBuf[ADC_CHANNEL_3_RAW]=ADC_GetRawChannelValue(3);
				usRegInputBuf[ADC_CHANNEL_4_RAW]=ADC_GetRawChannelValue(4);
				usRegInputBuf[ADC_CHANNEL_5_RAW]=ADC_GetRawChannelValue(5);
			
			
				tempADCVal=ADC_GetCalibratedChannelValue(ADC_CHN_CURRENT_1);
				Float_To_UINT16_Buf(tempADCVal, &usRegInputBuf[ADC_CHANNEL_0_RESULT]);
			
				tempADCVal=ADC_GetCalibratedChannelValue(ADC_CHN_CURRENT_2);
				Float_To_UINT16_Buf(tempADCVal, &usRegInputBuf[ADC_CHANNEL_1_RESULT]);	
			
				tempADCVal=ADC_GetCalibratedChannelValue(ADC_CHN_CURRENT_3);			
				Float_To_UINT16_Buf(tempADCVal, &usRegInputBuf[ADC_CHANNEL_2_RESULT]);
			
				tempADCVal=ADC_GetCalibratedChannelValue(ADC_CHN_CURRENT_4);			
				Float_To_UINT16_Buf(tempADCVal, &usRegInputBuf[ADC_CHANNEL_3_RESULT]);
				
				tempADCVal=ADC_GetCalibratedChannelValue(ADC_CHN_VOLTAGE);
				Float_To_UINT16_Buf(tempADCVal, &usRegInputBuf[ADC_CHANNEL_4_RESULT]);
				
				tempADCVal=ADC_GetCalibratedChannelValue(ADC_CHN_PRESSURE);
				Float_To_UINT16_Buf(tempADCVal, &usRegInputBuf[ADC_CHANNEL_5_RESULT]);
				
				tempADCVal=ADC_GetCalibratedChannelValue(ADC_CHN_CURRENT_CONV);
				Float_To_UINT16_Buf(tempADCVal, &usRegInputBuf[ADC_CHANNEL_CONV]);
			
				timestampTemp=DCMI_ADC_GetLastTimestamp();
				UINT64_To_UINT16_Buf(timestampTemp,&usRegInputBuf[ADC_TIMESTAMP_CURRENT]);
				
				usRegInputBuf[ADC_SAMPLING_FREQ_STATE]=ADC_GetSamplingState();
			
				/*возможно перенести в функцию*/

				/************копирование регистров ведомого*************************/
				memcpy(&usRegInputBuf[ADC_PYRO_SQUIB_0],&usMRegInBuf[0][REG_PIR_ADC_0],sizeof(float));
				memcpy(&usRegInputBuf[ADC_PYRO_SQUIB_1],&usMRegInBuf[0][REG_PIR_ADC_1],sizeof(float));
				memcpy(&usRegInputBuf[ADC_PYRO_SQUIB_2],&usMRegInBuf[0][REG_PIR_ADC_2],sizeof(float));
				memcpy(&usRegInputBuf[ADC_PYRO_SQUIB_3],&usMRegInBuf[0][REG_PIR_ADC_3],sizeof(float));
				memcpy(&usRegInputBuf[ADC_PYRO_SQUIB_4],&usMRegInBuf[0][REG_PIR_ADC_4],sizeof(float));
				memcpy(&usRegInputBuf[ADC_PYRO_SQUIB_5],&usMRegInBuf[0][REG_PIR_ADC_5],sizeof(float));
				memcpy(&usRegInputBuf[ADC_PYRO_SQUIB_6],&usMRegInBuf[0][REG_PIR_ADC_6],sizeof(float));
				memcpy(&usRegInputBuf[ADC_PYRO_SQUIB_7],&usMRegInBuf[0][REG_PIR_ADC_7],sizeof(float));
				
				usRegInputBuf[PYRO_SQUIB_PIR_STATE]=usMRegInBuf[0][REG_PIR_STATE];
				usRegInputBuf[PYRO_SQUIB_PIR_ERROR]=usMRegInBuf[0][REG_PIR_ERROR];
				usRegInputBuf[PYRO_SQUIB_PIR_IN_LINE]=usMRegInBuf[0][REG_PIR_IN_LINE];
				/*******************************************************************/
				
				
				usRegInputBuf[FAULT_OUT_1_SIG]=HAL_GPIO_ReadPin(FAULT_OUT_1_GPIO_Port,FAULT_OUT_1_Pin);
				usRegInputBuf[FAULT_OUT_7_SIG]=HAL_GPIO_ReadPin(FAULT_OUT_7_GPIO_Port,FAULT_OUT_7_Pin);
				
				usRegInputBuf[FAULT_250A_SIG]=HAL_GPIO_ReadPin(FAULT_250A_GPIO_Port,FAULT_250A_Pin);
				usRegInputBuf[FAULT_150A_SIG]=HAL_GPIO_ReadPin(FAULT_150A_GPIO_Port,FAULT_150A_Pin);
				usRegInputBuf[FAULT_75A_SIG]=HAL_GPIO_ReadPin(FAULT_75A_GPIO_Port,FAULT_75A_Pin);
				usRegInputBuf[FAULT_7_5A_SIG]=HAL_GPIO_ReadPin(FAULT_7_5A_GPIO_Port,FAULT_7_5A_Pin);
				usRegInputBuf[PYRO_SQUIB_MB_CONNECT_ERROR]=MBMaster_RTU_GetErrorCode();
				
				usRegInputBuf[DEV_SET_OUTPUTS_SEQUENCE_IN_PROGRESS]=DiscretOutputs_SequenceInProgress();
				

						
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


eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
		static  uint64_t	discrOutTempReg;
	
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

								usRegHoldingBuf[SERVER_PORT_REG]=configInfo.IPAdress_Server.port;


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
								
											
								Float_To_UINT16_Buf(configInfo.ConfigADC.freqCorrectionFactor,&usRegHoldingBuf[ADC_SAMPLERATE_FREQ_CORRECTION_FACTOR]);				
					
								
								usRegHoldingBuf[ADC_STARTED]=DCMI_ADC_Started();
								
								usRegHoldingBuf[ADC_UDP_PACKET_TRANSFER_ENABLE]=ADC_GetUDPTransferState();
								
								discrOutTempReg=DiscretOutputs_Get();
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
								
								stSequenceParams sequenceParams;
								DiscretOutputs_GetSequenceParams(&sequenceParams);
								
								UINT64_To_UINT16_Buf(sequenceParams.state_1, &usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_1]);
								UINT64_To_UINT16_Buf(sequenceParams.state_2, &usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_2]);
								UINT64_To_UINT16_Buf(sequenceParams.state_end, &usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_END]);
								usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_TIME]=sequenceParams.time;
								usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES]=sequenceParams.num_cycles;
								usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_START]=0;
								
								
								//читаем сохранненые значения резисторов
								uint8_t loadResCnt=0;
								
								for(loadResCnt=0;loadResCnt<LOAD_RESISTORS_NUM;loadResCnt++)
								{
										Float_To_UINT16_Buf(configInfo.resistors[loadResCnt],&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_1+loadResCnt*(sizeof(float)/sizeof(uint16_t))]);
								}
								

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

												case SERVER_PORT_REG:
												{
														if(configInfo.IPAdress_Server.port!=usRegHoldingBuf[SERVER_PORT_REG])
														{
															configInfo.IPAdress_Server.port=usRegHoldingBuf[SERVER_PORT_REG];
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
															
																if(Jumpers_DevIsMaster()==TRUE)
																{
																		DCMI_ADC_Clock_Start();
																}															
														}
														else
														{
																if(Jumpers_DevIsMaster()==TRUE)
																{
																		DCMI_ADC_Clock_Stop();
																}
														}
												}
												break;	
												
												case ADC_UDP_PACKET_TRANSFER_ENABLE:
												{
														if(usRegHoldingBuf[ADC_UDP_PACKET_TRANSFER_ENABLE])
														{
																ADC_SetUDPTransferEnabled(TRUE);
														}
														else
														{
																ADC_SetUDPTransferEnabled(FALSE);
														}
												}												
												break;
													
												case DEV_SET_OUTPUTS_0:
												{		
														if(!DiscretOutputs_SequenceInProgress())
														{
															discrOutTempReg=DiscretOutputs_Get();
															discrOutTempReg&=(~((uint64_t)0xFFFF));
															discrOutTempReg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_0];
															DiscretOutputs_Set(discrOutTempReg);
														}
												}
												break;

												case DEV_SET_OUTPUTS_1:
												{		
														if(!DiscretOutputs_SequenceInProgress())
														{
															discrOutTempReg=DiscretOutputs_Get();
															discrOutTempReg&=(~((uint64_t)0xFFFF<<16));
															discrOutTempReg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_1]<<16;
															DiscretOutputs_Set(discrOutTempReg);
														}
												}
												break;	

												case DEV_SET_OUTPUTS_2:
												{		
														if(!DiscretOutputs_SequenceInProgress())
														{
															discrOutTempReg=DiscretOutputs_Get();
															discrOutTempReg&=(~((uint64_t)0xFFFF<<32));
															discrOutTempReg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_2]<<32;
															DiscretOutputs_Set(discrOutTempReg);
														}
												}
												break;		

												case DEV_SET_OUTPUTS_3:
												{	
														if(!DiscretOutputs_SequenceInProgress())
														{
															discrOutTempReg=DiscretOutputs_Get();
															discrOutTempReg&=(~((uint64_t)0xFFFF<<48));
															discrOutTempReg|=(uint64_t)usRegHoldingBuf[DEV_SET_OUTPUTS_3]<<48;
															DiscretOutputs_Set(discrOutTempReg);
														}
												}
												break;		
												
												case DEV_SET_OUTPUTS_ALL+3:
												{
														if(!DiscretOutputs_SequenceInProgress())
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

												case ADC_TIMESTAMP_RESET:
												{
														if(usRegHoldingBuf[ADC_TIMESTAMP_RESET])//reset timestamp
														{
																DCMI_ADC_ResetTimestamp();
																SPI_ADC_ResetIndex();
																usRegHoldingBuf[ADC_TIMESTAMP_RESET]=0;
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
														if(IS_TIME_HOURS(usRegHoldingBuf[DEV_PROC_TIME_HOUR]))
														{
																TimeProc.hour=usRegHoldingBuf[DEV_PROC_TIME_HOUR];
														}
												}
												break;
												
												case DEV_PROC_TIME_MINUTE:
												{
														if(IS_TIME_MINUTS(usRegHoldingBuf[DEV_PROC_TIME_MINUTE]))
														{	
															TimeProc.minute=usRegHoldingBuf[DEV_PROC_TIME_MINUTE];
														}
												}
												break;	
												
												case DEV_PROC_TIME_SECOND:
												{
														if(IS_TIME_SECONDS(usRegHoldingBuf[DEV_PROC_TIME_SECOND]))
														{															
															TimeProc.second=usRegHoldingBuf[DEV_PROC_TIME_SECOND];
														}
												}
												break;		

												case DEV_SET_OUTPUTS_SEQUENCE_STATE_1+ (3):
												{
														uint64_t tempValue;
														UINT16_Buf_To_UINT64(&usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_1], &tempValue);												
														DiscretOutputs_SetSequenceState(DISCR_OUT_SEQ_STATE_1,tempValue);
												}
												break;
												
												case DEV_SET_OUTPUTS_SEQUENCE_STATE_2+ (3):
												{
														uint64_t tempValue;
														UINT16_Buf_To_UINT64(&usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_2], &tempValue);
														DiscretOutputs_SetSequenceState(DISCR_OUT_SEQ_STATE_2,tempValue);
												}
												break;			

												case DEV_SET_OUTPUTS_SEQUENCE_STATE_END+ (3):
												{
														uint64_t tempValue;
														UINT16_Buf_To_UINT64(&usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_STATE_END], &tempValue);
														DiscretOutputs_SetSequenceState(DISCR_OUT_SEQ_STATE_END,tempValue);
												}
												break;													
												
												case DEV_SET_OUTPUTS_SEQUENCE_TIME:
												{													
														DiscretOutputs_SetSequenceImpulseTime(usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_TIME]);													
												}
												break;	

												case DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES:
												{
														DiscretOutputs_SetSequenceNumCycles(usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_NUM_CYCLES]);
												}
												break;

												case DEV_SET_OUTPUTS_SEQUENCE_START:
												{
														DiscretOutputs_StartSequence();
														usRegHoldingBuf[DEV_SET_OUTPUTS_SEQUENCE_START]=0;
												}
												break;	

												case DEV_SOUND_ALARM:
												{
														if(usRegHoldingBuf[DEV_SOUND_ALARM])
														{
																
														}
												}
												break;
												
												case DEV_LOAD_RESISTOR_VALUE_1+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_1],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[0], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[0]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;
												
												case DEV_LOAD_RESISTOR_VALUE_2+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_2],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[1], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[1]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_3+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_3],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[2], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[2]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;		

												case DEV_LOAD_RESISTOR_VALUE_4+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_4],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[3], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[3]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;		

												case DEV_LOAD_RESISTOR_VALUE_5+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_5],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[4], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[4]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_6+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_6],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[5], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[5]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_7+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_7],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[6], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[6]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_8+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_8],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[7], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[7]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_9+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_9],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[8], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[8]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_10+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_10],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[9], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[9]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_11+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_11],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[10], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[10]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_12+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_12],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[11], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[11]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_13+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_13],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[12], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[12]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
												}
												break;	

												case DEV_LOAD_RESISTOR_VALUE_14+(1):
												{
														UINT16_Buf_To_Float(&usRegHoldingBuf[DEV_LOAD_RESISTOR_VALUE_14],&tempCoef);
														
														if(FloatCheckEquality(configInfo.resistors[13], tempCoef, FLOAT_EQ_EPSILON)==FALSE)
														{
																configInfo.resistors[13]=tempCoef;
																settingsNeedWrite=TRUE;		
														}
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
										//StartConfigInfoWrite();
										if(ConfigInfoWrite()<0)
										{
												eStatus=MB_EIO;
										}
								}
								
								if(TCPtoRTURegWrite.nRegs>0)//Пишем в MBMaster и ждем ответа
								{
										eMBMasterReqErrCode    err;
										if( xSemaphoreTake( xMBRTUMutex, portMAX_DELAY ) == pdTRUE )
										{
												err=MBMaster_RTU_WriteRegs(&TCPtoRTURegWrite);
												xSemaphoreGive( xMBRTUMutex );
										}
										
										if(err==MB_MRE_NO_ERR)
										{
												if((TCPtoRTURegWrite.regAddr==(M_REG_HOLDING_START+REG_PIR_START))&& DCMI_ADC_Started())//запрос на запуск пиропатронов прошел успешно
												{
														//запустим скоростное измерение пиропатронов
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
