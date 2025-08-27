#include "Sensor.h"
#include "main.h"
#include "cmsis_os.h"
#include "Comm.h"
#include "TMC2209.h"
#include "Jetting.h"
#include "arm_math.h"

extern osThreadId_t jettingTaskHandle;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint32_t last_trigger_time_mx  = 0;
	static uint32_t last_trigger_time_mz1 = 0;
	static uint32_t last_trigger_time_mz2 = 0;
	static uint32_t last_trigger_time_ms1 = 0;
	static uint32_t last_trigger_time_ms2 = 0;
	switch (GPIO_Pin)
	{
		case N_CRC_FAIL_Pin: {
			jettingInfo.status |= 2;
			osThreadFlagsSet(jettingTaskHandle, JETTING_FPGA_REPLY);
			break;
		}
		case JETTING_Pin: {
			jettingInfo.status |= 1;
			osThreadFlagsSet(jettingTaskHandle, JETTING_FPGA_REPLY);
			break;
		}
		case MX_TRIG_Pin:
			if (HAL_GetTick() - last_trigger_time_mx  > 100)
			{
				if (triggerHandler.MX)
				{
					TMC_reset(triggerHandler.MX);
					__HAL_TIM_SET_COUNTER(&htim3, 0);
				}
				last_trigger_time_mx  = HAL_GetTick();
			}
			break;
		case MZ1_TRIG_Pin:
			if (HAL_GetTick() - last_trigger_time_mz1 > 100)
			{
				if (triggerHandler.MZ1)
				{
					TMC_reset(triggerHandler.MZ1);
				}
				last_trigger_time_mz1 = HAL_GetTick();
			}
			break;
		case MZ2_TRIG_Pin:
			if (HAL_GetTick() - last_trigger_time_mz2 > 100)
			{
				if (triggerHandler.MZ2)
				{
					TMC_reset(triggerHandler.MZ2);
				}
				last_trigger_time_mz2 = HAL_GetTick();
			}
			break;
		case MS1_YW_Pin:
			if (HAL_GetTick() - last_trigger_time_ms1 > 100)
			{
				if (triggerHandler.YW1)
				{
					TMC_reset(triggerHandler.YW1);
					triggerHandler.YW1 = 0;
				}
				last_trigger_time_ms1 = HAL_GetTick();
			}
			break;
		case MS2_YW_Pin:
			if (HAL_GetTick() - last_trigger_time_ms2 > 100)
			{
				if (triggerHandler.YW2)
				{
					TMC_reset(triggerHandler.YW2);
					triggerHandler.YW2 = 0;
				}
				last_trigger_time_ms2 = HAL_GetTick();
			}
			break;
		
		default:
			__NOP;
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	float x = (float)HAL_ADC_GetValue(hadc);
	x = 51 * x / (4096 - x);
	x = 1 / (logf(x) / 3965 + 0.0021926f) - 273.15f;
	globalInfo.temperature = x * 10;
}

void StartSensorTask(void *argument) {
	static int32_t integeral = 0;
	
	while(1) {
		HAL_ADC_Start_IT(&hadc1);
		globalInfo.trigger_state.bits.MX  =  HAL_GPIO_ReadPin(MX_TRIG_GPIO_Port , MX_TRIG_Pin );
		globalInfo.trigger_state.bits.MZ1 =  HAL_GPIO_ReadPin(MZ1_TRIG_GPIO_Port, MZ1_TRIG_Pin);
		globalInfo.trigger_state.bits.MZ2 =  HAL_GPIO_ReadPin(MZ2_TRIG_GPIO_Port, MZ2_TRIG_Pin);
		globalInfo.trigger_state.bits.YW1 = !HAL_GPIO_ReadPin(MS1_YW_GPIO_Port,   MS1_YW_Pin  );
		globalInfo.trigger_state.bits.YW2 = !HAL_GPIO_ReadPin(MS2_YW_GPIO_Port,   MS2_YW_Pin  );
		globalInfo.trigger_state.bits.SW1 = !HAL_GPIO_ReadPin(SW1_GPIO_Port,      SW1_Pin     );
		globalInfo.trigger_state.bits.SW2 = !HAL_GPIO_ReadPin(SW2_GPIO_Port,      SW2_Pin     );
		globalInfo.trigger_state.bits.SW3 = !HAL_GPIO_ReadPin(SW3_GPIO_Port,      SW3_Pin     );
		globalInfo.x_encoder_pos = (int16_t)htim3.Instance->CNT * 0.035278f;
		
		if (globalInfo.temperature > -100 && globalInfo.temperature < 1000) {
			int32_t error = globalInfo.targetTemperature - globalInfo.temperature;
			if (error > 1) {
				htim10.Instance->CCR1=1000;
				integeral = 0;
			} else {
				if ((integeral ^ error)>>31) {
					integeral = (integeral * 0.9) + error;
				} else {
					integeral = integeral + error;
				}
				if (integeral > 0x7FFFFF) integeral = 0x7FFFFF;
				if (integeral <-0x7FFFFF) integeral =-0x7FFFFF;
				
				int32_t ccr = error * 5 + (integeral >> 2);
				if (ccr > 800) ccr = 800;
				if (ccr < 0) ccr = 0;
				htim10.Instance->CCR1=ccr;
			}
		}
		
		osDelay(5);
	}
}

