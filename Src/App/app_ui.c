#include "app_ui.h"
#include "drv_st7789.h"
#include "svc_input.h"
#include <stdio.h>
#include "drv_encoder.h"

static UITab_t  current_tab = TAB_MAIN;
static JoystickAxis_t* axes_data = NULL;

static uint16_t old_bar_w[7] = {0, 0, 0, 0, 0, 0, 0 };

//TOP BAR
static void UI_DrawStatusBar(void){
	uint16_t top_bg = 0x10A2;
	ST7789_FillRect(0, 0, 320, 20, top_bg);
	ST7789_DrawLine(0, 20, 320, 20, ST7789_GRAY);

	uint16_t sig_x = 5, sig_y = 14;
	ST7789_DrawString(sig_x, 6, "Tx", ST7789_CYAN, top_bg, 1);
	//vạch sóng
	ST7789_FillRect(sig_x + 20, sig_y - 2, 2, 2, ST7789_GREEN);
	ST7789_FillRect(sig_x + 24, sig_y - 4, 2, 4, ST7789_GREEN);
	ST7789_FillRect(sig_x + 28, sig_y - 6, 2, 6, ST7789_GREEN);
	ST7789_FillRect(sig_x + 32, sig_y - 8, 2, 8, ST7789_GRAY);

	//Time
	ST7789_DrawString(145, 6, "00:00", ST7789_YELLOW, top_bg, 1);

	// Pin
	ST7789_DrawString(260, 6, "11.2V", ST7789_GRAY, top_bg, 1);

	// Khung cục pin
	uint16_t bat_x = 295, bat_y = 5;
	ST7789_DrawRect(bat_x, bat_y, 18, 10, ST7789_WHITE);
	ST7789_FillRect(bat_x + 18, bat_y + 3, 2, 4, ST7789_WHITE);
	ST7789_FillRect(bat_x + 2, bat_y + 2, 12, 6, ST7789_GREEN);
}

//TAB BAR
static void UI_DrawTabBar(void){
	uint16_t y = 220;
	ST7789_FillRect(0, y, 320, 20, 0x18C3);

	const char *tabs[] = {"MAIN", "CH", "CAL", "ST"};
	uint16_t tab_w = 80;
	for(int i = 0; i < 4; i++){
		uint16_t fg = (i == current_tab) ? ST7789_CYAN : ST7789_GRAY;
		ST7789_DrawString(i * tab_w + 20, y + 6, tabs[i], fg, 0x18C3, 1);
		if (i == current_tab) {
			ST7789_FillRect(i * tab_w, y, tab_w, 2, ST7789_CYAN);
		}
	}
}


// MÀN HÌNH CH (Channels)
// 1. HÀM TĨNH: Chạy 1 lần khi mở Tab
static void UI_DrawChScreen_Static(void) {

	ST7789_FillRect(0, 21, 320, 199, ST7789_DARK_BG);
	ST7789_DrawString(8, 26, "CHANNELS", ST7789_WHITE, ST7789_DARK_BG, 1);

	const char *ch_names[] = {"Throttle","Yaw","Pitch","Roll", "Aux 1", "Aux 2", "Aux 3"};

    for (int i = 0; i < 7; i++) {
        uint16_t y = 45 + i * 23;
        char label[8];
        sprintf(label, "CH%d", i+1);
        ST7789_DrawString(10,  y+4, label,       ST7789_GRAY,  ST7789_DARK_BG, 1);
        ST7789_DrawString(40, y+4, ch_names[i], ST7789_WHITE, ST7789_DARK_BG, 1);

        // Vẽ khung nền thanh trượt
        ST7789_FillRect(120, y + 2, 120, 6, 0x2945);
        old_bar_w[i] = 0;
    }
}

