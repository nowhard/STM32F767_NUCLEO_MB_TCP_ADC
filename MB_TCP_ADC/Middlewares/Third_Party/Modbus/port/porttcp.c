#include <stdio.h>
#include <socket.h>
#include <string.h>

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"





/* ----------------------- Static variables ---------------------------------*/
SOCKET          xListenSocket;
//SOCKET          xClientSocket = INVALID_SOCKET;
//SOCKET          xClientSocket_2 = INVALID_SOCKET;
static fd_set   allset;

//static UCHAR    aucTCPBuf[MB_TCP_BUF_SIZE];
//static USHORT   usTCPBufPos;
//static USHORT   usTCPFrameBytesLeft;

//typedef struct
//{
//	SOCKET xClientSocket;
//	UCHAR    aucTCPBuf[MB_TCP_BUF_SIZE];
//	USHORT   usTCPBufPos;
//	USHORT   usTCPFrameBytesLeft;	
//}stMB_TCPClient;

stMB_TCPClient *Current_MB_TCPClient;

stMB_TCPClient MB_TCPClient_1={INVALID_SOCKET,{0},0,0};
stMB_TCPClient MB_TCPClient_2={INVALID_SOCKET,{0},0,0};

/* ----------------------- External functions -------------------------------*/
//CHAR           *WsaError2String( int dwError );

/* ----------------------- Static functions ---------------------------------*/
BOOL            prvMBTCPPortAddressToString( SOCKET xSocket, CHAR * szAddr, USHORT usBufSize );
CHAR           *prvMBTCPPortFrameToString( UCHAR * pucFrame, USHORT usFrameLen );
static BOOL     prvbMBPortAcceptClient( stMB_TCPClient *MB_TCPClient );
static void     prvvMBPortReleaseClient( stMB_TCPClient *MB_TCPClient);
static void 		usleep(uint32_t time);


/* ----------------------- Begin implementation -----------------------------*/

static void 		usleep(uint32_t time)
{
		while(time)
		{
				time--;
		}
}


BOOL
xMBTCPPortInit( USHORT usTCPPort )
{
    USHORT          usPort;
    struct sockaddr_in serveraddr;

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
        fprintf( stderr, "Create socket failed.\r\n" );
        return FALSE;
    }
    else if( bind( xListenSocket, ( struct sockaddr * )&serveraddr, sizeof( serveraddr ) ) == -1 )
    {
        fprintf( stderr, "Bind socket failed.\r\n" );
        return FALSE;
    }
    else if( listen( xListenSocket, 5 ) == -1 )
    {
        fprintf( stderr, "Listen socket failed.\r\n" );
        return FALSE;
    }
    FD_ZERO( &allset );
    FD_SET( xListenSocket, &allset );
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
vMBTCPPortDisable( void )
{
    /* Close all client sockets. */
    if( Current_MB_TCPClient->xClientSocket != SOCKET_ERROR )
    {
        prvvMBPortReleaseClient( Current_MB_TCPClient );
    }
}

/*! \ingroup port_win32tcp
 *
 * \brief Pool the listening socket and currently connected Modbus TCP clients
 *   for new events.
 * \internal
 *
 * This function checks if new clients want to connect or if already connected 
 * clients are sending requests. If a new client is connected and there are 
 * still client slots left (The current implementation supports only one)
 * then the connection is accepted and an event object for the new client
 * socket is activated (See prvbMBPortAcceptClient() ).
 * Events for already existing clients in \c FD_READ and \c FD_CLOSE. In case of
 * an \c FD_CLOSE the client connection is released (See prvvMBPortReleaseClient() ).
 * In case of an \c FD_READ command the existing data is read from the client
 * and if a complete frame has been received the Modbus Stack is notified.
 *
 * \return FALSE in case of an internal I/O error. For example if the internal
 *   event objects are in an invalid state. Note that this does not include any 
 *   client errors. In all other cases returns TRUE.
 */
