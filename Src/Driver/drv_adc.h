#ifndef DRV_ADC_H
#define DRV_ADC_H

#include "main.h"
#include <stdint.h>

#define ADC_CH_ROLL			0		// PA0 - IN0
#define ADC_CH_PITCH		1		// PA1 - IN1
#define ADC_CH_YAW			2		// PA6 - IN6
#define ADC_CH_THROTTLE		3		// PB0 - IN8

void Drv_ADC_Start(void);
uint16_t Drv_ADC_GetRaw(uint8_t channel);

#endif
