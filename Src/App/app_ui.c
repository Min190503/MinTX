#include "app_ui.h"
#include "drv_st7789.h"
#include "svc_input.h"
#include <stdio.h>
#include "drv_encoder.h"
#include "svc_storage.h"

static UITab_t  current_tab = TAB_MAIN;
static JoystickAxis_t* axes_data = NULL;

static uint16_t old_bar_w[7] = {0, 0, 0, 0, 0, 0, 0 };

static uint8_t cal_state = 0;
static uint16_t cal_min_temp[4];
static uint16_t cal_max_temp[4];
static uint8_t old_cal_state = 255;
static uint8_t old_is_armed = 255;
static uint8_t old_is_angle = 255;
static int16_t old_t = -999, old_y = -999, old_p = -999, old_r = -999;
static uint16_t old_main_l_x = 0, old_main_l_y = 0;
static uint16_t old_main_r_x = 0, old_main_r_y = 0;

// Các biến trạng thái Menu cho màn ST
uint8_t sys_protocol = 0; // 0 = CRSF, 1 = IBUS
static uint8_t old_sys_protocol = 255;
static uint8_t st_menu_index = 0;       // 0: Đang trỏ vào Protocol
static uint8_t st_dropdown_open = 0;    // 0: Đóng, 1: Đang sổ xuống
static uint8_t st_dropdown_index = 0;   // 0: Chọn CRSF, 1: Chọn IBUS
static uint8_t old_st_menu_index = 255;
static uint8_t old_st_dropdown_open = 255;
static uint8_t old_st_dropdown_index = 255;

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


// MÀN HÌNH CH
//1 lần khi mở Tab
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

//liên tục
static void UI_UpdateChScreen_Dynamic(void) {
	const uint16_t ch_colors[] = {
			ST7789_RED,    // CH1 (Thr)
			ST7789_PURPLE, // CH2 (Yaw)
			ST7789_BLUE,   // CH3 (Pitch)
			ST7789_CYAN,   // CH4 (Roll)
			ST7789_ORANGE, // CH5 (Công tắc 1 - Arm)
			ST7789_GRAY,   // CH6 (Công tắc để tạm)
			ST7789_GREEN   // CH7 (Công tắc 3 - Mode bay)
		};

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

	ST7789_DrawString(10, 30, "QUAD-X", ST7789_WHITE, ST7789_DARK_BG, 1);

	ST7789_DrawRoundRect(190, 25, 60, 20, 4, 0x4208);
	ST7789_DrawRoundRect(260, 25, 50, 20, 4, 0x4208);

	ST7789_DrawRoundRect(20, 55, 100, 100, 6, 0x2945);
	ST7789_DrawLine(20, 105, 120, 105, 0x2945);
	ST7789_DrawLine(70, 55, 70, 155, 0x2945);
	ST7789_DrawCircle(70, 105, 40, 0x2945);
	ST7789_DrawCircle(70, 105, 20, 0x2945);


	ST7789_DrawRoundRect(200, 55, 100, 100, 6, 0x2945);
	ST7789_DrawLine(200, 105, 300, 105, 0x2945);
	ST7789_DrawLine(250, 55, 250, 155, 0x2945);
	ST7789_DrawCircle(250, 105, 40, 0x2945);
	ST7789_DrawCircle(250, 105, 20, 0x2945);


	ST7789_FillRoundRect(10, 175, 300, 40, 5, 0x10A2);
	ST7789_DrawString(30, 180, "THR", ST7789_GRAY, 0x10A2, 1);
	ST7789_DrawString(110, 180, "YAW", ST7789_GRAY, 0x10A2, 1);
	ST7789_DrawString(190, 180, "PIT", ST7789_GRAY, 0x10A2, 1);
	ST7789_DrawString(270, 180, "ROL", ST7789_GRAY, 0x10A2, 1);

	old_is_armed = 255;
	old_is_angle = 255;
	old_t = -999; old_y = -999; old_p = -999; old_r = -999;
	old_main_l_x = 0; old_main_l_y = 0;
	old_main_r_x = 0; old_main_r_y = 0;
}

