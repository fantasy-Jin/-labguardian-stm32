#include "sht30.h"
#include "soft_i2c.h"

static uint8_t tx_buf[2];
static uint8_t rx_buf[6];

HAL_StatusTypeDef SHT30_Init(I2C_HandleTypeDef *hi2c) {
    tx_buf[0] = 0x30; // Soft reset
    tx_buf[1] = 0xA2;
    Soft_I2C_Start();
    Soft_I2C_Send_Byte(SHT30_ADDR);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    Soft_I2C_Send_Byte(tx_buf[0]);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    Soft_I2C_Send_Byte(tx_buf[1]);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    Soft_I2C_Stop();
    return HAL_OK;
}

HAL_StatusTypeDef SHT30_ReadData(I2C_HandleTypeDef *hi2c, SHT30_Data_t *data) {
    tx_buf[0] = 0x24;  // High repeatability, no clock stretching
    tx_buf[1] = 0x00;
    Soft_I2C_Start();
    Soft_I2C_Send_Byte(SHT30_ADDR);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    Soft_I2C_Send_Byte(tx_buf[0]);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    Soft_I2C_Send_Byte(tx_buf[1]);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }
    Soft_I2C_Stop();

    HAL_Delay(20); // Wait for measurement

    Soft_I2C_Start();
    Soft_I2C_Send_Byte(SHT30_ADDR | 1);
    if (Soft_I2C_Wait_Ack()) {
        Soft_I2C_Stop();
        return HAL_ERROR;
    }

    rx_buf[0] = Soft_I2C_Read_Byte(1);
    rx_buf[1] = Soft_I2C_Read_Byte(1);
    rx_buf[2] = Soft_I2C_Read_Byte(1);
    rx_buf[3] = Soft_I2C_Read_Byte(1);
    rx_buf[4] = Soft_I2C_Read_Byte(1);
    rx_buf[5] = Soft_I2C_Read_Byte(0);
    Soft_I2C_Stop();

    uint16_t temp_raw = (rx_buf[0] << 8) | rx_buf[1];
    uint16_t humi_raw = (rx_buf[3] << 8) | rx_buf[4];

    data->temperature = -45.0f + (175.0f * temp_raw / 65535.0f);
    data->humidity = 100.0f * humi_raw / 65535.0f;

    return HAL_OK;
}
