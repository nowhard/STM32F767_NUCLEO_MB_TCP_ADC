#ifndef MBCONTEXT_H
#define MBCONTEXT_H
#include "port.h"
#include "mbframe.h"
#include "mbproto.h"
#include <socket.h>

#define SOCKET 							int
#define INVALID_SOCKET			(-1)
#define SOCKET_ERROR				(-1)


#define MB_TCP_BUF_SIZE     ( 256 + 7 ) /* Must hold a complete Modbus TCP frame. */
typedef enum
{
    STATE_RX_INIT,              /*!< Receiver is in initial state. */
    STATE_RX_IDLE,              /*!< Receiver is in idle state. */
    STATE_RX_RCV,               /*!< Frame is beeing received. */
    STATE_RX_ERROR              /*!< If the frame is invalid. */
} eMBRcvState;

typedef enum
{
    STATE_TX_IDLE,              /*!< Transmitter is in idle state. */
    STATE_TX_XMIT               /*!< Transmitter is in transfer state. */
} eMBSndState;

typedef enum
{
    EV_READY            = 1<<0,         /*!< Startup finished. */
    EV_FRAME_RECEIVED   = 1<<1,         /*!< Frame received. */
    EV_EXECUTE          = 1<<2,         /*!< Execute function. */
    EV_FRAME_SENT       = 1<<3          /*!< Frame sent. */
} eMBEventType;

typedef enum
{
    STATE_ENABLED,
    STATE_DISABLED,
    STATE_NOT_INITIALIZED
} eMBState;

typedef struct
{
	eMBEventType eQueuedEvent;
	BOOL     xEventInQueue;
}stMBEvent;

    typedef enum
{
    MB_RTU,                     /*!< RTU transmission mode. */
    MB_ASCII,                   /*!< ASCII transmission mode. */
    MB_TCP                      /*!< TCP mode. */
} eMBMode;

//typedef struct
//{
//	SOCKET xClientSocket;
//	UCHAR    aucTCPBuf[MB_TCP_BUF_SIZE];
//	USHORT   usTCPBufPos;
//	USHORT   usTCPFrameBytesLeft;	
//} stMBTCPContext;

typedef struct
{
	UCHAR   		*ucMBFrame;
    UCHAR    		ucRcvAddress;
    UCHAR    		ucFunctionCode;
    USHORT   		usLength;
    eMBException 	eException;




    //typedef void( *pvMBFrameClose ) ( void );

//	BOOL( *pxMBFrameCBByteReceived ) ( stMBTCPContext *stTCPContext,stMBTimer *stTimer, stMBCommunication *stCommunication );
//	BOOL( *pxMBFrameCBTransmitterEmpty ) ( stMBTCPContext *stTCPContext, stMBCommunication *stCommunication, stMBEvent *stEvent );
//	BOOL( *pxMBPortCBTimerExpired ) ( stMBTCPContext *stTCPContext,stMBTimer *stTimer , stMBEvent *stEvent );

//-----------------TCP Context----------------
	SOCKET xClientSocket;
	UCHAR    aucTCPBuf[MB_TCP_BUF_SIZE];
	USHORT   usTCPBufPos;
	USHORT   usTCPFrameBytesLeft;	
//--------------------------------------------

	BOOL( *pxMBFrameCBReceiveFSMCur ) ( void );
	BOOL( *pxMBFrameCBTransmitFSMCur ) ( void );

	UCHAR    ucMBAddress;
	eMBMode  eMBCurrentMode;
	eMBState eState;

//	stMBCommunication stCommunication;
//	stMBTimer		  stTimer;
	stMBEvent		  stEvent;
//	stMBTCPContext	  stTCPContext;
	//stMBPort		  stPort;
}stMBContext;




#endif