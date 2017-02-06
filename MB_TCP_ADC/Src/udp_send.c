/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "main.h"
#include "memp.h"
#include "netif.h"
#include <stdio.h>
#include <string.h>



#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "data_converter.h"
#include "adc_dcmi.h"
#include "spi_adc.h"
#include "cfg_info.h"


#define UDP_ADC_PACKET_SIZE	1000//1024
#define UDP_PACKET_SEND_DELAY 1000



struct udp_pcb *client_pcb;
struct pbuf *pb;
ip_addr_t DestIPaddr;

//extern uint16_t *ADC_buf_pnt;

extern __IO uint32_t uwTick;
uint32_t Tick1,Tick2,Tick3;

extern uint8_t *ADC_buf_pnt;
extern __IO uint8_t DCMIAdcRxBuff[ADC_BUF_LEN];

extern uint16_t *currentSPI3_ADC_Buf;
extern uint16_t *currentSPI6_ADC_Buf;



extern SemaphoreHandle_t xAdcBuf_Send_Semaphore;
extern SemaphoreHandle_t xNetMutex;

void UDP_Send_Task( void *pvParameters );
/*inline*/ void delay(uint32_t time);

#pragma pack(push,1)
typedef struct
{
	uint8_t 	id;
	uint64_t 	timestamp;
	uint8_t 	data[UDP_ADC_PACKET_SIZE];
}stPacket;
#pragma pack(pop)

stPacket UDPPacket;

//extern sConfigInfo configInfo;

//#define ADC_RESULT_BUF_LEN (ADC_BUF_LEN/4)

/*uint16_t*/float ADC_resultBuf[ADC_RESULT_BUF_LEN];

void udp_client_init(void)
{
	ADC_buf_pnt=DCMIAdcRxBuff;
  client_pcb = udp_new();
  //IP4_ADDR( &DestIPaddr, configInfo.IPAdress_Server.ip_addr_0, configInfo.IPAdress_Server.ip_addr_1, configInfo.IPAdress_Server.ip_addr_2, configInfo.IPAdress_Server.ip_addr_3 );

  IP4_ADDR( &DestIPaddr, SERVER_IP_ADDR0, SERVER_IP_ADDR1, SERVER_IP_ADDR2, SERVER_IP_ADDR3 );

  pb = pbuf_alloc(PBUF_TRANSPORT,sizeof(UDPPacket), PBUF_REF);
  pb->len = pb->tot_len = sizeof(UDPPacket);
  pb->payload = (uint8_t*)&UDPPacket;


  xTaskCreate( UDP_Send_Task, "UDP Task", 512, NULL, 2, NULL );
}

/*inline*/ void delay(uint32_t time)
{
	while (time)
		time--;
}

void udp_client_send_buf(float *buf, uint16_t bufSize)
{
  err_t err;
  uint16_t adc_buf_offset=0;
	
  UDPPacket.id=0;
  UDPPacket.timestamp=DCMI_ADC_GetLastTimestamp();
  if (client_pcb != NULL)
  {
		while(adc_buf_offset<=(bufSize*sizeof(float)))
		{
			memcpy(&UDPPacket.data,((uint8_t*)buf+adc_buf_offset),UDP_ADC_PACKET_SIZE);
			err=udp_sendto(client_pcb, pb,&DestIPaddr,configInfo.IPAdress_Server.port);
//		  if( xSemaphoreTake( xNetMutex, portMAX_DELAY ) == pdTRUE )
//			{
//							//err=udp_sendto(client_pcb, pb,&DestIPaddr,SERVER_PORT);
//							xSemaphoreGive( xNetMutex );
//			}
			adc_buf_offset+=UDP_ADC_PACKET_SIZE;
			UDPPacket.id++;
			delay(UDP_PACKET_SEND_DELAY);
			taskYIELD();
		}
  }
}


void UDP_Send_Task( void *pvParameters )
{
	uint16_t result_buf_len=0;
	while(1)
	{
		//xSemaphoreTake( xAdcBuf_Send_Semaphore, portMAX_DELAY );
		vTaskDelay(10);
		//Tick1=uwTick;
		ADC_ConvertBuf(ADC_buf_pnt,(ADC_BUF_LEN>>1),currentSPI3_ADC_Buf,currentSPI3_ADC_Buf,(SPI_ADC_BUF_LEN>>1),ADC_resultBuf, &result_buf_len);
		//Tick2=uwTick-Tick1;
		//Tick1=uwTick;
		udp_client_send_buf(ADC_resultBuf,result_buf_len);
		//Tick3=uwTick-Tick1;
//		ADC_GetLastVal();
	}
}



