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
	
	TMC_init(TMC_MX , MRES_16);
	TMC_init(TMC_MZ1, MRES_16);
	TMC_init(TMC_MZ2, MRES_16);
	TMC_init(TMC_VAC, MRES_16);
	TMC_init(TMC_MS1, MRES_16);
	TMC_init(TMC_MS2, MRES_16);
	TMC_init(TMC_QJ , MRES_16);
	TMC_init(TMC_FY , MRES_16);
	
	TMC_softEnable(TMC_MX , true);
	TMC_softEnable(TMC_MZ1, true);
	TMC_softEnable(TMC_MZ2, true);
	TMC_softEnable(TMC_VAC, true);
	TMC_softEnable(TMC_MS1, true);
	TMC_softEnable(TMC_MS2, true);
	TMC_softEnable(TMC_QJ , true);
	TMC_softEnable(TMC_FY , true);
	
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
	// VAC2 Close
	command.code = M133;
	command.param1 = 1;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	CHECK_STATE;
	// VAC Set Target
	command.code = M105;
	command.param1 = -5000;
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
	// MS1 To Trigger
	command.code = M107;
	command.param1 = 0;
	command.param2 = 200;
	command.param3 = 0;
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
	// MS2 To Trigger
	command.code = M108;
	command.param1 = 0;
	command.param2 = 200;
	command.param3 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
}
