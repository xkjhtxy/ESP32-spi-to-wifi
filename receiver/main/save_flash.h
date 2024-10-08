#ifndef __SAVE_FLASH_H
#define __SAVE_FLASH_H
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//定义文件路径
#define WIFITXT 		"/spiffs/wifi.txt"
#define AP_WIFITXT 		"/spiffs/ap_wifi.txt"
#define MODETXT 		"/spiffs/mode.txt"
enum{
	FILE_AP_WIFI,
	FILE_WIFI,
	FILE_MODE
};//枚举文件类型
//默认WIFI参数定义
static const char *SSID = "MERCURY_7FFC";
static const char *PSW = "13695587503";
static const char *AP_SSID = "myssid";
static const char *AP_PSW = "mypassword";
static const char *IP_ADDR = "192.168.4.2";
static const int PORT = 8080;
//操作函数声明
void spiffs_flash_init(void);
int file_exist(const char *filename);
int file_read(const char *filename,char data[]);
int file_write(const char *filename,const char *data);
int file_create(const char *filename);
int write_default_wifi_cfg_to_file(void);
int write_default_ap_wifi_cfg_to_file();
int write_default_mode_cfg_to_file();
int write_wifi_cfg_to_file(char *mssid,char *mpswr);
int write_ap_wifi_cfg_to_file(char *map_ssid ,char *map_psw);
int write_mode_cfg_to_file(char *aap ,char *mserver,char *mIP_addr,int mPort);
int read_wifi_form_file(char *mSSID ,char *mPSW);
int read_ap_wifi_form_file(char *mAP_SSID ,char *mAP_PSW);
int read_mode_form_file(char *aAP ,char *mServer,char *mIP_addr,int *mPort);
void file_cfg_init(void);					//初始化时调用
int Read_cfg_from_JSON(char *byte);
void Encode_cfg_to_JSON(char *data);
//全局变量声明
extern char SSID1[64],PSWR[64];
extern char mSSID[64],mPSWR[64];
extern char ap[64],server[64];
extern char IP_address[64];
extern int port;

#endif
