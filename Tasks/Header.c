#include "Header.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"

extern osThreadId_t defaultTaskHandle;
extern SPI_HandleTypeDef hspi1, hspi4;
extern TIM_HandleTypeDef htim9;
void StartHeaderTask(void *argument) {
	static int32_t target_temperature = 0;
	static int32_t heater_working = 0;
	static int32_t integeral = 0;
	static uint8_t VoltageR;
	static uint32_t flag;
	while(1)
	{
		// Wait Forever Until This Thread Task Notified
		flag = osThreadFlagsWait(ALL_NEW_TASK|ALL_EMG_STOP, osFlagsWaitAny, 20);
		
		if (currentState == GlobalStateEStop || currentState == GlobalStateError)
		{
			heater_working = 0;
		}
		else if (flag & ALL_NEW_TASK) {
			switch(currentIntCommandPtr->code)
			{
				case M120: {
					target_temperature = currentIntCommandPtr->param1 * 10;
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
		if (heater_working) {
			int32_t error = target_temperature - globalInfo.temperature;
			if (error > 50) {
				htim9.Instance->CCR1=1000;
				integeral = 0;
			} else {
				integeral = integeral + error;
				int32_t ccr = error * 5 + (integeral >> 5);
				if (ccr > 800) ccr = 800;
				if (ccr < 0) ccr = 0;
				htim9.Instance->CCR1=ccr;
			}
		} else {
			htim9.Instance->CCMR1=0;
			integeral = 0;
		}
	}
}
