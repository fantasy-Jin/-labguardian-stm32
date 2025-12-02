#include "flash_storage.h"
#include <stdio.h>

System_Config_t sys_config;

/**
 * @brief  Read configuration from Flash
 */
void Flash_Read_Config(void)
{
    // Read from Flash address directly
    System_Config_t *pConfig = (System_Config_t *)FLASH_SAVE_ADDR;

    if (pConfig->magic == CONFIG_MAGIC)
    {
        // Valid config found, copy to RAM
        memcpy(&sys_config, pConfig, sizeof(System_Config_t));
        printf("[Flash] Config Loaded: SSID=%s, PID=%s, ID=%s\r\n", sys_config.wifi_ssid, sys_config.onenet_product_id, sys_config.onenet_id);
    }
    else
    {
        // No valid config, set defaults or empty
        memset(&sys_config, 0, sizeof(System_Config_t));
				sys_config.tem_alert = 40;
				sys_config.hum_alert = 80;
        printf("[Flash] No valid config found.\r\n");
    }
}

/**
 * @brief  Write configuration to Flash
 * @param  cfg: Pointer to config structure
 */
void Flash_Write_Config(System_Config_t *cfg)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    uint32_t *pData = (uint32_t *)cfg;
    uint32_t i;

    HAL_FLASH_Unlock();

    // 1. Erase the page
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_SAVE_ADDR;
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        printf("[Flash] Erase Error: %d\r\n", HAL_FLASH_GetError());
        HAL_FLASH_Lock();
        return;
    }

    // 2. Write data (Word by Word)
    cfg->magic = CONFIG_MAGIC; // Ensure magic is set
    for (i = 0; i < sizeof(System_Config_t); i += 4)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_SAVE_ADDR + i, *(pData + i / 4)) != HAL_OK)
        {
            printf("[Flash] Write Error at offset %d\r\n", i);
            break;
        }
    }

    HAL_FLASH_Lock();
    printf("[Flash] Write Success.\r\n");
}