// 2. HÀM ĐỘNG: Chạy liên tục (Vẽ đè cực nhanh)
static void UI_UpdateChScreen_Dynamic(void) {
	const uint16_t ch_colors[] = {ST7789_RED, ST7789_PURPLE, ST7789_BLUE, ST7789_CYAN,
	                                  ST7789_WHITE, ST7789_WHITE, ST7789_WHITE};
	const uint8_t ui_to_axis[] = {3, 2, 1, 0, 4, 5, 6};

    for (int i = 0; i < 7; i++) {
        uint16_t y = 45 + i * 23;
        uint16_t mapped_val = axes_data[ui_to_axis[i]].mapped;

        ST7789_FillRect(260, y+4, 40, 8, ST7789_DARK_BG);
        ST7789_DrawInt(260, y+4, mapped_val, ST7789_WHITE, ST7789_DARK_BG, 1);

        uint16_t bar_x = 120;
		uint16_t max_bar_w = 120;
		uint16_t bar_y = y + 2;
		uint16_t bar_h = 6;

        // Ép dữ liệu vào khung 1000 - 2000
        int32_t mapped_offset = (int32_t)mapped_val - 1000;
        if (mapped_offset < 0) mapped_offset = 0;
        if (mapped_offset > 1000) mapped_offset = 1000;

        uint16_t new_w = (uint16_t)((mapped_offset * max_bar_w) / 1000);

        if (new_w != old_bar_w[i]) {
			if (new_w > old_bar_w[i]) {
				ST7789_FillRect(bar_x + old_bar_w[i], bar_y, new_w - old_bar_w[i], bar_h, ch_colors[i]);
			} else {
				ST7789_FillRect(bar_x + new_w, bar_y, old_bar_w[i] - new_w, bar_h, 0x2945);
			}
			old_bar_w[i] = new_w;
		}
    }
}
// ===== MÀN HÌNH MAIN =====
static void UI_DrawMainScreen_Static(void) {
	ST7789_FillRect(0, 21, 320, 199, ST7789_DARK_BG);
	ST7789_DrawString(8, 26, "QUAD-X", ST7789_WHITE, ST7789_DARK_BG, 1);
}
static void UI_UpdateMainScreen_Dynamic(void) {
    // Sẽ thêm 2 vòng tròn tọa độ Gimbal
}
static void UI_DrawCalScreen_Static(void) {
	ST7789_FillRect(0, 21, 320, 199, ST7789_DARK_BG);
	ST7789_DrawString(8, 26, "CALIBRATION", ST7789_WHITE, ST7789_DARK_BG, 1);
}
static void UI_DrawStScreen_Static(void) {
	ST7789_FillRect(0, 21, 320, 199, ST7789_DARK_BG);
	ST7789_DrawString(8, 26, "SETTINGS", ST7789_WHITE, ST7789_DARK_BG, 1);
}


// ==============================================================
// API CÔNG KHAI
// ==============================================================

void UI_Init(void) {
    axes_data = Svc_Input_GetAxes();

    ST7789_FillScreen(ST7789_DARK_BG);
    UI_SwitchTab(TAB_CH); // Bắt đầu từ tab CH
}

void UI_SwitchTab(UITab_t tab) {
    current_tab = tab;
    // Tùy theo Tab nào mà gọi hàm Static của Tab đó (Chỉ 1 lần)
    if (tab == TAB_MAIN)      UI_DrawMainScreen_Static();
    else if (tab == TAB_CH)   UI_DrawChScreen_Static();
    else if (tab == TAB_CAL)  UI_DrawCalScreen_Static();
    else if (tab == TAB_ST)   UI_DrawStScreen_Static();

    UI_DrawStatusBar();
    UI_DrawTabBar();
}

void UI_Update(void) {

	//Encoder 1
	EncoderDir_t dir = Drv_Encoder_GetDir(ENCODER_1);
	if(dir == ENC_DIR_CW){
		if(current_tab < TAB_ST){
			UI_SwitchTab((UITab_t)(current_tab + 1));
		}
	}
	else if(dir == ENC_DIR_CCW){
		if(current_tab > TAB_MAIN){
			UI_SwitchTab((UITab_t)(current_tab - 1));
		}
	}

	if (Drv_Encoder_GetButton(ENCODER_1)) {

	}


    // Gọi hàm Dynamic liên tục để quét data
    if (current_tab == TAB_MAIN)      UI_UpdateMainScreen_Dynamic();
    else if (current_tab == TAB_CH)   UI_UpdateChScreen_Dynamic();
}
