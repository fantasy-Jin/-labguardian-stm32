/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "adc.h"
#include "bh1750.h"
#include "bsp_esp01s.h"
#include "buzzer.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "mq2.h"
#include "pir_sensor.h"
#include "sht30.h"
#include "led.h"
#include "soft_i2c.h"
#include "oled_s.h"
#include "relay.h"
#include "key.h"
#include "flash_storage.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const uint32_t DATA_INTERVAL = 5000; // 数据上传间隔（5秒）
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

void Display_Sensor_Data(void);
void extract_json_id(const char *json, char *id_buf, size_t size);
void Send_Data_to_OneNet(void);
extern const unsigned char wifi_icon_16x16[];
extern const unsigned char mqtt_icon_16x16[];
extern unsigned char security_icon_16x16[];
extern unsigned char blank_icon_16x16[];
uint32_t lastDataSend = 0;
char g_reply_payload[128];
uint8_t g_reply_pending = 0;

// MQTT Topics
char MQTT_TOPIC_POST[128];
char MQTT_TOPIC_SET[128];
char MQTT_TOPIC_REPLY[128];

static int led_value;
static int alarm_value;
static int relay1_value;
static int relay2_value;

static SHT30_Data_t env;
static float lux = 0.0f, gas_v = 0.0f;
static uint8_t human = 0;
static uint8_t buzzer = 0;
static uint8_t led = 0;
static uint8_t security_mode = 0;
static int security_val = 0;

static uint8_t modify_mode = 0; // 0: Normal, 1: Temp, 2: Hum
static int t_alert = 0;
static int h_alert = 0;

char mqtt_payload[512];                     // Global buffer for MQTT publish
char display_buffer[32];                    // Global buffer for display formatting
char modify_buffer[32];                     // Global buffer for modify interface
uint8_t key1_state, key2_state, key3_state; // Key scan states

