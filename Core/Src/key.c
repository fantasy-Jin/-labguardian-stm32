#include "key.h"

/**
 * @brief  Read key state with debounce
 * @param  GPIOx: GPIO Port
 * @param  GPIO_Pin: GPIO Pin
 * @retval 1 if pressed, 0 if not pressed (assuming active low)
 */
uint8_t Key_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET)
    {
        HAL_Delay(20); // Debounce
        if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET)
        {
            uint16_t time = 0;
            while(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET)
            {
                HAL_Delay(10);
                time++;
                if(time > 200) // 200 * 10ms = 2s Long Press
                {
                    while(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET); // Wait for release
                    return 2; // Long Press
                }
            }
            return 1; // Short Press
        }
    }
    return 0;
}
