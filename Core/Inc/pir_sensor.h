//
// Created by Jin on 2025/10/12.
//

#ifndef BIG_PROJECT_JIN_PIR_SENSOR_H
#define BIG_PROJECT_JIN_PIR_SENSOR_H
// 初始化（可选，CubeMX 已配置 GPIO）
#include "main.h"
void PIR_Init(void);

// 检测是否有人（1=检测到，0=未检测到）
uint8_t PIR_IsHumanDetected(void);
#endif //BIG_PROJECT_JIN_PIR_SENSOR_H
