#include "mbinit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"
#include "mbtcp.h"
#include "cfg_info.h"

#define MB_TCP_TASK_STACK_SIZE	2048
#define MB_TCP_TASK_PRIO				2
#define MB_TCP_CLIENT_NUM				2

stMBContext stTCPContext[MB_TCP_CLIENT_NUM];

void MB_TCP_Task(void *pvParameters);

void MB_TCP_Init(void)
{
	uint8_t contextCnt=0;
	
	for(contextCnt=0;contextCnt<MB_TCP_CLIENT_NUM;contextCnt++)
	{
			eMBTCPInit(&stTCPContext[contextCnt],0);
			eMBEnable(&stTCPContext[contextCnt]);

			xTaskCreate(MB_TCP_Task, "MB TCP Task", MB_TCP_TASK_STACK_SIZE, (void*)&stTCPContext[contextCnt], MB_TCP_TASK_PRIO, ( TaskHandle_t * ) NULL);
	}
}


void MB_TCP_Task(void *pvParameters)
{
	 stMBContext *stTCPContext;
	 stTCPContext=(stMBContext*)pvParameters;
   while(1)
   {
			eMBPoll(stTCPContext);
   }
}


