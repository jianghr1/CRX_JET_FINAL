#include "Pump.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define PUMP_CW_DIRECTION 1
#define PUMP_ROTATE_EDEG 0.5556
extern osThreadId_t defaultTaskHandle;

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
				TMC_move(TMC_MS1, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * currentIntCommandPtr->param1 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_MS1);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M101: {
				TMC_setSpeed(TMC_MS2, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_MS2, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * currentIntCommandPtr->param1 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_MS2);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M103: {
				TMC_setSpeed(TMC_QJ, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_QJ, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * currentIntCommandPtr->param1 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_QJ);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M104: {
				TMC_setSpeed(TMC_FY, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_FY, PUMP_ROTATE_EDEG * currentIntCommandPtr->param3 * currentIntCommandPtr->param1 * PUMP_CW_DIRECTION);
				TMC_wait_motor_stop(TMC_FY);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			
			// MS rotate until trigger
			case M107: {
				TMC_setSpeed(TMC_MS1, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_MS1, 0xFFFF * currentIntCommandPtr->param1 * PUMP_CW_DIRECTION);
				osThreadFlagsWait(PUMP_YW1_TRIG, osFlagsWaitAny, osWaitForever);
				TMC_reset(TMC_MS1);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M108: {
				TMC_setSpeed(TMC_MS2, PUMP_ROTATE_EDEG * currentIntCommandPtr->param2);
				TMC_move(TMC_MS2, 0xFFFF * currentIntCommandPtr->param1 * PUMP_CW_DIRECTION);
				osThreadFlagsWait(PUMP_YW2_TRIG, osFlagsWaitAny, osWaitForever);
				TMC_reset(TMC_MS2);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			default:
				__NOP;
		}
	}
}