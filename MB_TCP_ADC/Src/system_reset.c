#include "FreeRTOS.h"
#include "task.h"
#include "system_reset.h"
#include "stm32f7xx_hal.h"

void SystemReset_Task( void *pvParameters );

void SystemReset_Start(void)
{
	xTaskCreate( SystemReset_Task, "System Reset Task", 128, NULL, 5, NULL );
}

void SystemReset_Task( void *pvParameters )
{
	vTaskDelay(SYSTEM_RESET_DELAY);
	NVIC_SystemReset();
	vTaskDelete(NULL);
}