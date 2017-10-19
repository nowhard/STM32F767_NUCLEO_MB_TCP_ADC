#ifndef TCP_SEND_H
#define TCP_SEND_H
#include "stm32f7xx_hal.h"
#include "port.h"

#pragma anon_unions

#define PACKET_BUF_SIZE				32000
//#define SENDER_PORT_NUM				1001

#define TCP_ADC_START_FRAME_MAGIC	0x55AA55AA;

typedef enum
{
	PACKET_TYPE_BASE=0,
	PACKET_TYPE_ADC_PYRO=1,
}enPacketType;

#pragma pack(push,1)

typedef struct
{
	uint32_t 			startOfFrame;
	enPacketType 	type;//тип-базовые данне или данные пиропатронов
	uint64_t 			timestamp; //штамп времени последнего значения последнего пакета	
	uint16_t  		size;
	uint8_t				data[PACKET_BUF_SIZE];
}stPacket;

#pragma pack(pop)

#define TCP_PACKET_HEADER_SIZE	(sizeof(stPacket)-PACKET_BUF_SIZE)
uint8_t TCP_ADC_Server_Init( uint16_t usTCPPort );
BOOL TCP_ADC_Send_BaseBuf(stPacket *TCPPacket);
BOOL TCP_ADC_Send_PyroBuf(stPacket *TCPPacket);
#endif
