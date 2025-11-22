//
// Created by Jin on 2025/10/12.
//

#ifndef __BH1750_H
#define __BH1750_H

#include "main.h"

#define BH1750_ADDR 0x23 << 1

HAL_StatusTypeDef BH1750_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BH1750_ReadLux(I2C_HandleTypeDef *hi2c, float *lux);

#endif


