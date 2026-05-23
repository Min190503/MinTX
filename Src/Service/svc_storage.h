#ifndef SVC_STORAGE_H
#define SVC_STORAGE_H

#include "main.h"

typedef struct {
	uint16_t cal_min[4];
	uint16_t cal_max[4];
	uint16_t cal_mid[4];
	uint8_t  protocol;  //0: CRSF, 1:IBUS
	uint8_t  reserved[3];
	uint32_t magic_word;	//pass
}SystemSettings_t;

void Svc_Storage_Load(void);
void Svc_Storage_Save(void);

#endif
