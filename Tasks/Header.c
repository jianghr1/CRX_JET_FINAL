#include "Header.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"

extern osThreadId_t defaultTaskHandle;
extern SPI_HandleTypeDef hspi1, hspi4;
extern TIM_HandleTypeDef htim9;
void StartHeaderTask(void *argument) {
	static uint8_t VoltageR;
	static uint32_t flag;
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		flag = osThreadFlagsWait(ALL_NEW_TASK|ALL_EMG_STOP, osFlagsWaitAny, osWaitForever);
	
		switch(currentIntCommandPtr->code)
		{
			case M120: {
				globalInfo.targetTemperature = currentIntCommandPtr->param1 * 10;
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M121: {
				osDelay(1000);
				osThreadFlagsSet(defaultTaskHandle, MAIN_TASK_CPLT);
				break;
			}
			case M122: {
				VoltageR = currentIntCommandPtr->param1 * 9.7513e-3f + 2.5076f;
				switch (currentIntCommandPtr->param1)
				{
					case 0: {
						HAL_GPIO_WritePin(VSEL_A_GPIO_Port, VSEL_A_Pin, GPIO_PIN_RESET);
						HAL_SPI_Transmit(&hspi4, &VoltageR, 8, 10);
						HAL_GPIO_WritePin(VSEL_A_GPIO_Port, VSEL_A_Pin, GPIO_PIN_SET);
						break;
					}
					case 1: {
						HAL_GPIO_WritePin(VSEL_B_GPIO_Port, VSEL_B_Pin, GPIO_PIN_RESET);
						HAL_SPI_Transmit(&hspi4, &VoltageR, 8, 10);
						HAL_GPIO_WritePin(VSEL_B_GPIO_Port, VSEL_B_Pin, GPIO_PIN_SET);
						break;
					}
					case 2: {
						HAL_GPIO_WritePin(VSEL_C_GPIO_Port, VSEL_C_Pin, GPIO_PIN_RESET);
						HAL_SPI_Transmit(&hspi1, &VoltageR, 8, 10);
						HAL_GPIO_WritePin(VSEL_C_GPIO_Port, VSEL_C_Pin, GPIO_PIN_SET);
						break;
					}
					case 3: {
						HAL_GPIO_WritePin(VSEL_D_GPIO_Port, VSEL_D_Pin, GPIO_PIN_RESET);
						HAL_SPI_Transmit(&hspi1, &VoltageR, 8, 10);
						HAL_GPIO_WritePin(VSEL_D_GPIO_Port, VSEL_D_Pin, GPIO_PIN_SET);
						break;
					}
					default:
						__NOP;
				}
			}
			default: 
				__NOP;
		}
		
	}
}
