//
// Created by Jin on 2025/10/12.
//
#include "buzzer.h"

#define BUZZER_PIN GPIO_PIN_8
#define BUZZER_PORT GPIOB

void Buzzer_Init(void) {
    // CubeMX 已配置为推挽输出，此处可留空
}

void Buzzer_On(void) {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void Buzzer_Off(void) {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

void Buzzer_Beep(uint32_t ms) {
    Buzzer_On();
    HAL_Delay(ms);
    Buzzer_Off();
}
// 获取蜂鸣器的状态
uint8_t GetBuzzerStatus(void) {
    return  HAL_GPIO_ReadPin(BUZZER_PORT,BUZZER_PIN) == GPIO_PIN_SET;
}
