#include "mbfunc.h"
#include "mb.h"
#include "mbtcp.h"
#include "cfg_info.h"
#include "adc_dcmi.h"
#include "udp_send.h"
#include "discret_out.h"
//#include "main.h"


/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START         1001
#define REG_INPUT_NREGS         64
#define REG_HOLDING_START       2001
#define REG_HOLDING_NREGS       64

//extern uint32_t LocalTime;
////extern uint16_t ADC_last_data[ADC_CHN_NUM];
//extern QueueHandle_t xADC_MB_Queue;

//extern sConfigInfo configInfo;

//-------ADC RESULT REGS--------
#define ADC_CHANNEL_0_RESULT				0
#define ADC_CHANNEL_1_RESULT				2
#define ADC_CHANNEL_2_RESULT				4
#define ADC_CHANNEL_3_RESULT				6
#define ADC_CHANNEL_4_RESULT				8
#define ADC_CHANNEL_5_RESULT				10
#define TIMESTAMP_CURRENT						12

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

extern float ADC_resultBuf[ADC_RESULT_BUF_LEN];
extern SemaphoreHandle_t	xMBSaveSettingsSemaphore;

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
			
			*((float*)&usRegInputBuf[ADC_CHANNEL_0_RESULT])=ADC_resultBuf[0];
			*((float*)&usRegInputBuf[ADC_CHANNEL_1_RESULT])=ADC_resultBuf[1];
			*((float*)&usRegInputBuf[ADC_CHANNEL_2_RESULT])=ADC_resultBuf[2];
			*((float*)&usRegInputBuf[ADC_CHANNEL_3_RESULT])=ADC_resultBuf[3];
			*((float*)&usRegInputBuf[ADC_CHANNEL_4_RESULT])=ADC_resultBuf[4];
			*((float*)&usRegInputBuf[ADC_CHANNEL_5_RESULT])=ADC_resultBuf[5];
			*((uint64_t*)&usRegInputBuf[TIMESTAMP_CURRENT])=DCMI_ADC_GetLastTimestamp();
			
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

#define CLIENT_IP_REG_0		5
#define CLIENT_IP_REG_1		6
#define CLIENT_IP_REG_2		7
#define CLIENT_IP_REG_3		8
//-------ADC UDP DATA SETTINGS REGS--------
#define ADC_CHANNEL_MASK_REG	9

#define ADC_CHANNEL_0_K				10
#define ADC_CHANNEL_0_B				12
#define ADC_CHANNEL_1_K				14
#define ADC_CHANNEL_1_B				16
#define ADC_CHANNEL_2_K				18
#define ADC_CHANNEL_2_B				20
#define ADC_CHANNEL_3_K				22
#define ADC_CHANNEL_3_B				24
#define ADC_CHANNEL_4_K				26
#define ADC_CHANNEL_4_B				28
#define ADC_CHANNEL_5_K				30
#define ADC_CHANNEL_5_B				32

#define ADC_SAMPLERATE				34 //0-100000
#define ADC_STARTED						36

//--------BITFIELDS------------------------
//#define DEV_SET_OUTPUTS				37

#define DEV_SET_OUTPUTS_0			37
#define DEV_SET_OUTPUTS_1			38
#define DEV_SET_OUTPUTS_2			39
//--------SYNC DEV REGS--------------------
#define DEV_RESET_TIMESTAMP		40

static uint16_t outputs_temp_reg_0=0;
static uint16_t outputs_temp_reg_1=0;
static uint16_t outputs_temp_reg_2=0;

