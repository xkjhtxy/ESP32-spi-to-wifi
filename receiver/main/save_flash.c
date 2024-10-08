#include "save_flash.h"
char SSID1[64] = {0},PSWR[64] = {0};
char mSSID[64] = {0},mPSWR[64] = {0};
char ap[64] = {1},server[64] = {1},IP_address[64] = {0};
int port = PORT;
char myIP[64] = "null";
void spiffs_flash_init(){
	static const char *TAG = "example";
	sprintf(SSID1,"%s",SSID);
	sprintf(PSWR,"%s",PSW);
	sprintf(mSSID,"%s",AP_SSID);
	sprintf(mPSWR,"%s",AP_PSW);
	sprintf(IP_address,"%s",IP_ADDR);
    ESP_LOGI(TAG, "Initializing SPIFFS");
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}
/*
*函数功能:将data中的数据写入指名文件中
*filename:指名文件，为字符串
*data	 :写入数据缓冲区
*返回值  :
	1    :文件写入成功
	0	 :文件不存在
*/
int file_write(const char *filename,const char *data){
	FILE *f;
	ESP_LOGW("file_write","%s",filename);
	f = fopen(filename,"r");
	if(f == NULL){
		return 0;		//文件不存在
	}
	fclose(f);
	f = fopen(filename,"w+");
	fprintf(f,data);
	fclose(f);
	return 1;
}
/*
*函数功能:读取文件中的内容存储在data中
*filename:指名文件，为字符串
*data    :读出数据缓冲区
*返回值  :
*	  1  :文件成功被读出
*	  0  :文件不存在
*/
int file_read(const char *filename,char data[]){
	FILE *f;
	char line[64];
	ESP_LOGW("file_read","%s",filename);
	f = fopen(filename,"r");
	if(f == NULL){
		return 0;
	}
	while(fscanf(f,"%[^\n] ",line) != EOF){
		strcat(data,line);
	}
	//printf("%s",data);
	fclose(f);
	return 1;
}
/*
*函数功能：检测文件是否存在
*filename：文件名
*返回值  ：
*	  1  ：文件存在
*	  0  ：文件不存在
*/
int file_exist(const char *filename){
	struct stat buffer;
	const static char *TAG1 = "file_exist";
	ESP_LOGW(TAG1,"%s",filename);
	int exist = stat(filename, &buffer);
	if (exist == 0){
		ESP_LOGI(TAG1,"文件存在");
		return 1;
	}else{
		ESP_LOGE(TAG1,"%s文件不存在",filename);
		return 0;
	}
}
/*
*函数功能：创建一个新文件
*返回值	 ：
*	1	：创建成功
*	0	：创建失败
*/
int file_create(const char *filename){
	FILE *f;
	const static char *TAG2 = "file_create";
	ESP_LOGW(TAG2,"%s",filename);
	if(file_exist(filename) == 1){
		return 1;
	}
	f = fopen(filename,"w+");
	if(f != NULL){
		ESP_LOGI(TAG2,"%s文件创建成功",filename);
		fclose(f);
		return 1;
	}
	ESP_LOGE(TAG2,"%s文件创建失败",filename);
	return 0;
}
/*
*函数功能：若文件不存在则以JSON格式向WiFi.txt文件中写入默认的WIFI信息
*返回值  ：
*	1	：写入成功
*	0	：写入失败
*/
int write_default_wifi_cfg_to_file(){
	static const char *TAG = "默认WiFi写入";
	cJSON *wifi = cJSON_CreateObject();	//wiwiJSON 根节点
	cJSON_AddStringToObject(wifi,"SSID",SSID);
    cJSON_AddStringToObject(wifi,"PSWR",PSW);
	char *data = cJSON_Print(wifi);
	cJSON_Delete(wifi);
	if(file_exist(WIFITXT) == 1){
		cJSON_free((void *) data);
		//ESP_LOGI(TAG,"文件存在");
		return 1;
	}
	if(file_create(WIFITXT) == 1){
		if(file_write(WIFITXT,data) == 1){	//只有文件被创建这里才允许写入
			//ESP_LOGI(TAG,"写入成功");
			cJSON_free((void *) data);
			return 1;
		}
	}
	cJSON_free((void *) data);
	ESP_LOGI(TAG,"写入失败");
	return 0;
}
/*
*函数功能：以JSON格式向AP_WiFi.txt文件中写入默认的WIFI信息
*返回值  ：
*	1	：写入成功
*	0	：写入失败
*/
int write_default_ap_wifi_cfg_to_file(){
 	static const char *TAG = "默认AP_WiFi写入";
	cJSON *ap_wifi = cJSON_CreateObject();	//wiwiJSON 根节点
	cJSON_AddStringToObject(ap_wifi,"AP_SSID",AP_SSID);
    cJSON_AddStringToObject(ap_wifi,"AP_PSWR",AP_PSW);
	char *data = cJSON_Print(ap_wifi);
	cJSON_Delete(ap_wifi);
	if(file_exist(AP_WIFITXT) == 1){
		cJSON_free((void *) data);
		//ESP_LOGI(TAG,"文件存在");
		return 1;
	}
	if(file_create(AP_WIFITXT) == 1){
		if(file_write(AP_WIFITXT,data) == 1){	//只有文件被创建这里才允许写入
			//ESP_LOGI(TAG,"写入成功");
			cJSON_free((void *) data);
			return 1;
		}
	}
	cJSON_free((void *) data);
	ESP_LOGI(TAG,"写入失败");
	return 0;
}
/*
*函数功能：以JSON格式向MODE.txt文件中写入默认的WIFI信息
*返回值  ：
*	1	：写入成功
*	0	：写入失败
*/
int write_default_mode_cfg_to_file(){
	static const char *TAG = "默认MODE写入";
	cJSON *mode = cJSON_CreateObject();	//wiwiJSON 根节点
	cJSON_AddStringToObject(mode,"AP","1");			//置1为AP模式
    cJSON_AddStringToObject(mode,"SERVER","1");		//置1为Server端
	cJSON_AddStringToObject(mode,"IP",IP_ADDR);
	cJSON_AddNumberToObject(mode,"PORT",PORT);
	char *data = cJSON_Print(mode);
	cJSON_Delete(mode);
	if(file_exist(MODETXT) == 1){
		cJSON_free((void *) data);
		//ESP_LOGI(TAG,"文件存在");
		return 1;
	}
	if(file_create(MODETXT) == 1){
		if(file_write(MODETXT,data) == 1){	//只有文件被创建这里才允许写入
			//ESP_LOGI(TAG,"写入成功");
			cJSON_free((void *) data);
			return 1;
		}
	}
	cJSON_free((void *) data);
	ESP_LOGI(TAG,"写入失败");
	return 0;
}
/*
*函数功能：解析WIFI.TXT里的SSID和PSW
*SSID	 ：SSID缓冲区
*PSW	 ：PASSWORD缓冲区
*返回值	  ：
	1	 ：成功解析出SSID和PSW
	0	 ：SSID或PSW解析失败
*/
int read_wifi_form_file(char *mSSID ,char *mPSW){
	const static char *TAG = "解析WIFI.TXT";
	char byte[512] = {0};		//文件读出缓冲区
	cJSON *pJson;		//JSON解析根节点
	cJSON *ssid,*pswr;	//JSON解析子属性
	if(file_read(WIFITXT,byte) != 1){
		ESP_LOGE(TAG,"文件读取失败");
		return 0;
	}
	//ESP_LOGI(TAG,"%s",byte);
	pJson = cJSON_Parse(byte);
	if(pJson == NULL){
		ESP_LOGE(TAG,"根节点创建失败");
		return 0;
	}
	ssid = cJSON_GetObjectItem(pJson,"SSID");
	pswr = cJSON_GetObjectItem(pJson,"PSWR");
	if(!ssid || !pswr){
		ESP_LOGE(TAG,"不存在SSID和PSWR属性");
		return 0;
	}
	if(cJSON_IsString(ssid)){
		strcpy(mSSID,ssid->valuestring);
	}else{
		ESP_LOGE(TAG,"SSID错误");
		return 0;
	}
	if(cJSON_IsString(pswr)){
		strcpy(mPSW,pswr->valuestring);
	}else{
		ESP_LOGE(TAG,"PSWR错误");
		return 0;
	}
	cJSON_Delete(pJson);
	return 1;
}
/*
*函数功能：解析WIFI.TXT里的SSID和PSW
*mAP_SSID	 ：AP_SSID缓冲区
*mAP_PSW	 ：AP_PASSWORD缓冲区
*返回值	  ：
	1	 ：成功解析出AP_SSID和AP_PSW
	0	 ：AP_SSID或AP_PSW解析失败
*/
int read_ap_wifi_form_file(char *mAP_SSID ,char *mAP_PSW){
	const static char *TAG = "解析AP_WIFI.TXT";
	char byte[512] = {0};		//文件读出缓冲区
	cJSON *pJson;		//JSON解析根节点
	cJSON *ap_ssid,*ap_pswr;	//JSON解析子属性
	if(file_read(AP_WIFITXT,byte) != 1){
		ESP_LOGE(TAG,"文件读取失败");
		return 0;
	}
	pJson = cJSON_Parse(byte);
	if(pJson == NULL){
		ESP_LOGE(TAG,"根节点创建失败");
		return 0;
	}
	ap_ssid = cJSON_GetObjectItem(pJson,"AP_SSID");
	ap_pswr = cJSON_GetObjectItem(pJson,"AP_PSWR");
	if(!ap_ssid || !ap_pswr){
		ESP_LOGE(TAG,"不存在AP_SSID和AP_PSWR属性");
		return 0;
	}
	if(cJSON_IsString(ap_ssid)){
		strcpy(mAP_SSID,ap_ssid->valuestring);
	}else{
		ESP_LOGE(TAG,"AP_SSID错误");
		return 0;
	}
	if(cJSON_IsString(ap_pswr)){
		strcpy(mAP_PSW,ap_pswr->valuestring);
	}else{
		ESP_LOGE(TAG,"AP_PSWR错误");
		return 0;
	}
	cJSON_Delete(pJson);
	return 1;
}
/*
*函数功能：解析WIFI.TXT里的SSID和PSW
*SSID	 ：SSID缓冲区
*PSW	 ：PASSWORD缓冲区
*返回值	  ：
	1	 ：成功解析出SSID和PSW
	0	 ：SSID或PSW解析失败
*/
int read_mode_form_file(char *aAP ,char *mServer,char *mIP_addr,int *mPort){
	const static char *TAG = "解析MODE.TXT";
	char byte[512] = {0};	//文件读出缓冲区
	cJSON *pJson;		//JSON解析根节点
	cJSON *ap,*server,*IP_addr,*Port;	//JSON解析子属性
	if(file_read(MODETXT,byte) != 1){
		ESP_LOGE(TAG,"文件读取失败");
		return 0;
	}
	pJson = cJSON_Parse(byte);
	if(pJson == NULL){
		ESP_LOGE(TAG,"根节点创建失败");
		return 0;
	}
	ap = cJSON_GetObjectItem(pJson,"AP");
	server = cJSON_GetObjectItem(pJson,"SERVER");
	IP_addr = cJSON_GetObjectItem(pJson,"IP");
	Port = cJSON_GetObjectItem(pJson,"PORT");
	if(!ap || !server || !IP_addr || !Port){
		ESP_LOGE(TAG,"不存在ap,IP_Addr,Port和server属性");
		return 0;
	}
	if(cJSON_IsString(ap)){
		strcpy(aAP,ap->valuestring);
	}else{
		ESP_LOGE(TAG,"AP错误");
		return 0;
	}
	if(cJSON_IsString(server)){
		strcpy(mServer,server->valuestring);
	}else{
		ESP_LOGE(TAG,"SERVER错误");
		return 0;
	}
	if(cJSON_IsString(IP_addr)){
		strcpy(mIP_addr,IP_addr->valuestring);
	}else{
		ESP_LOGE(TAG,"IP_ADDR错误");
		return 0;
	}
	if(cJSON_IsNumber(Port)){
		*mPort = Port->valueint;
	}else{
		ESP_LOGE(TAG,"IP_ADDR错误");
		return 0;
	}
	cJSON_Delete(pJson);
	return 1;
}
/*
*函数功能：向WIFI.txt文件里写入数据
*mssid  : WIFI名称
*mpswr	: WIFI密码
*返回值 :
	0	:写入失败或文件不存在
	1	:写入成功
*/
int write_wifi_cfg_to_file(char *mssid,char *mpswr){
	static const char *TAG = "WiFi写入";
	cJSON *wifi = cJSON_CreateObject();	//wiwiJSON 根节点
	cJSON_AddStringToObject(wifi,"SSID",mssid);
    cJSON_AddStringToObject(wifi,"PSWR",mpswr);
	char *data = cJSON_Print(wifi);
	cJSON_Delete(wifi);
	if(file_exist(WIFITXT) == 0){
		cJSON_free((void *) data);
		return 0;
	}
	if(file_write(WIFITXT,data) == 1){
		//ESP_LOGI(TAG,"写入成功");
		cJSON_free((void *) data);
		return 1;
	}
	cJSON_free((void *) data);
	ESP_LOGI(TAG,"写入失败");
	return 0;
}
/*
*函数功能：向AP_WIFI.txt文件里写入数据
*map_ssid  : AP_WIFI名称
*map_psw	: AP_WIFI密码
*返回值 :
	0	:写入失败或文件不存在
	1	:写入成功
*/
int write_ap_wifi_cfg_to_file(char *map_ssid ,char *map_psw){
	static const char *TAG = "AP_WiFi写入";
	cJSON *ap_wifi = cJSON_CreateObject();	//wiwiJSON 根节点
	cJSON_AddStringToObject(ap_wifi,"AP_SSID",map_ssid);
    cJSON_AddStringToObject(ap_wifi,"AP_PSWR",map_psw);
	char *data = cJSON_Print(ap_wifi);
	cJSON_Delete(ap_wifi);
	if(file_exist(AP_WIFITXT) == 0){
		cJSON_free((void *) data);
		return 0;
	}
	if(file_write(AP_WIFITXT,data) == 1){
		//ESP_LOGI(TAG,"写入成功");
		cJSON_free((void *) data);
		return 1;
	}
	cJSON_free((void *) data);
	ESP_LOGI(TAG,"写入失败");
	return 0;
}
/*
*函数功能：向MODE.txt文件里写入数据
*aap    : ap模式，为1则为AP模式，为0则是STA模式。默认为1
*mserver: 为1则是server模式，为0则是client模式。默认为1
*返回值 :
	0	:写入失败或文件不存在
	1	:写入成功
*/
int write_mode_cfg_to_file(char *aap ,char *mserver,char *mIP_addr,int mPort){
	static const char *TAG = "默认MODE写入";
	cJSON *mode = cJSON_CreateObject();	//wiwiJSON 根节点
	cJSON_AddStringToObject(mode,"AP",aap);			//置1为AP模式
    cJSON_AddStringToObject(mode,"SERVER",mserver);		//置1为Server端
	cJSON_AddStringToObject(mode,"IP",mIP_addr);
	cJSON_AddNumberToObject(mode,"PORT",mPort);
	char *data = cJSON_Print(mode);
	cJSON_Delete(mode);
	if(file_exist(MODETXT) == 0){
		cJSON_free((void *) data);
		return 0;
	}
	if(file_write(MODETXT,data) == 1){
		//ESP_LOGI(TAG,"写入成功");
		cJSON_free((void *) data);
		return 1;
	}
	cJSON_Delete(mode);
	cJSON_free((void *) data);
	ESP_LOGI(TAG,"写入失败");
	return 0;
}
/*
*函数功能：初始化三个文件
*/
void file_cfg_init(void){
	const static char *TAG = "file_cfg_init";
	if(write_default_wifi_cfg_to_file()!=1){
		printf("失败\r\n");
		return;
	}
	if(read_wifi_form_file(SSID1,PSWR)){
		printf("解析完成：\n\r%s\r\n\r%s\n",SSID1,PSWR);
	}else{
		ESP_LOGE(TAG,"WIFI解析失败");
	}
	if(write_default_ap_wifi_cfg_to_file()!=1){
		printf("失败\r\n");
		return;
	}
	if(read_ap_wifi_form_file(mSSID,mPSWR)){
		printf("解析完成：\n\r%s\r\n\r%s\n",mSSID,mPSWR);
	}else{
		ESP_LOGE(TAG,"AP_WIFI解析失败");
	}
	if(write_default_mode_cfg_to_file()!=1){
		printf("失败\r\n");
		return;
	}
	if(read_mode_form_file(ap,server,IP_address,&port)){
		ap[0] = ap[0] - '0';
		server[0] = server[0] - '0';
		printf("解析完成：\n\r%d\r\n\r%d\n%s\r\n%d\n",ap[0],server[0],IP_address,port);
	}else{
		ESP_LOGE(TAG,"MODE解析失败");
	}
}
/*
*函数功能：从JSON数据中读取配置文件并写入到相应的文件中
*/
int Read_cfg_from_JSON(char *byte){
	const static char *TAG = "解析配置文件";
	cJSON *pJson;		//JSON解析根节点
	cJSON *ap,*server,*IP_addr,*Port;	//JSON解析子属性
	cJSON *ap_ssid,*ap_pswr;	//JSON解析子属性
	cJSON *ssid,*pswr;	//JSON解析子属性
	pJson = cJSON_Parse(byte);
	if(pJson == NULL){
		ESP_LOGE(TAG,"根节点创建失败");
		return 0;
	}
	ap = cJSON_GetObjectItem(pJson,"AP");
	server = cJSON_GetObjectItem(pJson,"SERVER");
	IP_addr = cJSON_GetObjectItem(pJson,"IP");
	Port = cJSON_GetObjectItem(pJson,"PORT");

	if(!ap || !server || !IP_addr || !Port){
		ESP_LOGE(TAG,"不存在ap,IP_Addr,Port和server属性");
		return 0;
	}
	if(cJSON_IsString(ap)){
		ESP_LOGE(TAG,"%s",ap->valuestring);
	}else{
		ESP_LOGE(TAG,"AP错误");
		return 0;
	}
	if(cJSON_IsString(server)){
		ESP_LOGE(TAG,"%s",server->valuestring);
	}else{
		ESP_LOGE(TAG,"SERVER错误");
		return 0;
	}
	if(cJSON_IsString(IP_addr)){
		ESP_LOGE(TAG,"%s",IP_addr->valuestring);
	}else{
		ESP_LOGE(TAG,"IP_ADDR错误");
		return 0;
	}
	if(cJSON_IsNumber(Port)){
		ESP_LOGE(TAG,"%d",Port->valueint);
	}else{
		ESP_LOGE(TAG,"Port错误");
		return 0;
	}
	if(write_mode_cfg_to_file(ap->valuestring,server->valuestring,IP_addr->valuestring,Port->valueint) != 1){
		return 0;
	}
	ap_ssid = cJSON_GetObjectItem(pJson,"AP_SSID");
	ap_pswr = cJSON_GetObjectItem(pJson,"AP_PSWR");
	if(!ap_ssid || !ap_pswr){
		ESP_LOGE(TAG,"不存在AP_SSID和AP_PSWR属性");
		return 0;
	}
	if(cJSON_IsString(ap_ssid)){
		ESP_LOGE(TAG,"%s",ap_ssid->valuestring);
	}else{
		ESP_LOGE(TAG,"AP_SSID错误");
		return 0;
	}
	if(cJSON_IsString(ap_pswr)){
		ESP_LOGE(TAG,"%s",ap_pswr->valuestring);
	}else{
		ESP_LOGE(TAG,"AP_PSWR错误");
		return 0;
	}
	if(write_ap_wifi_cfg_to_file(ap_ssid->valuestring,ap_pswr->valuestring) != 1){
		return 0;
	}
	ssid = cJSON_GetObjectItem(pJson,"SSID");
	pswr = cJSON_GetObjectItem(pJson,"PSWR");
	if(!ssid || !pswr){
		ESP_LOGE(TAG,"不存在SSID和PSWR属性");
		return 0;
	}
	if(cJSON_IsString(ssid)){
		ESP_LOGE(TAG,"%s",ssid->valuestring);
	}else{
		ESP_LOGE(TAG,"SSID错误");
		return 0;
	}
	if(cJSON_IsString(pswr)){
		ESP_LOGE(TAG,"%s",pswr->valuestring);
	}else{
		ESP_LOGE(TAG,"PSWR错误");
		return 0;
	}
	if(write_wifi_cfg_to_file(ssid->valuestring,pswr->valuestring) != 1){
		return 0;
	}
	cJSON_Delete(pJson);
	return 1;
}
void Encode_cfg_to_JSON(char *data){
	file_cfg_init();
	cJSON *wifi = cJSON_CreateObject();	//wifiJSON 根节点
	cJSON_AddStringToObject(wifi,"SSID",SSID1);
	cJSON_AddStringToObject(wifi,"PSWR",PSWR);
	cJSON_AddStringToObject(wifi,"AP_SSID",mSSID);
	cJSON_AddStringToObject(wifi,"AP_PSWR",mPSWR);
	ap[0] += '0';
	cJSON_AddStringToObject(wifi,"AP",ap);
	server[0] += '0';
	cJSON_AddStringToObject(wifi,"SERVER",server);
	cJSON_AddStringToObject(wifi,"IP",IP_address);
	cJSON_AddNumberToObject(wifi,"PORT",port);
	cJSON_AddStringToObject(wifi,"LOCAL_IP",myIP);
	char *Encode_data = cJSON_Print(wifi);
	cJSON_Delete(wifi);
	sprintf(data,"%s",Encode_data);
	cJSON_free((void *) Encode_data);
}
