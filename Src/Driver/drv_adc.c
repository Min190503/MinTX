#include "drv_adc.h"

extern ADC_HandleTypeDef hadc1;

//Mang DMA cap nhat lien tuc 4 kenh
static volatile uint16_t adc_raw[4];


void Drv_ADC_Start(void){
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw, 4);
}

uint16_t Drv_ADC_GetRaw(uint8_t channel){
	if(channel >= 4) return 0;
	return adc_raw[channel];
}
