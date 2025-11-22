#ifndef __BSP_ESP01S_H
#define __BSP_ESP01S_H

#include "main.h"
#include <stdint.h>


#define WIFI_SSID       "your wifi"
#define WIFI_PASS       "wifi password"

#define MQTT_BROKER     "mqtts.heclouds.com" 
#define MQTT_PORT       1883
#define MQTT_CLIENTID   "lab001"
#define MQTT_USERNAME   "your id"
#define MQTT_PASSWORD   "your key"


#define WIFI_RX_BUFFER_SIZE 512

// 
//#define ESP01S_DEBUG


typedef enum {
    WIFI_MODE_AP = 2,
    WIFI_MODE_STA = 1
} WIFI_WORK_MODE;


typedef struct {
    char topic[64];
    char payload[256];
    uint8_t qos;
} MQTT_MESSAGE;

extern uint8_t WIFI_RX_BUFFER[];
extern volatile uint16_t WIFI_RX_LEN;
extern volatile uint8_t WIFI_RX_FLAG;


void WIFI_ESP01S_Init(void);
void Clear_WIFI_RX_Buffer(void);
void WIFI_USART_Send_String(const char *str);
uint8_t WIFI_Send_AT_Cmd(const char *cmd, const char *ack, uint32_t timeout_ms, uint8_t retry);

uint8_t WIFI_Set_Mode(WIFI_WORK_MODE mode);
uint8_t WIFI_STA_Connect(void);
uint8_t WIFI_AP_Start(void);
uint8_t WIFI_Start_AP(const char *ssid, const char *pwd);
uint8_t WIFI_Start_Server(uint16_t port);
uint8_t WIFI_MQTT_Connect(void);
uint8_t WIFI_MQTT_Publish(const char* topic, const char* payload, uint8_t qos);
uint8_t WIFI_MQTT_Subscribe(const char* topic, uint8_t qos);

void Process_WIFI_RX(void);  // ��ѭ���е���
uint8_t Parse_MQTT_Message(const char *line, MQTT_MESSAGE *msg);
int extract_json_value(const char* json_payload, const char* key);
extern uint8_t wifi_connected_flag;
extern uint8_t mqtt_connected_flag;

#endif /* __BSP_ESP01S_H */

