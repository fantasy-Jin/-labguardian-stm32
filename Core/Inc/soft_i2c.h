#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#include "stm32f1xx_hal.h"

void Soft_I2C_Init(void);
void Soft_I2C_Start(void);
void Soft_I2C_Stop(void);
void Soft_I2C_Ack(void);
void Soft_I2C_NAck(void);
uint8_t Soft_I2C_Wait_Ack(void);
void Soft_I2C_Send_Byte(uint8_t txd);
uint8_t Soft_I2C_Read_Byte(unsigned char ack);

#endif
