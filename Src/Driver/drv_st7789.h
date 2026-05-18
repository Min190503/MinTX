#ifndef DRV_ST7789_H
#define DRV_ST7789_H

#include "main.h"

#define ST7789_BLACK   0x0000
#define ST7789_WHITE   0xFFFF
#define ST7789_RED     0xF800
#define ST7789_GREEN   0x07E0
#define ST7789_BLUE    0x001F


void ST7789_Init(void);
void ST7789_FillScreen(uint16_t color);
void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
#endif