static void UI_UpdateMainScreen_Dynamic(void) {


	uint8_t is_armed = (axes_data[4].mapped > 1500) ? 1 : 0;

	uint8_t is_angle = (axes_data[6].mapped < 1500) ? 1 : 0;


	if (is_armed != old_is_armed) {
		ST7789_FillRect(261, 26, 48, 18, ST7789_DARK_BG);
		if (is_armed) {
			ST7789_DrawRoundRect(260, 25, 50, 20, 4, ST7789_RED);
			ST7789_DrawString(272, 31, "ARM", ST7789_RED, ST7789_DARK_BG, 1);
		} else {
			ST7789_DrawRoundRect(260, 25, 50, 20, 4, ST7789_GRAY);
			ST7789_DrawString(272, 31, "DIS", ST7789_GRAY, ST7789_DARK_BG, 1);
		}
		old_is_armed = is_armed;
	}

	//MODE
	if (is_angle != old_is_angle) {
		ST7789_FillRect(191, 26, 58, 18, ST7789_DARK_BG);
		if (is_angle) {
			ST7789_DrawString(205, 31, "ANGLE", ST7789_CYAN, ST7789_DARK_BG, 1);
		} else {
			ST7789_DrawString(208, 31, "HOLD", ST7789_PURPLE, ST7789_DARK_BG, 1);
		}
		old_is_angle = is_angle;
	}



	int16_t thr_pct = (axes_data[3].mapped - 1500) / 5;
	int16_t yaw_pct = (axes_data[2].mapped - 1500) / 5;
	int16_t pit_pct = (axes_data[1].mapped - 1500) / 5;
	int16_t rol_pct = (axes_data[0].mapped - 1500) / 5;


	if (thr_pct != old_t || yaw_pct != old_y || pit_pct != old_p || rol_pct != old_r) {
		char bufL[30], bufR[30];
		sprintf(bufL, "YAW:%4d  THR:%4d", yaw_pct, thr_pct);
		sprintf(bufR, "ROL:%4d  PIT:%4d", rol_pct, pit_pct);
		ST7789_DrawString(20, 160, bufL, ST7789_GRAY, ST7789_DARK_BG, 1);
		ST7789_DrawString(200, 160, bufR, ST7789_GRAY, ST7789_DARK_BG, 1);

		char bufVal[8];
		sprintf(bufVal, "%4d", thr_pct); ST7789_DrawString(30,  195, bufVal, ST7789_YELLOW, 0x10A2, 1);
		sprintf(bufVal, "%4d", yaw_pct); ST7789_DrawString(110, 195, bufVal, ST7789_CYAN,   0x10A2, 1);
		sprintf(bufVal, "%4d", pit_pct); ST7789_DrawString(190, 195, bufVal, ST7789_PURPLE, 0x10A2, 1);
		sprintf(bufVal, "%4d", rol_pct); ST7789_DrawString(270, 195, bufVal, ST7789_RED,    0x10A2, 1);

		old_t = thr_pct; old_y = yaw_pct; old_p = pit_pct; old_r = rol_pct;
	}

	//chấm tròn trên Radar
	uint16_t dot_l_x = 70 + (yaw_pct * 40) / 100;
	uint16_t dot_l_y = 105 - (thr_pct * 40) / 100;

	uint16_t dot_r_x = 250 + (rol_pct * 40) / 100;
	uint16_t dot_r_y = 105 - (pit_pct * 40) / 100;

	if (dot_l_x != old_main_l_x || dot_l_y != old_main_l_y || old_main_l_x == 0) {
		if (old_main_l_x != 0) ST7789_FillCircle(old_main_l_x, old_main_l_y, 4, ST7789_DARK_BG);

		ST7789_FillRect(20, 105, 100, 1, 0x2945); // Kẻ ngang
		ST7789_FillRect(70, 55, 1, 100, 0x2945);  // Kẻ dọc

		ST7789_FillCircle(dot_l_x, dot_l_y, 4, ST7789_YELLOW);
		old_main_l_x = dot_l_x; old_main_l_y = dot_l_y;
	}

	if (dot_r_x != old_main_r_x || dot_r_y != old_main_r_y || old_main_r_x == 0) {
		if (old_main_r_x != 0) ST7789_FillCircle(old_main_r_x, old_main_r_y, 4, ST7789_DARK_BG);

		ST7789_FillRect(200, 105, 100, 1, 0x2945);
		ST7789_FillRect(250, 55, 1, 100, 0x2945);

		ST7789_FillCircle(dot_r_x, dot_r_y, 4, ST7789_YELLOW);
		old_main_r_x = dot_r_x; old_main_r_y = dot_r_y;
	}
}



