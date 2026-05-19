#ifndef DRV_ST7789_H
#define DRV_ST7789_H

#include "main.h"
#define ST7789_WIDTH 320
#define ST7789_HEIGHT 240

#define ST7789_BLACK   0x0000
#define ST7789_WHITE   0xFFFF
#define ST7789_RED     0xF800
#define ST7789_GREEN   0x07E0
#define ST7789_BLUE    0x001F
#define ST7789_CYAN        0x07FF
#define ST7789_DARK_BG     0x0861
#define ST7789_GRAY        0x4208
#define ST7789_YELLOW      0xFFE0
#define ST7789_ORANGE      0xFD20
#define ST7789_PURPLE      0x801F


void ST7789_Init(void);
void ST7789_FillScreen(uint16_t color);
void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7789_DrawBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *color_array);

void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t color);
void ST7789_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void ST7789_DrawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
void ST7789_FillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
void ST7789_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void ST7789_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

//TEXT
void ST7789_DrawChar  (int16_t x, int16_t y, char c,           uint16_t fg, uint16_t bg, uint8_t size);
void ST7789_DrawString(int16_t x, int16_t y, const char *str,  uint16_t fg, uint16_t bg, uint8_t size);
void ST7789_DrawInt   (int16_t x, int16_t y, int32_t val,      uint16_t fg, uint16_t bg, uint8_t size);
#endif
