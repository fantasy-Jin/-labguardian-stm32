//
// Created by Jin on 2025/10/16.
//
#include "led.h"

uint8_t GetLedState(void) {
    return HAL_GPIO_ReadPin(led_GPIO_Port, led_Pin) == GPIO_PIN_RESET;
}

void LedOn(void) {
     HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
}
void LedOff(void) {
    HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
}