//CAL
static void UI_DrawCalScreen_Static(void) {
	ST7789_FillRect(0, 21, 320, 199, ST7789_DARK_BG);

	//Left
	ST7789_DrawString(45, 55, "L", ST7789_GRAY, ST7789_DARK_BG, 1);
	ST7789_DrawRoundRect(20, 70, 60, 60, 4, ST7789_GRAY);

	//Right
	ST7789_DrawString(115, 55, "R", ST7789_GRAY, ST7789_DARK_BG, 1);
	ST7789_DrawRoundRect(90, 70, 60, 60, 4, ST7789_GRAY);

	ST7789_DrawString(200, 45, "CALIBRATION", ST7789_ORANGE, ST7789_DARK_BG, 1);

	ST7789_DrawRoundRect(175, 170, 50, 22, 4, ST7789_GRAY);
	ST7789_DrawString(190, 177, "ESC", ST7789_GRAY, ST7789_DARK_BG, 1);

	ST7789_DrawRoundRect(235, 170, 60, 22, 4, ST7789_YELLOW);
	ST7789_DrawString(252, 177, "NEXT", ST7789_YELLOW, ST7789_DARK_BG, 1);

	for(int i = 0; i < 4; i++) {
	    cal_min_temp[i] = 4095;
	    cal_max_temp[i] = 0;
	}
	cal_state = 0;
	old_cal_state = 255;
}

