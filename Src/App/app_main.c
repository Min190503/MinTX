#include "app_main.h"
#include "drv_st7789.h"
#include "cmsis_os.h"


void App_DisplayTask(void *argument){
	ST7789_Init();

	for(;;){
		ST7789_FillScreen(ST7789_RED);
		osDelay(1000);

		ST7789_FillScreen(ST7789_GREEN);
		osDelay(1000);

		ST7789_FillScreen(ST7789_BLUE);
		osDelay(1000);
	}
}

void App_InputTask(void *argument) {}
void App_CrsfTask(void *argument) {}
