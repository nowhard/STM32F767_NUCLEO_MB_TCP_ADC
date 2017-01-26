/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "main.h"
#include "memp.h"
#include "netif.h"
#include <stdio.h>
#include <string.h>
//#include "adc_dcmi.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "data_converter.h"
#include "cmsis_os.h"
#include "main.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"

extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi6;

uint8_t spi3_buf[3];
uint8_t spi6_buf[3];

extern osMessageQId ADC_SPI3_QueueHandle;
extern osMessageQId ADC_SPI6_QueueHandle;


#define SERVER_IP_ADDR0   192
#define SERVER_IP_ADDR1   168
#define SERVER_IP_ADDR2   109
#define SERVER_IP_ADDR3   140

#define SERVER_PORT		  1001

struct udp_pcb *client_pcb;
struct pbuf *pb;
ip_addr_t DestIPaddr;



void SPI_ADC_UDP_Send_Task( void *pvParameters );


void SPI_ADC_UDP_Init(void)
{


  client_pcb = udp_new();

  IP4_ADDR( &DestIPaddr, SERVER_IP_ADDR0, SERVER_IP_ADDR1, SERVER_IP_ADDR2, SERVER_IP_ADDR3 );

  pb = pbuf_alloc(PBUF_TRANSPORT,sizeof(UDPPacket), PBUF_REF);
  pb->len = pb->tot_len = sizeof(UDPPacket);
  pb->payload = (uint8_t*)&UDPPacket;

  xTaskCreate( SPI_ADC_UDP_Send_Task, "SPI ADC UDP Task", 768, NULL, 2, NULL );
}



void SPI_ADC_UDP_send_buf(void)
{
  err_t err;

  if (client_pcb != NULL)
  {

		memcpy(&UDPPacket.data,((uint8_t*)ADC_resultBuf+adc_buf_offset),UDP_ADC_PACKET_SIZE);

		err=udp_sendto(client_pcb, pb,&DestIPaddr,SERVER_PORT);

		delay(UDP_PACKET_SEND_DELAY);
  }
}


void SPI_ADC_UDP_Send_Task( void *pvParameters )
{
	while(1)
	{

		vTaskDelay(1);

		//ADC_ConvertBuf(ADC_buf_pnt,(ADC_BUF_LEN>>1),ADC_resultBuf);

		SPI_ADC_UDP_send_buf();

	}
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM9)
	{
		HAL_SPI_Receive_DMA(&hspi3, spi3_buf, 3);
		HAL_SPI_Receive_DMA(&hspi6, spi6_buf, 3);
	
		HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);
	} 
}


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance==SPI3)
	{
			osMessagePut( ADC_SPI3_QueueHandle, (uint32_t)&spi3_buf[1], osWaitForever );
	}
	else if(hspi->Instance==SPI6)
	{
			osMessagePut( ADC_SPI6_QueueHandle, (uint32_t)&spi6_buf[1], osWaitForever );
	}
}