static void UI_UpdateCalScreen_Dynamic(void) {
	static uint16_t old_l_x = 0, old_l_y = 0;
	static uint16_t old_r_x = 0, old_r_y = 0;


	uint32_t yaw_raw  = axes_data[2].invert ? (4095 - axes_data[2].raw) : axes_data[2].raw;
	uint32_t roll_raw = axes_data[0].invert ? (4095 - axes_data[0].raw) : axes_data[0].raw;
	uint16_t dot_l_x = 21 + yaw_raw * 56 / 4095;
	uint16_t dot_r_x = 91 + roll_raw * 56 / 4095;
	uint32_t thr_raw   = axes_data[3].invert ? axes_data[3].raw : (4095 - axes_data[3].raw);
	uint32_t pitch_raw = axes_data[1].invert ? axes_data[1].raw : (4095 - axes_data[1].raw);
	uint16_t dot_l_y = 71 + thr_raw * 56 / 4095;
	uint16_t dot_r_y = 71 + pitch_raw * 56 / 4095;
	if(dot_l_x < 25) dot_l_x = 25; if(dot_l_x > 74) dot_l_x = 74;
	if(dot_l_y < 75) dot_l_y = 75; if(dot_l_y > 124) dot_l_y = 124;
	if(dot_r_x < 95) dot_r_x = 95; if(dot_r_x > 144) dot_r_x = 144;
	if(dot_r_y < 75) dot_r_y = 75; if(dot_r_y > 124) dot_r_y = 124;


	if (dot_l_x != old_l_x || dot_l_y != old_l_y || old_l_x == 0) {
		if (old_l_x != 0) ST7789_FillCircle(old_l_x, old_l_y, 4, ST7789_DARK_BG);


		ST7789_DrawLine(21, 100, 78, 100, 0x2945);
		ST7789_DrawLine(50, 71, 50, 128, 0x2945);

		ST7789_FillCircle(dot_l_x, dot_l_y, 4, ST7789_YELLOW);
		old_l_x = dot_l_x; old_l_y = dot_l_y;
	}
	if (dot_r_x != old_r_x || dot_r_y != old_r_y || old_r_x == 0) {
		if (old_r_x != 0) ST7789_FillCircle(old_r_x, old_r_y, 4, ST7789_DARK_BG);

		ST7789_DrawLine(91, 100, 148, 100, 0x2945);
		ST7789_DrawLine(120, 71, 120, 128, 0x2945);

		ST7789_FillCircle(dot_r_x, dot_r_y, 4, ST7789_YELLOW);
		old_r_x = dot_r_x; old_r_y = dot_r_y;
	}

	char bufL[16], bufR[16];
	sprintf(bufL, "L:%d,%d  ", axes_data[2].raw, axes_data[3].raw);
	sprintf(bufR, "R:%d,%d  ", axes_data[0].raw, axes_data[1].raw);
	ST7789_DrawString(10, 140, bufL, ST7789_GRAY, ST7789_DARK_BG, 1);
	ST7789_DrawString(90, 140, bufR, ST7789_GRAY, ST7789_DARK_BG, 1);



	if (cal_state != old_cal_state) {
		ST7789_FillRect(180, 65, 130, 90, ST7789_DARK_BG);


		for(int i=0; i<6; i++) {
			uint16_t color = 0x2945;
			if (i < cal_state) color = ST7789_GREEN;
			else if (i == cal_state) color = ST7789_YELLOW;

			ST7789_FillRect(185 + i*19, 65, 15, 4, color);
		}

		//Mũi tên, chữ hướng dẫn
		if (cal_state == 0) {
			ST7789_DrawLine(240, 95, 240, 110, ST7789_WHITE); //XUỐNG
			ST7789_DrawLine(240, 110, 235, 105, ST7789_WHITE);
			ST7789_DrawLine(240, 110, 245, 105, ST7789_WHITE);
			ST7789_DrawString(205, 125, "MAX DOWN", ST7789_YELLOW, ST7789_DARK_BG, 1);
			ST7789_DrawString(205, 145, "Sticks DOWN", ST7789_GRAY, ST7789_DARK_BG, 1);
		} else if (cal_state == 1) {
			ST7789_DrawLine(240, 95, 240, 110, ST7789_WHITE); //LÊN
			ST7789_DrawLine(240, 95, 235, 100, ST7789_WHITE);
			ST7789_DrawLine(240, 95, 245, 100, ST7789_WHITE);
			ST7789_DrawString(215, 125, "MAX UP", ST7789_YELLOW, ST7789_DARK_BG, 1);
			ST7789_DrawString(215, 145, "Sticks UP", ST7789_GRAY, ST7789_DARK_BG, 1);
		} else if (cal_state == 2) {
			ST7789_DrawLine(230, 102, 250, 102, ST7789_WHITE); //RÁI
			ST7789_DrawLine(230, 102, 235, 97, ST7789_WHITE);
			ST7789_DrawLine(230, 102, 235, 107, ST7789_WHITE);
			ST7789_DrawString(205, 125, "MAX LEFT", ST7789_YELLOW, ST7789_DARK_BG, 1);
			ST7789_DrawString(205, 145, "Sticks LEFT", ST7789_GRAY, ST7789_DARK_BG, 1);
		} else if (cal_state == 3) {
			ST7789_DrawLine(230, 102, 250, 102, ST7789_WHITE); //PHẢI
			ST7789_DrawLine(250, 102, 245, 97, ST7789_WHITE);
			ST7789_DrawLine(250, 102, 245, 107, ST7789_WHITE);
			ST7789_DrawString(200, 125, "MAX RIGHT", ST7789_YELLOW, ST7789_DARK_BG, 1);
			ST7789_DrawString(200, 145, "Sticks RIGHT", ST7789_GRAY, ST7789_DARK_BG, 1);
		} else if (cal_state == 4) {
			ST7789_FillCircle(240, 102, 4, ST7789_WHITE);
			ST7789_DrawString(195, 125, "CENTER STICKS", ST7789_YELLOW, ST7789_DARK_BG, 1);
			ST7789_DrawString(200, 145, "Leave in MID", ST7789_GRAY, ST7789_DARK_BG, 1);
		} else if (cal_state == 5) {
			ST7789_DrawString(215, 125, "ALL DONE!", ST7789_GREEN, ST7789_DARK_BG, 1);
			ST7789_DrawString(210, 145, "Press to EXIT", ST7789_GRAY, ST7789_DARK_BG, 1);
		}
		old_cal_state = cal_state;
	}

	if (cal_state >= 0 && cal_state <= 3) {
		for(int i=0; i<4; i++){
			if(axes_data[i].raw < cal_min_temp[i]) cal_min_temp[i] = axes_data[i].raw;
			if(axes_data[i].raw > cal_max_temp[i]) cal_max_temp[i] = axes_data[i].raw;
		}
	}

}


