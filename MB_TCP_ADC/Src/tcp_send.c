/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "main.h"
#include "memp.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

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
//float ADC_resultBuf[ADC_DCMI_RESULT_BUF_LEN];

SemaphoreHandle_t xTCPClient_Connected_Semaphore=NULL;
SemaphoreHandle_t xTCPClient_Sent_Semaphore=NULL;

stPacket TCPPacket;

#define TCP_CLIENT_TASK_STACK_SIZE	1024
#define TCP_CLIENT_TASK_PRIO				4
void TCP_ADC_Server_Task( void *pvParameters );




/* ----------------------- Defines  -----------------------------------------*/
#define TCP_BUF_DEFAULT_PORT 1000 

/* ----------------------- Static variables ---------------------------------*/
static struct tcp_pcb *pxPCBListen;
static struct tcp_pcb *pxPCBClient;
/* ----------------------- Functions ---------------------------------*/
uint8_t TCP_ADC_Server_Init( uint16_t usTCPPort );
void TCP_ADC_Server_ReleaseClient( struct tcp_pcb *pxPCB );
uint8_t TCP_ADC_Server_SendBuf( const uint8_t * adcBuf, uint16_t adcBufLength );

static err_t    TCP_ADC_Server_Accept( void *pvArg, struct tcp_pcb *pxPCB, err_t xErr );
static void     TCP_ADC_Server_Error( void *pvArg, err_t xErr );
/* ----------------------- Begin implementation -----------------------------*/
uint8_t TCP_ADC_Server_Init( uint16_t usTCPPort )
{
    struct tcp_pcb *pxPCBListenNew, *pxPCBListenOld;
    uint8_t            bOkay = FALSE;
    uint16_t          usPort;

    if( usTCPPort == 0 )
    {
        usPort = TCP_BUF_DEFAULT_PORT;
    }
    else
    {
        usPort = ( uint16_t ) usTCPPort;
    }

    if( ( pxPCBListenNew = pxPCBListenOld = tcp_new(  ) ) == NULL )
    {
        /* Can't create TCP socket. */
        bOkay = FALSE;
    }
    else if( tcp_bind( pxPCBListenNew, IP_ADDR_ANY, ( u16_t ) usPort ) != ERR_OK )
    {
        /* Bind failed - Maybe illegal port value or in use. */
        ( void )tcp_close( pxPCBListenOld );
        bOkay = FALSE;
    }
    else if( ( pxPCBListenNew = tcp_listen( pxPCBListenNew ) ) == NULL )
    {
        ( void )tcp_close( pxPCBListenOld );
        bOkay = FALSE;
    }
    else
    {
        /* Register callback function for new clients. */
        tcp_accept( pxPCBListenNew, TCP_ADC_Server_Accept );

        /* Everything okay. Set global variable. */
        pxPCBListen = pxPCBListenNew;
    }
    bOkay = TRUE;
    return bOkay;
}

void TCP_ADC_Server_ReleaseClient( struct tcp_pcb *pxPCB )
{
    if( pxPCB != NULL )
    {
        if( tcp_close( pxPCB ) != ERR_OK )
        {
            tcp_abort( pxPCB );
        }

        if( pxPCB == pxPCBClient )
        {
            pxPCBClient = NULL;
        }
        if( pxPCB == pxPCBListen )
        {
            pxPCBListen = NULL;
        }
    }
}


err_t TCP_ADC_Server_Accept( void *pvArg, struct tcp_pcb *pxPCB, err_t xErr )
{
    err_t           error;

    if( xErr != ERR_OK )
    {
        return xErr;
    }

    /* We can handle only one client. */
    if( pxPCBClient == NULL )
    {
        /* Register the client. */
        pxPCBClient = pxPCB;

        /* Set up the receive function prvxMBTCPPortReceive( ) to be called when data
         * arrives.
         */
        tcp_recv( pxPCB, NULL );

        /* Register error handler. */
        tcp_err( pxPCB, TCP_ADC_Server_Error );

        /* Set callback argument later used in the error handler. */
        tcp_arg( pxPCB, pxPCB );

        error = ERR_OK;
    }
    else
    {
        TCP_ADC_Server_ReleaseClient( pxPCB );
        error = ERR_OK;
    }
    return error;
}

/* Called in case of an unrecoverable error. In any case we drop the client
 * connection. */
void TCP_ADC_Server_Error( void *pvArg, err_t xErr )
{
    struct tcp_pcb *pxPCB = pvArg;

    if( pxPCB != NULL )
    {
        TCP_ADC_Server_ReleaseClient( pxPCB );
    }
}


uint8_t TCP_ADC_Server_SendBuf( const uint8_t * adcBuf, uint16_t adcBufLength )
{
    uint8_t            bFrameSent = FALSE;

    if( pxPCBClient )
    {
        /* Make sure we can send the packet. */
        //assert( tcp_sndbuf( pxPCBClient ) >= adcBufLength );

        if( tcp_write( pxPCBClient, adcBuf, ( u16_t ) adcBufLength, TCP_WRITE_FLAG_COPY ) == ERR_OK )
        {
            /* Make sure data gets sent immediately. */
            ( void )tcp_output( pxPCBClient );
            bFrameSent = TRUE;
        }
        else
        {
            /* Drop the connection in case of an write error. */
            TCP_ADC_Server_ReleaseClient( pxPCBClient );
        }
    }
    return bFrameSent;
}



#define TCP_CLIENT_CONNECTED_TIMEOUT	100
#define TCP_CLIENT_SENT_TIMEOUT				100



void TCP_ADC_Server_Task( void *pvParameters )
{
	uint16_t resultBufLen=0;
	err_t err = ERR_OK;
	
	while(1)
	{
			if(TCP_ADC_Server_Init(TCP_BUF_DEFAULT_PORT)==TRUE)
			{
					err = ERR_OK;
					while(err == ERR_OK)
					{
						//Заполним структуру пакета данными
						ADC_ConvertDCMIAndAssembleUDPBuf(TCPPacket.data, &resultBufLen);		
						TCPPacket.size=resultBufLen*sizeof(float);
						TCPPacket.type=PACKET_TYPE_BASE;
						TCPPacket.timestamp=DCMI_ADC_GetLastTimestamp();
						
						err=TCP_ADC_Server_SendBuf((uint8_t *)&TCPPacket,TCP_PACKET_HEADER_SIZE+resultBufLen*sizeof(float));
					}
			}
			vTaskDelay(100);
	}
}

#endif /* LWIP_TCP */


