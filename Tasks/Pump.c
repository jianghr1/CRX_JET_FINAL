#include "Pump.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define PUMP_CW_DIRECTION (currentIntCommandPtr->param1*2-1)
#define PUMP_ROTATE_EDEG 0.5556f
#define YW_TRIGGERED_LVL GPIO_PIN_RESET

extern osThreadId_t defaultTaskHandle;
extern TIM_HandleTypeDef htim2;

void StartPumpTask(void *argument) {
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		osThreadFlagsWait(ALL_NEW_TASK, osFlagsWaitAny, osWaitForever);
		if (currentIntCommandPtr->code >= M100 && currentIntCommandPtr->code <= M107) {
			if (currentIntCommandPtr->param1 & (-2)) {
				if (currentIntCommandPtr->commandSource)
					usb_printf("ERROR\n");
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				continue;
			}
			if (currentIntCommandPtr->param2 < 0 || currentIntCommandPtr->param2 > 720) {
				if (currentIntCommandPtr->commandSource)
					usb_printf("ERROR\n");
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				continue;
			}
			if (currentIntCommandPtr->param3 < 0 || currentIntCommandPtr->param3 > 3600) {
				if (currentIntCommandPtr->commandSource)
					usb_printf("ERROR\n");
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				continue;
			}
			if (currentIntCommandPtr->commandSource)
				usb_printf("OK\n");
		} else if (currentIntCommandPtr->code >= M130 && currentIntCommandPtr->code <= M133) {
			if (currentIntCommandPtr->param1 & (-2)) {
				if (currentIntCommandPtr->commandSource)
					usb_printf("ERROR\n");
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				continue;
			}
			if (currentIntCommandPtr->commandSource)
				usb_printf("OK\n");
		}  else if (currentIntCommandPtr->code == M141) {
			if (currentIntCommandPtr->param1 & (-2)) {
				if (currentIntCommandPtr->commandSource)
					usb_printf("ERROR\n");
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				continue;
			}
			if (currentIntCommandPtr->commandSource)
				usb_printf("OK\n");
		} else if (currentIntCommandPtr->code == M140) {
			if (currentIntCommandPtr->param1 & (-8)) {
				if (currentIntCommandPtr->commandSource)
					usb_printf("ERROR\n");
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				continue;
			}
			if (currentIntCommandPtr->commandSource)
				usb_printf("OK\n");
		}

		switch(currentIntCommandPtr->code) 
		{
			// MS rotate specific degree
			case M100: {
				TMC_setSpeed(TMC_MS1, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_MS1, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_MS1);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M101: {
				TMC_setSpeed(TMC_MS2, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_MS2, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_MS2);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M102: {
				TMC_setSpeed(TMC_QJ, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_QJ, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_QJ);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M103: {
				TMC_setSpeed(TMC_FY, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_FY, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_FY);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			
			// MS rotate until trigger
			case M107: {
				if (HAL_GPIO_ReadPin(MS1_YW_GPIO_Port, MS1_YW_Pin) != YW_TRIGGERED_LVL) {
					if (currentIntCommandPtr->param3 == 0) {
						triggerHandler.YW1 = TMC_MS1;
						TMC_setSpeed(TMC_MS1, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
						TMC_move(TMC_MS1, 0xFFFF * PUMP_CW_DIRECTION);
						TMC_wait_motor_stop(TMC_MS1);
					} else {
						triggerHandler.YW1 = TMC_QJ;
						TMC_setSpeed(TMC_QJ, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
						TMC_move(TMC_QJ, 0xFFFF * PUMP_CW_DIRECTION);
						TMC_wait_motor_stop(TMC_QJ);
					}
				}
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M108: {
				if (HAL_GPIO_ReadPin(MS2_YW_GPIO_Port, MS2_YW_Pin) != YW_TRIGGERED_LVL) {
					if (currentIntCommandPtr->param3 == 0) {
						triggerHandler.YW2 = TMC_MS2;
						TMC_setSpeed(TMC_MS2, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
						TMC_move(TMC_MS2, 0xFFFF * PUMP_CW_DIRECTION);
						TMC_wait_motor_stop(TMC_MS2);
					} else {
						triggerHandler.YW2 = TMC_QJ;
						TMC_setSpeed(TMC_QJ, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
						TMC_move(TMC_QJ, 0xFFFF * PUMP_CW_DIRECTION);
						TMC_wait_motor_stop(TMC_QJ);
					}
				}				
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			
			// MS ctrl
			case M130: {
				HAL_GPIO_WritePin(MS1_CTL_GPIO_Port, MS1_CTL_Pin, currentIntCommandPtr->param1);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M131: {
				HAL_GPIO_WritePin(MS2_CTL_GPIO_Port, MS2_CTL_Pin, currentIntCommandPtr->param1);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			// VAC ctrl
			case M132: {
				HAL_GPIO_WritePin(VAC1_CTL_GPIO_Port, VAC1_CTL_Pin, (GPIO_PinState)currentIntCommandPtr->param1);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M133: {
				HAL_GPIO_WritePin(VAC2_CTL_GPIO_Port, VAC2_CTL_Pin, (GPIO_PinState)currentIntCommandPtr->param1);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			// UV ctrl
			case M140: {
				htim2.Instance->CCR2 = currentIntCommandPtr->param1 * 125;
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M141: {
				HAL_GPIO_WritePin(UVF_CTL_GPIO_Port, UVF_CTL_Pin, (GPIO_PinState)currentIntCommandPtr->param1);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			default:
				__NOP;
		}
	}
}
