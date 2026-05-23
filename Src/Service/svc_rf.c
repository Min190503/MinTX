#include "svc_rf.h"
#include "svc_input.h"
#include "usart.h" // Chứa huart1
#include "svc_storage.h"

extern uint8_t sys_protocol; // Lấy từ app_ui.c (0: CRSF, 1: IBUS)
static uint8_t current_baud_protocol = 255;

// Hàm tính mã bảo mật CRC-8 cho CRSF (Đa thức 0xD5)
static uint8_t crc8_calc(uint8_t *data, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0xD5;
            else crc <<= 1;
        }
    }
    return crc;
}

// Ép dải 1000-2000 về dải 172-1811 chuẩn Crossfire
static uint16_t map_crsf(uint16_t val) {
    return (uint16_t)((val - 1000) * (1811 - 172) / 1000 + 172);
}

static void send_crsf(JoystickAxis_t* axes) {
    uint8_t buf[26];
    buf[0] = 0xC8; // Ký tự đồng bộ CRSF
    buf[1] = 24;   // Chiều dài gói tin
    buf[2] = 0x16; // Mã Type: Gói kênh RC

    uint16_t ch[16] = {992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992};
    // Gán 4 trục (Sắp xếp AETR: Roll, Pitch, Throttle, Yaw)
    ch[0] = map_crsf(axes[0].mapped);
    ch[1] = map_crsf(axes[1].mapped);
    ch[2] = map_crsf(axes[3].mapped); // Throttle
    ch[3] = map_crsf(axes[2].mapped); // Yaw
    // Gán 3 Switch
    ch[4] = map_crsf(axes[4].mapped);
    ch[5] = map_crsf(axes[5].mapped);
    ch[6] = map_crsf(axes[6].mapped);

    //Nhồi 16 số nguyên 11-bit vào 22 byte (22 * 8 = 16 * 11 = 176 bit)
    buf[3] = (uint8_t)(ch[0] & 0x07FF);
    buf[4] = (uint8_t)((ch[0] & 0x07FF) >> 8 | (ch[1] & 0x07FF) << 3);
    buf[5] = (uint8_t)((ch[1] & 0x07FF) >> 5 | (ch[2] & 0x07FF) << 6);
    buf[6] = (uint8_t)((ch[2] & 0x07FF) >> 2);
    buf[7] = (uint8_t)((ch[2] & 0x07FF) >> 10 | (ch[3] & 0x07FF) << 1);
    buf[8] = (uint8_t)((ch[3] & 0x07FF) >> 7 | (ch[4] & 0x07FF) << 4);
    buf[9] = (uint8_t)((ch[4] & 0x07FF) >> 4 | (ch[5] & 0x07FF) << 7);
    buf[10] = (uint8_t)((ch[5] & 0x07FF) >> 1);
    buf[11] = (uint8_t)((ch[5] & 0x07FF) >> 9 | (ch[6] & 0x07FF) << 2);
    buf[12] = (uint8_t)((ch[6] & 0x07FF) >> 6 | (ch[7] & 0x07FF) << 5);
    buf[13] = (uint8_t)((ch[7] & 0x07FF) >> 3);
    buf[14] = (uint8_t)((ch[7] & 0x07FF) >> 11 | (ch[8] & 0x07FF) << 0);
    buf[15] = (uint8_t)((ch[8] & 0x07FF) >> 8 | (ch[9] & 0x07FF) << 3);
    buf[16] = (uint8_t)((ch[9] & 0x07FF) >> 5 | (ch[10] & 0x07FF) << 6);
    buf[17] = (uint8_t)((ch[10] & 0x07FF) >> 2);
    buf[18] = (uint8_t)((ch[10] & 0x07FF) >> 10 | (ch[11] & 0x07FF) << 1);
    buf[19] = (uint8_t)((ch[11] & 0x07FF) >> 7 | (ch[12] & 0x07FF) << 4);
    buf[20] = (uint8_t)((ch[12] & 0x07FF) >> 4 | (ch[13] & 0x07FF) << 7);
    buf[21] = (uint8_t)((ch[13] & 0x07FF) >> 1);
    buf[22] = (uint8_t)((ch[13] & 0x07FF) >> 9 | (ch[14] & 0x07FF) << 2);
    buf[23] = (uint8_t)((ch[14] & 0x07FF) >> 6 | (ch[15] & 0x07FF) << 5);
    buf[24] = (uint8_t)((ch[15] & 0x07FF) >> 3);

    // Quét CRC cho Dữ liệu Type + Payload
    buf[25] = crc8_calc(&buf[2], 23);

    // Bắn qua UART
    HAL_UART_Transmit(&huart1, buf, 26, 10);
}

static void send_ibus(JoystickAxis_t* axes) {
    uint8_t buf[32];
    buf[0] = 0x20; // Độ dài (32 byte)
    buf[1] = 0x40; // Mã lệnh IBUS

    uint16_t ch[14] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};
    ch[0] = axes[0].mapped;
    ch[1] = axes[1].mapped;
    ch[2] = axes[3].mapped; // Throttle
    ch[3] = axes[2].mapped; // Yaw
    ch[4] = axes[4].mapped;
    ch[5] = axes[5].mapped;
    ch[6] = axes[6].mapped;

    for(int i=0; i<14; i++) {
        buf[2 + i*2] = ch[i] & 0xFF;
        buf[3 + i*2] = (ch[i] >> 8) & 0xFF;
    }

    uint16_t chk = 0xFFFF;
    for(int i=0; i<30; i++) chk -= buf[i];

    buf[30] = chk & 0xFF;
    buf[31] = (chk >> 8) & 0xFF;

    HAL_UART_Transmit(&huart1, buf, 32, 10);
}

void Svc_RF_Init(void) { }

void Svc_RF_Update(void) {
    // Đổi tốc độ cổng UART1 ngay lập tức nếu nhảy Protocol
    if (sys_protocol != current_baud_protocol) {
        current_baud_protocol = sys_protocol;

        HAL_UART_DeInit(&huart1);
        if (sys_protocol == 0) huart1.Init.BaudRate = 420000; // CRSF
        else huart1.Init.BaudRate = 115200; // IBUS
        HAL_UART_Init(&huart1);
    }

    JoystickAxis_t* axes = Svc_Input_GetAxes();

    if (sys_protocol == 0) {
        send_crsf(axes);
    } else {
        send_ibus(axes);
    }
}
