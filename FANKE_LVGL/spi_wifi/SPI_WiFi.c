
/*
*若有外置SDRAM则忽略此条
*若LVGL用SPI驱动屏幕且没有其他外设占用0x24000000地址也忽略此条
*说明：
	如果用了LVGL用了LTDC驱动屏幕，或者其他外设占用了0x24000000以后得地址
	则DMA无法使用，原因如下：
								因为SPI3使用的DMA1和DMA2访问不了0x20000000到0x20020000之间的地址，
								其他SPI未测试
	因此本驱动用的是中断模式
*/


#include "SPI_WiFi.h"


#define SPI_DMA 0

char Default_WIFI_SSID[64] =  "Redmi_7084";
char Default_WIFI_PSWD[64] = "zhangdas7.";
char Default_AP_WIFI_SSID[64] = "yourssid";
char Default_AP_WIFI_PSWD[64] = "yourpassword";
char Default_AP_MODE[64] = "0";
char Default_SERVER_MODE[64] = "1";
char Default_IP[64] = "192.168.4.1";
char ESP32_IP[64] = "null";
int Default_PORT = 8080;
uint8_t data[1024]={0};

char SPI_Receive_Flag=0;
void ESP_RST(){
	int count=0;
	int timeout = 1000;
	//进行握手操作
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("失败1\r\n");
		}
	}
	#if SPI_DMA
		HAL_SPI_TransmitReceive_DMA(&hspi3,(uint8_t *)MODE_RST,data,8);
	#else
		HAL_SPI_TransmitReceive_IT(&hspi3,(uint8_t *)MODE_RST,data,8);
	#endif
}
int ESP_Init_Default_Cfg(){
	cJSON *wifi = cJSON_CreateObject();	//wiwiJSON 根节点
	cJSON_AddStringToObject(wifi,"SSID",Default_WIFI_SSID);
  cJSON_AddStringToObject(wifi,"PSWR",Default_WIFI_PSWD);
	cJSON_AddStringToObject(wifi,"AP_SSID",Default_AP_WIFI_SSID);
  cJSON_AddStringToObject(wifi,"AP_PSWR",Default_AP_WIFI_PSWD);
	cJSON_AddStringToObject(wifi,"AP",Default_AP_MODE);
  cJSON_AddStringToObject(wifi,"SERVER",Default_SERVER_MODE);
	cJSON_AddStringToObject(wifi,"IP",Default_IP);
  cJSON_AddNumberToObject(wifi,"PORT",Default_PORT);
	char *data = cJSON_Print(wifi);
	cJSON_Delete(wifi);
	//printf("%s",data);
	while(wtire_cfg_to_esp32(&hspi3,(uint8_t *)data,(uint8_t *)"ESP_OK",100) != 1){
		static char count = 0;
		if(count++>10){
			return 0;
		}
	}
	cJSON_free((void *) data);
	HAL_Delay(100);
	ESP_RST();
	return 1;
}
int write(SPI_HandleTypeDef* hspi,uint8_t * byte,uint16_t timeout){
	int count=0;
	
	//进行握手操作
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("失败1\r\n");
			return 0;
		}
	}
	#if SPI_DMA
		HAL_SPI_TransmitReceive_DMA(hspi,(uint8_t *)MODE_WRITE,data,8);
	#else
		HAL_SPI_TransmitReceive_IT(hspi,(uint8_t *)MODE_WRITE,data,8);
	#endif
	
	
	while(SPI_Receive_Flag == 0){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("失败2\r\n");
			return 0;
		}
	}
	SPI_Receive_Flag = 0;
		
		
	//开始传送数据
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout){
			printf("失败3\r\n"); //永不失败，这一步失败会出错
			//return 0;
		}
	}
	#if SPI_DMA
		HAL_SPI_Transmit_DMA(hspi,byte,1024);
	#else
		HAL_SPI_Transmit_IT(hspi,byte,1024);
	#endif
	
	return 1;
}


