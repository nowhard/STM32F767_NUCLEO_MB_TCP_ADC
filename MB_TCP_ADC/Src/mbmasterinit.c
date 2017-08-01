#include "mbmasterinit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"
#include "mb_m.h"
#include "mbrtu.h"
#include "mb_master_user.h"
#include "mbfunc.h"


#define MB_RTU_TASK_STACK_SIZE	2048
#define MB_RTU_POLL_TASK_STACK_SIZE	256

#define SLAVE_PYRO_SQUIB_ADDR					0xA
#define SLAVE_PYRO_SQUIB_TIMEOUT			100
#define SLAVE_PYRO_SQUIB_POLL_PERIOD	1

#define MODBUS_M_BAUDRATE			921600

xSemaphoreHandle xSendRTURegSem;
extern stTCPtoRTURegWrite TCPtoRTURegWrite;

void MBMaster_RTU_Task(void *pvParameters);
void MBMaster_RTU_Poll(void *pvParameters);


void MBMaster_RTU_Init(void)
{
	vSemaphoreCreateBinary(xSendRTURegSem);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = DATALED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DATALED_GPIO_Port, &GPIO_InitStruct);
	
	eMBMasterInit(MB_RTU, 2, MODBUS_M_BAUDRATE,  MB_PAR_NONE);
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



#define HOLDING_REG_POLL_PERIOD		200
eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
void MBMaster_RTU_Poll(void *pvParameters)
{
	static uint16_t tick_counter=0;
	portTickType xLastWakeTime;
	
	xLastWakeTime = xTaskGetTickCount();
	while (1)
	{		
		if(xSemaphoreTake( xSendRTURegSem, ( TickType_t )0 )==TRUE)
		{
				errorCode = eMBMasterReqWriteHoldingRegister(SLAVE_PYRO_SQUIB_ADDR,TCPtoRTURegWrite.regAddr,TCPtoRTURegWrite.regBuf[0],SLAVE_PYRO_SQUIB_TIMEOUT);
		}

		errorCode = eMBMasterReqReadInputRegister(SLAVE_PYRO_SQUIB_ADDR,M_REG_INPUT_START,M_REG_INPUT_NREGS,SLAVE_PYRO_SQUIB_TIMEOUT);
		
		if(tick_counter>=HOLDING_REG_POLL_PERIOD)
		{
				errorCode = eMBMasterReqReadHoldingRegister(SLAVE_PYRO_SQUIB_ADDR,M_REG_HOLDING_START,M_REG_HOLDING_NREGS,SLAVE_PYRO_SQUIB_TIMEOUT);	
				tick_counter=0;
		}
		tick_counter++;
	
//			errorCode = eMBMasterReqReadHoldingRegister(SLAVE_PYRO_SQUIB_ADDR,M_REG_HOLDING_START,M_REG_HOLDING_NREGS,SLAVE_PYRO_SQUIB_TIMEOUT);	
//			HAL_GPIO_TogglePin(DATALED_GPIO_Port,DATALED_Pin);	

		 //vTaskDelayUntil( &xLastWakeTime, 1 );
	}
}

