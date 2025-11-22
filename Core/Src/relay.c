#include "relay.h"

void Relay_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(RELAY1_GPIO_PORT, RELAY1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RELAY2_GPIO_PORT, RELAY2_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pins : PB12 PB13 */
    GPIO_InitStruct.Pin = RELAY1_PIN | RELAY2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Relay_Control(uint8_t relay_num, uint8_t state)
{
    if (relay_num == 1)
    {
        if (state)
        {
            HAL_GPIO_WritePin(RELAY1_GPIO_PORT, RELAY1_PIN, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(RELAY1_GPIO_PORT, RELAY1_PIN, GPIO_PIN_RESET);
        }
    }
    else if (relay_num == 2)
    {
        if (state)
        {
            HAL_GPIO_WritePin(RELAY2_GPIO_PORT, RELAY2_PIN, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(RELAY2_GPIO_PORT, RELAY2_PIN, GPIO_PIN_RESET);
        }
    }
}
uint8_t GetRelayState(uint8_t relay_num)
{
    if (relay_num == 1)
    {
     return HAL_GPIO_ReadPin(RELAY1_GPIO_PORT,RELAY1_PIN) == GPIO_PIN_SET;
    }
    else if (relay_num == 2)
    {
      return HAL_GPIO_ReadPin(RELAY2_GPIO_PORT,RELAY2_PIN) == GPIO_PIN_SET;
    }
		 return 0;
}
/**
  * @brief  打开继电器1
  * @param  None
  * @retval None
  */
void Relay1_On(void)
{
    HAL_GPIO_WritePin(RELAY1_GPIO_PORT, RELAY1_PIN, GPIO_PIN_SET);
}

/**
  * @brief  关闭继电器1
  * @param  None
  * @retval None
  */
void Relay1_Off(void)
{
    HAL_GPIO_WritePin(RELAY1_GPIO_PORT, RELAY1_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  打开继电器2
  * @param  None
  * @retval None
  */
void Relay2_On(void)
{
    HAL_GPIO_WritePin(RELAY2_GPIO_PORT, RELAY2_PIN, GPIO_PIN_SET);
}

/**
  * @brief  关闭继电器2
  * @param  None
  * @retval None
  */
void Relay2_Off(void)
{
    HAL_GPIO_WritePin(RELAY2_GPIO_PORT, RELAY2_PIN, GPIO_PIN_RESET);
}

