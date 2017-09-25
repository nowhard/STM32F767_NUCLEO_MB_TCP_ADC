#include "FreeRTOS.h"
#include "task.h"
#include "system_reset.h"
#include "stm32f7xx_hal.h"


#define SYSTEM_RESET_TASK_STACK_SIZE	128
#define SYSTEM_RESET_TASK_PRIO				5

void SystemReset_Task( void *pvParameters );

void SystemReset_Start(void)
{
	xTaskCreate( SystemReset_Task, "System Reset Task", SYSTEM_RESET_TASK_STACK_SIZE, NULL, SYSTEM_RESET_TASK_PRIO, NULL );
}

void SystemReset_Task( void *pvParameters )
{
	vTaskDelay(SYSTEM_RESET_DELAY);
	NVIC_SystemReset();
	vTaskDelete(NULL);
}