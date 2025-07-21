#include "Clean.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"

#define PUMP_CCW_DEGREE 2000

extern osThreadId_t motorTaskHandle;
extern osThreadId_t pumpTaskHandle;
extern osThreadId_t vacTaskHandle;
extern osThreadId_t headerTaskHandle;

void CleanTask(int32_t channel) {
	static GMCommand_t command;
	
	// MS1 Set To MS
	command.code = M130+channel;
	command.param1 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// VAC1 Open
	command.code = M132;
	command.param1 = (channel)? 1: 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// VAC2 Close
	command.code = M133;
	command.param1 = (channel)? 0: 1;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// VAC Set Target
	command.code = M105;
	command.param1 = -5000;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(vacTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// VAC Start
	command.code = M106;
	command.param1 = 1;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(vacTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// MS1 Rotate CCW
	command.code = M100+channel;
	command.param1 = 1;
	command.param2 = 200;
	command.param3 = PUMP_CCW_DEGREE;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// QJ set to MS
	command.code = M130+channel;
	command.param1 = 1;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// QJ To Trigger
	command.code = M107+channel;
	command.param1 = 0;
	command.param2 = 200;
	command.param3 = 1;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// QJ Jetting
	command.code = M121;
	command.param1 = 0+channel;
	command.param2 = 200;
	command.param3 = 400;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// QJ Rotate CCW
	command.code = M102;
	command.param1 = 1;
	command.param2 = 200;
	command.param3 = PUMP_CCW_DEGREE;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}
	// MS1 Set To MS
	command.code = M130+channel;
	command.param1 = 0;
	currentIntCommandPtr = &command;
	osThreadFlagsSet(pumpTaskHandle, ALL_NEW_TASK);
	osThreadFlagsWait(MAIN_TASK_CPLT|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	if (currentState == GlobalStateEStop || currentState == GlobalStateError)
	{
		return;
	}

}