//ST
static void UI_DrawStScreen_Static(void) {
	ST7789_FillRect(0, 21, 320, 199, ST7789_DARK_BG);
	ST7789_DrawString(8, 26, "SETTINGS", ST7789_WHITE, ST7789_DARK_BG, 1);


	ST7789_DrawRoundRect(10, 45, 300, 35, 4, 0x4208);
	ST7789_DrawString(25, 55, "RF Protocol", ST7789_GRAY, ST7789_DARK_BG, 1);
	ST7789_DrawRoundRect(230, 50, 70, 25, 4, 0x4208);




	ST7789_DrawRoundRect(10, 135, 300, 75, 4, 0x4208);


	ST7789_DrawString(25, 145, "Firmware:", ST7789_GRAY, ST7789_DARK_BG, 1);
	ST7789_DrawString(25, 165, "Model ID:", ST7789_GRAY, ST7789_DARK_BG, 1);
	ST7789_DrawString(25, 185, "Bind UID:", ST7789_GRAY, ST7789_DARK_BG, 1);


	ST7789_DrawString(160, 145, "v1.0.0", ST7789_WHITE, ST7789_DARK_BG, 1);
	ST7789_DrawString(160, 165, "QUAD-X", ST7789_WHITE, ST7789_DARK_BG, 1);
	ST7789_DrawString(160, 185, "A3F2B1", ST7789_YELLOW, ST7789_DARK_BG, 1);


	old_sys_protocol = 255;
	old_st_menu_index = 255;
	old_st_dropdown_open = 255;
	old_st_dropdown_index = 255;
	st_dropdown_open = 0;
}

static void UI_UpdateStScreen_Dynamic(void) {

	if (st_menu_index != old_st_menu_index) {
		uint16_t border_color = (st_menu_index == 0 && !st_dropdown_open) ? ST7789_YELLOW : 0x4208;
		ST7789_DrawRoundRect(10, 45, 300, 35, 4, border_color);
		old_st_menu_index = st_menu_index;
	}


	if (sys_protocol != old_sys_protocol) {
		ST7789_FillRect(232, 52, 66, 21, ST7789_DARK_BG);
		if (sys_protocol == 0) {
			ST7789_DrawString(245, 55, "CRSF", ST7789_YELLOW, ST7789_DARK_BG, 1);
		} else {
			ST7789_DrawString(245, 55, "IBUS", ST7789_GREEN, ST7789_DARK_BG, 1);
		}
		old_sys_protocol = sys_protocol;
	}


	if (st_dropdown_open != old_st_dropdown_open ||
	   (st_dropdown_open && st_dropdown_index != old_st_dropdown_index)) {

		if (st_dropdown_open) {

			ST7789_FillRect(230, 78, 70, 50, ST7789_DARK_BG);
			ST7789_DrawRoundRect(230, 78, 70, 50, 4, ST7789_CYAN);

			//CRSF
			uint16_t t_crsf = (st_dropdown_index == 0) ? ST7789_YELLOW : ST7789_GRAY;
			if(st_dropdown_index == 0) ST7789_FillRect(232, 80, 66, 20, 0x4208); // Bôi đen dòng đang trỏ
			else ST7789_FillRect(232, 80, 66, 20, ST7789_DARK_BG);
			ST7789_DrawString(245, 85, "CRSF", t_crsf, (st_dropdown_index == 0)? 0x4208 : ST7789_DARK_BG, 1);

			//IBUS
			uint16_t t_ibus = (st_dropdown_index == 1) ? ST7789_GREEN : ST7789_GRAY;
			if(st_dropdown_index == 1) ST7789_FillRect(232, 102, 66, 20, 0x4208);
			else ST7789_FillRect(232, 102, 66, 20, ST7789_DARK_BG);
			ST7789_DrawString(245, 107, "IBUS", t_ibus, (st_dropdown_index == 1)? 0x4208 : ST7789_DARK_BG, 1);

		} else if (old_st_dropdown_open == 1) {

			ST7789_FillRect(229, 77, 72, 52, ST7789_DARK_BG);
		}

		old_st_dropdown_open = st_dropdown_open;
		old_st_dropdown_index = st_dropdown_index;
		old_st_menu_index = 255;
	}
}





