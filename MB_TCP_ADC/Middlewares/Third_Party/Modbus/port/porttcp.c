#include <stdio.h>
#include <socket.h>
#include <string.h>

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define PTCP_DEBUG

#ifdef PTCP_DEBUG
#define PTCP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define PTCP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


SOCKET          xListenSocket=INVALID_SOCKET;
static fd_set   allset;





#define MB_TCP_CLIENT_STACK_SIZE	1024
#define MB_TCP_POOL_STACK_SIZE		512



SemaphoreHandle_t xMB_FrameRec_Mutex;

uint8_t flag_mb_ready=0;

/* ----------------------- Static functions ---------------------------------*/
BOOL            prvMBTCPPortAddressToString( SOCKET xSocket, CHAR * szAddr, USHORT usBufSize );
CHAR           *prvMBTCPPortFrameToString( UCHAR * pucFrame, USHORT usFrameLen );
static BOOL     prvbMBPortAcceptClient( stMBContext*	  stTCPContext );
static void     prvvMBPortReleaseClient( stMBContext*	  stTCPContext);
static void 		usleep(uint32_t time);
void xMBTCPPort_HandlingTask(void *pvParameters );
void xMBTCPPort_PoolTask(void *pvParameters );

/* ----------------------- Begin implementation -----------------------------*/

static void 		usleep(uint32_t time)
{
		while(time)
		{
				time--;
		}
}


