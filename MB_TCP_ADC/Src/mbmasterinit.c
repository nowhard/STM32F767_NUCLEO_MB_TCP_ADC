#include "mbmasterinit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"
#include "mb_m.h"
#include "mbrtu.h"


#define MB_RTU_TASK_STACK_SIZE	2048
#define MB_RTU_POLL_TASK_STACK_SIZE	256


void MBMaster_RTU_Task(void *pvParameters);
void MBMaster_RTU_Poll(void *pvParameters);


void MBMaster_RTU_Init(void)
{
	eMBMasterInit(MB_RTU, 2, 9600,  MB_PAR_NONE);
	eMBMasterEnable();
	xTaskCreate(MBMaster_RTU_Task, "MB RTU Task", MB_RTU_TASK_STACK_SIZE, NULL, 2, ( TaskHandle_t * ) NULL);
	xTaskCreate(MBMaster_RTU_Poll, "MB RTU Poll Task", MB_RTU_POLL_TASK_STACK_SIZE, NULL, 2, ( TaskHandle_t * ) NULL);
}


void MBMaster_RTU_Task(void *pvParameters)
{

	while (1)
	{
		eMBMasterPoll();
	}
}

void MBMaster_RTU_Poll(void *pvParameters)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
	while (1)
	{
		vTaskDelay(500);
		errorCode = eMBMasterReqReadInputRegister(0xA,3,2,100);
		
	}
}

