#include "Init.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define CHECK_STATE if ((currentState & 0xF) == GlobalStateEStop || (currentState & 0xF) == GlobalStateError) return

extern osThreadId_t motorTaskHandle;
extern osThreadId_t pumpTaskHandle;
extern osThreadId_t vacTaskHandle;
extern osThreadId_t headerTaskHandle;

void InitTask(void) {
	static GMCommand_t command;
	
	// X zero
	command.code = G114;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(motorTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	// Z zero
	command.code = G115;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(motorTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	// MS1 Set To MS
	command.code = M130;
	command.param1 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	// MS2 Set To MS
	command.code = M131;
	command.param1 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	// VAC1 Open
	command.code = M132;
	command.param1 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	// VAC2 Open
	command.code = M133;
	command.param1 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	// VAC Set Target
	command.code = M105;
	command.param1 = -800;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(vacTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	// VAC Start
	command.code = M106;
	command.param1 = 1;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(vacTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	while(globalInfo.vac_pressure > -500) {
		osDelay(500);
		CHECK_STATE;
	}
	// MS1 To Trigger
	command.code = M100;
	command.param1 = 0;
	command.param2 = 180;
	command.param3 = 720;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	command.code = M107;
	command.param1 = 1;
	command.param2 = 36;
	command.param3 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	osDelay(500);
	command.code = M100;
	command.param1 = 1;
	command.param2 = 36;
	command.param3 = 60;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	osDelay(500);
	// MS2 To Trigger
	command.code = M101;
	command.param1 = 0;
	command.param2 = 180;
	command.param3 = 720;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	osDelay(500);
	command.code = M108;
	command.param1 = 1;
	command.param2 = 36;
	command.param3 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	osDelay(500);
	command.code = M101;
	command.param1 = 1;
	command.param2 = 36;
	command.param3 = 60;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	currentState = GlobalStateIdle;
}