BOOL
xMBPortTCPPool( stMB_TCPClient *MB_TCPClient )
{
    int             n;
    fd_set          fread;
    struct timeval  tval;

    tval.tv_sec = 0;
    tval.tv_usec = 5000;
    int             ret;
    USHORT          usLength;

	
			Current_MB_TCPClient=MB_TCPClient;
	
    if( MB_TCPClient->xClientSocket == INVALID_SOCKET )
    {
        /* Accept to client */
        select( xListenSocket + 1, &allset, NULL, NULL, NULL );
        if( FD_ISSET( xListenSocket, &allset ) )
        {
            ( void )prvbMBPortAcceptClient( MB_TCPClient );
        }
    }
		
		
		
    while( TRUE )
    {
        FD_ZERO( &fread );
        FD_SET( MB_TCPClient->xClientSocket, &fread );
			
			
        if( ( ( ret = select( MB_TCPClient->xClientSocket + 1, &fread, NULL, NULL, &tval ) ) == SOCKET_ERROR )
            || !ret )
        {
            continue;
        }
				
				
        if( ret > 0 )
        {
            if( FD_ISSET( MB_TCPClient->xClientSocket, &fread ) )
            {
                if( ( ( ret =
                        recv( MB_TCPClient->xClientSocket, &MB_TCPClient->aucTCPBuf[MB_TCPClient->usTCPBufPos], MB_TCPClient->usTCPFrameBytesLeft,
                              0 ) ) == SOCKET_ERROR ) || ( !ret ) )
                {
                    close( MB_TCPClient->xClientSocket );
                    MB_TCPClient->xClientSocket = INVALID_SOCKET;
                    return TRUE;
                }
								
                MB_TCPClient->usTCPBufPos += ret;
                MB_TCPClient->usTCPFrameBytesLeft -= ret;
								
                if( MB_TCPClient->usTCPBufPos >= MB_TCP_FUNC )
                {
                    /* Length is a byte count of Modbus PDU (function code + data) and the
                     * unit identifier. */
                    usLength = MB_TCPClient->aucTCPBuf[MB_TCP_LEN] << 8U;
                    usLength |= MB_TCPClient->aucTCPBuf[MB_TCP_LEN + 1];

                    /* Is the frame already complete. */
                    if( MB_TCPClient->usTCPBufPos < ( MB_TCP_UID + usLength ) )
                    {
                        MB_TCPClient->usTCPFrameBytesLeft = usLength + MB_TCP_UID - MB_TCPClient->usTCPBufPos;
                    }
                    /* The frame is complete. */
                    else if( MB_TCPClient->usTCPBufPos == ( MB_TCP_UID + usLength ) )
                    {
												
                        //( void )xMBPortEventPost( EV_FRAME_RECEIVED );
                        return TRUE;
                    }
                    /* This can not happend because we always calculate the number of bytes
                     * to receive. */
                    else
                    {
                        assert( MB_TCPClient->usTCPBufPos <= ( MB_TCP_UID + usLength ) );
                    }
                }
            }
        }
    }
    return TRUE;
}

/*!
 * \ingroup port_win32tcp
 * \brief Receives parts of a Modbus TCP frame and if complete notifies
 *    the protocol stack.
 * \internal 
 *
 * This function reads a complete Modbus TCP frame from the protocol stack.
 * It starts by reading the header with an initial request size for
 * usTCPFrameBytesLeft = MB_TCP_FUNC. If the header is complete the 
 * number of bytes left can be calculated from it (See Length in MBAP header).
 * Further read calls are issued until the frame is complete.
 *
 * \return \c TRUE if part of a Modbus TCP frame could be processed. In case
 *   of a communication error the function returns \c FALSE.
 */

BOOL
xMBTCPPortGetRequest( UCHAR ** ppucMBTCPFrame, USHORT * usTCPLength )
{
    *ppucMBTCPFrame = &Current_MB_TCPClient->aucTCPBuf[0];
    *usTCPLength = Current_MB_TCPClient->usTCPBufPos;

    /* Reset the buffer. */
    Current_MB_TCPClient->usTCPBufPos = 0;
    Current_MB_TCPClient->usTCPFrameBytesLeft = MB_TCP_FUNC;
    return TRUE;
}


uint8_t activeSocket=0;

BOOL
xMBTCPPortSendResponse( const UCHAR * pucMBTCPFrame, USHORT usTCPLength )
{
    BOOL            bFrameSent = FALSE;
    BOOL            bAbort = FALSE;
    int             res;
    int             iBytesSent = 0;
    int             iTimeOut = MB_TCP_READ_TIMEOUT;

    do
    {
        res = send( Current_MB_TCPClient->xClientSocket, &pucMBTCPFrame[iBytesSent], usTCPLength - iBytesSent, 0 );
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
            prvvMBPortReleaseClient(Current_MB_TCPClient );
            bAbort = TRUE;
            break;
        default:
            iBytesSent += res;
            break;
        }
    }
    while( ( iBytesSent != usTCPLength ) && !bAbort );

    bFrameSent = iBytesSent == usTCPLength ? TRUE : FALSE;


		activeSocket^=0x1;	

		
    return bFrameSent;
}

void
prvvMBPortReleaseClient( stMB_TCPClient *MB_TCPClient )
{
    ( void )recv( MB_TCPClient->xClientSocket, &MB_TCPClient->aucTCPBuf[0], MB_TCP_BUF_SIZE, 0 );

    ( void )close( MB_TCPClient->xClientSocket );
    MB_TCPClient->xClientSocket = INVALID_SOCKET;
}

BOOL
prvbMBPortAcceptClient( stMB_TCPClient *MB_TCPClient )
{
    SOCKET          xNewSocket;
    BOOL            bOkay;

    /* Check if we can handle a new connection. */

    if( MB_TCPClient->xClientSocket != INVALID_SOCKET )
    {
        fprintf( stderr, "can't accept new client. all connections in use.\n" );
        bOkay = FALSE;
    }
    else if( ( xNewSocket = accept( xListenSocket, NULL, NULL ) ) == INVALID_SOCKET )
    {
        bOkay = FALSE;
    }
    else
    {
        MB_TCPClient->xClientSocket = xNewSocket;
        MB_TCPClient->usTCPBufPos = 0;
        MB_TCPClient->usTCPFrameBytesLeft = MB_TCP_FUNC;
        bOkay = TRUE;
    }
    return bOkay;
}