#include "mServer.h"
#include "spi.h"
#include "Queue.h"
#define TCP_PORT             3333                // TCP服务器端口号

TaskHandle_t Tcp_Transmit_handle = NULL;        //任务句柄
int connect_sock;        //给Client创建Socket使用 以及给Server bind 使用  给发送接收WIFI信息使用
int listen_sock;        //给Server模式创建Socket使用
extern char spi_ok;
static char *TAG="TCP_Server";
int tcp_server_connect=0;
extern LinkQueue Queue_Wifi_Transmit ;              //wifi发送队列
extern SemaphoreHandle_t Mutex_Queue;               //wifi发送队列互斥量
extern SemaphoreHandle_t Mutex_Queue_Receive;       //wifi接受队列互斥量
extern LinkQueue Queue_Wifi_Receive ;               //wifi接受队列
void Tcp_Transmit(void *pvParameters){
    ESP_LOGI(TAG,"TCP发送任务开始");
    err_t ret;
    unsigned char data[1024];
    for(;;){
        if(xSemaphoreTake(Mutex_Queue,portMAX_DELAY)==pdTRUE){
            if(GetLength(Queue_Wifi_Transmit)!=0 ){
                memset(data,0,1024);
                if(GetHead(Queue_Wifi_Transmit,data)==1){
                    //printf("成功获取头结点\r\n");
                    // printf("\nWIFI发送出：\n");
                    // for(int i =0;i<1024;i++){
                    //     printf("%c",data[i]);
                    // }
                    ret =  send(connect_sock, data,1024, 0);
                    if(ret == ESP_OK){
                        if(DeLinkQueue(&Queue_Wifi_Transmit)==1){
                            //printf("出队成功：%s\r\n",Spi_Transmit_buffer);
                            //printf("长度%d\r\n",GetLength(Q));
                        }
                        //memset(Spi_Transmit_buffer,0,1024);
                    }else{
                        printf("SPI发送失败\r\n");
                    }
                }
            }
            xSemaphoreGive( Mutex_Queue );
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }

}
void Tcp_Receive(){
    while(spi_ok==0);
    xTaskCreate(Tcp_Transmit,"tcp_transmit_task",4096,NULL,2,&Tcp_Transmit_handle);  //创建WIFI发送任务
    ESP_LOGI(TAG,"TCP接收任务开始");
    int len;
    unsigned char rx_buffer[1024];
    while(1)
    {
        if(GetLength(Queue_Wifi_Receive)<50){
            memset(rx_buffer, 0, sizeof(rx_buffer));    // 清空缓存
            len = recv(connect_sock, rx_buffer, 1024, 0);  // 阻塞读取接收数据
            if(len < 0)
            {
                ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
                break;
            }
            else if (len == 0)
            {
                ESP_LOGW(TAG, "Connection closed");
                break;
            }
            else
            {
                //ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
                if(xSemaphoreTake(Mutex_Queue_Receive,portMAX_DELAY)==pdTRUE){
                    if(EnLinkQueue(&Queue_Wifi_Receive,rx_buffer)==1){
                        //ESP_LOGI(TAG, "Received %d bytes: %s ", len, rx_buffer);
                        WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1<<GPIO_DATA_OUT));    //置 1      //告诉主机有数据可读
                    }
                    xSemaphoreGive( Mutex_Queue_Receive );
                }
            }
        }
        else{
           //printf("WIFI接收数组太长 %d\r\n",GetLength(Queue_Wifi_Receive));
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}
void mServer_Init(void *pvParameters){
    //int *pserver = (int *)pvParameters;

    int addr_family = 0;
    int ip_protocol = 0;
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    while(ip_flag==0){
        printf("等待获取IP....\r\n");
        vTaskDelay(500);
    }
    printf("server = %d\nip = %s\nport = %d",server[0],IP_address,port);
    if(server[0]== SERVER){
        listen_sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if(listen_sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            //close(listen_sock);
        }
    }else if(server[0] == CLIENT){
        connect_sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if(connect_sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            close(connect_sock);
        }
    }
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    if(server[0] == SERVER){
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if(err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
            close(listen_sock);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", port);
        err = listen(listen_sock, 1);
        if(err != 0)
        {
            ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
            close(listen_sock);
        }
    }else if(server[0] == CLIENT){
        //sprintf(dest_addr.sin_addr.s_addr,"%s",inet_addr(IP_address));
        dest_addr.sin_addr.s_addr = inet_addr(IP_address);
    }

    while(1)
    {
        if(server[0] == SERVER){
            ESP_LOGI(TAG, "Socket listening");
            struct sockaddr_in6 source_addr;
            uint16_t addr_len = sizeof(source_addr);
            connect_sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
            if(connect_sock < 0)
            {
                ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
                close(listen_sock);
            }
            else{
                tcp_server_connect =1;
                ESP_LOGI("SERVICE", "连接成功");
                Tcp_Receive();
                ESP_LOGE(TAG,"tcp接收任务终止");
            }
            tcp_server_connect =0;
            vTaskDelete(Tcp_Transmit_handle);
            ESP_LOGE(TAG,"tcp发送任务终止");
            shutdown(connect_sock, 0);
            close(connect_sock);
        }else if(server[0] == CLIENT){
            connect_sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
            if(connect_sock < 0)
            {
                ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
                close(connect_sock);
            }
            while(connect(connect_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0){
                ESP_LOGE("连接IP","失败");
                vTaskDelay(500);
                close(connect_sock);
                connect_sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
                if(connect_sock < 0)
                {
                    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
                    close(connect_sock);
                }
            }
            ESP_LOGI("CLIENT", "连接成功");
            Tcp_Receive();
            ESP_LOGE(TAG,"tcp接收任务终止");
            vTaskDelete(Tcp_Transmit_handle);
            ESP_LOGE(TAG,"tcp发送任务终止");
            close(connect_sock);
        }
    }
}
