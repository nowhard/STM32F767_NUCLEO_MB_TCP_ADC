#ifndef UDP_SEND_H
#define UDP_SEND_H
#include "stm32f7xx_hal.h"

#pragma anon_unions

#define UDP_BASE_DATA_SIZE						1000
#define UDP_PYRO_DATA_SIZE						1024

#define UDP_PACKET_SEND_DELAY 				1000

#define SENDER_PORT_NUM								1001

typedef enum
{
	UDP_PACKET_TYPE_BASE=0,
	UDP_PACKET_TYPE_ADC_PYRO=1,
}enUDPPacketType;

#pragma pack(push,1)
typedef struct
{
	uint8_t 	data[UDP_BASE_DATA_SIZE];
}stBasePacket;

typedef struct
{
	uint16_t  	size;
	uint8_t			data[UDP_PYRO_DATA_SIZE];
}stADCPyroPacket;

typedef struct
{
	enUDPPacketType type;//тип-базовые данне или данные пиропатронов
	uint8_t 	id;				 //пор€дковый номер пакета в группе пакетов
	uint64_t 	timestamp; //штамп времени последнего значени€ последнего пакета	
	union
	{
		stBasePacket BasePacket;
		stADCPyroPacket ADCPyroPacket;
	};
}stPacket;

#pragma pack(pop)

#define UDP_BASE_PACKET_SIZE				(sizeof(enUDPPacketType)+sizeof(uint8_t)+sizeof(uint64_t)+UDP_BASE_DATA_SIZE)		
#define UDP_PYRO_MAX_PACKET_SIZE		(sizeof(enUDPPacketType)+sizeof(uint8_t)+sizeof(uint64_t)+sizeof(stADCPyroPacket))		

void UDP_Send_Init(void);


#endif
