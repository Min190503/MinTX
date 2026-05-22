#include "drv_st7789.h"
#include "cmsis_os.h"
#include "math.h"
#include "font6x8.h"
#include <stdio.h>

extern SPI_HandleTypeDef hspi1;

#define ST7789_CS_LOW()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define ST7789_CS_HIGH()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define ST7789_DC_CMD()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define ST7789_DC_DATA()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)
#define ST7789_RST_LOW()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET)
#define ST7789_RST_HIGH()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET)

volatile uint8_t spi_dma_busy = 0;

static void ST7789_WriteCmd(uint8_t cmd){
	ST7789_DC_CMD();
	ST7789_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
	ST7789_CS_HIGH();
}


static void ST7789_WriteData(uint8_t data){
	ST7789_DC_DATA();
	ST7789_CS_LOW();
	HAL_SPI_Transmit(&hspi1, &data, 1, 100);
	ST7789_CS_HIGH();
}

static void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
	ST7789_WriteCmd(0x2A);
	ST7789_WriteData(x0 >> 8); ST7789_WriteData(x0 & 0xFF);
	ST7789_WriteData(x1 >> 8); ST7789_WriteData(x1 & 0xFF);
	ST7789_WriteCmd(0x2B);
	ST7789_WriteData(y0 >> 8); ST7789_WriteData(y0 & 0xFF);
	ST7789_WriteData(y1 >> 8); ST7789_WriteData(y1 & 0xFF);
	ST7789_WriteCmd(0x2C);
}

void ST7789_Init(void){
	//reset phan cung
	ST7789_RST_LOW();
	osDelay(50);
	ST7789_RST_HIGH();
	osDelay(50);


	ST7789_WriteCmd(0x11);		//Sleep Out
	osDelay(120);

	//Pixel Format(RGB565 - 16 bit mau)
	ST7789_WriteCmd(0x3A);
	ST7789_WriteData(0x55);


	//Xoay man hinh
	ST7789_WriteCmd(0x36);
	ST7789_WriteData(0x70);		//0x00, 0x70, 0xA0, 0xC0

	//bat hien thi
	ST7789_WriteCmd(0x29); 		//Display on
	osDelay(10);
}

//do mau toan man hinh
void ST7789_FillScreen(uint16_t color){
	uint8_t data[2] = {color >> 8, color & 0xFF};


	ST7789_WriteCmd(0x2A);		//cot X
	ST7789_WriteData(0x00);		ST7789_WriteData(0x00);
	ST7789_WriteData((ST7789_WIDTH - 1) >> 8);
	ST7789_WriteData((ST7789_WIDTH - 1) & 0xFF);

	ST7789_WriteCmd(0x2B);		//hang y
	ST7789_WriteData(0x00);		ST7789_WriteData(0x00);
	ST7789_WriteData((ST7789_HEIGHT - 1) >> 8);
	ST7789_WriteData((ST7789_HEIGHT - 1) & 0xFF);

	ST7789_WriteCmd(0x2C);

	ST7789_DC_DATA();
	ST7789_CS_LOW();

	for(uint32_t i = 0; i < ST7789_WIDTH * ST7789_HEIGHT; i++){
		HAL_SPI_Transmit(&hspi1, data, 2, 10);
	}
	ST7789_CS_HIGH();
}

static uint16_t dma_color_buf[6000];

void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
    if(w == 0 || h == 0) return;

    uint32_t total_pixels = w * h;

    while(spi_dma_busy == 1) { osDelay(1); }

    uint16_t swapped_color = (color >> 8) | (color << 8);
    uint32_t fill_size = (total_pixels > 6000) ? 6000 : total_pixels;
    for(uint32_t i = 0; i < fill_size; i++){
        dma_color_buf[i] = swapped_color;
    }

    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);
    ST7789_DC_DATA();
    ST7789_CS_LOW();

    uint32_t remaining = total_pixels;
    while (remaining > 0) {
        uint32_t send_size = (remaining > 6000) ? 6000 : remaining;

        spi_dma_busy = 1;
        ST7789_CS_LOW();
        HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)dma_color_buf, send_size * 2);

        while(spi_dma_busy == 1) { osDelay(1); }

        remaining -= send_size;
    }

    ST7789_CS_HIGH();
}


void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	ST7789_FillRect(x, 			y, 			w, 1, color);
	ST7789_FillRect(x, 			y + h - 1, 	w, 1, color);
	ST7789_FillRect(x, 			y, 			1, h, color);
	ST7789_FillRect(x + w - 1, 	y, 			1, h, color);
}



void ST7789_DrawBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *color_array){
	while(spi_dma_busy == 1){
		osDelay(1);
	}

	spi_dma_busy = 1;
	ST7789_SetWindow(x, y, x + w  - 1, y +h - 1);
	ST7789_DC_DATA();
	ST7789_CS_LOW();
	HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)color_array, w * h * 2);

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        ST7789_CS_HIGH();   // Kéo CS lên để chốt khung hình
        spi_dma_busy = 0;   // Mở khóa cờ cho khung hình tiếp theo
    }
}


//=========================================================
void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t color){
	if(x < 0 || x >= ST7789_WIDTH || y <0 || y >= ST7789_HEIGHT) return;
		while(spi_dma_busy == 1){
		osDelay(1);
	}

	ST7789_SetWindow(x, y, x, y);
	uint8_t data[2] = {color >> 8, color & 0xFF};
	ST7789_DC_DATA();
	ST7789_CS_LOW();
	HAL_SPI_Transmit(&hspi1, data, 2, 10);
	ST7789_CS_HIGH();
}

