#ifndef PRESSURE_H
#define PRESSURE_H

#include "main.h"

#define PRESSURE_I2C_ADDR (0x6D << 1)  // HAL uses 8-bit addressing
#define PRESSURE_K_VALUE 64            // Adjust according to actual sensor range

typedef struct {
    float pressure;     // Unit: Pa
    float temperature;  // Unit: degree C
} Pressure_Data;

HAL_StatusTypeDef Pressure_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef Pressure_Read(I2C_HandleTypeDef *hi2c);

#endif // PRESSURE_H