static uint16_t adc_started_flag=0;

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
        case MB_REG_READ:
        {
        	usRegHoldingBuf[SERVER_IP_REG_0]=configInfo.IPAdress_Server.ip_addr_0;
        	usRegHoldingBuf[SERVER_IP_REG_1]=configInfo.IPAdress_Server.ip_addr_1;
        	usRegHoldingBuf[SERVER_IP_REG_2]=configInfo.IPAdress_Server.ip_addr_2;
        	usRegHoldingBuf[SERVER_IP_REG_3]=configInfo.IPAdress_Server.ip_addr_3;

        	usRegHoldingBuf[SERVER_PORT_REG_0]=configInfo.IPAdress_Server.port;

        	usRegHoldingBuf[CLIENT_IP_REG_0]=configInfo.IPAdress_Client.ip_addr_0;
        	usRegHoldingBuf[CLIENT_IP_REG_1]=configInfo.IPAdress_Client.ip_addr_1;
        	usRegHoldingBuf[CLIENT_IP_REG_2]=configInfo.IPAdress_Client.ip_addr_2;
        	usRegHoldingBuf[CLIENT_IP_REG_3]=configInfo.IPAdress_Client.ip_addr_3;
					
					usRegHoldingBuf[ADC_CHANNEL_MASK_REG]=(uint16_t)configInfo.ConfigADC.channelMask;
					
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_0_K])=configInfo.ConfigADC.calibrChannel[0].k;
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_0_B])=configInfo.ConfigADC.calibrChannel[0].b;
					
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_1_K])=configInfo.ConfigADC.calibrChannel[1].k;
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_1_B])=configInfo.ConfigADC.calibrChannel[1].b;
					
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_2_K])=configInfo.ConfigADC.calibrChannel[2].k;
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_2_B])=configInfo.ConfigADC.calibrChannel[2].b;
					
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_3_K])=configInfo.ConfigADC.calibrChannel[3].k;
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_3_B])=configInfo.ConfigADC.calibrChannel[3].b;
					
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_4_K])=configInfo.ConfigADC.calibrChannel[4].k;
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_4_B])=configInfo.ConfigADC.calibrChannel[4].b;
					
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_5_K])=configInfo.ConfigADC.calibrChannel[5].k;
					*((float*)&usRegHoldingBuf[ADC_CHANNEL_5_B])=configInfo.ConfigADC.calibrChannel[5].b;
					
					*((uint32_t*)&usRegHoldingBuf[ADC_SAMPLERATE])=configInfo.ConfigADC.sampleRate;
					
					usRegHoldingBuf[ADC_STARTED]=adc_started_flag;
					usRegHoldingBuf[DEV_SET_OUTPUTS_0] = outputs_temp_reg_0;
					usRegHoldingBuf[DEV_SET_OUTPUTS_1] = outputs_temp_reg_1;
					usRegHoldingBuf[DEV_SET_OUTPUTS_2] = outputs_temp_reg_2;


            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;
        }
            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
        {
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                
								switch(iRegIndex)
								{
									case SERVER_IP_REG_0:
									{
											configInfo.IPAdress_Server.ip_addr_0=usRegHoldingBuf[SERVER_IP_REG_0];
									}
									break;
									
									case SERVER_IP_REG_1:
									{
										  configInfo.IPAdress_Server.ip_addr_1=usRegHoldingBuf[SERVER_IP_REG_1];
									}
									break;
									
									case SERVER_IP_REG_2:
									{
											configInfo.IPAdress_Server.ip_addr_2=usRegHoldingBuf[SERVER_IP_REG_2];
									}
									break;	

									case SERVER_IP_REG_3:
									{
											configInfo.IPAdress_Server.ip_addr_3=usRegHoldingBuf[SERVER_IP_REG_3];
									}
									break;

									case SERVER_PORT_REG_0:
									{
											configInfo.IPAdress_Server.port=usRegHoldingBuf[SERVER_PORT_REG_0];
									}
									break;

									case CLIENT_IP_REG_0:
									{
											configInfo.IPAdress_Client.ip_addr_0=usRegHoldingBuf[CLIENT_IP_REG_0];
									}
									break;	

									case CLIENT_IP_REG_1:
									{
											configInfo.IPAdress_Client.ip_addr_1=usRegHoldingBuf[CLIENT_IP_REG_1];
									}
									break;		
									
									case CLIENT_IP_REG_2:
									{
											configInfo.IPAdress_Client.ip_addr_2=usRegHoldingBuf[CLIENT_IP_REG_2];
									}
									break;	

									case CLIENT_IP_REG_3:
									{
											configInfo.IPAdress_Client.ip_addr_3=usRegHoldingBuf[CLIENT_IP_REG_3];
									}
									break;	

									case ADC_CHANNEL_MASK_REG:
									{
											(uint16_t)configInfo.ConfigADC.channelMask=usRegHoldingBuf[ADC_CHANNEL_MASK_REG];
									}
									break;	

									case ADC_CHANNEL_0_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[0].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_0_K]);
									}
									break;

									case ADC_CHANNEL_0_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[0].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_0_B]);	
									}
									break;

									case ADC_CHANNEL_1_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[1].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_1_K]);
									}
									break;

									case ADC_CHANNEL_1_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[1].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_1_B]);	
									}
									break;	

									case ADC_CHANNEL_2_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[2].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_2_K]);
									}
									break;

									case ADC_CHANNEL_2_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[2].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_2_B]);
									}
									break;	

									case ADC_CHANNEL_3_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[3].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_3_K]);
									}
									break;

									case ADC_CHANNEL_3_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[3].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_3_B]);	
									}
									break;	

									case ADC_CHANNEL_4_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[4].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_4_K]);
									}
									break;

									case ADC_CHANNEL_4_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[4].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_4_B]);
									}
									break;	

									case ADC_CHANNEL_5_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[5].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_5_K]);
									}
									break;

									case ADC_CHANNEL_5_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[5].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_5_B]);	
									}
									break;

									case ADC_SAMPLERATE+(1):
									{
											DCMI_ADC_SetSamplerate(*((uint32_t*)&usRegHoldingBuf[ADC_SAMPLERATE]));
									}
									break;	
									
									case ADC_STARTED:
									{
											if(usRegHoldingBuf[ADC_STARTED])
											{
													DCMI_ADC_Clock_Start();
													adc_started_flag=1;
											}
											else
											{
													DCMI_ADC_Clock_Stop();
													adc_started_flag=0;
											}
											usRegHoldingBuf[ADC_STARTED]=0;
									}
									break;	
									
