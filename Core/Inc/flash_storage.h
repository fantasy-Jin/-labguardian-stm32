#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#include "main.h"
#include <string.h>

// STM32F103C8T6 has 64KB Flash. Page size 1KB.
// Last Page (Page 63) Start Address: 0x0800FC00
#define FLASH_SAVE_ADDR  0x0800FC00
#define CONFIG_MAGIC     0x5AA51234

typedef struct {
    uint32_t magic;           // Magic number to verify validity
    char wifi_ssid[32];
    char wifi_pwd[64];
    char onenet_product_id[32];
    char onenet_id[32];
    char onenet_key[256]; // Increased size for long tokens
} System_Config_t;

extern System_Config_t sys_config;

void Flash_Read_Config(void);
void Flash_Write_Config(System_Config_t *cfg);

#endif
