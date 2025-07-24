#include "Motor.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define MOTOR_X_MM_TO_ESTEP 5.305f
#define MOTOR_Z_MM_TO_ESTEP 20 // dummy value
#define MOTOR_X_HOME_DIRECTION +1
#define MOTOR_Z_HOME_DIRECTION +1
#define MOTOR_X_MOVE_DIRECTION (currentIntCommandPtr->param1 * 2 - 1)
#define MOTOR_Z_MOVE_DIRECTION (currentIntCommandPtr->param1 * 2 - 1)
#define MOTOR_TRIGGERED_LVL GPIO_PIN_RESET

extern osThreadId_t defaultTaskHandle;
extern osMutexId_t MUart1MutexHandle;
extern osMutexId_t MTim8MutexHandle;

void StartMotorTask(void* arg) {
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		osThreadFlagsWait(ALL_NEW_TASK, osFlagsWaitAny, osWaitForever);
		
		switch(currentIntCommandPtr->code)
		{
			case G110: {
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				TMC_setSpeed(TMC_MX, MOTOR_X_MM_TO_ESTEP * currentIntCommandPtr->param2);
				TMC_move(TMC_MX, MOTOR_X_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_X_MOVE_DIRECTION);
				osMutexRelease(MUart1MutexHandle);
				TMC_wait_motor_stop(TMC_MX);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G111: {
				TMC_wait_motor_stop(TMC_VAC);
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				osMutexAcquire(MTim8MutexHandle, osWaitForever);
				TMC_setSpeed(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param2);
				TMC_move(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION);
				osMutexRelease(MUart1MutexHandle);
				TMC_wait_motor_stop(TMC_MZ1);
				osMutexRelease(MTim8MutexHandle);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G112: {
				TMC_wait_motor_stop(TMC_VAC);
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				osMutexAcquire(MTim8MutexHandle, osWaitForever);
				TMC_setSpeed(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param2);
				TMC_move(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION);
				osMutexRelease(MUart1MutexHandle);
				TMC_wait_motor_stop(TMC_MZ2);
				osMutexRelease(MTim8MutexHandle);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G113: {
				TMC_wait_motor_stop(TMC_VAC);
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				osMutexAcquire(MTim8MutexHandle, osWaitForever);
				TMC_setSpeed(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param2);
				TMC_move(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION);
				TMC_move(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * currentIntCommandPtr->param3 * MOTOR_Z_MOVE_DIRECTION);
				osMutexRelease(MUart1MutexHandle);
				TMC_wait_motor_stop(TMC_MZ1);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_wait_motor_stop(TMC_MZ2);
				osMutexRelease(MTim8MutexHandle);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G114: {
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				if (HAL_GPIO_ReadPin(MX_TRIG_GPIO_Port, MX_TRIG_Pin) == MOTOR_TRIGGERED_LVL)
				{
					TMC_move(TMC_MX, - 15 * MOTOR_X_MM_TO_ESTEP * MOTOR_X_HOME_DIRECTION);
				}
				TMC_wait_motor_stop(TMC_MX);
				TMC_setSpeed(TMC_MX, 20 * currentIntCommandPtr->param2);
				TMC_move(TMC_MX, MOTOR_X_MM_TO_ESTEP * 800 * MOTOR_X_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MX);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexAcquire(MUart1MutexHandle, osWaitForever);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_move(TMC_MX, - 2 * MOTOR_X_MM_TO_ESTEP * MOTOR_X_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MX);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexAcquire(MUart1MutexHandle, osWaitForever);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_setSpeed(TMC_MX, 0.5 * currentIntCommandPtr->param2);
				TMC_move(TMC_MX, MOTOR_X_MM_TO_ESTEP * 800 * MOTOR_X_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MX);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexAcquire(MUart1MutexHandle, osWaitForever);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_moveTo(TMC_MX, - 10 * MOTOR_X_MM_TO_ESTEP * MOTOR_X_HOME_DIRECTION);
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G115: {
				TMC_wait_motor_stop(TMC_VAC);
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				osMutexAcquire(MTim8MutexHandle, osWaitForever);
				if (HAL_GPIO_ReadPin(MZ1_TRIG_GPIO_Port, MZ1_TRIG_Pin) == MOTOR_TRIGGERED_LVL)
				{
					TMC_move(TMC_MZ1, - 10 * MOTOR_Z_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				}
				if (HAL_GPIO_ReadPin(MZ2_TRIG_GPIO_Port, MZ2_TRIG_Pin) == MOTOR_TRIGGERED_LVL)
				{
					TMC_move(TMC_MZ2, - 10 * MOTOR_Z_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				}
				TMC_wait_motor_stop(TMC_MZ1);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_wait_motor_stop(TMC_MZ2);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_setSpeed(TMC_MZ1, 10 * currentIntCommandPtr->param2);
				TMC_move(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * 800 * MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * 800 * MOTOR_Z_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MZ1);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_wait_motor_stop(TMC_MZ2);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_move(TMC_MZ1, - 2 * MOTOR_X_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ2, - 2 * MOTOR_X_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MZ1);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_wait_motor_stop(TMC_MZ2);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				
				TMC_wait_motor_stop(TMC_VAC);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_setSpeed(TMC_MZ1, 0.5 * currentIntCommandPtr->param2);
				TMC_move(TMC_MZ1, MOTOR_Z_MM_TO_ESTEP * 800 * MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ2, MOTOR_Z_MM_TO_ESTEP * 800 * MOTOR_Z_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MZ1);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_wait_motor_stop(TMC_MZ2);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_move(TMC_MZ1, - 10 * MOTOR_X_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				TMC_move(TMC_MZ2, - 10 * MOTOR_X_MM_TO_ESTEP * MOTOR_Z_HOME_DIRECTION);
				TMC_wait_motor_stop(TMC_MZ1);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				TMC_wait_motor_stop(TMC_MZ2);
				if (currentState == GlobalStateEStop || currentState == GlobalStateError) {
					osMutexRelease(MUart1MutexHandle);
					osMutexRelease(MTim8MutexHandle);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
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