void ST7789_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
	int16_t dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int16_t err = dx + dy, e2;

	for(;;){
		ST7789_DrawPixel(x0, y0, color);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 >= dy) { err += dy; x0 += sx; }
		if(e2 <= dx) { err += dx; y0 += sy; }
	}
}

void ST7789_DrawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color){
	int16_t x =0, y = r, d = 3 -2 * r;
	while ( x <= y){
		ST7789_DrawPixel(cx+x, cy+y, color);
		ST7789_DrawPixel(cx-x, cy+y, color);
		ST7789_DrawPixel(cx+x, cy-y, color);
		ST7789_DrawPixel(cx-x, cy-y, color);
		ST7789_DrawPixel(cx+y, cy+x, color);
		ST7789_DrawPixel(cx-y, cy+x, color);
		ST7789_DrawPixel(cx+y, cy-x, color);
		ST7789_DrawPixel(cx-y, cy-x, color);
		d += (d < 0) ? (4*x + 6) : (4*(x - y--) + 10);
		x++;
	}
}


void ST7789_FillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    int16_t x = 0, y = r, d = 3 - 2 * r;
    while (x <= y) {
        // Vẽ 2 đường ngang tại mỗi cặp y đối xứng
        ST7789_FillRect(cx - x, cy - y, 2*x + 1, 1, color);
        ST7789_FillRect(cx - x, cy + y, 2*x + 1, 1, color);
        ST7789_FillRect(cx - y, cy - x, 2*y + 1, 1, color);
        ST7789_FillRect(cx - y, cy + x, 2*y + 1, 1, color);
        d += (d < 0) ? (4*x + 6) : (4*(x - y--) + 10);
        x++;
    }
}

void ST7789_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // 4 cạnh thẳng
    ST7789_DrawLine(x+r,   y,     x+w-r-1, y,       color); // trên
    ST7789_DrawLine(x+r,   y+h-1, x+w-r-1, y+h-1,   color); // dưới
    ST7789_DrawLine(x,     y+r,   x,       y+h-r-1, color); // trái
    ST7789_DrawLine(x+w-1, y+r,   x+w-1,   y+h-r-1, color); // phải
    // 4 cung tròn ở góc - dùng Bresenham
    int16_t px = 0, py = r, d = 3 - 2*r;
    while (px <= py) {
        ST7789_DrawPixel(x+w-r-1+px, y+r-py,   color); // góc trên phải
        ST7789_DrawPixel(x+w-r-1+py, y+r-px,   color);
        ST7789_DrawPixel(x+r-px,     y+r-py,   color); // góc trên trái
        ST7789_DrawPixel(x+r-py,     y+r-px,   color);
        ST7789_DrawPixel(x+r-px,     y+h-r-1+py, color); // góc dưới trái
        ST7789_DrawPixel(x+r-py,     y+h-r-1+px, color);
        ST7789_DrawPixel(x+w-r-1+px, y+h-r-1+py, color); // góc dưới phải
        ST7789_DrawPixel(x+w-r-1+py, y+h-r-1+px, color);
        d += (d < 0) ? (4*px + 6) : (4*(px - py--) + 10);
        px++;
    }
}
void ST7789_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Phần chữ nhật giữa
    ST7789_FillRect(x, y+r, w, h - 2*r, color);
    // Phần bo góc trên và dưới - scan-line
    int16_t px = 0, py = r, d = 3 - 2*r;
    while (px <= py) {
        // Quét ngang cho phần góc bo
        ST7789_FillRect(x + r - py, y + r - px, w - 2*(r-py), 1, color);
        ST7789_FillRect(x + r - px, y + r - py, w - 2*(r-px), 1, color);
        ST7789_FillRect(x + r - py, y + h - r + px - 1, w - 2*(r-py), 1, color);
        ST7789_FillRect(x + r - px, y + h - r + py - 1, w - 2*(r-px), 1, color);
        d += (d < 0) ? (4*px + 6) : (4*(px - py--) + 10);
        px++;
    }
}

void ST7789_DrawChar(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t size) {
    if(c < 32 || c > 127) c = '?';

    const uint8_t *bitmap = font6x8[c - 32];

    for (uint8_t col = 0; col < 6; col++) {
        uint8_t line = bitmap[col];
        for (uint8_t row = 0; row < 8; row++) {
            if (line & (1 << row)) {
                if (size == 1) ST7789_DrawPixel(x + col, y + row, fg);
                else ST7789_FillRect(x + (col * size), y + (row * size), size, size, fg);
            } else if (bg != fg) {
                if (size == 1) ST7789_DrawPixel(x + col, y + row, bg);
                else ST7789_FillRect(x + (col * size), y + (row * size), size, size, bg);
            }
        }
    }
}
void ST7789_DrawString(int16_t x, int16_t y, const char *str, uint16_t fg, uint16_t bg, uint8_t size) {
    while (*str) {
        ST7789_DrawChar(x, y, *str++, fg, bg, size);
        x += 6 * size; // Mỗi ký tự rộng 6 pixel * scale
    }
}
void ST7789_DrawInt(int16_t x, int16_t y, int32_t val, uint16_t fg, uint16_t bg, uint8_t size) {
    char buf[12];
    sprintf(buf, "%ld", val);
    ST7789_DrawString(x, y, buf, fg, bg, size);
}






