// API
void UI_Init(void) {
    axes_data = Svc_Input_GetAxes();

    ST7789_FillScreen(ST7789_DARK_BG);
    UI_SwitchTab(TAB_MAIN); // Bắt đầu từ tab CH
}

void UI_SwitchTab(UITab_t tab) {
    current_tab = tab;

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
		if(current_tab == TAB_CAL){
			if (cal_state == 0) {
				cal_state = 1;
			}
			else if (cal_state == 1) { cal_state = 2; }
			else if (cal_state == 2) { cal_state = 3; }
			else if (cal_state == 3) {

				for(int i=0; i<4; i++){
					axes_data[i].cal_min = cal_min_temp[i] + 30;
					axes_data[i].cal_max = cal_max_temp[i] - 30;
				}
				cal_state = 4;
			}
			else if (cal_state == 4) {

				axes_data[0].cal_mid = axes_data[0].raw;
				axes_data[1].cal_mid = axes_data[1].raw;
				axes_data[2].cal_mid = axes_data[2].raw;
				cal_state = 5;
			}
			else if (cal_state == 5) {
				cal_state = 0;
				Svc_Storage_Save();
				UI_SwitchTab(TAB_MAIN);
			}
		}

	}

	//encoder 2 o tab st
	if (current_tab == TAB_ST) {
		EncoderDir_t dir2 = Drv_Encoder_GetDir(ENCODER_2);

		if (dir2 == ENC_DIR_CW) {
			if (!st_dropdown_open) {

			} else {
				if (st_dropdown_index < 1) st_dropdown_index++;
			}
		}
		else if (dir2 == ENC_DIR_CCW) {
			if (!st_dropdown_open) {
				// Cuộn menu ngoài lên
			} else {
				// Cuộn trong Dropdown
				if (st_dropdown_index > 0) st_dropdown_index--;
			}
		}

		// Xử lý Bấm nút Encoder 2
		if (Drv_Encoder_GetButton(ENCODER_2)) {
			if (!st_dropdown_open) {
				if (st_menu_index == 0) {
					st_dropdown_open = 1;
					st_dropdown_index = sys_protocol;
				}
			} else {
				sys_protocol = st_dropdown_index;
				st_dropdown_open = 0;
				Svc_Storage_Save();
			}
		}
	}


    // Gọi hàm Dynamic liên tục để quét data
    if (current_tab == TAB_MAIN)      UI_UpdateMainScreen_Dynamic();
    else if (current_tab == TAB_CH)   UI_UpdateChScreen_Dynamic();
    else if (current_tab == TAB_CAL)  UI_UpdateCalScreen_Dynamic();
    else if (current_tab == TAB_ST)   UI_UpdateStScreen_Dynamic();
}
