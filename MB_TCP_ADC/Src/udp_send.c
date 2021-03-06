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


extern ip4_addr_t ipaddr;

static ip_addr_t DestIPaddr;
static int socket_fd;
static struct sockaddr_in sa, ra;
static stPacket UDPPacket;

#define UDP_SEND_INTERPACKAGE_PERIOD	4// ms


void UDP_DestAddr_Reinit(void);


void UDP_Send_Init(void)
{
	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);

	if ( socket_fd < 0 )
	{
		return;
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ipaddr.addr;
	sa.sin_port = htons(SENDER_PORT_NUM);

	if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1)
	{
		close(socket_fd);
		return;
	}

	UDP_DestAddr_Reinit();
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

		uint16_t adcBufOffset=0;
		
		UDPPacket.id=0;
		UDPPacket.type=UDP_PACKET_TYPE_BASE;
		UDPPacket.timestamp=DCMI_ADC_GetLastTimestamp();
	
		UDP_DestAddr_Reinit();
	
		while(adcBufOffset<(bufSize*sizeof(float)))
		{
			memcpy(&UDPPacket.BasePacket.data,((uint8_t*)buf+adcBufOffset),UDP_BASE_DATA_SIZE);			
			sendto(socket_fd, &UDPPacket,UDP_BASE_PACKET_SIZE,0,(struct sockaddr*)&ra,sizeof(ra));	

			adcBufOffset+=UDP_BASE_DATA_SIZE;
			UDPPacket.id++;
			vTaskDelay(UDP_SEND_INTERPACKAGE_PERIOD);
		}

}

void UDP_SendPyroBuf(void)
{
	static uint16_t dataSize=0; 
	
	UDP_DestAddr_Reinit();
	
	if((ADC_PyroBuf_GetState()==ADC_PYRO_BUF_FILL_STOP) && ADC_PyroBuf_GetCurrentLength())//�������� ������ ��� ����������� ���������
	{
		UDPPacket.id=0;
		UDPPacket.type=UDP_PACKET_TYPE_ADC_PYRO;
		UDPPacket.timestamp=ADC_PyroBuf_GetTimestamp();
		while(dataSize=ADC_PyroBuf_Copy((void *)UDPPacket.ADCPyroPacket.data,UDP_PYRO_DATA_SIZE))
		{
			UDPPacket.ADCPyroPacket.size=dataSize;
			sendto(socket_fd, &UDPPacket,UDP_PYRO_MAX_PACKET_SIZE,0,(struct sockaddr*)&ra,sizeof(ra));	

			UDPPacket.id++;
			vTaskDelay(UDP_SEND_INTERPACKAGE_PERIOD);
		}
	}
}



