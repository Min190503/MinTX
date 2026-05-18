#include "svc_input.h"
#include "drv_adc.h"


JoystickAxis_t g_axes[4] = {
    {0, 1500, 200, 3900, 2048, 1}, // Roll
    {0, 1500, 200, 3900, 2048, 1}, // Pitch
    {0, 1500, 200, 3900, 2048, 1}, // Yaw
    {0, 1000, 200, 3900, 2048, 0},  // Throttle
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
        g_axes[i].raw = Drv_ADC_GetRaw(i);
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
    }
}
