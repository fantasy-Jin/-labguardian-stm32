//
// Created by Jin on 2025/10/12.
//

#include "mq2.h"
#include "stdio.h"


void MQ2_ADC_Init(ADC_HandleTypeDef *hadc)
{
    // 通常不需要额外操作，ADC 已由 MX_ADC1_Init() 初始化
}

uint32_t MQ2_ReadRaw(ADC_HandleTypeDef *hadc)
{
    HAL_ADC_Start(hadc);
    if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
    {
        return HAL_ADC_GetValue(hadc);
    }
    return 0;
}

uint32_t MQ2_ReadAverage(ADC_HandleTypeDef *hadc)
{
    uint64_t sum = 0; // 防止溢出
    for (int i = 0; i < SAMPLES; i++)
    {
        sum += MQ2_ReadRaw(hadc);
        HAL_Delay(2); // 小延时，稳定采样
    }
    return (uint32_t)(sum / SAMPLES);
}

float MQ2_ReadVoltage(ADC_HandleTypeDef *hadc)
{
    uint32_t raw = MQ2_ReadAverage(hadc);
    return (raw * 3.3f) / 4096.0f; // 12-bit, Vref=3.3V
}

uint8_t MQ2_ReadPercentage(ADC_HandleTypeDef *hadc)
{
    uint32_t raw = MQ2_ReadAverage(hadc);
    if (raw > 4095) raw = 4095;
    return (uint8_t)((raw * 100U) / 4095U);
}
