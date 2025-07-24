#include "Vac.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define VAC_CW_DIRECTION +1
#define VAC_PRESSURE_DIRECTION -1
#define MOTOR_DEGREE_TO_ESTEP 0.5556f
extern osThreadId_t defaultTaskHandle;
extern osMutexId_t MUart1MutexHandle;
extern osMutexId_t MTim8MutexHandle;

void StartVacTask(void* arg) {
	
	static int32_t target_pressure = 0;
	static int32_t vac_working = 0;
	uint32_t flag;
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		flag = osThreadFlagsWait(ALL_NEW_TASK|ALL_EMG_STOP, osFlagsWaitAny, 20);
		
		if (currentState == GlobalStateEStop || currentState == GlobalStateError)
		{
			vac_working = 0;
			continue;
		}
		if (flag & ALL_NEW_TASK) {
			switch(currentIntCommandPtr->code)
			{
				case M104: {
					TMC_setSpeed(TMC_VAC, MOTOR_DEGREE_TO_ESTEP * currentIntCommandPtr->param2);
					TMC_move(TMC_VAC, MOTOR_DEGREE_TO_ESTEP * currentIntCommandPtr->param3 * currentIntCommandPtr->param1 * VAC_CW_DIRECTION);
					TMC_wait_motor_stop(TMC_VAC);
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				case M105: {
					target_pressure = currentIntCommandPtr->param1;
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				case M106: {
					vac_working = currentIntCommandPtr->param1;
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				default: 
					__NOP;
			}
		}
		if (vac_working)
		{
			if (target_pressure - globalInfo.vac_pressure > 100)
			{
				// VAC conflict with Motor Z1 and Motor Z2
				TMC_wait_motor_stop(TMC_MZ1);
				TMC_wait_motor_stop(TMC_MZ2);
				TMC_wait_motor_stop(TMC_VAC);
				osMutexAcquire(MTim8MutexHandle, osWaitForever);
				TMC_setSpeed(TMC_VAC, 200);
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				TMC_move(TMC_VAC, 4 * VAC_PRESSURE_DIRECTION);
				osMutexRelease(MUart1MutexHandle);
				osMutexRelease(MTim8MutexHandle);
			} else if (globalInfo.vac_pressure - target_pressure > 100)
			{
				// VAC conflict with Motor Z1 and Motor Z2
				TMC_wait_motor_stop(TMC_MZ1);
				TMC_wait_motor_stop(TMC_MZ2);
				TMC_wait_motor_stop(TMC_VAC);
				osMutexAcquire(MTim8MutexHandle, osWaitForever);
				TMC_setSpeed(TMC_VAC, 200);
				osMutexAcquire(MUart1MutexHandle, osWaitForever);
				TMC_move(TMC_VAC,-4 * VAC_PRESSURE_DIRECTION);
				osMutexRelease(MUart1MutexHandle);
				osMutexRelease(MTim8MutexHandle);
			}
		}
	}
}