void Display_Modify_Interface(uint8_t mode)
{
    OLED_ShowString(1, 1, "Modify Alert");

    snprintf(modify_buffer, sizeof(modify_buffer), "%sTemp: %d", (mode == 1 ? ">" : " "), sys_config.tem_alert);
    OLED_ShowString(2, 1, modify_buffer);

    snprintf(modify_buffer, sizeof(modify_buffer), "%sHum : %d", (mode == 2 ? ">" : " "), sys_config.hum_alert);
    OLED_ShowString(3, 1, modify_buffer);

    OLED_ShowString(4, 1, "Save:Lng K1");
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void Build_MQTT_Topics(void)
{
    if (strlen(sys_config.onenet_product_id) > 0 && strlen(sys_config.onenet_id) > 0)
    {
        sprintf(MQTT_TOPIC_POST, "$sys/%s/%s/thing/property/post", sys_config.onenet_product_id, sys_config.onenet_id);
        sprintf(MQTT_TOPIC_SET, "$sys/%s/%s/thing/property/set", sys_config.onenet_product_id, sys_config.onenet_id);
        sprintf(MQTT_TOPIC_REPLY, "$sys/%s/%s/thing/property/set_reply", sys_config.onenet_product_id, sys_config.onenet_id);
    }
    else
    {
        // Fallback to defaults if config is missing
        sprintf(MQTT_TOPIC_POST, "$sys/%s/%s/thing/property/post", MQTT_USERNAME, MQTT_CLIENTID);
        sprintf(MQTT_TOPIC_SET, "$sys/%s/%s/thing/property/set", MQTT_USERNAME, MQTT_CLIENTID);
        sprintf(MQTT_TOPIC_REPLY, "$sys/%s/%s/thing/property/set_reply", MQTT_USERNAME, MQTT_CLIENTID);
    }
    printf("[MQTT] Topics Built:\r\n POST: %s\r\n SET: %s\r\n", MQTT_TOPIC_POST, MQTT_TOPIC_SET);
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC1_Init();
    // MX_I2C1_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_TIM3_Init();
    /* USER CODE BEGIN 2 */
    Soft_I2C_Init();
    SHT30_Init(&hi2c1);
    BH1750_Init(&hi2c1);
    OLED_Init();
    Flash_Read_Config(); // Load config from Flash
    Build_MQTT_Topics(); // Build topics based on config
    printf("=== STM32 init ok ===\r\n");

    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

    WIFI_ESP01S_Init();
    Relay_Init();
    HAL_Delay(2000);
    printf("=== WIFI init ok  ===\r\n");
    if (WIFI_STA_Connect())
    {
        if (wifi_connected_flag)
        {
            OLED_Show16x16Icon(112, 48, wifi_icon_16x16);
        }
        if (WIFI_MQTT_Connect())
        {
            if (mqtt_connected_flag)
            {
                OLED_Show16x16Icon(90, 48, mqtt_icon_16x16);
            }
            WIFI_MQTT_Subscribe(MQTT_TOPIC_SET, 0);
            HAL_Delay(1000);
            // Send_Data_to_OneNet();
        }
    }

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1)
    {
        // 1. Process any incoming data from WiFi module
        Process_WIFI_RX();

        // 2. Send any pending MQTT reply
        if (g_reply_pending)
        {
            printf("[OneNet REPLY] Sending: %s\r\n", g_reply_payload);
            WIFI_MQTT_Publish(MQTT_TOPIC_REPLY, g_reply_payload, 0);
            g_reply_pending = 0;
            memset(g_reply_payload, 0, sizeof(g_reply_payload));
        }

        // 3. Send periodic sensor data
        if (HAL_GetTick() - lastDataSend >= DATA_INTERVAL)
        {
            lastDataSend = HAL_GetTick();
            Send_Data_to_OneNet();
            if (modify_mode == 0)
                Display_Sensor_Data();
        }

        // 4. Handle MQTT disconnections
        if (!mqtt_connected_flag)
        {
            static uint32_t lastReconnect = 0;
            if (HAL_GetTick() - lastReconnect > 10000)
            {
                lastReconnect = HAL_GetTick();
                printf("[MQTT] Lost connection, retrying...\r\n");
                WIFI_MQTT_Connect();
            }
        }
        else
        {
        }

        // 5. Security Mode & Config Logic
        key1_state = Key_Scan(key1_GPIO_Port, key1_Pin);
        key2_state = Key_Scan(key2_GPIO_Port, key2_Pin);
        key3_state = Key_Scan(key3_GPIO_Port, key3_Pin);

        if (modify_mode == 0)
        {
            // Normal Mode
            if (key1_state == KEY_SHORT_PRESS) // Enable Security
            {
                security_mode = 1;
                OLED_Show16x16Icon(0, 48, security_icon_16x16); // Show icon at bottom left
                printf("Security Mode ON\r\n");
            }
            else if (key1_state == KEY_LONG_PRESS) // Enter Config Mode
            {
                printf("Enter Config Mode...\r\n");
                OLED_Clear();
                OLED_ShowString(1, 1, "Config Mode");
                OLED_ShowString(2, 1, "SSID:STM32_Cfg");
                OLED_ShowString(3, 1, "IP:192.168.4.1");
                WIFI_Start_AP("STM32_Config", "12345678");
                WIFI_Start_Server(8080);
                while (1)
                { // Stuck in config mode until reboot
                    Process_WIFI_RX();
                    HAL_Delay(10);
                }
            }

            if (key2_state == KEY_SHORT_PRESS) // Disable Security
            {
                security_mode = 0;
                OLED_Show16x16Icon(0, 48, blank_icon_16x16); // Clear icon
                Buzzer_Off();                                // Stop alarm if ringing
                printf("Security Mode OFF\r\n");
            }

            if (key3_state == KEY_LONG_PRESS) // Enter Alert Modify Mode
            {
                modify_mode = 1;
                OLED_Clear();
                printf("Enter Alert Modify Mode\r\n");
            }

            // Alert Logic
            if (env.temperature > sys_config.tem_alert)
            {
                // Simple Alert Trigger
                if (GetBuzzerStatus() == 0)
                { // 温度超出的操作
                    // Buzzer_On();

                    printf("temp=%d, tem_alert=%d\r\n",
                           (int)env.temperature,
                           sys_config.tem_alert);
                }
            }
            if (env.humidity > sys_config.hum_alert)
            {
                if (GetBuzzerStatus() == 0)
                {
                    // 湿度超出的操作
                    printf("hum=%d, hum_alert=%d\r\n",
                           (int)env.humidity,
                           sys_config.hum_alert);
                }
            }
        }
        else
        {
            // Modify Mode
            if (key1_state == KEY_SHORT_PRESS)
            {
                if (modify_mode == 1)
                {
                    if (sys_config.tem_alert < 100)
                        sys_config.tem_alert++;
                }
                else
                {
                    if (sys_config.hum_alert < 100)
                        sys_config.hum_alert++;
                }
            }
            else if (key1_state == KEY_LONG_PRESS)
            {
                Flash_Write_Config(&sys_config);
                modify_mode = 0;
                OLED_Clear();
                OLED_ShowString(1, 1, "Saved!");
                HAL_Delay(1000);
                OLED_Clear();
                // Restore status icons
                if (wifi_connected_flag)
                    OLED_Show16x16Icon(112, 48, wifi_icon_16x16);
                if (mqtt_connected_flag)
                    OLED_Show16x16Icon(90, 48, mqtt_icon_16x16);
                if (security_mode)
                    OLED_Show16x16Icon(0, 48, security_icon_16x16);
                Display_Sensor_Data(); // Immediately refresh display
                lastDataSend = 0;      // Force immediate display update
            }

            if (key2_state == KEY_SHORT_PRESS)
            {
                if (modify_mode == 1)
                {
                    if (sys_config.tem_alert > 1)
                        sys_config.tem_alert--;
                }
                else
                {
                    if (sys_config.hum_alert > 1)
                        sys_config.hum_alert--;
                }
            }

            if (key3_state == KEY_SHORT_PRESS)
            {
                modify_mode = (modify_mode == 1) ? 2 : 1;
            }

            Display_Modify_Interface(modify_mode);
        }

        if (security_mode)
        {
            // Check Door Sensor (Active High or Low? Assuming High trigger based on typical modules, but checking logic)
            // Usually Door sensor: Magnet near = Low, Magnet far = High. Alarm on Open (High).
            // Check PIR: High = Motion.
            if (HAL_GPIO_ReadPin(door_GPIO_Port, door_Pin) == GPIO_PIN_SET || PIR_IsHumanDetected())
            {
                Buzzer_On();
                printf("[DEBUG] isBuzzer_On door_value: %d, Human: %d\r\n", HAL_GPIO_ReadPin(door_GPIO_Port, door_Pin), PIR_IsHumanDetected());
            }
            else
            {
                // Optional: Auto turn off buzzer if sensors clear? Or latch?
                // User request says "if reaction then alarm". Usually implies latching or continuous while active.
                // Let's keep it simple: if active -> beep. If not active -> silence (unless latched, but simple is better for now).
                // Wait, if I just turn it on, it stays on. I should probably allow it to turn off if sensors are clear,
                // OR require manual reset. "If reaction then alarm" -> usually alarm stays until reset.
                // But for simple driver:
                // Let's make it: Sensor Active -> Buzzer On. Sensor Inactive -> Buzzer Off (or stay on?).
                // Let's stick to: If sensors active, Buzzer On. If sensors inactive, Buzzer Off (to avoid stuck alarm).
                // BUT, usually alarms latch.
                // Let's try non-latching first for simplicity, or maybe latching until Key2.
                // "key2 click close security mode" -> implies Key2 stops the alarm too.
                // So I won't turn OFF buzzer here automatically.
                // But if I don't turn it off, it will never stop even if I close the door.
                // I'll leave it as: Trigger -> On. Key2 -> Off.
            }
        }

        HAL_Delay(100);

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

// Helper function to escape quotes in a JSON string for AT commands
void escape_json_quotes(const char *input, char *output, size_t out_size)
{
    size_t i = 0, j = 0;
    while (input[i] != '\0' && j < out_size - 1)
    {
        if (input[i] == '"')
        {
            if (j < out_size - 2)
            {
                output[j++] = '\\';
                output[j++] = '"';
            }
            else
            {
                break; // Not enough space in output buffer
            }
        }
        else
        {
            output[j++] = input[i];
        }
        i++;
    }
    output[j] = '\0';
}

void Send_Data_to_OneNet(void)
{

    if (SHT30_ReadData(&hi2c1, &env) != HAL_OK)
    {
        env.temperature = 0;
        env.humidity = 0;
    }
    if (BH1750_ReadLux(&hi2c1, &lux) != HAL_OK)
        lux = 1;
    human = PIR_IsHumanDetected();
    gas_v = MQ2_ReadPercentage(&hadc1);
    buzzer = GetBuzzerStatus();
    led = GetLedState();

    snprintf(mqtt_payload, sizeof(mqtt_payload),
             "{\"id\":\"123\",\"params\":{"
             "\"Temp\":{\"value\":%d},"
             "\"Hum\":{\"value\":%d},"
             "\"Alarm\":{\"value\":%s},"
             "\"LED\":{\"value\":%s},"
             "\"gas\":{\"value\":%d},"
             "\"lux\":{\"value\":%d},"
             "\"human\":{\"value\":%s},"
             "\"door\":{\"value\":%s},"
             "\"RELAY1\":{\"value\":%s},"
             "\"RELAY2\":{\"value\":%s},"
             "\"security\":{\"value\":%s},"
             "\"tem_alert\":{\"value\":%d},"
             "\"hum_alert\":{\"value\":%d}"
             "}}",
             (int)env.temperature,
             (int)env.humidity,
             (buzzer ? "true" : "false"),
             (led ? "true" : "false"),
             (int)gas_v,
             (int)lux,
             (human ? "true" : "false"),
             HAL_GPIO_ReadPin(door_GPIO_Port, door_Pin) == GPIO_PIN_SET ? "true" : "false",
             (GetRelayState(1) ? "true" : "false"),
             (GetRelayState(2) ? "true" : "false"),
             security_mode ? "true" : "false",
             sys_config.tem_alert,
             sys_config.hum_alert);

    WIFI_MQTT_Publish(MQTT_TOPIC_POST, mqtt_payload, 0);
    printf("Payload: %s\r\n", mqtt_payload);
    // OLED_ShowNum(1, 1, (int)env.temperature, 5);
    // OLED_ShowNum(2, 1, (int)env.humidity, 5);
}

int get_param_value(const char *params_content, const char *key)
{
    char search_key[48];
    sprintf(search_key, "\"%s\":", key);

    const char *key_ptr = strstr(params_content, search_key);
    if (!key_ptr)
    {
        return -1; // Key not found
    }

    const char *value_ptr = key_ptr + strlen(search_key);

    while (*value_ptr == ' ')
    {
        value_ptr++;
    }

    if (strncmp(value_ptr, "true", 4) == 0)
    {
        return 1;
    }
    if (strncmp(value_ptr, "false", 5) == 0)
    {
        return 0;
    }

    if ((*value_ptr >= '0' && *value_ptr <= '9') || *value_ptr == '-')
    {
        return atoi(value_ptr);
    }

    return -1;
}

void Process_OneNet_Command(MQTT_MESSAGE *msg)
{
    if (strcmp(msg->topic, MQTT_TOPIC_SET) != 0)
        return;

    char *json_data = msg->payload;
    printf("[OneNet CMD] Received: %s\r\n", json_data);

    char *params_start = strstr(json_data, "{");
    if (!params_start)
    {
        return;
    }

    char *params_end = strrchr(params_start, '}');
    if (!params_end)
    {
        return;
    }

    *params_end = '\0';

    led_value = get_param_value(params_start, "LED");
    alarm_value = get_param_value(params_start, "Alarm");
    relay1_value = get_param_value(params_start, "RELAY1");
    relay2_value = get_param_value(params_start, "RELAY2");
    relay2_value = get_param_value(params_start, "RELAY2");
    security_val = get_param_value(params_start, "security");
    t_alert = get_param_value(params_start, "tem_alert");
    h_alert = get_param_value(params_start, "hum_alert");

    *params_end = '}';

    // printf("[DEBUG] led_value: %d, security: %d\r\n", led_value, security_val);

    if (security_val >= 0)
    {
        if (security_val == 1)
        {
            security_mode = 1;
            OLED_Show16x16Icon(0, 48, security_icon_16x16);
        }
        else
        {
            security_mode = 0;
            OLED_Show16x16Icon(0, 48, blank_icon_16x16);
            Buzzer_Off();
        }
    }

    char id_buf[32] = "0";
    extract_json_id(json_data, id_buf, sizeof(id_buf));

    if (alarm_value >= 0)
    {
        if (alarm_value == 1)
        {
            Buzzer_On();
            printf("beep on\r\n");
        }
        else
        {
            Buzzer_Off();
            printf("beep off\r\n");
        }
    }

    if (led_value >= 0)
    {
        if (led_value == 1)
        {
            LedOn();
            // printf("LED on\r\n");
        }
        else
        {
            LedOff();
            // printf("LED off\r\n");
        }
    }
    // ProcessRelayCommands(relay1_value,relay2_value);
    if (relay1_value >= 0)
    {
        if (relay1_value == 1)
        {
            Relay1_On();
        }
        else
        {
            Relay1_Off();
        }
        // Relay_Control(1, relay1_value);
    }

    if (relay2_value >= 0)
    {
        if (relay2_value == 1)
        {
            Relay2_On();
        }
        else
        {
            Relay2_Off();
        }
    }

    if (t_alert > 0)
    {
        // Limit to 1-100 range for OneNet platform compatibility
        if (t_alert >= 1 && t_alert <= 100)
        {
            sys_config.tem_alert = t_alert;
            printf("Set tem_alert: %d\r\n", t_alert);
        }
        else
        {
            printf("Invalid tem_alert: %d (must be 1-100)\r\n", t_alert);
        }
    }
    if (h_alert > 0)
    {
        // Limit to 1-100 range for OneNet platform compatibility
        if (h_alert >= 1 && h_alert <= 100)
        {
            sys_config.hum_alert = h_alert;
            printf("Set hum_alert: %d\r\n", h_alert);
        }
        else
        {
            printf("Invalid hum_alert: %d (must be 1-100)\r\n", h_alert);
        }
    }

    char reply_payload[128];
    snprintf(reply_payload, sizeof(reply_payload),
             "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}", id_buf);

    // DECOUPLED SEND: Set flag and copy data instead of publishing directly
    strcpy(g_reply_payload, reply_payload);
    g_reply_pending = 1;
}

void extract_json_id(const char *json, char *id_buf, size_t size)
{
    const char *p = strstr(json, "\"id\"");
    if (!p)
        return;
    p = strchr(p, ':');
    if (!p)
        return;
    p++;
    while (*p && (*p == ' ' || *p == '"'))
        p++;
    size_t i = 0;
    while (*p && *p != '"' && *p != ',' && i < size - 1)
        id_buf[i++] = *p++;
    id_buf[i] = '\0';
}
void Display_Sensor_Data(void)
{
    // Display temperature (pad to 13 chars to clear modify mode text)
    snprintf(display_buffer, sizeof(display_buffer), "T:%d.C        ", (int)env.temperature);
    OLED_ShowString(1, 1, display_buffer);

    // Display humidity
    snprintf(display_buffer, sizeof(display_buffer), "H:%d%%         ", (int)env.humidity);
    OLED_ShowString(2, 1, display_buffer);

    // Display light
    snprintf(display_buffer, sizeof(display_buffer), "L:%dLux       ", (int)lux);
    OLED_ShowString(3, 1, display_buffer);

    // Clear Line 4 - 11 chars to remove "Save:Lng K1" (88px, still preserves icons at X=90+)
    OLED_ShowString(4, 1, "           ");

    // Restore status icons after clearing
    if (security_mode)
        OLED_Show16x16Icon(0, 48, security_icon_16x16);
    if (wifi_connected_flag)
        OLED_Show16x16Icon(112, 48, wifi_icon_16x16);
    if (mqtt_connected_flag)
        OLED_Show16x16Icon(90, 48, mqtt_icon_16x16);
}

void extract_json_string(const char *json, const char *key, char *out_buf, size_t max_len)
{
    char search[32];
    sprintf(search, "\"%s\":\"", key);
    char *start = strstr(json, search);
    if (start)
    {
        start += strlen(search);
        char *end = strchr(start, '"');
        if (end)
        {
            size_t len = end - start;
            if (len >= max_len)
                len = max_len - 1;
            strncpy(out_buf, start, len);
            out_buf[len] = '\0';
        }
    }
}

void Process_Config_Data(char *data)
{
    printf("[Config] Received: %s\r\n", data);
    // Expected: {"ssid":"...","pwd":"...","id":"...","key":"..."}

    char ssid[32] = {0};
    char pwd[64] = {0};
    char pid[32] = {0};
    char id[32] = {0};
    char key[256] = {0};

    extract_json_string(data, "ssid", ssid, sizeof(ssid));
    extract_json_string(data, "pwd", pwd, sizeof(pwd));
    extract_json_string(data, "pid", pid, sizeof(pid));
    extract_json_string(data, "id", id, sizeof(id));
    extract_json_string(data, "key", key, sizeof(key));

    if (strlen(ssid) > 0 && strlen(pwd) > 0)
    {
        printf("[Config] Saving...\r\n");
        memset(&sys_config, 0, sizeof(sys_config));
        strcpy(sys_config.wifi_ssid, ssid);
        strcpy(sys_config.wifi_pwd, pwd);
        strcpy(sys_config.onenet_product_id, pid);
        strcpy(sys_config.onenet_id, id);
        strcpy(sys_config.onenet_key, key);

        Flash_Write_Config(&sys_config);

        OLED_Clear();
        OLED_ShowString(1, 1, "Config Saved!");
        OLED_ShowString(2, 1, "Rebooting...");
        HAL_Delay(2000);
        NVIC_SystemReset();
    }
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
