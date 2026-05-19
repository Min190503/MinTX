#include "app_main.h"
#include "drv_st7789.h"
#include "svc_input.h"
#include "cmsis_os.h"

#define BAR_X			20
#define BAR_W			200
#define BAR_H			30
#define BAR_START_Y		30
#define BAR_GAP			70

static const uint16_t AXIS_COLOR[4] = {
    0xF800,  // Roll     - Đỏ
    0x07E0,  // Pitch    - Xanh lá
    0x001F,  // Yaw      - Xanh dương
    0xFFE0   // Throttle - Vàng
};

static void UI_DrawBackground(void){
	ST7789_FillScreen(ST7789_BLACK);
	for (int i = 0; i < 4; i++) {
	        uint16_t y = BAR_START_Y + i * BAR_GAP;
	        ST7789_DrawRect(BAR_X - 2, y - 2, BAR_W + 4, BAR_H + 4, 0x4208); // viền xám
	    }
}

// Cập nhật thanh bargraph theo giá trị mapped (1000-2000)
static void UI_UpdateBargraph(void) {
    for (int i = 0; i < 4; i++) {
        uint16_t y    = BAR_START_Y + i * BAR_GAP;
        uint16_t fill = (uint16_t)((uint32_t)(g_axes[i].mapped - 1000) * BAR_W / 1000);
        if (fill > BAR_W) fill = BAR_W; // Clamp an toàn
        if (fill > 0)
            ST7789_FillRect(BAR_X, y, fill, BAR_H, AXIS_COLOR[i]);
        if (fill < BAR_W)
            ST7789_FillRect(BAR_X + fill, y, BAR_W - fill, BAR_H, ST7789_BLACK);
    }
}

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
	for(;;){
		Svc_Input_Update();
		osDelay(10);
	}
}
void App_CrsfTask(void *argument) {}
void App_Init(void){}
