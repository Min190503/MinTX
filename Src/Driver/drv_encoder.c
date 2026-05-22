#include "drv_encoder.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

typedef struct {
	TIM_HandleTypeDef *htim;
	uint32_t last_counter;
	int32_t diff_acc;
	GPIO_TypeDef *btn_port;
	uint16_t btn_pin;
	uint8_t btn_state;
	uint8_t btn_flag;
	uint32_t btn_debounce_time;
} EncoderState_t;

static EncoderState_t encoders[ENCODER_MAX] = {
		{
				.htim = &htim3, .last_counter = 0, .diff_acc = 0,
				.btn_port = GPIOB, .btn_pin = GPIO_PIN_3,
				.btn_state = 1, .btn_flag = 0, .btn_debounce_time = 0
		},
		{
				.htim = &htim4, .last_counter = 0, .diff_acc = 0,
				.btn_port = GPIOB, .btn_pin = GPIO_PIN_8,
				.btn_state = 1, .btn_flag = 0, .btn_debounce_time = 0
		}
};

void Drv_Encoder_Init(void){
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

	encoders[ENCODER_1].last_counter = __HAL_TIM_GET_COUNTER(&htim3);
	encoders[ENCODER_2].last_counter = __HAL_TIM_GET_COUNTER(&htim4);

}

void Drv_Encoder_Update(void){
	uint32_t current_time = HAL_GetTick();

	for(int i = 0; i < ENCODER_MAX; i++){
		uint32_t current_counter = __HAL_TIM_GET_COUNTER(encoders[i].htim);
		int16_t diff = (int16_t)(current_counter - encoders[i].last_counter);

		if(diff != 0){
			encoders[i].diff_acc += diff;
			encoders[i].last_counter = current_counter;
		}

		uint8_t current_btn = HAL_GPIO_ReadPin(encoders[i].btn_port, encoders[i].btn_pin);

		if(current_btn != encoders[i].btn_state){
			if(current_time - encoders[i].btn_debounce_time > 50){
				encoders[i].btn_state = current_btn;
				encoders[i].btn_debounce_time = current_time;

				if(current_btn == GPIO_PIN_RESET){
					encoders[i].btn_flag = 1;
				}
			}
		}
	}
}
EncoderDir_t Drv_Encoder_GetDir(EncoderId_t id) {
    if (id >= ENCODER_MAX) return ENC_DIR_NONE;

    if (encoders[id].diff_acc >= 4) {
        encoders[id].diff_acc -= 4;
        return ENC_DIR_CW;
    } else if (encoders[id].diff_acc <= -4) {
        encoders[id].diff_acc += 4;
        return ENC_DIR_CCW;
    }
    return ENC_DIR_NONE;
}
uint8_t Drv_Encoder_GetButton(EncoderId_t id) {
    if (id >= ENCODER_MAX) return 0;

    if (encoders[id].btn_flag) {
        encoders[id].btn_flag = 0;
        return 1;
    }
    return 0;
}
