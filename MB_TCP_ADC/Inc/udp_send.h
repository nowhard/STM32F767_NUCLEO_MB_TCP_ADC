#ifndef UDP_SEND_H
#define UDP_SEND_H
#include "stm32f7xx_hal.h"

#pragma anon_unions


#define UDP_ADC_PACKET_SIZE						1000
#define UDP_ADC_PYRO_MAX_PACKET_SIZE	1280
#define UDP_PACKET_SEND_DELAY 				1000

#define SENDER_PORT_NUM								1001

typedef enum
{
	UDP_PACKET_TYPE_BASE,
	UDP_PACKET_TYPE_ADC_PYRO,
}enUDPPacketType;

#pragma pack(push,1)
typedef struct
{
	uint8_t 	id;
	uint64_t 	timestamp;
	uint8_t 	data[UDP_ADC_PACKET_SIZE];
}stBasePacket;

typedef struct
{
	uint8_t 	id;
	uint64_t 	timestamp;
	uint16_t  size;
	float			data[UDP_ADC_PYRO_MAX_PACKET_SIZE];
}stADCPyroPacket;

typedef struct
{
	enUDPPacketType type;
	union
	{
		stBasePacket BasePacket;
		stADCPyroPacket ADCPyroPacket;
	};
}stPacket;


#pragma pack(pop)

void udp_client_init(void);


#endif
