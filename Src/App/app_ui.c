#include "app_ui.h"
#include "drv_st7789.h"
#include "svc_input.h"

static UITab_t  current_tab = TAB_MAIN;
static JoystickAxis_t* axes_data = NULL;

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
			// Gạch chân tab đang active
			ST7789_FillRect(i * tab_w, y, tab_w, 2, ST7789_CYAN);
		}
	}
}


// MÀN HÌNH CH (Channels)
// 1. HÀM TĨNH: Chạy 1 lần khi mở Tab
static void UI_DrawChScreen_Static(void) {
	ST7789_FillScreen(ST7789_DARK_BG);
	ST7789_DrawString(8, 8, "CHANNELS", ST7789_WHITE, ST7789_DARK_BG, 1);

    const char *ch_names[] = {"Throttle","Yaw","Pitch","Roll"};

    for (int i = 0; i < 4; i++) {
        uint16_t y = 30 + i * 45;
        char label[8];
        sprintf(label, "CH%d", i+1);
        ST7789_DrawString(10,  y+4, label,       ST7789_GRAY,  ST7789_DARK_BG, 1);
        ST7789_DrawString(40, y+4, ch_names[i], ST7789_WHITE, ST7789_DARK_BG, 1);

        // Vẽ khung nền thanh trượt
        ST7789_FillRect(120, y + 7, 90, 6, 0x2945);
    }
}

// 2. HÀM ĐỘNG: Chạy liên tục (Vẽ đè cực nhanh)
static void UI_UpdateChScreen_Dynamic(void) {
    const uint16_t ch_colors[] = {ST7789_YELLOW, ST7789_GREEN, ST7789_CYAN, ST7789_PURPLE};
    const uint8_t ui_to_axis[] = {3, 2, 1, 0};

    for (int i = 0; i < 4; i++) {
        uint16_t y = 30 + i * 45;
        uint16_t mapped_val = axes_data[ui_to_axis[i]].mapped;

        ST7789_DrawInt(260, y+4, mapped_val, ST7789_WHITE, ST7789_DARK_BG, 1);
        ST7789_FillRect(192 + 30, y+4, 18, 8, ST7789_DARK_BG);

        uint16_t bar_x = 120, bar_w = 120, bar_mid = bar_x + bar_w/2;
        uint16_t bar_y = y + 7;

        // Phục hồi lại nền thanh trượt
        ST7789_FillRect(bar_x, bar_y, bar_w, 6, 0x2945);

        // Tính toán và vẽ chấm mới
        int32_t mapped_offset = (int32_t)mapped_val - 1000;
        if (mapped_offset < 0) mapped_offset = 0;
        if (mapped_offset > 1000) mapped_offset = 1000;
        uint16_t dot_x = (uint16_t)(bar_x + (mapped_offset * bar_w) / 1000 - 4);

        // Chặn chấm không lọt ra ngoài thanh trượt
        if (dot_x < bar_x) dot_x = bar_x;
        if (dot_x + 8 > bar_x + bar_w) dot_x = bar_x + bar_w - 8;

        ST7789_FillRect(dot_x, bar_y - 1, 8, 8, ch_colors[i]);
    }
}
// ===== MÀN HÌNH MAIN =====
static void UI_DrawMainScreen_Static(void) {
    ST7789_FillRect(0, 0, 320, 220, ST7789_DARK_BG);
    ST7789_DrawString(8, 8, "QUAD-X", ST7789_WHITE, ST7789_DARK_BG, 1);
}
static void UI_UpdateMainScreen_Dynamic(void) {
    // Sẽ thêm 2 vòng tròn tọa độ Gimbal ở đây sau
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

    UI_DrawTabBar(); // Tab bar cũng là tĩnh, vẽ 1 lần
}

void UI_Update(void) {
    // Gọi hàm Dynamic liên tục để quét data
    if (current_tab == TAB_MAIN)      UI_UpdateMainScreen_Dynamic();
    else if (current_tab == TAB_CH)   UI_UpdateChScreen_Dynamic();
}
