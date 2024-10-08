#include "wifi_cfg_page.h"

lv_obj_t *wifi_uiScreen;	//创建wifi名称和密码界面
lv_obj_t *wifi_cfgListScreen;

lv_obj_t *bt1;
lv_obj_t *label_bt1;

lv_obj_t *bt2;
lv_obj_t *label_bt2;

lv_obj_t *text_psd;
lv_obj_t* text_username;

lv_obj_t *bt3;
lv_obj_t *label_bt3;
void wifi_uiInit(void);
void wifi_cfgList_uiInit(void);
static void keyboard_cb(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);//获得事件码
	lv_obj_t *text = lv_event_get_target(e);//获得事件对象
	lv_obj_t *keyboard = lv_event_get_user_data(e);//获得与实践关联的对象
	if(code == LV_EVENT_DEFOCUSED){
		lv_keyboard_set_textarea(keyboard, NULL);    // 取消文本区与键盘关联
		lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);
	}else if(code == LV_EVENT_FOCUSED){
		lv_keyboard_set_textarea(keyboard, text);    // 取消文本区与键盘关联
		lv_obj_clear_flag(keyboard,LV_OBJ_FLAG_HIDDEN);
	}
	
}
void gui_init(){	
	
	wifi_uiInit();
	lv_scr_load(wifi_uiScreen);
	
//	wifi_cfgList_uiInit();
//	lv_scr_load(wifi_cfgListScreen);
	//label ：单片机型号主频显示
  lv_obj_t *label = lv_label_create(lv_scr_act());
	lv_obj_set_style_text_font(label,&my_CN_Font,0);
  lv_label_set_text_fmt(label, "单片机:STM32H7B0VBT6 主频: %ld MHz 剩余内存还可解码 40*40 JPEG图片",HAL_RCC_GetSysClockFreq()/1000000);  //设置标签文字
	lv_obj_align(label,LV_ALIGN_TOP_MID,0,0);
	lv_label_set_long_mode(label,LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(label,300,50);//设置宽300高50	

} 


static void wifi_btn_handler(lv_event_t *e){
	lv_obj_t *target = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);//获得事件码
	if(target ==bt1){
		if(code == LV_EVENT_CLICKED){
//			lv_label_set_text(label_bt1,"按键松开");
		}else if(code == LV_EVENT_PRESSED){
			sprintf(Default_WIFI_SSID,"%s",lv_textarea_get_text(text_username));
			sprintf(Default_WIFI_PSWD,"%s",lv_textarea_get_text(text_psd));
			ESP_Init_Default_Cfg();
			//lv_label_set_text(label_bt1,"按键按下");
		}
	}else if(target ==bt2){
		if(code == LV_EVENT_PRESSED){
			wifi_cfgList_uiInit();
			lv_scr_load_anim(wifi_cfgListScreen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, true); 
		}
	}else if(target ==bt3){
		if(code == LV_EVENT_PRESSED){
			wifi_uiInit();
			lv_scr_load_anim(wifi_uiScreen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, true); 
		}
	}
}
void wifi_cfgList_uiInit(void){
	while(read_cfg_from_esp32(&hspi3,100) == 0){
		HAL_Delay(100);
	}
	wifi_cfgListScreen = lv_obj_create(NULL);
	
	lv_obj_t *label_ssid = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_ssid,"WIFI名称:%s",Default_WIFI_SSID);
	lv_obj_align(label_ssid,LV_ALIGN_TOP_MID,0,30);
		
	lv_obj_t *label_psd = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_psd,"WIFI密码:%s",Default_WIFI_PSWD);
	lv_obj_align_to(label_psd,label_ssid,LV_ALIGN_TOP_MID,0,30);
			
	lv_obj_t *label_ap_ssid = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_ap_ssid,"AP_WIFI名称:%s",Default_AP_WIFI_SSID);
	lv_obj_align_to(label_ap_ssid,label_psd,LV_ALIGN_TOP_MID,0,30);
			
	lv_obj_t *label_ap_psd = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_ap_psd,"AP_WIFI密码:%s",Default_AP_WIFI_PSWD);
	lv_obj_align_to(label_ap_psd,label_ap_ssid,LV_ALIGN_TOP_MID,0,30);
			
	lv_obj_t *label_ap_mode = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_ap_mode,"AP:%s",Default_AP_MODE);
	lv_obj_align_to(label_ap_mode,label_ap_psd,LV_ALIGN_TOP_MID,0,30);
			
	lv_obj_t *label_server_mode = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_server_mode,"Server:%s",Default_SERVER_MODE);
	lv_obj_align_to(label_server_mode,label_ap_mode,LV_ALIGN_TOP_MID,0,30);
			
	lv_obj_t *label_ip = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_ip,"目标IP:%s",Default_IP);
	lv_obj_align_to(label_ip,label_server_mode,LV_ALIGN_TOP_MID,0,30);
			
	lv_obj_t *label_ESP32_ip = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_ESP32_ip,"ESP32 IP:%s",ESP32_IP);
	lv_obj_align_to(label_ESP32_ip,label_ip,LV_ALIGN_TOP_MID,0,30);
			
	lv_obj_t *label_port = lv_label_create(wifi_cfgListScreen);
	lv_label_set_text_fmt(label_port,"端口:%d",Default_PORT);
	lv_obj_align_to(label_port,label_ESP32_ip,LV_ALIGN_TOP_MID,0,30);
	
	lv_obj_set_style_text_font(label_ssid,&my_CN_Font,0);
	lv_obj_set_style_text_font(label_psd,&my_CN_Font,0);
	lv_obj_set_style_text_font(label_ap_ssid,&my_CN_Font,0);
	lv_obj_set_style_text_font(label_ap_psd,&my_CN_Font,0);
	lv_obj_set_style_text_font(label_ap_mode,&my_CN_Font,0);
	lv_obj_set_style_text_font(label_server_mode,&my_CN_Font,0);
	lv_obj_set_style_text_font(label_ip,&my_CN_Font,0);
	lv_obj_set_style_text_font(label_ESP32_ip,&my_CN_Font,0);
	lv_obj_set_style_text_font(label_port,&my_CN_Font,0);
	
	bt3 = lv_btn_create(wifi_cfgListScreen);
	lv_obj_add_event_cb(bt3,wifi_btn_handler,LV_EVENT_ALL,NULL);
	label_bt3 = lv_label_create(bt3);
	lv_obj_set_style_text_font(label_bt3,&my_CN_Font,0);
	lv_label_set_text(label_bt3,"返回");
	lv_obj_align(bt3,LV_ALIGN_BOTTOM_RIGHT,0,0);
}
void wifi_uiInit(void){
	//创建wifi名称和密码界面
	wifi_uiScreen = lv_obj_create(NULL);	
	lv_obj_set_size(wifi_uiScreen, LV_HOR_RES, LV_VER_RES); 
	//bt1发送按键
	bt1 = lv_btn_create(wifi_uiScreen);
	lv_obj_add_event_cb(bt1,wifi_btn_handler,LV_EVENT_ALL,NULL);
	label_bt1 = lv_label_create(bt1);
	lv_obj_set_style_text_font(label_bt1,&my_CN_Font,0);
	lv_label_set_text(label_bt1,"连接");
	lv_obj_align(bt1,LV_ALIGN_CENTER,0,0);
	
	//切换界面按键
	bt2 = lv_btn_create(wifi_uiScreen);
	lv_obj_add_event_cb(bt2,wifi_btn_handler,LV_EVENT_ALL,NULL);
	label_bt2 = lv_label_create(bt2);
	lv_obj_set_style_text_font(label_bt2,&my_CN_Font,0);
	lv_label_set_text(label_bt2,"查看WIFI配置");
	lv_obj_align(bt2,LV_ALIGN_BOTTOM_MID,0,0);
	
	//创建键盘控件
	lv_obj_t *keyboard = lv_keyboard_create(wifi_uiScreen);
	lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);				 // 先将键盘隐藏，当控件获得焦点时才显现
	
	//创建第一个键盘文本框
	text_username = lv_textarea_create(wifi_uiScreen);   // 创建文本输入框对象
	lv_obj_set_size(text_username, 240, 70);  // 设置大小
	lv_obj_set_style_text_font(text_username,&my_CN_Font,0);
	lv_textarea_set_placeholder_text(text_username,"请输入WIFI名称");
	//lv_keyboard_set_textarea(keyboard,text_username);
	lv_obj_add_event_cb(text_username,keyboard_cb,LV_EVENT_ALL,keyboard);
	lv_obj_align(text_username,LV_ALIGN_TOP_MID,0,50); 
	//创建第二个键盘文本框
	text_psd = lv_textarea_create(wifi_uiScreen);
	lv_obj_set_size(text_psd,240,70);
	lv_obj_set_style_text_font(text_psd,&my_CN_Font,0);
	lv_obj_align_to(text_psd,text_username,LV_ALIGN_TOP_MID,0,70);
	lv_textarea_set_placeholder_text(text_psd,"请输入密码");
	lv_obj_add_event_cb(text_psd,keyboard_cb,LV_EVENT_ALL,keyboard);
	//创建第一个文本框的Label ： WIFI名
	lv_obj_t *label_usernam = lv_label_create(wifi_uiScreen);
	lv_obj_set_style_text_font(label_usernam,&my_CN_Font,0);
	lv_label_set_text(label_usernam,"WIFI名称:");
	lv_obj_set_size(label_usernam,168,20);
	lv_obj_align_to(label_usernam,text_username,LV_ALIGN_LEFT_MID,-120,0);
	
	//创建第二个文本框的Label ： WIFI名
	lv_obj_t *label_psd = lv_label_create(wifi_uiScreen);
	lv_obj_set_style_text_font(label_psd,&my_CN_Font,0);
	lv_label_set_text(label_psd,"WIFI密码:");
	lv_obj_set_size(label_psd,168,20);
	lv_obj_align_to(label_psd,text_psd,LV_ALIGN_LEFT_MID,-120,0);
}