BOOL
xMBTCPPortInit(stMBContext*	  stTCPContext, USHORT usTCPPort )
{
    USHORT          usPort;
    struct sockaddr_in serveraddr;
	
		 xMB_FrameRec_Mutex = xSemaphoreCreateMutex();
	
			int timeoutTimeInMiliSeconds=100;
	
		stTCPContext->usTCPBufPos=0;
		stTCPContext->usTCPFrameBytesLeft=0;
		stTCPContext->xClientSocket=INVALID_SOCKET;
		
	  if(xListenSocket==INVALID_SOCKET)
	  {
				if( usTCPPort == 0 )
				{
						usPort = MB_TCP_DEFAULT_PORT;
				}
				else
				{
						usPort = ( USHORT ) usTCPPort;
				}
				memset( &serveraddr, 0, sizeof( serveraddr ) );
				serveraddr.sin_family = AF_INET;
				serveraddr.sin_addr.s_addr = htonl( INADDR_ANY );
				serveraddr.sin_port = htons( usPort );
				if( ( xListenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == -1 )
				{
						PTCP_DEBUG_PRINT( "Create socket failed.\r\n" );
						return FALSE;
				}
		//		else if(setsockopt( xListenSocket, SOL_SOCKET, SO_RCVTIMEO,&timeoutTimeInMiliSeconds, sizeof(int)) == -1)
		//		{
		//				PTCP_DEBUG_PRINT( "Set timeout socket failed.\r\n" );
		//        return FALSE;
		//		}
				else if( bind( xListenSocket, ( struct sockaddr * )&serveraddr, sizeof( serveraddr ) ) == -1 )
				{
						PTCP_DEBUG_PRINT( "Bind socket failed.\r\n" );
						return FALSE;
				}
				else if( listen( xListenSocket, 5 ) == -1 )
				{
						PTCP_DEBUG_PRINT( "Listen socket failed.\r\n" );
						return FALSE;
				}
				FD_ZERO( &allset );
				FD_SET( xListenSocket, &allset );
				
				listen(xListenSocket,5);
				
				xTaskCreate( xMBTCPPort_PoolTask, "MBTCP HANDLE", MB_TCP_POOL_STACK_SIZE, NULL , 2, NULL );
		
	 }	



		xTaskCreate( xMBTCPPort_HandlingTask, "MBTCP HANDLE", MB_TCP_CLIENT_STACK_SIZE, (void*)stTCPContext, 3, NULL );

    return TRUE;
}

//void
//vMBTCPPortClose(  )
//{
//    // Close all client sockets. 
//    if( xClientSocket != SOCKET_ERROR )
//    {
//        prvvMBPortReleaseClient(  );
//    }
//    // Close the listener socket.
//    if( xListenSocket != SOCKET_ERROR )
//    {
//        close( xListenSocket );
//    }
//}

void
vMBTCPPortDisable( stMBContext *stTCPContext )
{
    /* Close all client sockets. */
    if( stTCPContext->xClientSocket != SOCKET_ERROR )
    {
        prvvMBPortReleaseClient( stTCPContext );
    }
}

//BOOL
//xMBPortTCPPool( stMBContext *stTCPContext )
//{
//		int	clnt_index=0;
//	
//		BaseType_t task_ret=pdPASS;

//			if( stTCPContext->xClientSocket == INVALID_SOCKET )
//			{
//					if(prvbMBPortAcceptClient(stTCPContext)==TRUE)
//					{
//							PTCP_DEBUG_PRINT("New client accepted, Socket=%i\n",stTCPContext[clnt_index].xClientSocket);
//							taskYIELD();
//					}
//			}	
//    return TRUE;
//}


BOOL
xMBTCPPortGetRequest(stMBContext*	  stTCPContext, UCHAR ** ppucMBTCPFrame, USHORT * usTCPLength )
{
    *ppucMBTCPFrame = &stTCPContext->aucTCPBuf[0];
    *usTCPLength = stTCPContext->usTCPBufPos;

    /* Reset the buffer. */
    stTCPContext->usTCPBufPos = 0;
    stTCPContext->usTCPFrameBytesLeft = MB_TCP_FUNC;
    return TRUE;
}


uint8_t activeSocket=0;

BOOL
xMBTCPPortSendResponse( stMBContext*	  stTCPContext, const UCHAR * pucMBTCPFrame, USHORT usTCPLength )
{
    BOOL            bFrameSent = FALSE;
    BOOL            bAbort = FALSE;
    int             res;
    int             iBytesSent = 0;
    int             iTimeOut = MB_TCP_READ_TIMEOUT;
	
		if(stTCPContext->xClientSocket==INVALID_SOCKET)
		{
				return FALSE;
		}

    do
    {
        res = send(stTCPContext->xClientSocket, &pucMBTCPFrame[iBytesSent], usTCPLength - iBytesSent, 0 );
        switch ( res )
        {
        case -1:
            if( iTimeOut > 0 )
            {
                iTimeOut -= MB_TCP_READ_CYCLE;
                usleep( MB_TCP_READ_CYCLE );
            }
            else
            {
                bAbort = TRUE;
            }
            break;
        case 0:
            prvvMBPortReleaseClient(stTCPContext );
            bAbort = TRUE;
            break;
        default:
            iBytesSent += res;
            break;
        }
    }
    while( ( iBytesSent != usTCPLength ) && !bAbort );

    bFrameSent = iBytesSent == usTCPLength ? TRUE : FALSE;

		flag_mb_ready=1;
    return bFrameSent;
}

void
prvvMBPortReleaseClient( stMBContext	  *stTCPContext)
{
    ( void )recv( stTCPContext->xClientSocket, &stTCPContext->aucTCPBuf[0], MB_TCP_BUF_SIZE, 0 );

    ( void )close( stTCPContext->xClientSocket );
    stTCPContext->xClientSocket = INVALID_SOCKET;
}

BOOL
prvbMBPortAcceptClient( stMBContext	  *stTCPContext )
{
    SOCKET          xNewSocket;
    BOOL            bOkay;

    /* Check if we can handle a new connection. */

    if( stTCPContext->xClientSocket != INVALID_SOCKET )
    {
        PTCP_DEBUG_PRINT( "can't accept new client %i. all connections in use.\n",stTCPContext->xClientSocket );
        bOkay = FALSE;
    }
    else if( ( xNewSocket = accept( xListenSocket, NULL, NULL ) ) == INVALID_SOCKET )
    {
        bOkay = FALSE;
    }
    else
    {
        stTCPContext->xClientSocket = xNewSocket;
        stTCPContext->usTCPBufPos = 0;
        stTCPContext->usTCPFrameBytesLeft = MB_TCP_FUNC;
        bOkay = TRUE;
    }
    return bOkay;
}

extern stMBContext stTCPContext[MB_TCP_CLIENT_NUM];
void xMBTCPPort_PoolTask( void *pvParameters )
{

	uint8_t context_cnt=0;
	while(1)
	{
		for(context_cnt=0;context_cnt<MB_TCP_CLIENT_NUM;context_cnt++)
		{
				if(stTCPContext[context_cnt].xClientSocket==INVALID_SOCKET)
				{				
					if(prvbMBPortAcceptClient(&stTCPContext[context_cnt])==TRUE)
					{
							PTCP_DEBUG_PRINT("New client accepted, Socket=%i\n",stTCPContext[context_cnt].xClientSocket);
							continue;
					}
				}
		}
	}
}




void xMBTCPPort_HandlingTask(void *pvParameters )
{
		fd_set          fread;
		int             ret;
		USHORT          usLength;
	  struct timeval  tval;

    tval.tv_sec =  1;
    tval.tv_usec = 0;
	
		
	
		stMBContext *stTCPContext;
		stTCPContext=(stMBContext*)pvParameters;
	
		PTCP_DEBUG_PRINT("Task of socket %i started\r\n",stTCPContext->xClientSocket);
	
	  while(1)
    {		
				if(stTCPContext->xClientSocket!=INVALID_SOCKET)
				{				
						FD_ZERO( &fread );
						FD_SET( stTCPContext->xClientSocket, &fread );
						PTCP_DEBUG_PRINT("Wait for Socket %i data... \r\n",stTCPContext->xClientSocket);
						ret = select( stTCPContext->xClientSocket + 1, &fread, NULL, NULL, &tval );
						if(!ret)
						{
							PTCP_DEBUG_PRINT("Continue for Socket %i... \r\n",stTCPContext->xClientSocket);  
							continue;
						}
						else if(ret == SOCKET_ERROR )
						{
								PTCP_DEBUG_PRINT("Select return SOCKET_ERROR\r\n");
								PTCP_DEBUG_PRINT("Socket %i closed(select)\r\n",stTCPContext->xClientSocket);
								stTCPContext->xClientSocket = INVALID_SOCKET;
								continue;
							 // vTaskDelete(NULL);
						}
						else if( ret > 0 )
						{
								PTCP_DEBUG_PRINT("Socket %i has new data\r\n",stTCPContext->xClientSocket);
							
								if( FD_ISSET( stTCPContext->xClientSocket, &fread ) )
								{
										ret = recv( stTCPContext->xClientSocket, &stTCPContext->aucTCPBuf[stTCPContext->usTCPBufPos], stTCPContext->usTCPFrameBytesLeft,0 );
										if(( ret == SOCKET_ERROR ) || ( !ret ) )
										{
												
												close( stTCPContext->xClientSocket );
												PTCP_DEBUG_PRINT("Socket %i closed\r\n",stTCPContext->xClientSocket);
												stTCPContext->xClientSocket = INVALID_SOCKET;
												//vTaskDelete(NULL);
												continue;
										}
										
										PTCP_DEBUG_PRINT("Task of socket %i receive %i data\r\n",stTCPContext->xClientSocket, ret);
										
										stTCPContext->usTCPBufPos += ret;
										stTCPContext->usTCPFrameBytesLeft -= ret;
										
										if( stTCPContext->usTCPBufPos >= MB_TCP_FUNC )
										{
												/* Length is a byte count of Modbus PDU (function code + data) and the
												 * unit identifier. */
												usLength = stTCPContext->aucTCPBuf[MB_TCP_LEN] << 8U;
												usLength |= stTCPContext->aucTCPBuf[MB_TCP_LEN + 1];

												/* Is the frame already complete. */
												if( stTCPContext->usTCPBufPos < ( MB_TCP_UID + usLength ) )
												{
														stTCPContext->usTCPFrameBytesLeft = usLength + MB_TCP_UID - stTCPContext->usTCPBufPos;
												}
												/* The frame is complete. */
												else if( stTCPContext->usTCPBufPos == ( MB_TCP_UID + usLength ) )
												{
													
														if(xSemaphoreTake( xMB_FrameRec_Mutex, ( TickType_t ) 10 ) == pdTRUE )
														{
													
															
																( void )xMBPortEventPost(&stTCPContext->stEvent, EV_FRAME_RECEIVED );
																//Current_stTCPContext=stTCPContext;		
																PTCP_DEBUG_PRINT("Task of socket %i send command EV_FRAME_RECEIVED\r\n",stTCPContext->xClientSocket);
		//														while(flag_mb_ready==0)
		//														{
		//															__NOP();
		//														}

																flag_mb_ready=0;
																
																xSemaphoreGive( xMB_FrameRec_Mutex );
														}
														else
														{
															
																close( stTCPContext->xClientSocket );
																stTCPContext->xClientSocket = INVALID_SOCKET;
																//vTaskDelete(NULL);
																continue;
														}
												}
												/* This can not happend because we always calculate the number of bytes
												 * to receive. */
												else
												{
														assert( stTCPContext->usTCPBufPos <= ( MB_TCP_UID + usLength ) );
												}
										}
								}
						}
				}
				else
				{
					vTaskDelay(50);
				}
	}
}