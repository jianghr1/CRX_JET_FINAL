#include "Motor.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define MOTOR_X_MM_TO_ESTEP 5.305f
extern osThreadId_t defaultTaskHandle;

void StartMotorTask(void* arg) {
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		osThreadFlagsWait(ALL_NEW_TASK, osFlagsWaitAny, osWaitForever);
		
		switch(currentIntCommandPtr->code)
		{
			case G110: {
				TMC_setSpeed(TMC_MX, MOTOR_X_MM_TO_ESTEP * currentIntCommandPtr->param2);
				TMC_move(TMC_MX, MOTOR_X_MM_TO_ESTEP * currentIntCommandPtr->param3 * currentIntCommandPtr->param1);
				TMC_wait_motor_stop(TMC_MX);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case G111: {
				
			}
			default:
				__NOP;
		}
	}
}

