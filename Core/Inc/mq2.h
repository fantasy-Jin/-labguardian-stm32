//
// Created by Jin on 2025/10/12.
//

#ifndef BIG_PROJECT_JIN_MQ2_H
#define BIG_PROJECT_JIN_MQ2_H

#include "main.h"  // 包含 HAL 和 MCU 定义

#define MQ2_ADC_CHANNEL     ADC_CHANNEL_0      // PA5 对应 ADC1_IN5
#define SAMPLES             10                 // 采样次数

void MQ2_ADC_Init(ADC_HandleTypeDef *hadc);
uint32_t MQ2_ReadRaw(ADC_HandleTypeDef *hadc);
uint32_t MQ2_ReadAverage(ADC_HandleTypeDef *hadc);
float MQ2_ReadVoltage(ADC_HandleTypeDef *hadc);
uint8_t MQ2_ReadPercentage(ADC_HandleTypeDef *hadc);


#endif //BIG_PROJECT_JIN_MQ2_H

