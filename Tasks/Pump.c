#include "Pump.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define PUMP_CW_DIRECTION (currentIntCommandPtr->param1*2-1)
#define PUMP_ROTATE_EDEG 0.5556
#define YW_TRIGGERED_LVL GPIO_PIN_RESET

extern osThreadId_t defaultTaskHandle;
extern TIM_HandleTypeDef htim2;

void StartPumpTask(void *argument) {
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		osThreadFlagsWait(ALL_NEW_TASK, osFlagsWaitAny, osWaitForever);
		
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
			case M103: {
				TMC_setSpeed(TMC_QJ, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_QJ, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_QJ);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M104: {
				TMC_setSpeed(TMC_FY, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_FY, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_FY);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			
			// MS rotate until trigger
			case M107: {
				if (HAL_GPIO_ReadPin(MS1_YW_GPIO_Port, MS1_YW_Pin) != YW_TRIGGERED_LVL) {
					TMC_setSpeed(TMC_MS1, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
					TMC_move(TMC_MS1, 0xFFFF * PUMP_CW_DIRECTION);
					TMC_wait_motor_stop(TMC_MS1);
				}
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M108: {
				if (HAL_GPIO_ReadPin(MS2_YW_GPIO_Port, MS2_YW_Pin) != YW_TRIGGERED_LVL) {
					TMC_setSpeed(TMC_MS2, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
					TMC_move(TMC_MS2, 0xFFFF * PUMP_CW_DIRECTION);
					TMC_wait_motor_stop(TMC_MS2);
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