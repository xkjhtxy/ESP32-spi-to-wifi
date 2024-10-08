/*
JSON��ʽ��
{
	"SSID":"targetSSID"		//STAҪ���ӵ�WIFI��
	"PSWR":"targetPSWR"
	"AP_SSID":"yourSSID"	//APģʽ��WIFI��
	"AP_PSWR":"yourPSWR"
	"AP":"0"							//Ϊ1��APģʽ��Ϊ0��STAģʽ
	"SERVER":"0"					//Ϊ1��Serverģʽ��Ϊ0��Clientģʽ
	"IP":"192.168.40.1"		//Clientģʽ�ķ�����IP
	"PORT":8080						//�˿ں�
}
*/

#ifndef __SPI_WIFI_H__
#define __SPI_WIFI_H__

#include "main.h"
#include "spi.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include "cJSON.h"

extern char Default_WIFI_SSID[64];
extern char Default_WIFI_PSWD[64];
extern char Default_AP_WIFI_SSID[64];
extern char Default_AP_WIFI_PSWD[64];
extern char Default_AP_MODE[64];
extern char Default_SERVER_MODE[64];
extern char Default_IP[64];
extern char ESP32_IP[64];
extern int Default_PORT;

#define MODE_WRITE_CFG "Config_Save"
#define MODE_WRITE "Write"
#define MODE_READ "Read"
#define MODE_READ_CFG "Config_List"
#define MODE_RST "ESP_RST"

int write(SPI_HandleTypeDef* hspi,uint8_t * byte,uint16_t timeout);
int read(SPI_HandleTypeDef* hspi,uint8_t * byte,uint16_t timeout);
int wtire_cfg_to_esp32(SPI_HandleTypeDef* hspi,uint8_t *mdata,uint8_t *msg,uint16_t timeout);
int read_cfg_from_esp32(SPI_HandleTypeDef* hspi,uint16_t timeout);
int ESP_Init_Default_Cfg(void);
void ESP_RST(void);
extern char SPI_Receive_Flag;


#endif


