#include "soft_i2c.h"

#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_6
#define I2C_SDA_Pin GPIO_PIN_7

#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(uint32_t)8<<28;}
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(uint32_t)3<<28;}

#define I2C_SCL_1() HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET)
#define I2C_SCL_0() HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET)

#define I2C_SDA_1() HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET)
#define I2C_SDA_0() HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_RESET)

#define READ_SDA() HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin)

void Soft_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = I2C_SCL_Pin | I2C_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    I2C_SCL_1();
    I2C_SDA_1();
}

static void delay_us(uint32_t us)
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 1000000 * us);
    while (delay--);
}

void Soft_I2C_Start(void)
{
    __disable_irq();
    SDA_OUT();
    I2C_SDA_1();
    I2C_SCL_1();
    delay_us(4);
    I2C_SDA_0();
    delay_us(4);
    I2C_SCL_0();
    __enable_irq();
}

void Soft_I2C_Stop(void)
{
    __disable_irq();
    SDA_OUT();
    I2C_SCL_0();
    I2C_SDA_0();
    delay_us(4);
    I2C_SCL_1();
    I2C_SDA_1();
    delay_us(4);
    __enable_irq();
}

uint8_t Soft_I2C_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;
    SDA_IN();
    I2C_SDA_1();
    delay_us(1);
    I2C_SCL_1();
    delay_us(1);
    while (READ_SDA())
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            Soft_I2C_Stop();
            return 1;
        }
    }
    I2C_SCL_0();
    return 0;
}

void Soft_I2C_Ack(void)
{
    I2C_SCL_0();
    SDA_OUT();
    I2C_SDA_0();
    delay_us(2);
    I2C_SCL_1();
    delay_us(2);
    I2C_SCL_0();
}

void Soft_I2C_NAck(void)
{
    I2C_SCL_0();
    SDA_OUT();
    I2C_SDA_1();
    delay_us(2);
    I2C_SCL_1();
    delay_us(2);
    I2C_SCL_0();
}

void Soft_I2C_Send_Byte(uint8_t txd)
{
    uint8_t t;
    SDA_OUT();
    I2C_SCL_0();
    __disable_irq();
    for (t = 0; t < 8; t++)
    {
        if ((txd & 0x80) >> 7)
            I2C_SDA_1();
        else
            I2C_SDA_0();
        txd <<= 1;
        delay_us(2);
        I2C_SCL_1();
        delay_us(2);
        I2C_SCL_0();
        delay_us(2);
    }
    __enable_irq();
}

uint8_t Soft_I2C_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();
    __disable_irq();
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_0();
        delay_us(2);
        I2C_SCL_1();
        receive <<= 1;
        if (READ_SDA())
            receive++;
        delay_us(1);
    }
    __enable_irq();
    if (!ack)
        Soft_I2C_NAck();
    else
        Soft_I2C_Ack();
    return receive;
}
