#ifndef DRV_ENCODER_H
#define DRV_ENCODER_H

#include "main.h"

typedef enum {
	ENCODER_1 = 0,
	ENCODER_2 = 1,
	ENCODER_MAX
}EncoderId_t;

typedef enum {
	ENC_DIR_NONE = 0,
	ENC_DIR_CW = 1,
	ENC_DIR_CCW = -1
}EncoderDir_t;

void Drv_Encoder_Init(void);
void Drv_Encoder_Update(void);

EncoderDir_t Drv_Encoder_GetDir(EncoderId_t id);


uint8_t Drv_Encoder_GetButton(EncoderId_t id);

#endif
