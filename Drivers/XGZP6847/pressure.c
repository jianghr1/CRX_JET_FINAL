#include "pressure.h"
#include "cmsis_os.h"
#include "Comm.h"

HAL_StatusTypeDef Pressure_Init(I2C_HandleTypeDef *hi2c)
{
	
    uint8_t cmd = 0x0A; // Sleep Mode + 62.5 ms

    // Start measurement
    return HAL_I2C_Mem_Write(hi2c, PRESSURE_I2C_ADDR, 0x30, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 100);
}

HAL_StatusTypeDef Pressure_Read(I2C_HandleTypeDef *hi2c)
{

    uint8_t pressure_buf[3];
    uint32_t pressure_adc;
	
    uint8_t cmd = 0x0A; // Sleep Mode + 62.5 ms
    // Start measurement
    HAL_I2C_Mem_Write(hi2c, PRESSURE_I2C_ADDR, 0x30, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 100);
    osDelay(20);
    // Read pressure data
    if (HAL_I2C_Mem_Read(hi2c, PRESSURE_I2C_ADDR, 0x06, I2C_MEMADD_SIZE_8BIT, pressure_buf, 3, 100) != HAL_OK)
        return HAL_ERROR;

    pressure_adc = (pressure_buf[0] << 16) | (pressure_buf[1] << 8) | pressure_buf[2];
    if (pressure_adc & 0x800000)
        globalInfo.vac_pressure = ((int32_t)pressure_adc - 16777216) / (float)PRESSURE_K_VALUE;
    else
        globalInfo.vac_pressure = pressure_adc / (float)PRESSURE_K_VALUE;
    return HAL_OK;
}
