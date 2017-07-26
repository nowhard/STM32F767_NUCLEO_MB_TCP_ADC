/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (c) 2013 China Beijing Armink <armink.ztl@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: mbrtu_m.c,v 1.60 2013/08/17 11:42:56 Armink Add Master Functions $
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbrtu.h"
#include "mbframe.h"

#include "mbcrc.h"
#include "mbport.h"

#include "stm32f7xx_hal.h"

extern UART_HandleTypeDef huart1;

#if MB_MASTER_RTU_ENABLED > 0
/* ----------------------- Defines ------------------------------------------*/
#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */


static volatile uint8_t  ucMasterRTUSndBuf[MB_PDU_SIZE_MAX];
static volatile uint8_t  ucMasterRTURcvBuf[MB_SER_PDU_SIZE_MAX];
static volatile USHORT usMasterSendPDULength;

static volatile UCHAR *pucMasterSndBufferCur;
static volatile USHORT usMasterSndBufferCount;

static volatile USHORT usMasterRcvBufferPos;
static volatile BOOL   xFrameIsBroadcast = FALSE;

static volatile eMBMasterTimerMode eMasterCurTimerMode;

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBMasterRTUInit(UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    ULONG           usTimerT35_50us;

    if( xMBMasterPortSerialInit( ucPort, ulBaudRate, 8, eParity ) != TRUE )
    {
        eStatus = MB_EPORTERR;
    }
    else
    {

        if( xMBMasterPortTimersInit( ( USHORT ) usTimerT35_50us ) != TRUE )
        {
            eStatus = MB_EPORTERR;
        }
    }
    return eStatus;
}

void
eMBMasterRTUStart( void )
{
		//HAL_UART_Receive_DMA(&huart1,(uint8_t*)ucMasterRTURcvBuf,MB_SER_PDU_SIZE_MAX);
}

void
eMBMasterRTUStop( void )
{
    vMBMasterPortTimersDisable(  );
}

eMBErrorCode
eMBMasterRTUReceive( UCHAR * pucRcvAddress, UCHAR ** pucFrame, USHORT * pusLength )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    assert_param( usMasterRcvBufferPos < MB_SER_PDU_SIZE_MAX );

    /* Length and CRC check */
    if( ( usMasterRcvBufferPos >= MB_SER_PDU_SIZE_MIN )
        && ( usMBCRC16( ( UCHAR * ) ucMasterRTURcvBuf, usMasterRcvBufferPos ) == 0 ) )
    {
        /* Save the address field. All frames are passed to the upper layed
         * and the decision if a frame is used is done there.
         */
        *pucRcvAddress = ucMasterRTURcvBuf[MB_SER_PDU_ADDR_OFF];

        /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
         * size of address field and CRC checksum.
         */
        *pusLength = ( USHORT )( usMasterRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC );

        /* Return the start of the Modbus PDU to the caller. */
        *pucFrame = ( UCHAR * ) & ucMasterRTURcvBuf[MB_SER_PDU_PDU_OFF];
    }
    else
    {
        eStatus = MB_EIO;
    }
		
    return eStatus;
}

eMBErrorCode
eMBMasterRTUSend( UCHAR ucSlaveAddress, const UCHAR * pucFrame, USHORT usLength )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          usCRC16;

    if ( ucSlaveAddress > MB_MASTER_TOTAL_SLAVE_NUM ) return MB_EINVAL;


		/* First byte before the Modbus-PDU is the slave address. */
		pucMasterSndBufferCur = ( UCHAR * ) pucFrame - 1;
		usMasterSndBufferCount = 1;

		/* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
		pucMasterSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
		usMasterSndBufferCount += usLength;

		/* Calculate CRC16 checksum for Modbus-Serial-Line-PDU. */
		usCRC16 = usMBCRC16( ( UCHAR * ) pucMasterSndBufferCur, usMasterSndBufferCount );
		ucMasterRTUSndBuf[usMasterSndBufferCount++] = ( UCHAR )( usCRC16 & 0xFF );
		ucMasterRTUSndBuf[usMasterSndBufferCount++] = ( UCHAR )( usCRC16 >> 8 );
	
		HAL_UART_Transmit_DMA(&huart1,(uint8_t*)ucMasterRTUSndBuf,usMasterSndBufferCount);
		//	HAL_UART_Transmit_IT(&huart1,(uint8_t*)ucMasterRTUSndBuf,usMasterSndBufferCount);
    return eStatus;
}



