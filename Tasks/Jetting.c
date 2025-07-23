#include "Jetting.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "Sensor.h"
#include "TMC2209.h"

extern UART_HandleTypeDef huart7;
extern osThreadId_t jettingTaskHandle;

JettingInfo_t jettingInfo;
uint32_t retry;
void StartJettingTask(void *argument) {
	while(true) {
		osThreadFlagsWait(ALL_NEW_TASK, osFlagsWaitAny, osWaitForever);
		retry = 3;
		while(retry) {
			jettingInfo.status = 0;
			if(HAL_UART_GetState(&huart7) != HAL_UART_STATE_READY) {
				osThreadFlagsWait(JETTING_UART_CPLT, osFlagsWaitAny, 10);
			}
			if (HAL_UART_Transmit_DMA(&huart7, (uint8_t *)jettingInfo.data, 42) != HAL_OK) {
				retry--;
			}
			else if (osThreadFlagsWait(JETTING_FPGA_REPLY, osFlagsWaitAny, 10) > 0xFFFFFFF0) {
				retry--;
			}
			else if (jettingInfo.status != 0x01) {
				retry--;

			} else {
				osThreadFlagsSet(jettingInfo.threadId, MAIN_TASK_CPLT);
				break;
			}
		}
		if (retry == 0) {
			EmergencyStop(GlobalStateError);
		}
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart7)
    {
        osThreadFlagsSet(jettingTaskHandle, JETTING_UART_CPLT);
    }
}
