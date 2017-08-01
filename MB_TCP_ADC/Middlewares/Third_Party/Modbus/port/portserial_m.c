/*
 * FreeModbus Libary: RT-Thread Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#include "port.h"
#include "main.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mbconfig.h"

extern UART_HandleTypeDef huart1;

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Static variables ---------------------------------*/


/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
#define TIMEOUT_BIT_NUM		32

BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
        eMBParity eParity)
{

  huart1.Instance = USART1;	
  huart1.Init.BaudRate = ulBaudRate;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = eParity;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
	
	SET_BIT(huart1.Instance->CR1, USART_CR1_RTOIE);//timeout 3.5
	SET_BIT(huart1.Instance->CR2, USART_CR2_RTOEN);
	WRITE_REG(huart1.Instance->RTOR,TIMEOUT_BIT_NUM);
	
	__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
	

	
  return TRUE;
		
}



BOOL UART_IRQ_Handler(USART_TypeDef * usart) 
{
	if (usart == huart1.Instance) 
	{
//		if((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE) != RESET)) 
//		{
//			//pxMBMasterFrameCBByteReceived();
//			return TRUE;
//		}
//		
//		if((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) != RESET) &&(__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE) != RESET)) 
//		{
//			//pxMBMasterFrameCBTransmitterEmpty();
//			return TRUE;
//		}
		
		if((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RTOF) != RESET)) 
		{
			SET_BIT(huart1.Instance->ICR, USART_ICR_RTOCF);//clear timeout flag
			pxMBMasterFrameCBByteReceived();
			return TRUE;
		}
	}
	return FALSE;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)
	{
			pxMBMasterFrameCBTransmitterEmpty();
	}
}

#endif
