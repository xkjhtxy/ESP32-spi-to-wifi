
/*
*��������SDRAM����Դ���
*��LVGL��SPI������Ļ��û����������ռ��0x24000000��ַҲ���Դ���
*˵����
	�������LVGL����LTDC������Ļ��������������ռ����0x24000000�Ժ�õ�ַ
	��DMA�޷�ʹ�ã�ԭ�����£�
								��ΪSPI3ʹ�õ�DMA1��DMA2���ʲ���0x20000000��0x20020000֮��ĵ�ַ��
								����SPIδ����
	��˱������õ����ж�ģʽ
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
	//�������ֲ���
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("ʧ��1\r\n");
		}
	}
	#if SPI_DMA
		HAL_SPI_TransmitReceive_DMA(&hspi3,(uint8_t *)MODE_RST,data,8);
	#else
		HAL_SPI_TransmitReceive_IT(&hspi3,(uint8_t *)MODE_RST,data,8);
	#endif
}
int ESP_Init_Default_Cfg(){
	cJSON *wifi = cJSON_CreateObject();	//wiwiJSON ���ڵ�
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
	
	//�������ֲ���
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("ʧ��1\r\n");
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
			printf("ʧ��2\r\n");
			return 0;
		}
	}
	SPI_Receive_Flag = 0;
		
		
	//��ʼ��������
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout){
			printf("ʧ��3\r\n"); //����ʧ�ܣ���һ��ʧ�ܻ����
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
				printf("��ʧ��1\r\n");
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
				printf("��ʧ��2\r\n");
				return 0;
			}
		}
		SPI_Receive_Flag = 0;
		
		while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
			HAL_Delay(1);
			if(count++>timeout) {
				printf("��ʧ��3\r\n");
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
		printf("��ʧ��4\r\n");
		return 0;
	}else{
		return 0;
	}

}
/*
*�������ܣ���ESP32д��������Ϣ
*hspi   ��Ӳ��SPIѡ��
*mdata  ������JSON������������Ϣ
*msg		�������õ��Ļظ������� "OK"
*timeout����ʱʱ��
*����ֵ	��
*		1		��д��ɹ�
*		0		��д��ʧ��
*/
int wtire_cfg_to_esp32(SPI_HandleTypeDef* hspi,uint8_t *mdata,uint8_t *msg,uint16_t timeout){
	memset(data,0,1024);
	int count=0;
	//�������ֲ���
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("ʧ��1\r\n");
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
			printf("ʧ��2\r\n");
			return 0;
		}
	}
	SPI_Receive_Flag = 0;
	if(strstr((char *)MODE_WRITE_CFG,MODE_WRITE_CFG) != NULL){
		//��ʼ��������
		while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
			HAL_Delay(1);
			if(count++>timeout){
				printf("ʧ��3\r\n"); //����ʧ�ܣ���һ��ʧ�ܻ����
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
		printf("�ȴ���������ʧ��\r\n");
		return 0;
	}

	//HAL_UART_Transmit(&huart1,(uint8_t *)data,sizeof(data),100);	
	char *ret = strstr((char *)data,(char *)msg);
	if(ret == NULL){
		printf("���ؽ������\r\n");
		return 0;
	}
	return 1;
}
/*
*�������ܣ���ESP32д��������Ϣ
*hspi   ��Ӳ��SPIѡ��
*timeout����ʱʱ��
*����ֵ	��
*		1		��д��ɹ�
*		0		��д��ʧ��
*/
int read_cfg_from_esp32(SPI_HandleTypeDef* hspi,uint16_t timeout){
	cJSON *pJson;	
	memset(data,0,1024);
	int count=0;
	//�������ֲ���
	while(HAL_GPIO_ReadPin(Hand_Shank_GPIO_Port,Hand_Shank_Pin) == GPIO_PIN_RESET){
		HAL_Delay(1);
		if(count++>timeout) {
			printf("ʧ��1\r\n");
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
			printf("ʧ��2\r\n");
			return 0;
		}
	}
	SPI_Receive_Flag = 0;
	HAL_Delay(100);
	if(read(&hspi3,data,timeout) != 1){
		printf("�ȴ���������ʧ��\r\n");
		return 0;
	}
	printf("���\n%s",data);
	pJson = cJSON_Parse((char *)data);
	if(pJson == NULL){
		printf("���ڵ㴴��ʧ��");
		return 0;
	}else{
		cJSON *ap,*server,*IP_addr,*Port,*Local_IP;	//JSON����������
		cJSON *ap_ssid,*ap_pswr;	//JSON����������
		cJSON *ssid,*pswr;	//JSON����������
		ap = cJSON_GetObjectItem(pJson,"AP");
		server = cJSON_GetObjectItem(pJson,"SERVER");
		IP_addr = cJSON_GetObjectItem(pJson,"IP");
		Port = cJSON_GetObjectItem(pJson,"PORT");
		Local_IP = cJSON_GetObjectItem(pJson,"LOCAL_IP");
		if(!ap || !server || !IP_addr || !Port || !Local_IP){
			printf("������ap,IP_Addr,Port��server����");
			return 0;
		}
		if(cJSON_IsString(ap)){
			sprintf(Default_AP_MODE,"%s",ap->valuestring);
		}else{
			printf("AP����");
			return 0;
		}
		if(cJSON_IsString(server)){
			sprintf(Default_SERVER_MODE,"%s",server->valuestring);
		}else{
			printf("SERVER����");
			return 0;
		}
		if(cJSON_IsString(IP_addr)){
			sprintf(Default_IP,"%s",IP_addr->valuestring);
		}else{
			printf("IP_ADDR����");
			return 0;
		}
		if(cJSON_IsString(Local_IP)){
			sprintf(ESP32_IP,"%s",Local_IP->valuestring);
		}else{
			printf("LOCAL_IP_ADDR����");
			return 0;
		}
		if(cJSON_IsNumber(Port)){
			//printf("%d",Port->valueint);
			Default_PORT = Port->valueint;
		}else{
			printf("Port����");
			return 0;
		}
		ap_ssid = cJSON_GetObjectItem(pJson,"AP_SSID");
		ap_pswr = cJSON_GetObjectItem(pJson,"AP_PSWR");
		if(!ap_ssid || !ap_pswr){
			printf("������AP_SSID��AP_PSWR����");
			return 0;
		}
		if(cJSON_IsString(ap_ssid)){
			sprintf(Default_AP_WIFI_SSID,"%s",ap_ssid->valuestring);
		}else{
			printf("AP_SSID����");
			return 0;
		}
		if(cJSON_IsString(ap_pswr)){
			sprintf(Default_AP_WIFI_PSWD,"%s",ap_pswr->valuestring);
		}else{
			printf("AP_PSWR����");
			return 0;
		}
		ssid = cJSON_GetObjectItem(pJson,"SSID");
		pswr = cJSON_GetObjectItem(pJson,"PSWR");
		if(!ssid || !pswr){
			printf("������SSID��PSWR����");
			return 0;
		}
		if(cJSON_IsString(ssid)){
			sprintf(Default_WIFI_SSID,"%s",ssid->valuestring);
		}else{
			printf("SSID����");
			return 0;
		}
		if(cJSON_IsString(pswr)){
			sprintf(Default_WIFI_PSWD,"%s",pswr->valuestring);
		}else{
			printf("PSWR����");
			return 0;
		}
		printf("���ڵ㴴���ɹ�");
	}
	cJSON_Delete(pJson);
	return 1;
}

