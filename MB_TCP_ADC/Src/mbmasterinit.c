#include "mbmasterinit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"
#include "mb_m.h"
#include "mbrtu.h"
#include "mb_master_user.h"


#define MB_RTU_TASK_STACK_SIZE	2048
#define MB_RTU_POLL_TASK_STACK_SIZE	256

#define SLAVE_PYRO_SQUIB_ADDR					0xA
#define SLAVE_PYRO_SQUIB_TIMEOUT			100
#define SLAVE_PYRO_SQUIB_POLL_PERIOD	500



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

eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
void MBMaster_RTU_Poll(void *pvParameters)
{

	while (1)
	{
		vTaskDelay(SLAVE_PYRO_SQUIB_POLL_PERIOD);
		errorCode = eMBMasterReqReadInputRegister(SLAVE_PYRO_SQUIB_ADDR,M_REG_INPUT_START,M_REG_INPUT_NREGS,SLAVE_PYRO_SQUIB_TIMEOUT);
		errorCode = eMBMasterReqReadHoldingRegister(SLAVE_PYRO_SQUIB_ADDR,M_REG_HOLDING_START,M_REG_HOLDING_NREGS,SLAVE_PYRO_SQUIB_TIMEOUT);
	}
}

