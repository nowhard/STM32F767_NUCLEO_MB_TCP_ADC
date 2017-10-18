/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include <socket.h>
#include "main.h"
#include "memp.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "mbcontext.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "data_converter.h"
#include "adc_dcmi.h"
#include "adc_pyro_buf.h"
#include "spi_adc.h"
#include "cfg_info.h"
#include "tcp_send.h"
#include "utilities.h"

#if LWIP_TCP


extern SemaphoreHandle_t xAdcBuf_Send_Semaphore;
extern enADCPyroBufState ADCPyroBufState;
extern uint64_t ADC_Pyro_Timestamp;
extern sConfigInfo configInfo;	


stPacket TCPPacket;




/* ----------------------- Defines  -----------------------------------------*/
#define TCP_BUF_DEFAULT_PORT 1000 

/* ----------------------- Static variables ---------------------------------*/
SOCKET          xADCServListenSocket=INVALID_SOCKET;
SOCKET 					xClientSocket;
uint8_t    			aucTCPBuf[100];
uint16_t   			usTCPBufPos;
uint16_t   			usTCPFrameBytesLeft;	

static fd_set   allset;
/* ----------------------- Functions ---------------------------------*/

void TCP_ADC_Server_ReleaseClient(void);
BOOL TCP_ADC_Server_SendBuf(const uint8_t * pucTCPFrame, uint16_t usTCPLength );
BOOL TCP_ADC_Server_AcceptClient(void);
//void TCP_ADC_Server_Handling(void);

#define TCP_ADC_SERVER_TASK_STACK_SIZE	1024
#define TCP_ADC_SERVER_TASK_PRIO				4
void TCP_ADC_Server_Task( void *pvParameters );
/* ----------------------- Begin implementation -----------------------------*/

static void 		usleep(uint32_t time)
{
		while(time)
		{
				time--;
		}
}

uint8_t TCP_ADC_Server_Init( uint16_t usTCPPort )
{
   uint16_t          usPort;
   struct sockaddr_in serveraddr;
	
//	 int timeoutTimeInMiliSeconds=100;
	
			usTCPBufPos=0;
			usTCPFrameBytesLeft=0;
			xClientSocket=INVALID_SOCKET;
		
	  if(xADCServListenSocket==INVALID_SOCKET)
	  {
				if( usTCPPort == 0 )
				{
						usPort = TCP_BUF_DEFAULT_PORT;
				}
				else
				{
						usPort = ( uint16_t ) usTCPPort;
				}
				memset( &serveraddr, 0, sizeof( serveraddr ) );
				serveraddr.sin_family = AF_INET;
				serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );
				serveraddr.sin_port = htons( usPort );
				
				if( ( xADCServListenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == -1 )//create socket failed
				{
						return FALSE;
				}
		//		else if(setsockopt( xADCServListenSocket, SOL_SOCKET, SO_RCVTIMEO,&timeoutTimeInMiliSeconds, sizeof(int)) == -1)//Set timeout socket failed
		//		{
		//        return FALSE;
		//		}
				else if( bind( xADCServListenSocket, ( struct sockaddr * )&serveraddr, sizeof( serveraddr ) ) == -1 )//bind  socket failed
				{
						return FALSE;
				}
				else if( listen( xADCServListenSocket, 5 ) == -1 )//listen socket failed
				{
						return FALSE;
				}
				FD_ZERO( &allset );
				FD_SET( xADCServListenSocket, &allset );
				
				listen(xADCServListenSocket,5);		
	 }	

		xTaskCreate( TCP_ADC_Server_Task, "MBTCP HANDLE", TCP_ADC_SERVER_TASK_STACK_SIZE, NULL, TCP_ADC_SERVER_TASK_PRIO, NULL );

    return TRUE;
}



#define TCP_ADC_READ_TIMEOUT 1000        /* Maximum timeout to wait for packets. */
#define TCP_ADC_READ_CYCLE   100 /* Time between checking for new data. */

#define TCP_ADC_MAX_MSG_LEN		1000

BOOL TCP_ADC_Server_SendBuf(const uint8_t * pucTCPFrame, uint16_t usTCPLength )
{
    BOOL            bFrameSent = FALSE;
    BOOL            bAbort = FALSE;
    int             res;
    int             iBytesSent = 0;
    int             iTimeOut = TCP_ADC_READ_TIMEOUT;
	
		if(xClientSocket==INVALID_SOCKET)
		{
				return FALSE;
		}

    do
    {
				if((usTCPLength - iBytesSent)>=TCP_ADC_MAX_MSG_LEN)
				{
						res = send(xClientSocket, &pucTCPFrame[iBytesSent], TCP_ADC_MAX_MSG_LEN, 0 );
				}
				else
				{
						res = send(xClientSocket, &pucTCPFrame[iBytesSent], usTCPLength - iBytesSent, 0 );
				}
				
        switch ( res )
        {
        case -1:
            if( iTimeOut > 0 )
            {
                iTimeOut -= TCP_ADC_READ_CYCLE;
                usleep( TCP_ADC_READ_CYCLE );
            }
            else
            {
                bAbort = TRUE;
            }
            break;
        case 0:
            TCP_ADC_Server_ReleaseClient( );
            bAbort = TRUE;
            break;
        default:
            iBytesSent += res;
            break;
        }
    }
    while( ( iBytesSent != usTCPLength ) && !bAbort );

    bFrameSent = iBytesSent == usTCPLength ? TRUE : FALSE;

    return bFrameSent;
}

void TCP_ADC_Server_ReleaseClient(void)
{
//    ( void )recv( stTCPContext->xClientSocket, &stTCPContext->aucTCPBuf[0], MB_TCP_BUF_SIZE, 0 );

    ( void )close(xClientSocket );
    xClientSocket = INVALID_SOCKET;
}

BOOL TCP_ADC_Server_AcceptClient(void)
{
    SOCKET          xNewSocket;
    BOOL            bOkay;

    /* Check if we can handle a new connection. */

    if(xClientSocket != INVALID_SOCKET )//can't accept new client
    {				
        bOkay = FALSE;
    }
    else if( ( xNewSocket = accept( xADCServListenSocket, NULL, NULL ) ) == INVALID_SOCKET )
    {
        bOkay = FALSE;
    }
    else
    {
        xClientSocket = xNewSocket;
        usTCPBufPos = 0;
        usTCPFrameBytesLeft = 0;
        bOkay = TRUE;
    }
    return bOkay;
}


void TCP_ADC_Server_Task( void *pvParameters )
{
	uint16_t resultBufLen=3000;
	BOOL frameSent=FALSE;
	
	while(1)
	{
				if(xClientSocket==INVALID_SOCKET) //wait new client
				{				
						if(TCP_ADC_Server_AcceptClient()==TRUE)
						{
								continue;
						}
				}			
				else
				{	
						//Заполним структуру пакета данными
//						ADC_ConvertDCMIAndAssembleUDPBuf(TCPPacket.data, &resultBufLen);		
						TCPPacket.size=resultBufLen*sizeof(float);
						TCPPacket.type=PACKET_TYPE_BASE;
						TCPPacket.timestamp=0xFFFF;//DCMI_ADC_GetLastTimestamp();			
					
						frameSent=TCP_ADC_Server_SendBuf((uint8_t *)&TCPPacket,TCP_PACKET_HEADER_SIZE+resultBufLen*sizeof(float));
					
						if(frameSent)
						{
								vTaskDelay(5000);
						}
						else
						{
								TCP_ADC_Server_ReleaseClient();
						}					
				}
	}
}

#endif /* LWIP_TCP */


