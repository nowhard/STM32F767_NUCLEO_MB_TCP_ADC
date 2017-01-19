#include "mbfunc.h"
#include "mb.h"
#include "mbtcp.h"

//#include "main.h"


/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START         1001
#define REG_INPUT_NREGS         10
#define REG_HOLDING_START       2001
#define REG_HOLDING_NREGS       10

//extern uint32_t LocalTime;
////extern uint16_t ADC_last_data[ADC_CHN_NUM];
//extern QueueHandle_t xADC_MB_Queue;

//extern sConfigInfo configInfo;

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

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

#define SERVER_IP_REG_0		0
#define SERVER_IP_REG_1		1
#define SERVER_IP_REG_2		2
#define SERVER_IP_REG_3		3

#define SERVER_PORT_REG_0	4

#define CLIENT_IP_REG_0		5
#define CLIENT_IP_REG_1		6
#define CLIENT_IP_REG_2		7
#define CLIENT_IP_REG_3		8

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
//        	usRegHoldingBuf[SERVER_IP_REG_0]=configInfo.IPAdress_Server.ip_addr_0;
//        	usRegHoldingBuf[SERVER_IP_REG_1]=configInfo.IPAdress_Server.ip_addr_1;
//        	usRegHoldingBuf[SERVER_IP_REG_2]=configInfo.IPAdress_Server.ip_addr_2;
//        	usRegHoldingBuf[SERVER_IP_REG_3]=configInfo.IPAdress_Server.ip_addr_3;

//        	usRegHoldingBuf[SERVER_PORT_REG_0]=configInfo.IPAdress_Server.port;

//        	usRegHoldingBuf[CLIENT_IP_REG_0]=configInfo.IPAdress_Client.ip_addr_0;
//        	usRegHoldingBuf[CLIENT_IP_REG_1]=configInfo.IPAdress_Client.ip_addr_1;
//        	usRegHoldingBuf[CLIENT_IP_REG_2]=configInfo.IPAdress_Client.ip_addr_2;
//        	usRegHoldingBuf[CLIENT_IP_REG_3]=configInfo.IPAdress_Client.ip_addr_3;


//            while( usNRegs > 0 )
//            {
//                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] >> 8 );
//                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] & 0xFF );
//                iRegIndex++;
//                usNRegs--;
//            }
//            break;
        }
            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
        {
//            while( usNRegs > 0 )
//            {
//                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
//                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
//                iRegIndex++;
//                usNRegs--;
//            }

//            configInfo.IPAdress_Server.ip_addr_0=usRegHoldingBuf[SERVER_IP_REG_0];
//            configInfo.IPAdress_Server.ip_addr_1=usRegHoldingBuf[SERVER_IP_REG_1];
//            configInfo.IPAdress_Server.ip_addr_2=usRegHoldingBuf[SERVER_IP_REG_2];
//            configInfo.IPAdress_Server.ip_addr_3=usRegHoldingBuf[SERVER_IP_REG_3];

//            configInfo.IPAdress_Server.port=usRegHoldingBuf[SERVER_PORT_REG_0];

//            configInfo.IPAdress_Client.ip_addr_0=usRegHoldingBuf[CLIENT_IP_REG_0];
//            configInfo.IPAdress_Client.ip_addr_1=usRegHoldingBuf[CLIENT_IP_REG_1];
//            configInfo.IPAdress_Client.ip_addr_2=usRegHoldingBuf[CLIENT_IP_REG_2];
//            configInfo.IPAdress_Client.ip_addr_3=usRegHoldingBuf[CLIENT_IP_REG_3];

//            StartConfigInfoWrite();

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
