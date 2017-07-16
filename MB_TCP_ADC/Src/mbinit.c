#include "mbinit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"
#include "mbtcp.h"
#include "cfg_info.h"

#define MB_TCP_TASK_STACK_SIZE	2048
#define MB_TCP_SAVE_SETTINGS_TASK_STACK_SIZE	256

#define MB_TCP_CLIENT_NUM					2

static uint8_t Vendor[] = "GEOS";

stMBContext stTCPContext[MB_TCP_CLIENT_NUM];

void MB_TCP_Task(void *pvParameters);


void MB_TCP_Init(void)
{
	eMBTCPInit(&stTCPContext[0],0);
	eMBEnable(&stTCPContext[0]);
	//eMBSetSlaveID( MB_TCP_PSEUDO_ADDRESS, TRUE, Vendor, sizeof(Vendor) );
//	
	xTaskCreate(MB_TCP_Task, "MB TCP Task", MB_TCP_TASK_STACK_SIZE, NULL, 2, ( TaskHandle_t * ) NULL);
}


void MB_TCP_Task(void *pvParameters)
{
   while(1)
    {
			eMBPoll(&stTCPContext[0]);
    }
}


