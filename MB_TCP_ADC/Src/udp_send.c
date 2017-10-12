/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "main.h"
#include "memp.h"
#include "netif.h"
#include <stdio.h>
#include <string.h>
#include "socket.h"
#include "udp_send.h"



#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "data_converter.h"
#include "adc_dcmi.h"
#include "adc_pyro_buf.h"
#include "spi_adc.h"
#include "cfg_info.h"



ip_addr_t DestIPaddr;
extern ip4_addr_t ipaddr;

extern SemaphoreHandle_t xAdcBuf_Send_Semaphore;
extern enADCPyroBufState ADCPyroBufState;
extern uint64_t ADC_Pyro_Timestamp;

float ADC_resultBuf[ADC_DCMI_RESULT_BUF_LEN];

int socket_fd;
struct sockaddr_in sa, ra;
stPacket UDPPacket;

#define UDP_SEND_INTERPACKAGE_PERIOD	1// ms

#define UDP_SEND_TASK_STACK_SIZE	1024
#define UDP_SEND_TASK_PRIO				3
void UDP_Send_Task( void *pvParameters );

void UDP_SendBaseBuf(float *buf, uint16_t bufSize);
void UDP_SendPyroBuf(void);
void UDP_DestAddr_Reinit(void);


uint8_t socket_err=0;
void UDP_Send_Init(void)
{
	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);

	if ( socket_fd < 0 )
	{
		socket_err=1;
		return;
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ipaddr.addr;
	sa.sin_port = htons(SENDER_PORT_NUM);

	if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1)
	{
		close(socket_fd);
		socket_err=1;
		return;
	}

	UDP_DestAddr_Reinit();

  xTaskCreate( UDP_Send_Task, "UDP Task", UDP_SEND_TASK_STACK_SIZE, NULL, UDP_SEND_TASK_PRIO, NULL );
}

void UDP_DestAddr_Reinit(void)
{
		IP4_ADDR( &DestIPaddr, configInfo.IPAdress_Server.ip_addr_0, configInfo.IPAdress_Server.ip_addr_1, configInfo.IPAdress_Server.ip_addr_2, configInfo.IPAdress_Server.ip_addr_3 );
		memset(&ra, 0, sizeof(struct sockaddr_in));
		ra.sin_family = AF_INET;
		ra.sin_addr.s_addr = DestIPaddr.addr;
		ra.sin_port = htons(configInfo.IPAdress_Server.port);
}

void UDP_SendBaseBuf(float *buf, uint16_t bufSize)
{
		err_t err;
		uint16_t adc_buf_offset=0;
		
		UDPPacket.id=0;
		UDPPacket.type=UDP_PACKET_TYPE_BASE;
		UDPPacket.timestamp=DCMI_ADC_GetLastTimestamp();
	
		UDP_DestAddr_Reinit();
	
		while(adc_buf_offset<(bufSize*sizeof(float)))
		{
			memcpy(&UDPPacket.BasePacket.data,((uint8_t*)buf+adc_buf_offset),UDP_BASE_DATA_SIZE);			
			sendto(socket_fd, &UDPPacket,UDP_BASE_PACKET_SIZE,0,(struct sockaddr*)&ra,sizeof(ra));			
			adc_buf_offset+=UDP_BASE_DATA_SIZE;
			UDPPacket.id++;
			vTaskDelay(UDP_SEND_INTERPACKAGE_PERIOD);
		}

}

void UDP_SendPyroBuf(void)
{
	static uint16_t dataSize=0;
	
	UDP_DestAddr_Reinit();
	
	if((ADCPyroBufState==ADC_PYRO_BUF_FILL_STOP) && ADC_PyroBuf_GetCurrentLength())//передача данных ацп пиропатрона закончена
	{
		UDPPacket.id=0;
		UDPPacket.type=UDP_PACKET_TYPE_ADC_PYRO;
		UDPPacket.timestamp=ADC_Pyro_Timestamp;
		while(dataSize=ADC_PyroBuf_Copy((void *)UDPPacket.ADCPyroPacket.data,UDP_PYRO_DATA_SIZE))
		{
			UDPPacket.ADCPyroPacket.size=dataSize;
			sendto(socket_fd, &UDPPacket,UDP_PYRO_MAX_PACKET_SIZE,0,(struct sockaddr*)&ra,sizeof(ra));				
			UDPPacket.id++;
			vTaskDelay(UDP_SEND_INTERPACKAGE_PERIOD);
		}
	}
}


void UDP_Send_Task( void *pvParameters )
{
	static uint16_t resultBufLen=0;
	while(1)
	{
		xSemaphoreTake( xAdcBuf_Send_Semaphore, portMAX_DELAY );
		
		ADC_ConvertDCMIAndAssembleUDPBuf(ADC_resultBuf, &resultBufLen);
		UDP_SendBaseBuf(ADC_resultBuf,resultBufLen);
		UDP_SendPyroBuf();
	}
}


