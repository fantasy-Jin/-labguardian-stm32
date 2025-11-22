//
// Created by Jin on 2025/10/12.
//
#include "pir_sensor.h"
void PIR_Init(void) {
    // CubeMX 已配置为输入上拉，此处可留空或用于复位逻辑
}

uint8_t PIR_IsHumanDetected(void) {
    return HAL_GPIO_ReadPin(GPIOA, HC_SR501_Pin) == GPIO_PIN_SET;
}
