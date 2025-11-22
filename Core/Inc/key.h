#ifndef __KEY_H
#define __KEY_H

#include "main.h"

// Define Key States
#define KEY_ON  0
#define KEY_OFF 1

#define KEY_SHORT_PRESS 1
#define KEY_LONG_PRESS  2

// Function Prototypes
uint8_t Key_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif
