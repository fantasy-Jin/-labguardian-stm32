#include "bh1750.h"
#include "soft_i2c.h"

static uint8_t cmd = 0x10; // Continuous H-resolution mode
static uint8_t rx_buf[2];

HAL_StatusTypeDef BH1750_Init(I2C_HandleTypeDef *hi2c) {
    Soft_I2C_Start();
    Soft_I2C_Send_Byte(BH1750_ADDR);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    Soft_I2C_Send_Byte(cmd);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    Soft_I2C_Stop();
    return HAL_OK;
}

HAL_StatusTypeDef BH1750_ReadLux(I2C_HandleTypeDef *hi2c, float *lux) {
    Soft_I2C_Start();
    Soft_I2C_Send_Byte(BH1750_ADDR | 1);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    rx_buf[0] = Soft_I2C_Read_Byte(1);
    rx_buf[1] = Soft_I2C_Read_Byte(0);
    Soft_I2C_Stop();

    uint16_t raw = (rx_buf[0] << 8) | rx_buf[1];
    *lux = raw / 1.2f; // Conversion factor
    return HAL_OK;
}
