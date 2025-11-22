#include "bsp_esp01s.h"
#include "flash_storage.h"

#include <stdlib.h>

#include "string.h"
#include "stdio.h"
#include "usart.h"

// ?? static��??
uint8_t WIFI_RX_BUFFER[WIFI_RX_BUFFER_SIZE] = {0};
volatile uint16_t WIFI_RX_LEN = 0;
volatile uint8_t WIFI_RX_FLAG = 0;

uint8_t wifi_connected_flag = 0;
uint8_t mqtt_connected_flag = 0;
extern void Process_OneNet_Command(MQTT_MESSAGE *msg);
// ?
void Clear_WIFI_RX_Buffer(void)
{
    memset(WIFI_RX_BUFFER, 0, sizeof(WIFI_RX_BUFFER));
    WIFI_RX_LEN = 0;
    WIFI_RX_FLAG = 0;
}

// ?
void WIFI_USART_Send_String(const char *str)
{
#ifdef ESP01S_DEBUG
    printf("TX: %s", str);
#endif
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

//  AT ?????
uint8_t WIFI_Send_AT_Cmd(const char *cmd, const char *ack, uint32_t timeout_ms, uint8_t retry)
{
    for (uint8_t i = 0; i < retry; i++)
    {
        WIFI_USART_Send_String(cmd);
        for (uint32_t t = 0; t < timeout_ms / 10; t++)
        {
            HAL_Delay(10);
            if (WIFI_RX_FLAG)
            {
                WIFI_RX_FLAG = 0;
#ifdef ESP01S_DEBUG
                printf("RX: %s", WIFI_RX_BUFFER);
#endif
                if (strstr((char*)WIFI_RX_BUFFER, ack))
                {
                    Clear_WIFI_RX_Buffer();
                    return 1;
                }
                // ?????
                // ??? AT ?
                Clear_WIFI_RX_Buffer();
            }
        }
    }
    return 0;
}

// ?
void WIFI_ESP01S_Init(void)
{
    WIFI_Send_AT_Cmd("AT+RST\r\n", "ready", 1000,2); // ? 1 ? ready
    HAL_Delay(1000); // ??
    Clear_WIFI_RX_Buffer();
    HAL_UART_Receive_IT(&huart2, &WIFI_RX_BUFFER[WIFI_RX_LEN], 1);
}

//  WiFi ??
uint8_t WIFI_Set_Mode(WIFI_WORK_MODE mode)
{
    char cmd[32];
    sprintf(cmd, "AT+CWMODE=%d\r\n", mode);
    return WIFI_Send_AT_Cmd(cmd, "OK", 100, 3);
}

// Start AP Mode
uint8_t WIFI_Start_AP(const char *ssid, const char *pwd)
{
    if (!WIFI_Send_AT_Cmd("AT+CWMODE=2\r\n", "OK", 500, 2)) return 0;
    
    char cmd[128];
    sprintf(cmd, "AT+CWSAP=\"%s\",\"%s\",5,3\r\n", ssid, pwd);
    return WIFI_Send_AT_Cmd(cmd, "OK", 2000, 2);
}

// Start TCP Server
uint8_t WIFI_Start_Server(uint16_t port)
{
    if (!WIFI_Send_AT_Cmd("AT+CIPMUX=1\r\n", "OK", 500, 2)) return 0;
    
    char cmd[64];
    sprintf(cmd, "AT+CIPSERVER=1,%d\r\n", port);
    return WIFI_Send_AT_Cmd(cmd, "OK", 1000, 2);
}

// STA ??
uint8_t WIFI_STA_Connect(void)
{
    if (!WIFI_Send_AT_Cmd("AT\r\n", "OK", 50, 2)) return 0;

    if (!WIFI_Send_AT_Cmd("ATE0\r\n", "OK", 50, 2)) return 0; // ??

    if (!WIFI_Set_Mode(WIFI_MODE_STA)) return 0;

    if (!WIFI_Set_Mode(WIFI_MODE_STA)) return 0;

    char cmd[128];
    if (sys_config.magic == CONFIG_MAGIC && strlen(sys_config.wifi_ssid) > 0) {
        sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", sys_config.wifi_ssid, sys_config.wifi_pwd);
    } else {
        sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASS);
    }
    if (!WIFI_Send_AT_Cmd(cmd, "OK", 5000, 1)) return 0; // Increased timeout

    if (!WIFI_Send_AT_Cmd("AT+CIPMUX=0\r\n", "OK", 100, 2)) return 0;

    wifi_connected_flag = 1;

    return 1;
}

// MQTT 
uint8_t WIFI_MQTT_Connect(void)
{
    if (!wifi_connected_flag) return 0;

    // ????
    WIFI_Send_AT_Cmd("AT+MQTTDISCONN=0\r\n", "OK", 200, 1);
    HAL_Delay(300);

    char cmd[256];
    // Check if Magic is valid AND ID is not empty
    if (sys_config.magic == CONFIG_MAGIC && strlen(sys_config.onenet_id) > 0) {
        sprintf(cmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
            sys_config.onenet_id, sys_config.onenet_product_id, sys_config.onenet_key);
    } else {
        sprintf(cmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
            MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);
    }
    printf("[DEBUG] MQTT User Config CMD: %s", cmd); // Add debug print
    if (!WIFI_Send_AT_Cmd(cmd, "OK", 300, 2)) return 0;

    sprintf(cmd, "AT+MQTTCONN=0,\"%s\",%d,1\r\n", MQTT_BROKER, MQTT_PORT);
    printf("[DEBUG] MQTT Connect CMD: %s", cmd); // Add debug print
    if (!WIFI_Send_AT_Cmd(cmd, "OK", 1000, 2)) return 0;
    // ?? ?��1
    mqtt_connected_flag = 1;
    return 1;
}

