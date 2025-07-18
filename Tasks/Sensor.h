#ifndef _SENSOR_H
#define _SENSOR_H
#include "stdint.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void StartSensorTask(void *argument);
#endif//_SENSOR_H
