#include "mbfunc.h"
#include "mb.h"
#include "mbtcp.h"
#include "cfg_info.h"
#include "adc_dcmi.h"
#include "udp_send.h"
#include "discret_out.h"
#include "mb_master_user.h"
#include "data_converter.h"
//#include "main.h"


/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START         1001
#define REG_INPUT_NREGS         64
#define REG_HOLDING_START       2001
#define REG_HOLDING_NREGS       64


//-------ADC RESULT REGS--------
#define ADC_CHANNEL_0_RESULT				0
#define ADC_CHANNEL_1_RESULT				2
#define ADC_CHANNEL_2_RESULT				4
#define ADC_CHANNEL_3_RESULT				6
#define ADC_CHANNEL_4_RESULT				8
#define ADC_CHANNEL_5_RESULT				10
#define TIMESTAMP_CURRENT						12

#define ADC_PYRO_SQUIB_0						16
#define ADC_PYRO_SQUIB_1						18
#define ADC_PYRO_SQUIB_2						20
#define ADC_PYRO_SQUIB_3						22
#define ADC_PYRO_SQUIB_4						24
#define ADC_PYRO_SQUIB_5						26
#define ADC_PYRO_SQUIB_6						28
#define ADC_PYRO_SQUIB_7						30

//--------FAULT SIGNALS-----------
#define FAULT_OUT_1_SIG							31
#define FAULT_OUT_7_SIG							32

#define FAULT_250A_SIG							33
#define FAULT_150A_SIG							34
#define FAULT_75A_SIG								35
#define FAULT_7_5A_SIG							36

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
			

			
			*((float*)&usRegInputBuf[ADC_CHANNEL_0_RESULT])=ChnCalibrValues.val_250A;
			*((float*)&usRegInputBuf[ADC_CHANNEL_1_RESULT])=ChnCalibrValues.val_150A;
			*((float*)&usRegInputBuf[ADC_CHANNEL_2_RESULT])=ChnCalibrValues.val_75A;
			*((float*)&usRegInputBuf[ADC_CHANNEL_3_RESULT])=ChnCalibrValues.val_7_5A;
			*((float*)&usRegInputBuf[ADC_CHANNEL_4_RESULT])=ChnCalibrValues.val_voltage_1;
			*((float*)&usRegInputBuf[ADC_CHANNEL_5_RESULT])=ChnCalibrValues.val_voltage_2;
			*((uint64_t*)&usRegInputBuf[TIMESTAMP_CURRENT])=DCMI_ADC_GetLastTimestamp();
			
			memcpy((void *)&usRegInputBuf[ADC_PYRO_SQUIB_0],(const void*)&usMRegInBuf[0][0],M_REG_INPUT_NREGS);
			
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

#define ADC_SAMPLERATE				34 
#define ADC_STARTED						36

//--------BITFIELDS------------------------
//#define DEV_SET_OUTPUTS				37

#define DEV_SET_OUTPUTS_0			37
#define DEV_SET_OUTPUTS_1			38
#define DEV_SET_OUTPUTS_2			39
#define DEV_SET_OUTPUTS_3			40

#define DEV_ENABLE_OUT_1			41
#define DEV_ENABLE_OUT_7			42

#define DEV_EN_VCC_250				43
#define DEV_EN_VCC_150				44
#define DEV_EN_VCC_75					45
#define DEV_EN_VCC_7_5				46

#define DEV_ENABLE_AIR				47
//--------SYNC DEV REGS--------------------
#define DEV_RESET_TIMESTAMP		48
#define DEV_RESET_CONTROLLER	61
//--------REGS STM32F100DISCOVERY----------
#define PIR_EN_PYRO_SQUIB_0		49
#define PIR_EN_PYRO_SQUIB_1		50
#define PIR_EN_PYRO_SQUIB_2		51
#define PIR_EN_PYRO_SQUIB_3		52
#define PIR_EN_PYRO_SQUIB_4		53
#define PIR_EN_PYRO_SQUIB_5		54
#define PIR_EN_PYRO_SQUIB_6		55
#define PIR_EN_PYRO_SQUIB_7		56

//--DIGITAL POTENTIOMETERS-----------------
#define PIR_SET_POT1			57
#define PIR_SET_POT2			58
#define PIR_SET_POT3			59
#define PIR_SET_POT4			60