// MQTT 
uint8_t WIFI_MQTT_Publish(const char* topic, const char* payload, uint8_t qos)
{
    if (!mqtt_connected_flag) return 0;
    char cmd[300];

    sprintf(cmd, "AT+MQTTPUBRAW=0,\"%s\",%d,%d,0\r\n", topic, strlen(payload), qos);
    WIFI_USART_Send_String(cmd);
    HAL_Delay(500); // Increased delay
    WIFI_USART_Send_String(payload);
    //HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 1000);
    return 1;
}

// MQTT 
uint8_t WIFI_MQTT_Subscribe(const char* topic, uint8_t qos)
{
    if (!mqtt_connected_flag) return 0;
    char cmd[100];
    sprintf(cmd, "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
    uint8_t result = WIFI_Send_AT_Cmd(cmd, "OK", 200, 2);
    return result;
}

uint8_t Parse_MQTT_Message(const char *line, MQTT_MESSAGE *msg)
{
    const char *p = strstr(line, "+MQTTSUBRECV:");
    if (!p) return 0;

#ifdef ESP01S_DEBUG
    printf("MQTT?: %s\n", line);
#endif

    p = strchr(p, ':');
    if (!p) return 0;
    p++; 

    p = strchr(p, ',');
    if (!p) return 0;
    p++; 

    if (*p == '"') {
        p++;
        const char *topic_end = strchr(p, '"');
        if (!topic_end) {
            printf("???\n");
            return 0;
        }
        uint16_t topic_len = topic_end - p;
        if (topic_len >= sizeof(msg->topic)) topic_len = sizeof(msg->topic) - 1;
        strncpy(msg->topic, p, topic_len);
        msg->topic[topic_len] = '\0';
        p = topic_end + 1;
    } else {
        msg->topic[0] = '\0';
        return 0;
    }

    if (*p == ',') p++;
    else return 0;

    p = strchr(p, ',');
    if (!p) return 0;
    p++;

    const char *line_end = strchr(p, '\r');
    if (!line_end) line_end = p + strlen(p);

    uint16_t payload_len = line_end - p;
    if (payload_len >= sizeof(msg->payload)) payload_len = sizeof(msg->payload) - 1;
    strncpy(msg->payload, p, payload_len);
    msg->payload[payload_len] = '\0';

#ifdef ESP01S_DEBUG
    printf(": %s\n", msg->topic);
    printf(": %s\n", msg->payload);
#endif

    msg->qos = 0;
    return 1;
}

void Process_WIFI_RX(void)
{
    if (!WIFI_RX_FLAG) return;

    WIFI_RX_BUFFER[WIFI_RX_LEN] = '\0';
    WIFI_RX_FLAG = 0;

    char *buf = (char*)WIFI_RX_BUFFER;
    char *p = buf;

#ifdef ESP01S_DEBUG
    printf("Received raw data: %s\n", buf);
#endif

    while (p && *p) {
        char *end = strstr(p, "\r\n");
        if (!end) break;

        uint16_t len = end - p;
        char line[256];
        if (len >= sizeof(line)) len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = '\0';

#ifdef ESP01S_DEBUG
        printf("Processing line: %s\n", line);
#endif

        if (strstr(line, "+MQTTSUBRECV:")) {
            MQTT_MESSAGE msg;
            if (Parse_MQTT_Message(line, &msg)) {
                printf("MQTT message parsed successfully: %s => %s\n", msg.topic, msg.payload);
                Process_OneNet_Command(&msg);
            } else {
                printf("MQTT message parsing failed\n");
            }
        } else if (strstr(line, "CLOSED")) {
            mqtt_connected_flag = 0;
            printf("[MQTT] Connection closed by server.\n");
        } else if (strstr(line, "+IPD")) {
             extern void Process_Config_Data(char *data);
             Process_Config_Data(line);
        }

        p = end + 2;
    }

    if (p > buf) {
        uint16_t remain = strlen(p);
        if (remain > 0 && remain < WIFI_RX_BUFFER_SIZE) {
            memmove(buf, p, remain);
            memset(buf + remain, 0, WIFI_RX_BUFFER_SIZE - remain);
            WIFI_RX_LEN = remain;
        } else {
            Clear_WIFI_RX_Buffer();
        }
    } else {
        Clear_WIFI_RX_Buffer();
    }
}

int extract_json_value(const char* json_payload, const char* key) {
    char key_str[32];
    sprintf(key_str, "\"%s\"", key);

    const char* start = strstr(json_payload, key_str);
    if (!start) return -1;

    start += strlen(key_str);

    if (*start != ':') return -1;
    start++;

    while (*start == ' ') start++;

    if (strncmp(start, "true", 4) == 0) return 1;
    if (strncmp(start, "false", 5) == 0) return 0;

    if (*start == '{' || *start == '[') return -1;

    char num_str[16];
    int i = 0;
    while (*start >= '0' && *start <= '9') {
        num_str[i++] = *start++;
    }
    num_str[i] = '\0';

    if (i == 0) return -1;

    return atoi(num_str);
}
