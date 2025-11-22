#ifndef __SHT30_H
#define __SHT30_H

#include "main.h"

#define SHT30_ADDR 0x44 << 1  // 7-bit to 8-bit

typedef struct {
    float temperature;
    float humidity;
} SHT30_Data_t;

HAL_StatusTypeDef SHT30_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef SHT30_ReadData(I2C_HandleTypeDef *hi2c, SHT30_Data_t *data);

#endif

