#include "drv_st7789.h"
#include "cmsis_os.h"

extern SPI_HandleTypeDef hspi1;

#define ST7789_CS_LOW()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define ST7789_CS_HIGH()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define ST7789_DC_CMD()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define ST7789_DC_DATA()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)
#define ST7789_RST_LOW()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET)
#define ST7789_RST_HIGH()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET)

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
	ST7789_WriteData(0x00);		//0x00, 0x70, 0xA0, 0xC0

	//bat hien thi
	ST7789_WriteCmd(0x29); 		//Display on
	osDelay(10);
}

//do mau toan man hinh
void ST7789_FillScreen(uint16_t color){
	uint8_t data[2] = {color >> 8, color & 0xFF};


	ST7789_WriteCmd(0x2A);		//cot X
	ST7789_WriteData(0x00);		ST7789_WriteData(0x00);
	ST7789_WriteData(0x00);		ST7789_WriteData(0xEF);

	ST7789_WriteCmd(0x2B);		//hang y
	ST7789_WriteData(0x00);		ST7789_WriteData(0x00);
	ST7789_WriteData(0x01);		ST7789_WriteData(0x3F);

	ST7789_WriteCmd(0x2C);

	ST7789_DC_DATA();
	ST7789_CS_LOW();

	for(uint32_t i = 0; i < 240 * 320; i++){
		HAL_SPI_Transmit(&hspi1, data, 2, 10);
	}
	ST7789_CS_HIGH();
}

void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	if(w == 0 || h == 0) return;
	ST7789_SetWindow(x, y, x + w -1, y + h - 1);
	uint8_t data[2] = {color >> 8, color & 0xFF};
	ST7789_DC_DATA();
	ST7789_CS_LOW();
	for(uint32_t i =0; i < (uint32_t)w * h; i++){
		HAL_SPI_Transmit(&hspi1, data, 2, 10);
	}
	ST7789_CS_HIGH();
}

void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
	ST7789_FillRect(x, 			y, 			w, 1, color);
	ST7789_FillRect(x, 			y + h - 1, 	w, 1, color);
	ST7789_FillRect(x, 			y, 			1, h, color);
	ST7789_FillRect(x + w - 1, 	y, 			1, h, color);
}
















