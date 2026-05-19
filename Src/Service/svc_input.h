#ifndef SVC_INPUT_H
#define SVC_INPUT_H

#include <stdint.h>


//struct data joystick
typedef struct {
	uint16_t raw;			//ADC tho(0-4095)
	uint16_t mapped;		//gia tri map(1000-2000)
	uint16_t cal_min;		// ADC min
	uint16_t cal_max;
	uint16_t cal_mid;
	uint8_t use_center;
	uint8_t invert;
} JoystickAxis_t;


extern JoystickAxis_t g_axes[7];


void Svc_Input_Init(void);
void Svc_Input_Update(void);
JoystickAxis_t* Svc_Input_GetAxes(void);
#endif