extern uint16_t outputs_temp_reg_0;
extern uint16_t outputs_temp_reg_1;
extern uint16_t outputs_temp_reg_2;
extern uint16_t outputs_temp_reg_3;

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
					
					usRegHoldingBuf[ADC_SAMPLERATE]=(uint16_t)configInfo.ConfigADC.sampleRate;
					
					usRegHoldingBuf[ADC_STARTED]=adc_started_flag;
					usRegHoldingBuf[DEV_SET_OUTPUTS_0] = outputs_temp_reg_0;
					usRegHoldingBuf[DEV_SET_OUTPUTS_1] = outputs_temp_reg_1;
					usRegHoldingBuf[DEV_SET_OUTPUTS_2] = outputs_temp_reg_2;
					usRegHoldingBuf[DEV_SET_OUTPUTS_3] = outputs_temp_reg_3;
					
					usRegHoldingBuf[DEV_ENABLE_OUT_1]=HAL_GPIO_ReadPin(ENABLE_OUT_1_GPIO_Port,ENABLE_OUT_1_Pin);
					usRegHoldingBuf[DEV_ENABLE_OUT_7]=HAL_GPIO_ReadPin(ENABLE_OUT_7_GPIO_Port,ENABLE_OUT_7_Pin);
					
					usRegHoldingBuf[DEV_EN_VCC_250]=HAL_GPIO_ReadPin(EN_VCC_250_GPIO_Port,EN_VCC_250_Pin);
					usRegHoldingBuf[DEV_EN_VCC_150]=HAL_GPIO_ReadPin(EN_VCC_150_GPIO_Port,EN_VCC_150_Pin);
					usRegHoldingBuf[DEV_EN_VCC_75]=HAL_GPIO_ReadPin(EN_VCC_75_GPIO_Port,EN_VCC_75_Pin);
					usRegHoldingBuf[DEV_EN_VCC_7_5]=HAL_GPIO_ReadPin(EN_VCC_7_5_GPIO_Port,EN_VCC_7_5_Pin);
					
					usRegHoldingBuf[DEV_ENABLE_AIR]=HAL_GPIO_ReadPin(ENABLE_AIR_GPIO_Port,ENABLE_AIR_Pin);
					

					memcpy((void *)&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_0],(const void*)&usMRegHoldBuf[0][0],M_REG_HOLDING_NREGS);

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

									case CLIENT_IP_REG_0:
									{
											configInfo.IPAdress_Client.ip_addr_0=usRegHoldingBuf[CLIENT_IP_REG_0];
											settings_need_write=1;
									}
									break;	

									case CLIENT_IP_REG_1:
									{
											configInfo.IPAdress_Client.ip_addr_1=usRegHoldingBuf[CLIENT_IP_REG_1];
											settings_need_write=1;
									}
									break;		
									
									case CLIENT_IP_REG_2:
									{
											configInfo.IPAdress_Client.ip_addr_2=usRegHoldingBuf[CLIENT_IP_REG_2];
											settings_need_write=1;
									}
									break;	

									case CLIENT_IP_REG_3:
									{
											configInfo.IPAdress_Client.ip_addr_3=usRegHoldingBuf[CLIENT_IP_REG_3];
											settings_need_write=1;
									}
									break;	

									case ADC_CHANNEL_MASK_REG:
									{
											(uint16_t)configInfo.ConfigADC.channelMask=usRegHoldingBuf[ADC_CHANNEL_MASK_REG];
											settings_need_write=1;
									}
									break;	

									case ADC_CHANNEL_0_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[0].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_0_K]);
											settings_need_write=1;
									}
									break;

									case ADC_CHANNEL_0_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[0].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_0_B]);	
											settings_need_write=1;
									}
									break;

									case ADC_CHANNEL_1_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[1].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_1_K]);
											settings_need_write=1;										
									}
									break;

									case ADC_CHANNEL_1_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[1].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_1_B]);	
											settings_need_write=1;										
									}
									break;	

									case ADC_CHANNEL_2_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[2].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_2_K]);
											settings_need_write=1;										
									}
									break;

									case ADC_CHANNEL_2_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[2].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_2_B]);
											settings_need_write=1;										
									}
									break;	

									case ADC_CHANNEL_3_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[3].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_3_K]);
											settings_need_write=1;										
									}
									break;

									case ADC_CHANNEL_3_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[3].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_3_B]);	
											settings_need_write=1;										
									}
									break;	

									case ADC_CHANNEL_4_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[4].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_4_K]);
											settings_need_write=1;										
									}
									break;

									case ADC_CHANNEL_4_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[4].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_4_B]);
											settings_need_write=1;										
									}
									break;	

									case ADC_CHANNEL_5_K+(1):
									{
											configInfo.ConfigADC.calibrChannel[5].k =*((float*)&usRegHoldingBuf[ADC_CHANNEL_5_K]);
											settings_need_write=1;										
									}
									break;

									case ADC_CHANNEL_5_B+(1):
									{
											configInfo.ConfigADC.calibrChannel[5].b =*((float*)&usRegHoldingBuf[ADC_CHANNEL_5_B]);
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
											DiscretOutputs_Set((((uint64_t)outputs_temp_reg_0))|(((uint64_t)outputs_temp_reg_1)<<16)|(((uint64_t)outputs_temp_reg_2)<<32)|(((uint64_t)outputs_temp_reg_3)<<48));
									}
									break;

									case DEV_SET_OUTPUTS_1:
									{											
											outputs_temp_reg_1=0x0;
											outputs_temp_reg_1|=usRegHoldingBuf[DEV_SET_OUTPUTS_1];
											DiscretOutputs_Set((((uint64_t)outputs_temp_reg_0))|(((uint64_t)outputs_temp_reg_1)<<16)|(((uint64_t)outputs_temp_reg_2)<<32)|(((uint64_t)outputs_temp_reg_3)<<48));
									}
									break;	

									case DEV_SET_OUTPUTS_2:
									{											
											outputs_temp_reg_2=0x0;
											outputs_temp_reg_2|=usRegHoldingBuf[DEV_SET_OUTPUTS_2];
											DiscretOutputs_Set((((uint64_t)outputs_temp_reg_0))|(((uint64_t)outputs_temp_reg_1)<<16)|(((uint64_t)outputs_temp_reg_2)<<32)|(((uint64_t)outputs_temp_reg_3)<<48));
									}
									break;		

									case DEV_SET_OUTPUTS_3:
									{											
											outputs_temp_reg_3=0x0;
											outputs_temp_reg_3|=usRegHoldingBuf[DEV_SET_OUTPUTS_3];
											DiscretOutputs_Set((((uint64_t)outputs_temp_reg_0))|(((uint64_t)outputs_temp_reg_1)<<16)|(((uint64_t)outputs_temp_reg_2)<<32)|(((uint64_t)outputs_temp_reg_3)<<48));
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

									case PIR_EN_PYRO_SQUIB_0:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+0;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_0];
											xSemaphoreGive(xSendRTURegSem);
									}
									break;
									
									case PIR_EN_PYRO_SQUIB_1:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+1;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_1];
											xSemaphoreGive(xSendRTURegSem);	
									}
									break;
									
									case PIR_EN_PYRO_SQUIB_2:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+2;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_2];
											xSemaphoreGive(xSendRTURegSem);										
									}
									break;
									
									case PIR_EN_PYRO_SQUIB_3:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+3;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_3];
											xSemaphoreGive(xSendRTURegSem);											
									}
									break;	

									case PIR_EN_PYRO_SQUIB_4:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+4;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_4];
											xSemaphoreGive(xSendRTURegSem);											
									}
									break;
									
									case PIR_EN_PYRO_SQUIB_5:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+5;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_5];
											xSemaphoreGive(xSendRTURegSem);											
									}
									break;
									
									case PIR_EN_PYRO_SQUIB_6:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+6;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_6];
											xSemaphoreGive(xSendRTURegSem);											
									}
									break;
									
									case PIR_EN_PYRO_SQUIB_7:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+7;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_EN_PYRO_SQUIB_7];
											xSemaphoreGive(xSendRTURegSem);											
									}
									break;	

									case PIR_SET_POT1:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+8;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_SET_POT1];
											xSemaphoreGive(xSendRTURegSem);		
									}
									break;
									
									case PIR_SET_POT2:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+9;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_SET_POT2];
											xSemaphoreGive(xSendRTURegSem);		
									}
									break;
									
									case PIR_SET_POT3:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+10;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_SET_POT3];
											xSemaphoreGive(xSendRTURegSem);		
									}
									break;
									
									case PIR_SET_POT4:
									{
											TCPtoRTURegWrite.nRegs=1;
											TCPtoRTURegWrite.regAddr=M_REG_HOLDING_START+11;
											TCPtoRTURegWrite.regBuf=&usRegHoldingBuf[PIR_SET_POT4];
											xSemaphoreGive(xSendRTURegSem);		
									}
									break;																																	
								}
														
								iRegIndex++;
                usNRegs--;							
            }

						if(settings_need_write)
						{
								StartConfigInfoWrite();
						}
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