int read(SPI_HandleTypeDef* hspi,uint8_t * byte,uint16_t timeout){
	int count = 0;
	if(HAL_GPIO_ReadPin(Data_in_GPIO_Port,Data_in_Pin) == GPIO_PIN_SET){
		while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
			HAL_Delay(1);
			if(count++>timeout) {
				printf("读失败1\r\n");
				return 0;
			}
		}
	#if SPI_DMA
		HAL_SPI_TransmitReceive_DMA(hspi,(uint8_t *)MODE_READ,data,8);
	#else
		HAL_SPI_TransmitReceive_IT(hspi,(uint8_t *)MODE_READ,data,8);
	#endif
		
		
		while(SPI_Receive_Flag == 0){
			HAL_Delay(1);
			if(count++>timeout) {
				printf("读失败2\r\n");
				return 0;
			}
		}
		SPI_Receive_Flag = 0;
		
		while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
			HAL_Delay(1);
			if(count++>timeout) {
				printf("读失败3\r\n");
				return 0;
			}
		}
		
		if(HAL_GPIO_ReadPin(Data_in_GPIO_Port,Data_in_Pin) == GPIO_PIN_SET){
			#if SPI_DMA
				HAL_SPI_Receive_DMA(&hspi3,byte,1024);
			#else
				HAL_SPI_Receive_IT(&hspi3,byte,1024);
			#endif
			return 1;
		}
		printf("读失败4\r\n");
		return 0;
	}else{
		return 0;
	}

}
/*
*函数功能：向ESP32写入配置信息
*hspi   ：硬件SPI选择
*mdata  ：进行JSON编码后的配置信息
*msg		：期望得到的回复，例如 "OK"
*timeout：超时时间
*返回值	：
*		1		：写入成功
*		0		：写入失败
*/
int wtire_cfg_to_esp32(SPI_HandleTypeDef* hspi,uint8_t *mdata,uint8_t *msg,uint16_t timeout){
	memset(data,0,1024);
	int count=0;
	//进行握手操作
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("失败1\r\n");
			return 0;
		}
	}
	#if SPI_DMA
		HAL_SPI_TransmitReceive_DMA(hspi,(uint8_t *)MODE_WRITE_CFG,data,20);
	#else
		HAL_SPI_TransmitReceive_IT(hspi,(uint8_t *)MODE_WRITE_CFG,data,20);
	#endif
	while(SPI_Receive_Flag == 0){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("失败2\r\n");
			return 0;
		}
	}
	SPI_Receive_Flag = 0;
	if(strstr((char *)MODE_WRITE_CFG,MODE_WRITE_CFG) != NULL){
		//开始传送数据
		while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
			HAL_Delay(1);
			if(count++>timeout){
				printf("失败3\r\n"); //永不失败，这一步失败会出错
				//return 0;
			}
		}
		#if SPI_DMA
			HAL_SPI_Transmit_DMA(hspi,mdata,1024);
		#else
			HAL_SPI_Transmit_IT(hspi,mdata,1024);
		#endif
	}
	HAL_Delay(100);
	if(read(&hspi3,data,timeout) != 1){
		printf("等待返回数据失败\r\n");
		return 0;
	}

	//HAL_UART_Transmit(&huart1,(uint8_t *)data,sizeof(data),100);	
	char *ret = strstr((char *)data,(char *)msg);
	if(ret == NULL){
		printf("返回结果错误\r\n");
		return 0;
	}
	return 1;
}
/*
*函数功能：向ESP32写入配置信息
*hspi   ：硬件SPI选择
*timeout：超时时间
*返回值	：
*		1		：写入成功
*		0		：写入失败
*/
int read_cfg_from_esp32(SPI_HandleTypeDef* hspi,uint16_t timeout){
	cJSON *pJson;	
	memset(data,0,1024);
	int count=0;
	//进行握手操作
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("失败1\r\n");
			return 0;
		}
	}
	#if SPI_DMA
		HAL_SPI_TransmitReceive_DMA(hspi,(uint8_t *)MODE_READ_CFG,data,20);
	#else
		HAL_SPI_TransmitReceive_IT(hspi,(uint8_t *)MODE_READ_CFG,data,20);
	#endif
	while(SPI_Receive_Flag == 0){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("失败2\r\n");
			return 0;
		}
	}
	SPI_Receive_Flag = 0;
	HAL_Delay(100);
	if(read(&hspi3,data,timeout) != 1){
		printf("等待返回数据失败\r\n");
		return 0;
	}
	printf("结果\n%s",data);
	pJson = cJSON_Parse((char *)data);
	if(pJson == NULL){
		printf("根节点创建失败");
		return 0;
	}else{
		cJSON *ap,*server,*IP_addr,*Port,*Local_IP;	//JSON解析子属性
		cJSON *ap_ssid,*ap_pswr;	//JSON解析子属性
		cJSON *ssid,*pswr;	//JSON解析子属性
		ap = cJSON_GetObjectItem(pJson,"AP");
		server = cJSON_GetObjectItem(pJson,"SERVER");
		IP_addr = cJSON_GetObjectItem(pJson,"IP");
		Port = cJSON_GetObjectItem(pJson,"PORT");
		Local_IP = cJSON_GetObjectItem(pJson,"LOCAL_IP");
		if(!ap || !server || !IP_addr || !Port || !Local_IP){
			printf("不存在ap,IP_Addr,Port和server属性");
			return 0;
		}
		if(cJSON_IsString(ap)){
			sprintf(Default_AP_MODE,"%s",ap->valuestring);
		}else{
			printf("AP错误");
			return 0;
		}
		if(cJSON_IsString(server)){
			sprintf(Default_SERVER_MODE,"%s",server->valuestring);
		}else{
			printf("SERVER错误");
			return 0;
		}
		if(cJSON_IsString(IP_addr)){
			sprintf(Default_IP,"%s",IP_addr->valuestring);
		}else{
			printf("IP_ADDR错误");
			return 0;
		}
		if(cJSON_IsString(Local_IP)){
			sprintf(ESP32_IP,"%s",Local_IP->valuestring);
		}else{
			printf("LOCAL_IP_ADDR错误");
			return 0;
		}
		if(cJSON_IsNumber(Port)){
			//printf("%d",Port->valueint);
			Default_PORT = Port->valueint;
		}else{
			printf("Port错误");
			return 0;
		}
		ap_ssid = cJSON_GetObjectItem(pJson,"AP_SSID");
		ap_pswr = cJSON_GetObjectItem(pJson,"AP_PSWR");
		if(!ap_ssid || !ap_pswr){
			printf("不存在AP_SSID和AP_PSWR属性");
			return 0;
		}
		if(cJSON_IsString(ap_ssid)){
			sprintf(Default_AP_WIFI_SSID,"%s",ap_ssid->valuestring);
		}else{
			printf("AP_SSID错误");
			return 0;
		}
		if(cJSON_IsString(ap_pswr)){
			sprintf(Default_AP_WIFI_PSWD,"%s",ap_pswr->valuestring);
		}else{
			printf("AP_PSWR错误");
			return 0;
		}
		ssid = cJSON_GetObjectItem(pJson,"SSID");
		pswr = cJSON_GetObjectItem(pJson,"PSWR");
		if(!ssid || !pswr){
			printf("不存在SSID和PSWR属性");
			return 0;
		}
		if(cJSON_IsString(ssid)){
			sprintf(Default_WIFI_SSID,"%s",ssid->valuestring);
		}else{
			printf("SSID错误");
			return 0;
		}
		if(cJSON_IsString(pswr)){
			sprintf(Default_WIFI_PSWD,"%s",pswr->valuestring);
		}else{
			printf("PSWR错误");
			return 0;
		}
		printf("根节点创建成功");
	}
	cJSON_Delete(pJson);
	return 1;
}

