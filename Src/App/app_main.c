#include "app_main.h"
#include "drv_st7789.h"
#include "svc_input.h"
#include "cmsis_os.h"
#include "app_ui.h"
#include "drv_encoder.h"


void App_DisplayTask(void *argument){
	osDelay(500);
	ST7789_Init();
	UI_Init();
	for(;;){
		UI_Update();
		osDelay(33);
	}
}

void App_InputTask(void *argument) {
	Svc_Input_Init();
	Drv_Encoder_Init();
	for(;;){
		Svc_Input_Update();
		Drv_Encoder_Update();
		osDelay(10);
	}
}
void App_CrsfTask(void *argument) {}
void App_Init(void){}
