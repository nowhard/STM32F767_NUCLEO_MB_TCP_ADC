#include "mbmasterpyro.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"
#include "mb_m.h"
#include "mbrtu.h"
#include "mb_master_user.h"
#include "mbfunc.h"
#include "adc_pyro_buf.h"
#include "adc_dcmi.h"
#include "udp_send.h"


#define MB_RTU_TASK_STACK_SIZE			2048
#define MB_RTU_TASK_PRIO						2

#define MB_RTU_POLL_TASK_STACK_SIZE	256
#define MB_RTU_POLL_TASK_PRIO				2

#define SLAVE_PYRO_SQUIB_ADDR					0xA
#define SLAVE_PYRO_SQUIB_TIMEOUT			100
#define SLAVE_PYRO_SQUIB_POLL_PERIOD	1

#define MODBUS_M_BAUDRATE							921600

xSemaphoreHandle xStartReadPyroADCSem;
xSemaphoreHandle xMBRTUMutex;

extern stTCPtoRTURegWrite TCPtoRTURegWrite;
extern stPacket UDPPacket;
extern enADCPyroBufState ADCPyroBufState;
extern uint64_t ADC_Pyro_Timestamp;


TaskHandle_t RTUTaskHandle;
TaskHandle_t RTUPollTaskHandle;
void MBMaster_RTU_Task(void *pvParameters);
void MBMaster_RTU_Poll(void *pvParameters);


void MBMaster_RTU_Init(void)
{
	xStartReadPyroADCSem=xSemaphoreCreateBinary();
	xMBRTUMutex=xSemaphoreCreateMutex();
	
	eMBMasterInit(MB_RTU, 2, MODBUS_M_BAUDRATE,  MB_PAR_NONE);
	eMBMasterEnable();
	xTaskCreate(MBMaster_RTU_Task, "MB RTU Task", MB_RTU_TASK_STACK_SIZE, NULL, MB_RTU_TASK_PRIO, &RTUTaskHandle);
	xTaskCreate(MBMaster_RTU_Poll, "MB RTU Poll Task", MB_RTU_POLL_TASK_STACK_SIZE, NULL, MB_RTU_POLL_TASK_PRIO, &RTUPollTaskHandle);
}


void MBMaster_RTU_Task(void *pvParameters)
{
	while (1)
	{
		eMBMasterPoll();
	}
}



#define SLAVE_REGS_POLL_PERIOD		200
#define REG_ADC_0									0
#define REG_PIR_STATE							16
#define READ_ERR_MAX							10
enum
{
  PYRO_SQUIB_STOP=0,
	PYRO_SQUIB_RUN,
};


eMBMasterReqErrCode    MB_Master_ErrorCode = MB_MRE_NO_ERR;

extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];

void MBMaster_RTU_Poll(void *pvParameters)
{
	portTickType xLastWakeTime;
	uint8_t readErrCnt=0;
	
	
	while (1)
	{		
			if( xSemaphoreTake( xStartReadPyroADCSem, SLAVE_REGS_POLL_PERIOD ) == pdTRUE )
			{//проверяем состояние пиропатрона, если включен-заполняем буфер
					xLastWakeTime = xTaskGetTickCount();
					readErrCnt=0;

					vTaskPrioritySet(RTUPollTaskHandle,MB_RTU_POLL_TASK_PRIO+1);
					vTaskPrioritySet(RTUTaskHandle,MB_RTU_TASK_PRIO+1);
					do
					{
								vTaskDelayUntil( &xLastWakeTime, ADC_PYRO_SAMPLING_PERIOD );
						
								if( xSemaphoreTake( xMBRTUMutex, portMAX_DELAY ) == pdTRUE )
								{
										MB_Master_ErrorCode = eMBMasterReqReadInputRegister(SLAVE_PYRO_SQUIB_ADDR,M_REG_INPUT_START,M_REG_INPUT_NREGS,SLAVE_PYRO_SQUIB_TIMEOUT);//читаем регистры ведомого		
										xSemaphoreGive( xMBRTUMutex );
								}

								if(MB_Master_ErrorCode == MB_MRE_NO_ERR)
								{
										ADC_PyroBuf_Add((float*)&usMRegInBuf[0][REG_ADC_0]);//эмулируем 1мс опрос
										ADC_PyroBuf_Add((float*)&usMRegInBuf[0][REG_ADC_0]);
									
										ADC_Pyro_Timestamp=DCMI_ADC_GetCurrentTimestamp();
										ADCPyroBufState=ADC_PYRO_BUF_FILL_START;
										readErrCnt=0;
								}
								else
								{
										ADCPyroBufState=ADC_PYRO_BUF_FILL_STOP;
										readErrCnt++;
										if(readErrCnt>=READ_ERR_MAX)
										{
												break;
										}
								}	
					}
					while(usMRegInBuf[0][REG_PIR_STATE]!=PYRO_SQUIB_STOP);
					ADCPyroBufState=ADC_PYRO_BUF_FILL_STOP;
					vTaskPrioritySet(RTUTaskHandle,MB_RTU_TASK_PRIO);
					vTaskPrioritySet(RTUPollTaskHandle,MB_RTU_POLL_TASK_PRIO);
					
			}
			else//опрос в стандартном режиме
			{
					if( xSemaphoreTake( xMBRTUMutex, portMAX_DELAY ) == pdTRUE )
					{
							MB_Master_ErrorCode = eMBMasterReqReadHoldingRegister(SLAVE_PYRO_SQUIB_ADDR,M_REG_HOLDING_START,M_REG_HOLDING_NREGS,SLAVE_PYRO_SQUIB_TIMEOUT);	
							MB_Master_ErrorCode = eMBMasterReqReadInputRegister(SLAVE_PYRO_SQUIB_ADDR,M_REG_INPUT_START,M_REG_INPUT_NREGS,SLAVE_PYRO_SQUIB_TIMEOUT);//читаем регистры ведомого		
							xSemaphoreGive( xMBRTUMutex );
					}
			}
	}
}

eMBMasterReqErrCode MBMaster_RTU_WriteRegs(stTCPtoRTURegWrite *regs)
{
	eMBMasterReqErrCode    err=MB_MRE_NO_ERR; 
	
	if(regs->nRegs==1)
	{
			err = eMBMasterReqWriteHoldingRegister(SLAVE_PYRO_SQUIB_ADDR,regs->regAddr,regs->regBuf[0],SLAVE_PYRO_SQUIB_TIMEOUT);
	}
	else 
	{
			err = eMBMasterReqWriteMultipleHoldingRegister(SLAVE_PYRO_SQUIB_ADDR,regs->regAddr,regs->nRegs,&regs->regBuf[0],SLAVE_PYRO_SQUIB_TIMEOUT);
	}
	
	return err;
}

