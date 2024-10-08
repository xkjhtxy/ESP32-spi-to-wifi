/* SPI Slave example, receiver (uses SPI Slave driver to communicate with sender)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/igmp.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "soc/rtc_periph.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "Queue.h"
#include "spi.h"
#include "mWifi.h"
#include "mServer.h"
#include "save_flash.h"

char *TAG = "spi";
//优先级数字越大优先级越高
//创建句柄
TaskHandle_t task_recc = NULL;


SemaphoreHandle_t Mutex_spi;
SemaphoreHandle_t Mutex_Queue;              //wifi发送队列互斥量
SemaphoreHandle_t Mutex_Queue_Receive;      //wifi接受队列互斥量
LinkQueue Queue_Wifi_Transmit;
LinkQueue Queue_Wifi_Receive;

char spi_ok =0;
char spi_Receive_flag = 0;
char JSON_Encode_data[1024];
//Main application
void app_main(void)
{
    //每次切换芯片型号时要记得改引脚
    esp_err_t ret;
    ESP_LOGI(TAG,"程序开始运行");
    ESP_LOGE(TAG,"每次切换芯片型号时要记得改引脚");
    gpio_set_direction(GPIO_DATA_OUT,GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DATA_OUT,0);
    spiffs_flash_init();
	file_cfg_init();
    Mutex_spi            = xSemaphoreCreateMutex();
    Mutex_Queue          = xSemaphoreCreateMutex();
    Mutex_Queue_Receive  = xSemaphoreCreateMutex();

    InitLinkQueue( &Queue_Wifi_Transmit);
    InitLinkQueue( &Queue_Wifi_Receive );
    printf("队列创建完成\r\n");
    spi_Init();
    //开始SPI接受任务
    t.length=1024*8;
    t.rx_buffer = Spi_Receive_buffer;
    t.tx_buffer = Spi_Transmit_buffer;
    spi_ok = 1;

    ESP_LOGE(TAG,"ap = %d",ap[0]);
    mWifi_Init(ap[0]);
    xTaskCreate(mServer_Init,"tcp_receive_task",8192,NULL,2,NULL);      //创建Wifi接收任务
    ESP_LOGI(TAG,"接受任务开始");
    for(;;) {
        memset(Spi_Receive_buffer,0,1024);
        ret = spi_slave_transmit(RCV_HOST,&t,portMAX_DELAY);
        if(ret == ESP_OK){
            if(strstr((char *)Spi_Receive_buffer,"Read") != NULL){
                memset(Spi_Receive_buffer,0,1024);
                memset(Spi_Transmit_buffer,0,1024);
                if(xSemaphoreTake(Mutex_Queue_Receive,portMAX_DELAY)==pdTRUE){
                    if(GetLength(Queue_Wifi_Receive)!=0){
                        if(GetHead(Queue_Wifi_Receive,Spi_Transmit_buffer) == 1){
                            //ESP_LOGI(TAG, "Send: %s ", Spi_Transmit_buffer);
                            ret = spi_slave_transmit(RCV_HOST,&t,portMAX_DELAY);
                            if(ret == ESP_OK){
                                //出队
                                if(DeLinkQueue(&Queue_Wifi_Receive)==1){
                                    if(GetLength(Queue_Wifi_Receive) == 0){
                                        WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1<<GPIO_DATA_OUT));     //置 0
                                    }
                                }
                                else{
                                    printf("出队失败\r\n");
                                }
                            }else{
                                printf("读失败\r\n");
                            }
                        }else{
                            printf("头结点获取失败\r\n");
                        }
                    }else{
                        printf("长度为零\r\n");
                        gpio_set_level(GPIO_DATA_OUT,0);
                    }
                    xSemaphoreGive(Mutex_Queue_Receive);
                }
            }else if(strstr((char *)Spi_Receive_buffer,"Write") != NULL){
                memset(Spi_Receive_buffer,0,1024);
                if(xSemaphoreTake(Mutex_Queue,portMAX_DELAY) == pdTRUE){
                    if(GetLength(Queue_Wifi_Transmit)<50){
                        ret = spi_slave_transmit(RCV_HOST,&t,portMAX_DELAY);
                        if(ret == ESP_OK){
                            if(EnLinkQueue(&Queue_Wifi_Transmit,Spi_Receive_buffer) == 1){
                                // for(int i =0;i<1024;i++){
                                //     printf("%c",Spi_Receive_buffer[i]);
                                // }
                                memset(Spi_Receive_buffer,0,1024);
                            }
                        }
                    }
                    else{
                        WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1<<GPIO_HANDSHAKE));    //置 1
                        //printf("SPI接收数组太长\r\n");
                    }
                    xSemaphoreGive(Mutex_Queue);
                }
            }else if(strstr((char *)Spi_Receive_buffer,"Config_Save") != NULL){
                memset(Spi_Receive_buffer,0,1024);
                ret = spi_slave_transmit(RCV_HOST,&t,portMAX_DELAY);
                if(ret == ESP_OK){
                    if(Read_cfg_from_JSON((char *)Spi_Receive_buffer) == 1){
                        if(GetLength(Queue_Wifi_Receive)<50){
                            if(xSemaphoreTake(Mutex_Queue_Receive,portMAX_DELAY)==pdTRUE){
                                if(EnLinkQueue(&Queue_Wifi_Receive,(uint8_t *)"ESP_OK")==1){
                                    WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1<<GPIO_DATA_OUT));    //置 1      //告诉主机有数据可读
                                }
                                xSemaphoreGive( Mutex_Queue_Receive );
                            }
                        }
                        else{
                            //printf("WIFI接收数组太长 %d\r\n",GetLength(Queue_Wifi_Receive));
                        }
                    }
                }
             }else if(strstr((char *)Spi_Receive_buffer,"Config_List") != NULL){
                memset(JSON_Encode_data,0,1024);
                Encode_cfg_to_JSON((char *)JSON_Encode_data);
                if(GetLength(Queue_Wifi_Receive)<50){
                    if(xSemaphoreTake(Mutex_Queue_Receive,portMAX_DELAY)==pdTRUE){
                        if(EnLinkQueue(&Queue_Wifi_Receive,(uint8_t *)JSON_Encode_data)==1){
                            WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1<<GPIO_DATA_OUT));    //置 1      //告诉主机有数据可读
                        }
                        xSemaphoreGive( Mutex_Queue_Receive );
                    }
                }
                else{
                  //  printf("WIFI接收数组太长 %d\r\n",GetLength(Queue_Wifi_Receive));
                }
            }else if(strstr((char *)Spi_Receive_buffer,"ESP_RST") != NULL){
                esp_restart();
            }
        }
        else{
            printf("模式选择失败\r\n");
        }
        vTaskDelay(1 / portTICK_RATE_MS);
    }
    DesLinkQueue(&Spi_Transmit_buffer);
}

