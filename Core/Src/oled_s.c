#include "OLED_Font.h"
#include "gpio.h"

/*-----------------------------------------------------------
  OLED I2C脚定义与操作宏
-----------------------------------------------------------*/
#define OLED_W_SCL(x)	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define OLED_W_SDA(x)	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_6
#define I2C_SDA_Pin GPIO_PIN_7

/*-----------------------------------------------------------
  初始化I2C引脚
-----------------------------------------------------------*/
void OLED_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = I2C_SCL_Pin | I2C_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    OLED_W_SCL(GPIO_PIN_SET);
    OLED_W_SDA(GPIO_PIN_SET);
}

/*-----------------------------------------------------------
  I2C起始信号
-----------------------------------------------------------*/
void OLED_I2C_Start(void)
{
    OLED_W_SDA(GPIO_PIN_SET);
    OLED_W_SCL(GPIO_PIN_SET);
    OLED_W_SDA(GPIO_PIN_RESET);
    OLED_W_SCL(GPIO_PIN_RESET);
}

/*-----------------------------------------------------------
  I2C停止信号
-----------------------------------------------------------*/
void OLED_I2C_Stop(void)
{
    OLED_W_SDA(GPIO_PIN_RESET);
    OLED_W_SCL(GPIO_PIN_SET);
    OLED_W_SDA(GPIO_PIN_SET);
}

/*-----------------------------------------------------------
  I2C发送一个字节
-----------------------------------------------------------*/
void OLED_I2C_SendByte(uint8_t Byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_W_SDA((Byte & (0x80 >> i)) != 0);
        OLED_W_SCL(GPIO_PIN_SET);
        OLED_W_SCL(GPIO_PIN_RESET);
    }
    OLED_W_SCL(GPIO_PIN_SET); // 等待应答
    OLED_W_SCL(GPIO_PIN_RESET);
}

/*-----------------------------------------------------------
  写入命令
-----------------------------------------------------------*/
void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);   // 器件地址
    OLED_I2C_SendByte(0x00);   // 写命令
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

/*-----------------------------------------------------------
  写入数据
-----------------------------------------------------------*/
void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);   // 器件地址
    OLED_I2C_SendByte(0x40);   // 写数据
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}

/*-----------------------------------------------------------
  设置光标位置
-----------------------------------------------------------*/
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                    // 设置页地址（Y）
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));    // 设置列高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));           // 设置列低4位
}

/*-----------------------------------------------------------
  清屏
-----------------------------------------------------------*/
void OLED_Clear(void)
{
    for (uint8_t j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for (uint8_t i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

/*-----------------------------------------------------------
  显示单个字符（8x16）
-----------------------------------------------------------*/
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t c = Char - ' ';
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8 + 2);
    for (uint8_t i = 0; i < 8; i++)
        OLED_WriteData(OLED_F8x16[c][i]);

    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8 + 2);
    for (uint8_t i = 0; i < 8; i++)
        OLED_WriteData(OLED_F8x16[c][i + 8]);
}


/*-----------------------------------------------------------
  显示字符串
-----------------------------------------------------------*/
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    for (uint8_t i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/*-----------------------------------------------------------
  幂运算（用于数值显示）
-----------------------------------------------------------*/
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
        Result *= X;
    return Result;
}

/*-----------------------------------------------------------
  显示无符号数字
-----------------------------------------------------------*/
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    for (uint8_t i = 0; i < Length; i++)
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
}

/*-----------------------------------------------------------
  显示有符号数字
-----------------------------------------------------------*/
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint32_t num;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        num = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        num = -Number;
    }
    for (uint8_t i = 0; i < Length; i++)
        OLED_ShowChar(Line, Column + i + 1, num / OLED_Pow(10, Length - i - 1) % 10 + '0');
}

/*-----------------------------------------------------------
  显示十六进制数
-----------------------------------------------------------*/
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t val;
    for (uint8_t i = 0; i < Length; i++)
    {
        val = Number / OLED_Pow(16, Length - i - 1) % 16;
        OLED_ShowChar(Line, Column + i, val < 10 ? val + '0' : val - 10 + 'A');
    }
}

/*-----------------------------------------------------------
  显示二进制数
-----------------------------------------------------------*/
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    for (uint8_t i = 0; i < Length; i++)
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
}

/*-----------------------------------------------------------
  显示 16x16 图标（如 WiFi 图标）
-----------------------------------------------------------*/
void OLED_Show16x16Icon(uint8_t X, uint8_t Y, const unsigned char *Icon)
{
    uint8_t col, row;
    uint8_t byte_page0, byte_page1;
    uint8_t byte_index, bit_index, icon_byte;

    for (col = 0; col < 16; col++)
    {
        byte_page0 = 0;
        byte_page1 = 0;

        for (row = 0; row < 8; row++)
        {
            byte_index = col / 8;
            bit_index = 7 - (col % 8);
            icon_byte = Icon[row * 2 + byte_index];

            if ((icon_byte >> bit_index) & 0x01)
                byte_page0 |= (0x01 << row);
        }

        for (row = 8; row < 16; row++)
        {
            byte_index = col / 8;
            bit_index = 7 - (col % 8);
            icon_byte = Icon[row * 2 + byte_index];

            if ((icon_byte >> bit_index) & 0x01)
                byte_page1 |= (0x01 << (row - 8));
        }

        OLED_SetCursor(Y / 8, X + col);
        OLED_WriteData(byte_page0);
        OLED_SetCursor(Y / 8 + 1, X + col);
        OLED_WriteData(byte_page1);
    }
}

/*-----------------------------------------------------------
  OLED初始化
-----------------------------------------------------------*/
void OLED_Init(void)
{
    for (volatile uint32_t i = 0; i < 1000000; i++); // 延时等待上电稳定

    OLED_I2C_Init();

    OLED_WriteCommand(0xAE); // 关闭显示
    OLED_WriteCommand(0xD5); OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3); OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0xDA); OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9); OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB); OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0x8D); OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF); // 打开显示

    OLED_Clear();
}
