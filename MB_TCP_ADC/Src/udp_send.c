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


extern uint8_t *ADC_buf_pnt;
extern __IO uint8_t DCMIAdcRxBuff[ADC_BUF_LEN];

extern uint16_t *currentSPI3_ADC_Buf;
extern uint16_t *currentSPI6_ADC_Buf;


extern SemaphoreHandle_t xAdcBuf_Send_Semaphore;
extern enADCPyroBufState ADCPyroBufState;
extern uint64_t ADC_Pyro_Timestamp;



void UDP_Send_Task( void *pvParameters );
void udp_client_send_base_buf(float *buf, uint16_t bufSize);
void udp_client_send_pyro_buf(void);

stPacket UDPPacket;

float ADC_resultBuf[ADC_RESULT_BUF_LEN];


int socket_fd;
struct sockaddr_in sa, ra;

uint8_t socket_err=0;
void udp_client_init(void)
{
	ADC_buf_pnt=DCMIAdcRxBuff;

  IP4_ADDR( &DestIPaddr, configInfo.IPAdress_Server.ip_addr_0, configInfo.IPAdress_Server.ip_addr_1, configInfo.IPAdress_Server.ip_addr_2, configInfo.IPAdress_Server.ip_addr_3 );

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

	memset(&ra, 0, sizeof(struct sockaddr_in));
	ra.sin_family = AF_INET;
	ra.sin_addr.s_addr = DestIPaddr.addr;
	ra.sin_port = htons(configInfo.IPAdress_Server.port);

  xTaskCreate( UDP_Send_Task, "UDP Task", 1024, NULL, 3, NULL );
}


void udp_client_send_base_buf(float *buf, uint16_t bufSize)
{
		err_t err;
		uint16_t adc_buf_offset=0;
		
		UDPPacket.id=0;
		UDPPacket.type=UDP_PACKET_TYPE_BASE;
		UDPPacket.timestamp=DCMI_ADC_GetLastTimestamp();
	
		ra.sin_addr.s_addr = DestIPaddr.addr;
		ra.sin_port = htons(configInfo.IPAdress_Server.port);
	
		while(adc_buf_offset<(bufSize*sizeof(float)))
		{
			memcpy(&UDPPacket.BasePacket.data,((uint8_t*)buf+adc_buf_offset),UDP_BASE_DATA_SIZE);			
			sendto(socket_fd, &UDPPacket,UDP_BASE_PACKET_SIZE,0,(struct sockaddr*)&ra,sizeof(ra));			
			adc_buf_offset+=UDP_BASE_DATA_SIZE;
			UDPPacket.id++;
		}

}

void udp_client_send_pyro_buf(void)
{
	uint16_t data_size=0;
	
	ra.sin_addr.s_addr = DestIPaddr.addr;
	ra.sin_port = htons(configInfo.IPAdress_Server.port);
	
	if((ADCPyroBufState==ADC_PYRO_BUF_FILL_STOP)&&ADC_PyroBuf_GetCurrentLength())//передача данных ацп пиропатрона закончена
	{
		UDPPacket.id=0;
		UDPPacket.type=UDP_PACKET_TYPE_ADC_PYRO;
		UDPPacket.timestamp=ADC_Pyro_Timestamp;
		while(data_size=ADC_PyroBuf_Copy((void *)UDPPacket.ADCPyroPacket.data,UDP_PYRO_DATA_SIZE))
		{
			UDPPacket.ADCPyroPacket.size=data_size;
			sendto(socket_fd, &UDPPacket,UDP_PYRO_MAX_PACKET_SIZE,0,(struct sockaddr*)&ra,sizeof(ra));				
			UDPPacket.id++;
		}
		data_size=0;
	}
}

void UDP_Send_Task( void *pvParameters )
{
	uint16_t result_buf_len=0;
	while(1)
	{
		xSemaphoreTake( xAdcBuf_Send_Semaphore, portMAX_DELAY );
		
		ADC_ConvertBuf(ADC_buf_pnt,(ADC_BUF_LEN>>1),currentSPI6_ADC_Buf,currentSPI3_ADC_Buf,(SPI_ADC_BUF_LEN>>1),ADC_resultBuf, &result_buf_len);
		udp_client_send_base_buf(ADC_resultBuf,result_buf_len);
		udp_client_send_pyro_buf();
	}
}


