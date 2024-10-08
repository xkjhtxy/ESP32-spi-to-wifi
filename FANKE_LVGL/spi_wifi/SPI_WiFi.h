/*
JSON格式：
{
	"SSID":"targetSSID"		//STA要连接的WIFI名
	"PSWR":"targetPSWR"
	"AP_SSID":"yourSSID"	//AP模式的WIFI名
	"AP_PSWR":"yourPSWR"
	"AP":"0"							//为1是AP模式，为0是STA模式
	"SERVER":"0"					//为1是Server模式，为0是Client模式
	"IP":"192.168.40.1"		//Client模式的服务器IP
	"PORT":8080						//端口号
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


