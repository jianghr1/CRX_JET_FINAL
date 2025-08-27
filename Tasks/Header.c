#include "Header.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "Jetting.h"

extern osThreadId_t defaultTaskHandle;
extern osThreadId_t headerTaskHandle;
extern osThreadId_t jettingTaskHandle;
extern SPI_HandleTypeDef hspi4;
#define Voltage_K 9.5e-3f
#define Voltage_B 5.0f
void StartHeaderTask(void *argument) {
	static uint8_t VoltageR;
	static Jetting_t jetting;
	jetting.SOF = 0x2A;
	for(uint8_t i = 0; i < 40; ++i) jetting.data[i] = 0xFF;
	
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		osThreadFlagsWait(ALL_NEW_TASK|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	
		switch(currentIntCommandPtr->code)
		{
			case M120: {
				if (currentIntCommandPtr->param1 < 0 || currentIntCommandPtr->param1 > 100) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				if (currentIntCommandPtr->commandSource)
					usb_printf("OK\n");
				globalInfo.targetTemperature = currentIntCommandPtr->param1 * 10;
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M121: {
				if (currentIntCommandPtr->param1 & (-2)) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				if (currentIntCommandPtr->param2 < 1 || currentIntCommandPtr->param2 > 300) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				if (currentIntCommandPtr->param3 < 0 || currentIntCommandPtr->param3 > 10000 || currentIntCommandPtr->param3 != (int)currentIntCommandPtr->param3) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				if (currentIntCommandPtr->commandSource)
					usb_printf("OK\n");
				jettingInfo.threadId = headerTaskHandle;
				uint32_t tick_start = xTaskGetTickCount();
				for (uint32_t i = 0; i < currentIntCommandPtr->param3; i++) {
					uint32_t tick = tick_start + 1000 * (i+1) / currentIntCommandPtr->param2;
					jetting.channel = 2 - currentIntCommandPtr->param1 * 2;
					jettingInfo.data = &jetting;
					osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
					osThreadFlagsWait(JETTING_FPGA_REPLY, osFlagsWaitAny, osWaitForever);
					jetting.channel = jetting.channel + 1;
					osThreadFlagsSet(jettingTaskHandle, ALL_NEW_TASK);
					osThreadFlagsWait(JETTING_FPGA_REPLY, osFlagsWaitAny, osWaitForever);
					if (xTaskGetTickCount() < tick)
					{
						osDelayUntil(tick);
					}
				}
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M122: {
				if (currentIntCommandPtr->param1 < 0 || currentIntCommandPtr->param1 > 3) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				if (currentIntCommandPtr->param2 < 17000 || currentIntCommandPtr->param2 > 19000) {
					if (currentIntCommandPtr->commandSource)
						usb_printf("ERROR\n");
					osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
					break;
				}
				if (currentIntCommandPtr->commandSource)
					usb_printf("OK\n");
				VoltageR = currentIntCommandPtr->param2 * Voltage_K + Voltage_B;
				switch (currentIntCommandPtr->param1)
				{
					case 0: {
						HAL_GPIO_WritePin(VSEL_A_GPIO_Port, VSEL_A_Pin, GPIO_PIN_RESET);
						HAL_SPI_Transmit(&hspi4, &VoltageR, 1, 10);
						HAL_GPIO_WritePin(VSEL_A_GPIO_Port, VSEL_A_Pin, GPIO_PIN_SET);
						break;
					}
					case 1: {
						HAL_GPIO_WritePin(VSEL_B_GPIO_Port, VSEL_B_Pin, GPIO_PIN_RESET);
						HAL_SPI_Transmit(&hspi4, &VoltageR, 1, 10);
						HAL_GPIO_WritePin(VSEL_B_GPIO_Port, VSEL_B_Pin, GPIO_PIN_SET);
						break;
					}
					case 2: {
						HAL_GPIO_WritePin(VSEL_C_GPIO_Port, VSEL_C_Pin, GPIO_PIN_RESET);
						HAL_SPI_Transmit(&hspi4, &VoltageR, 1, 10);
						HAL_GPIO_WritePin(VSEL_C_GPIO_Port, VSEL_C_Pin, GPIO_PIN_SET);
						break;
					}
					case 3: {
						HAL_GPIO_WritePin(VSEL_D_GPIO_Port, VSEL_D_Pin, GPIO_PIN_RESET);
						HAL_SPI_Transmit(&hspi4, &VoltageR, 1, 10);
						HAL_GPIO_WritePin(VSEL_D_GPIO_Port, VSEL_D_Pin, GPIO_PIN_SET);
						break;
					}
					default:
						__NOP;
				}
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
			}
			default: 
				__NOP;
		}
		
	}
}
