#include "svc_input.h"
#include "drv_adc.h"


JoystickAxis_t g_axes[7] = {
    {0, 1500, 200, 3900, 2048, 1, 1}, // Roll
    {0, 1500, 200, 3900, 2048, 1, 1}, // Pitch
    {0, 1500, 200, 3900, 2048, 1, 0}, // Yaw
    {0, 1000, 200, 3900, 2048, 0, 0},  // Throttle

	{0, 1000, 0, 4095, 2048, 0, 0},   // CH5: Aux 1 (Mặc định mức thấp 1000)
	{0, 1000, 0, 4095, 2048, 0, 0},   // CH6: Aux 2 (Mặc định mức thấp 1000)
	{0, 1000, 0, 4095, 2048, 0, 0},   // CH7: Aux 3 (Mặc định mức thấp 1000)
};


static uint16_t map_linear(uint16_t val,
                            uint16_t in_min, uint16_t in_max,
                            uint16_t out_min, uint16_t out_max) {
    if (val <= in_min) return out_min;
    if (val >= in_max) return out_max;
    return (uint16_t)((uint32_t)(val - in_min) * (out_max - out_min)
                      / (in_max - in_min) + out_min);
}
static uint16_t map_dual_slope(uint16_t val,
                               uint16_t in_min, uint16_t in_mid, uint16_t in_max,
                               uint16_t out_min, uint16_t out_mid, uint16_t out_max){
    if(val <= in_min) return out_min;
    if(val >= in_max) return out_max;

    // Khoảng chết +- 15 đơn vị chống rung lò xo
    if(val >= (in_mid - 15) && val <= (in_mid + 15)) return out_mid;

    if(val < in_mid){
        return (uint16_t)((uint32_t)(val - in_min) * (out_mid - out_min) / (in_mid - in_min) + out_min);
    } else {
        return (uint16_t)((uint32_t)(val - in_mid) * (out_max - out_mid) / (in_max - in_mid) + out_mid);
    }
}

void Svc_Input_Init(void) {
    Drv_ADC_Start();
}

void Svc_Input_Update(void) {
    for (int i = 0; i < 4; i++) {
        uint16_t new_raw = Drv_ADC_GetRaw(i);
        g_axes[i].raw = (g_axes[i].raw == 0) ? new_raw : (g_axes[i].raw * 8 + new_raw * 2) / 10;


        if (g_axes[i].use_center) {
            // Roll, Pitch, Yaw → dual-slope
            g_axes[i].mapped = map_dual_slope(g_axes[i].raw,
                                              g_axes[i].cal_min,
                                              g_axes[i].cal_mid,
                                              g_axes[i].cal_max,
                                              1000, 1500, 2000);
        } else {
            // Throttle → linear
            g_axes[i].mapped = map_linear(g_axes[i].raw,
                                          g_axes[i].cal_min,
                                          g_axes[i].cal_max,
                                          1000, 2000);
        }
        if (g_axes[i].invert == 1){
        	g_axes[i].mapped = 3000 - g_axes[i].mapped;
        }
    }

    //Cong tac 1
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_RESET){
    	g_axes[4].mapped = 2000;
    } else {
    	g_axes[4].mapped = 1000;
    }
    //cong tac 2
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET){
    	g_axes[5].mapped = 2000;
    } else {
    	g_axes[5].mapped = 1000;
    }
    //cong tac 3
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET){
		g_axes[6].mapped = 2000;
	} else {
		g_axes[6].mapped = 1000;
	}
}
JoystickAxis_t* Svc_Input_GetAxes(void) {
    return g_axes;
}
