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

/* ----------------------- Defines  -----------------------------------------*/
#define TCP_BUF_DEFAULT_PORT 1000 

/* ----------------------- Static variables ---------------------------------*/
static SOCKET          xADCServListenSocket=INVALID_SOCKET;
static SOCKET 					xClientSocket;


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

		xTaskCreate( TCP_ADC_Server_Task, "TCP SERVER HANDLE", TCP_ADC_SERVER_TASK_STACK_SIZE, NULL, TCP_ADC_SERVER_TASK_PRIO, NULL );

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
        bOkay = TRUE;
    }
    return bOkay;
}

BOOL TCP_ADC_Send_BaseBuf(stPacket *TCPPacket)
{
		BOOL frameSent=FALSE;
	
		if(xClientSocket==INVALID_SOCKET)
		{
				return FALSE;
		}
	
		TCPPacket->startOfFrame=TCP_ADC_START_FRAME_MAGIC;
		TCPPacket->type=PACKET_TYPE_BASE;
		TCPPacket->timestamp=DCMI_ADC_GetLastTimestamp();		
		
		frameSent=TCP_ADC_Server_SendBuf((uint8_t*)TCPPacket,TCP_PACKET_HEADER_SIZE+TCPPacket->size);
	
		if(!frameSent)
		{
				TCP_ADC_Server_ReleaseClient();
		}
		
		return frameSent;
}

BOOL TCP_ADC_Send_PyroBuf(stPacket *TCPPacket)
{
		BOOL frameSent=FALSE;
	
		if(xClientSocket==INVALID_SOCKET)
		{
				return FALSE;
		}
		
		TCPPacket->startOfFrame=TCP_ADC_START_FRAME_MAGIC;
		TCPPacket->type=PACKET_TYPE_ADC_PYRO;
		TCPPacket->timestamp=ADC_Pyro_Timestamp;		
	
		if((ADCPyroBufState==ADC_PYRO_BUF_FILL_STOP) && ADC_PyroBuf_GetCurrentLength())//передача данных ацп пиропатрона закончена
		{
			TCPPacket->size=ADC_PyroBuf_Copy((void *)TCPPacket->data,PACKET_BUF_SIZE);
			frameSent=TCP_ADC_Server_SendBuf((uint8_t *)TCPPacket,TCP_PACKET_HEADER_SIZE+TCPPacket->size);
		}
		else
		{
				frameSent=TRUE;
		}
		
		if(!frameSent)
		{
				TCP_ADC_Server_ReleaseClient();
		}
			
		return frameSent;
}


#define SERVER_TASK_IDLE_CYCLE		100
void TCP_ADC_Server_Task( void *pvParameters )
{
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
						vTaskDelay(SERVER_TASK_IDLE_CYCLE);									
				}
	}
}

#endif /* LWIP_TCP */