//									case DEV_SET_OUTPUTS+3:
//									{
//											DiscretOutputs_Set(*(uint64_t*)&usRegHoldingBuf[DEV_SET_OUTPUTS]);
//									}
//									break;		

										
									case DEV_SET_OUTPUTS_0:
									{											
											outputs_temp_reg_0=0x0;
											outputs_temp_reg_0|=usRegHoldingBuf[DEV_SET_OUTPUTS_0];
											DiscretOutputs_Set((((uint64_t)outputs_temp_reg_0))|(((uint64_t)outputs_temp_reg_1)<<16)|(((uint64_t)outputs_temp_reg_2)<<32));
									}
									break;

									case DEV_SET_OUTPUTS_1:
									{											
											outputs_temp_reg_1=0x0;
											outputs_temp_reg_1|=usRegHoldingBuf[DEV_SET_OUTPUTS_1];
											DiscretOutputs_Set((((uint64_t)outputs_temp_reg_0))|(((uint64_t)outputs_temp_reg_1)<<16)|(((uint64_t)outputs_temp_reg_2)<<32));
									}
									break;	

									case DEV_SET_OUTPUTS_2:
									{											
											outputs_temp_reg_2=0x0;
											outputs_temp_reg_2|=usRegHoldingBuf[DEV_SET_OUTPUTS_2];
											DiscretOutputs_Set((((uint64_t)outputs_temp_reg_0))|(((uint64_t)outputs_temp_reg_1)<<16)|(((uint64_t)outputs_temp_reg_2)<<32));
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
								
								}
														
								iRegIndex++;
                usNRegs--;							
            }

						StartConfigInfoWrite();
        }
      }
    }
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
