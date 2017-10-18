#ifndef TCP_SEND_H
#define TCP_SEND_H
#include "stm32f7xx_hal.h"


#pragma anon_unions

#define PACKET_BUF_SIZE				8000
//#define SENDER_PORT_NUM				1001

typedef enum
{
	PACKET_TYPE_BASE=0,
	PACKET_TYPE_ADC_PYRO=1,
}enPacketType;

#pragma pack(push,1)

typedef struct
{
	enPacketType 	type;//тип-базовые данне или данные пиропатронов
	uint64_t 			timestamp; //штамп времени последнего значения последнего пакета	
	uint16_t  		size;
	float					data[PACKET_BUF_SIZE];
}stPacket;

#pragma pack(pop)

#define TCP_PACKET_HEADER_SIZE	(sizeof(stPacket)-(PACKET_BUF_SIZE*sizeof(float)))

uint8_t TCP_ADC_Server_Init( uint16_t usTCPPort );
#endif
