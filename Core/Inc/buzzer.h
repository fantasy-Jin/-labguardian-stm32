//
// Created by Jin on 2025/10/12.
//

#ifndef BIG_PROJECT_JIN_BUZZER_H
#define BIG_PROJECT_JIN_BUZZER_H
#include "main.h"

// 初始化（可选）
void Buzzer_Init(void);

// 开启蜂鸣器
void Buzzer_On(void);

// 关闭蜂鸣器
void Buzzer_Off(void);

// 获取蜂鸣器的状态
uint8_t GetBuzzerStatus(void);

// 蜂鸣 n 毫秒（阻塞）
void Buzzer_Beep(uint32_t ms);
#endif //BIG_PROJECT_JIN_BUZZER_H

