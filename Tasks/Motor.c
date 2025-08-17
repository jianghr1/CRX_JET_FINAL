#include "Motor.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define MOTOR_X_MM_TO_ESTEP 2.502f
#define MOTOR_Z_MM_TO_ESTEP 50.0f // 4mm per round
#define MOTOR_X_HOME_DIRECTION +1
#define MOTOR_Z_HOME_DIRECTION +1
#define MOTOR_X_MOVE_DIRECTION (currentIntCommandPtr->param1 * 2 - 1)
#define MOTOR_Z_MOVE_DIRECTION (1 - currentIntCommandPtr->param1 * 2)
#define MOTOR_TRIGGERED_LVL GPIO_PIN_SET

#define CHECK_STATE if ((currentState & 0xF) == GlobalStateEStop || (currentState & 0xF) == GlobalStateError) {osMutexRelease(MUart1MutexHandle); osMutexRelease(MTim8MutexHandle); osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT); }

extern osThreadId_t defaultTaskHandle;
extern osMutexId_t MUart1MutexHandle;
extern osMutexId_t MTim8MutexHandle;
extern TIM_HandleTypeDef htim3;

void StartMotorTask(void* arg) {
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		osThreadFlagsWait(ALL_NEW_TASK, osFlagsWaitAny, osWaitForever);
		if (currentIntCommandPtr->code >= G110 && currentIntCommandPtr->code <= G113) {
			if (currentIntCommandPtr->param1 & (-2)) {
				if (currentIntCommandPtr->commandSource)
					usb_printf("ERROR\n");
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				continue;
			}
		}
		
		switch(currentIntCommandPtr->code)
		{
			case G110: {
				float target_pos = globalInfo.x_target_pos - currentIntCommandPtr->param3 * MOTOR_X_MOVE_DIRECTION;
				if (target_pos < 0 || target_pos > 220) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
				} else if (currentIntCommandPtr->param2 < 0 || currentIntCommandPtr->param2 > 50) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
				} else {
					if (currentIntCommandPtr->commandSource)
						usb_printf("OK\n");
					globalInfo.x_target_pos = target_pos;
					TMC_setSpeed(TMC_MX, MOTOR_X_MM_TO_ESTEP * currentIntCommandPtr->param2 * 2);
					osMutexAcquire(MUart1MutexHandle, osWaitForever);
					TMC_move(TMC_MX, MOTOR_X_MM_TO_ESTEP * (globalInfo.x_encoder_pos - globalInfo.x_target_pos));
					osMutexRelease(MUart1MutexHandle);
					TMC_wait_motor_stop(TMC_MX);	
				}
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);

				break;
			}
			case G111: {
				float target_pos = MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION + TMC_MZ1->current_pos;
				if (target_pos > 0 || target_pos < -60 * MOTOR_Z_MM_TO_ESTEP) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
				} else if (currentIntCommandPtr->param2 < 0 || currentIntCommandPtr->param2 > 5) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
				} else {
					if (currentIntCommandPtr->commandSource)
						usb_printf("OK\n");
					TMC_wait_motor_stop(TMC_VAC);
					osMutexAcquire(MUart1MutexHandle, osWaitForever);
					osMutexAcquire(MTim8MutexHandle, osWaitForever);
					TMC_setSpeed(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param2);
					TMC_move(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION);
					osMutexRelease(MUart1MutexHandle);
					TMC_wait_motor_stop(TMC_MZ1);
					osMutexRelease(MTim8MutexHandle);
				}
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G112: {
				float target_pos = MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION + TMC_MZ2->current_pos;
				if (target_pos > 0 || target_pos < -50 * MOTOR_Z_MM_TO_ESTEP) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
				} else if (currentIntCommandPtr->param2 < 0 || currentIntCommandPtr->param2 > 5) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
				} else {
					if (currentIntCommandPtr->commandSource)
						usb_printf("OK\n");
					TMC_wait_motor_stop(TMC_VAC);
					osMutexAcquire(MUart1MutexHandle, osWaitForever);
					osMutexAcquire(MTim8MutexHandle, osWaitForever);
					TMC_setSpeed(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param2);
					TMC_move(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION);
					osMutexRelease(MUart1MutexHandle);
					TMC_wait_motor_stop(TMC_MZ2);
					osMutexRelease(MTim8MutexHandle);
				}
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G113: {
				float target_pos1 = MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION + TMC_MZ1->current_pos;
				float target_pos2 = MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION + TMC_MZ2->current_pos;
				if (target_pos1 > 0 || target_pos1 < -50 * MOTOR_Z_MM_TO_ESTEP || target_pos2 > 0 || target_pos2 < -50 * MOTOR_Z_MM_TO_ESTEP) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
				}
				else {
					if (currentIntCommandPtr->commandSource)
						usb_printf("OK\n");
					TMC_wait_motor_stop(TMC_VAC);
					osMutexAcquire(MUart1MutexHandle, osWaitForever);
					osMutexAcquire(MTim8MutexHandle, osWaitForever);
					TMC_setSpeed(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param2);
					TMC_move(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION);
					TMC_move(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION);
					osMutexRelease(MUart1MutexHandle);
					TMC_wait_motor_stop(TMC_MZ1);
					CHECK_STATE;
					TMC_wait_motor_stop(TMC_MZ2);
					osMutexRelease(MTim8MutexHandle);
				}
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G114: {
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				if (HAL_GPIO_ReadPin(MX_TRIG_GPIO_Port, MX_TRIG_Pin) == MOTOR_TRIGGERED_LVL)
				{
					triggerHandler.MX = 0;
					TMC_move(TMC_MX, - 10 * MOTOR_X_MM_TO_ESTEP * MOTOR_X_HOME_DIRECTION);
				}
				TMC_wait_motor_stop(TMC_MX);
				triggerHandler.MX = TMC_MX;
				TMC_setSpeed(TMC_MX, 10 * MOTOR_X_MM_TO_ESTEP);
				TMC_move(TMC_MX, MOTOR_X_MM_TO_ESTEP * 800 * MOTOR_X_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MX);
				CHECK_STATE;
				triggerHandler.MX = 0;
				TMC_move(TMC_MX, - 2 * MOTOR_X_MM_TO_ESTEP * MOTOR_X_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MX);
				CHECK_STATE;
				triggerHandler.MX = TMC_MX;
				TMC_setSpeed(TMC_MX, 0.5 * MOTOR_X_MM_TO_ESTEP);
				TMC_move(TMC_MX, MOTOR_X_MM_TO_ESTEP * 800 * MOTOR_X_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MX);
				osDelay(200);
				__HAL_TIM_SET_COUNTER(&htim3, 0);
				osDelay(200);
				CHECK_STATE;
				triggerHandler.MX = 0;
				TMC_setSpeed(TMC_MX, 5 * MOTOR_X_MM_TO_ESTEP);
				TMC_moveTo(TMC_MX, - 10 * MOTOR_X_MM_TO_ESTEP * MOTOR_X_HOME_DIRECTION);
				triggerHandler.MX = TMC_MX;
				osMutexRelease(MUart1MutexHandle);
				osDelay(200);
				globalInfo.x_target_pos = 0;
				__HAL_TIM_SET_COUNTER(&htim3, 0);
				osDelay(200);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G115: {
				TMC_wait_motor_stop(TMC_VAC);
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				osMutexAcquire(MTim8MutexHandle, osWaitForever);
				TMC_setSpeed(TMC_MZ1, 1 * MOTOR_Z_MM_TO_ESTEP);
				if (HAL_GPIO_ReadPin(MZ1_TRIG_GPIO_Port, MZ1_TRIG_Pin) == MOTOR_TRIGGERED_LVL)
				{
					triggerHandler.MZ1 = 0;
					TMC_move(TMC_MZ1, - 3 * MOTOR_Z_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				}
				if (HAL_GPIO_ReadPin(MZ2_TRIG_GPIO_Port, MZ2_TRIG_Pin) == MOTOR_TRIGGERED_LVL)
				{
					triggerHandler.MZ2 = 0;
					TMC_move(TMC_MZ2, - 3 * MOTOR_Z_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				}
				TMC_wait_motor_stop(TMC_MZ1);
				CHECK_STATE;
				TMC_wait_motor_stop(TMC_MZ2);
				CHECK_STATE;
				osDelay(10);
				triggerHandler.MZ1 = TMC_MZ1;
				triggerHandler.MZ2 = TMC_MZ2;
				TMC_setDirection(TMC_MZ1, MOTOR_Z_HOME_DIRECTION);
				TMC_setDirection(TMC_MZ2, MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * 100 * MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * 100 * MOTOR_Z_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MZ1);
				CHECK_STATE;
				TMC_wait_motor_stop(TMC_MZ2);
				CHECK_STATE;
				triggerHandler.MZ1 = 0;
				triggerHandler.MZ2 = 0;
				TMC_move(TMC_MZ1, - 2 * MOTOR_Z_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ2, - 2 * MOTOR_Z_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MZ1);
				CHECK_STATE;
				TMC_wait_motor_stop(TMC_MZ2);
				CHECK_STATE;
				osDelay(10);
				TMC_wait_motor_stop(TMC_VAC);
				CHECK_STATE;
				triggerHandler.MZ1 = TMC_MZ1;
				triggerHandler.MZ2 = TMC_MZ2;
				TMC_setSpeed(TMC_MZ1, 0.5 * MOTOR_Z_MM_TO_ESTEP);
				TMC_move(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * 100 * MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * 100 * MOTOR_Z_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MZ1);
				CHECK_STATE;
				TMC_wait_motor_stop(TMC_MZ2);
				CHECK_STATE;
				triggerHandler.MZ1 = 0;
				triggerHandler.MZ2 = 0;
				TMC_move(TMC_MZ1, 1 * MOTOR_Z_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ2, 1 * MOTOR_Z_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MZ1);
				CHECK_STATE;
				TMC_wait_motor_stop(TMC_MZ2);
				CHECK_STATE;
				TMC_reset(TMC_MZ1);
				TMC_reset(TMC_MZ2);
				osMutexRelease(MUart1MutexHandle);
				osMutexRelease(MTim8MutexHandle);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			default:
				__NOP;
		}
	}
}

