#include "Sensor.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"
#include "pressure.h"
extern osThreadId_t defaultTaskHandle;

void StartJettingTask(void *argument) {
	while(true) {
		osDelay(10);
	}
}
