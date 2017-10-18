/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
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

#if LWIP_TCP

typedef enum
{
		TCP_CLIENT_DISCONNECTED=0,
		TCP_CLIENT_CONNECTED,
	
}enTCPClientState;

static enTCPClientState TCPClientState=TCP_CLIENT_DISCONNECTED;

struct tcp_pcb *clientPcb;
static err_t cbErr=ERR_OK;

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
void TCPClient_Task( void *pvParameters );

/* Private function prototypes -----------------------------------------------*/
static err_t TCPClient_Connect(void);
static err_t TCPClient_ConnectionClose(struct tcp_pcb *tpcb);
static err_t TCPClient_Sent_CB(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t TCPClient_SendBuf(struct tcp_pcb *tpcb, uint8_t *buf, uint16_t bufSize);
static err_t TCPClient_Connected_CB(void *arg, struct tcp_pcb *tpcb, err_t err);
/* Private functions ---------------------------------------------------------*/

void TCPClient_Init(void)
{
	xTCPClient_Connected_Semaphore=xSemaphoreCreateBinary();
	xTCPClient_Sent_Semaphore=xSemaphoreCreateBinary();
	xTaskCreate( TCPClient_Task, "TCP Client Task", TCP_CLIENT_TASK_STACK_SIZE, NULL, TCP_CLIENT_TASK_PRIO, NULL );
}

static err_t TCPClient_Connect(void)
{
	err_t err = ERR_OK;
  ip_addr_t DestIPaddr;
  clientPcb = tcp_new();
  if (clientPcb != NULL)
  {
    IP4_ADDR( &DestIPaddr, configInfo.IPAdress_Server.ip_addr_0, configInfo.IPAdress_Server.ip_addr_1, configInfo.IPAdress_Server.ip_addr_2, configInfo.IPAdress_Server.ip_addr_3 );
    err=tcp_connect(clientPcb,&DestIPaddr,configInfo.IPAdress_Server.port,TCPClient_Connected_CB);
  }
	
	return err;
}

static err_t TCPClient_ConnectionClose(struct tcp_pcb *tpcb)
{
	err_t err = ERR_OK;
  tcp_sent(tpcb, NULL);
  err=tcp_close(tpcb);
	TCPClientState=TCP_CLIENT_DISCONNECTED;
	
	return err;
}

/*****************************************************************************
												TCP connected callback
******************************************************************************/
static err_t TCPClient_Connected_CB(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  if (err == ERR_OK)
  {
       tcp_sent(tpcb, TCPClient_Sent_CB);
			 TCPClientState=TCP_CLIENT_CONNECTED;
  }
  else
  {
	  	 TCPClient_ConnectionClose(tpcb);
			 TCPClientState=TCP_CLIENT_DISCONNECTED;
  }
	
	cbErr=err;
	xSemaphoreGive(xTCPClient_Connected_Semaphore);
  return err;
}
/*****************************************************************************
												TCP sent callback
******************************************************************************/
static err_t TCPClient_Sent_CB(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  LWIP_UNUSED_ARG(len);
	xSemaphoreGive(xTCPClient_Sent_Semaphore);

  return ERR_OK;
}

#define TCP_CLIENT_CONNECTED_TIMEOUT	100
#define TCP_CLIENT_SENT_TIMEOUT				100
static err_t TCPClient_SendBuf(struct tcp_pcb *tpcb, uint8_t *buf, uint16_t bufSize)
{
	err_t err = ERR_OK;

		if(TCPClientState!=TCP_CLIENT_CONNECTED)
		{
					err=TCPClient_Connect();
					
					if(err!=ERR_OK)
					{
							return err;
					}
					
					if(xSemaphoreTake( xTCPClient_Connected_Semaphore, TCP_CLIENT_CONNECTED_TIMEOUT ))
					{
							if(cbErr!=ERR_OK)
							{
									return cbErr;
							}	
					}
					else
					{
						return ERR_CONN;	
					}
					TCPClientState=TCP_CLIENT_CONNECTED;
		}
	
	err = tcp_write(tpcb, (uint8_t*)buf,bufSize,0);
	
	if(err!=ERR_OK)
	{
			//TCPClientState=TCP_CLIENT_DISCONNECTED;
			return err;
	}
	
	err=tcp_output(tpcb);
	
	if(err!=ERR_OK)
	{
			return err;
	}
	
	if(xSemaphoreTake( xTCPClient_Sent_Semaphore, TCP_CLIENT_SENT_TIMEOUT ))
	{

	}
	else
	{
		return ERR_CONN;	
	}
	
	return err;
}


void TCPClient_Task( void *pvParameters )
{
	uint16_t resultBufLen=0;
	err_t err = ERR_OK;
	while(1)
	{
		xSemaphoreTake( xAdcBuf_Send_Semaphore, portMAX_DELAY );
		
		//Заполним структуру пакета данными
		ADC_ConvertDCMIAndAssembleUDPBuf(TCPPacket.data, &resultBufLen);		
		TCPPacket.size=resultBufLen*sizeof(float);
		TCPPacket.type=PACKET_TYPE_BASE;
		TCPPacket.timestamp=DCMI_ADC_GetLastTimestamp();
		
		err=TCPClient_SendBuf(clientPcb,(uint8_t *)&TCPPacket,TCP_PACKET_HEADER_SIZE+resultBufLen*sizeof(float));
	}
}

#endif /* LWIP_TCP */


