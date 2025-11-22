/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

extern I2C_HandleTypeDef hi2c1;

extern uint8_t g_mqtt_connect_status;
extern char g_mqtt_status_str[20];

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MQ2_Pin GPIO_PIN_0
#define MQ2_GPIO_Port GPIOA
#define HC_SR501_Pin GPIO_PIN_1
#define HC_SR501_GPIO_Port GPIOA
#define key1_Pin GPIO_PIN_5
#define key1_GPIO_Port GPIOA
#define key2_Pin GPIO_PIN_6
#define key2_GPIO_Port GPIOA
#define key3_Pin GPIO_PIN_7
#define key3_GPIO_Port GPIOA
#define c1_Pin GPIO_PIN_12
#define c1_GPIO_Port GPIOB
#define c2_Pin GPIO_PIN_13
#define c2_GPIO_Port GPIOB
#define door_Pin GPIO_PIN_15
#define door_GPIO_Port GPIOB
#define led_Pin GPIO_PIN_5
#define led_GPIO_Port GPIOB
#define beep_Pin GPIO_PIN_8
#define beep_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
