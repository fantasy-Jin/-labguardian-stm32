#ifndef __RELAY_H
#define __RELAY_H

#include "stm32f1xx_hal.h"

// Define relay control pins
#define RELAY1_PIN GPIO_PIN_12
#define RELAY1_GPIO_PORT GPIOB
#define RELAY2_PIN GPIO_PIN_13
#define RELAY2_GPIO_PORT GPIOB

void Relay_Init(void);
void Relay_Control(uint8_t relay_num, uint8_t state);
uint8_t GetRelayState(uint8_t relay_num);
void Relay1_On(void);
void Relay1_Off(void);
void Relay2_On(void);
void Relay2_Off(void);
#endif /* __RELAY_H */