BOOL
xMBMasterRTUReceiveDone( void )
{
    BOOL            xTaskNeedSwitch = FALSE;
	
		
		usMasterRcvBufferPos=huart1.RxXferSize-huart1.hdmarx->Instance->NDTR;
		HAL_UART_DMAStop(&huart1);
		vMBMasterPortTimersDisable(  );
	
		if( usMasterRcvBufferPos >= MB_SER_PDU_SIZE_MAX )
		{
				vMBMasterSetErrorType(EV_ERROR_RECEIVE_DATA);
				xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
		}
		else
		{
				xMBMasterPortEventPost(EV_MASTER_FRAME_RECEIVED);
		}

    return xTaskNeedSwitch;
}


BOOL
xMBMasterRTUTransmitDone( void )
{
		xFrameIsBroadcast = ( ucMasterRTUSndBuf[MB_SER_PDU_ADDR_OFF] == MB_ADDRESS_BROADCAST ) ? TRUE : FALSE;
		HAL_UART_Receive_DMA(&huart1,(uint8_t*)ucMasterRTURcvBuf,MB_SER_PDU_SIZE_MAX);
		
	
	
		if ( xFrameIsBroadcast == TRUE )
		{
			vMBMasterPortTimersConvertDelayEnable( );
		}
		else
		{
			vMBMasterPortTimersRespondTimeoutEnable( );
		}	
}


BOOL
xMBMasterRTUTimerConvDelayExpired(void)
{
	BOOL xNeedPoll = FALSE;


	if ( xFrameIsBroadcast == FALSE ) 
	{
				vMBMasterSetErrorType(EV_ERROR_RESPOND_TIMEOUT);
				xNeedPoll = xMBMasterPortEventPost(EV_MASTER_ERROR_PROCESS);
	}

	HAL_UART_DMAStop(&huart1);
	vMBMasterPortTimersDisable( );
	/* If timer mode is convert delay, the master event then turns EV_MASTER_EXECUTE status. */
	if (eMasterCurTimerMode == MB_TMODE_CONVERT_DELAY) 
	{
		xNeedPoll = xMBMasterPortEventPost( EV_MASTER_EXECUTE );
	}

	return xNeedPoll;
}




/* Get Modbus Master send RTU's buffer address pointer.*/
void vMBMasterGetRTUSndBuf( UCHAR ** pucFrame )
{
	*pucFrame = ( UCHAR * ) ucMasterRTUSndBuf;
}

/* Get Modbus Master send PDU's buffer address pointer.*/
void vMBMasterGetPDUSndBuf( UCHAR ** pucFrame )
{
	*pucFrame = ( UCHAR * ) &ucMasterRTUSndBuf[MB_SER_PDU_PDU_OFF];
}

/* Set Modbus Master send PDU's buffer length.*/
void vMBMasterSetPDUSndLength( USHORT SendPDULength )
{
	usMasterSendPDULength = SendPDULength;
}

/* Get Modbus Master send PDU's buffer length.*/
USHORT usMBMasterGetPDUSndLength( void )
{
	return usMasterSendPDULength;
}

/* Set Modbus Master current timer mode.*/
void vMBMasterSetCurTimerMode( eMBMasterTimerMode eMBTimerMode )
{
	eMasterCurTimerMode = eMBTimerMode;
}

/* The master request is broadcast? */
BOOL xMBMasterRequestIsBroadcast( void ){
	return xFrameIsBroadcast;
}
#endif

