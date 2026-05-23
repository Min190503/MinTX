#include "svc_storage.h"
#include "svc_input.h"
#include "FreeRTOS.h"
#include "task.h"

extern uint8_t sys_protocol;

void Svc_Storage_Load(void) {
    uint32_t* flash_data = (uint32_t*)0x08060000; // Đọc thẳng mảng 32-bit từ Sector 7
    JoystickAxis_t* axes = Svc_Input_GetAxes();

    if (flash_data[7] == 0xDEADBEEF) { // Check Magic Word ở Word cuối cùng
        // Giải mã bằng tay, đố compiler làm sai được
        axes[0].cal_min = flash_data[0] & 0xFFFF;
        axes[1].cal_min = (flash_data[0] >> 16) & 0xFFFF;
        axes[2].cal_min = flash_data[1] & 0xFFFF;
        axes[3].cal_min = (flash_data[1] >> 16) & 0xFFFF;

        axes[0].cal_max = flash_data[2] & 0xFFFF;
        axes[1].cal_max = (flash_data[2] >> 16) & 0xFFFF;
        axes[2].cal_max = flash_data[3] & 0xFFFF;
        axes[3].cal_max = (flash_data[3] >> 16) & 0xFFFF;

        axes[0].cal_mid = flash_data[4] & 0xFFFF;
        axes[1].cal_mid = (flash_data[4] >> 16) & 0xFFFF;
        axes[2].cal_mid = flash_data[5] & 0xFFFF;
        axes[3].cal_mid = (flash_data[5] >> 16) & 0xFFFF;

        sys_protocol = flash_data[6] & 0xFF;
        if (sys_protocol > 1) {
                    sys_protocol = 0;
                }
    } else {
        // Nạp an toàn nếu Flash rỗng
        for(int i=0; i<4; i++) {
            axes[i].cal_min = 200;
            axes[i].cal_max = 3900;
            axes[i].cal_mid = 2048;
        }
        sys_protocol = 0;
    }
}

void Svc_Storage_Save(void) {
    uint32_t flash_buffer[8];
    JoystickAxis_t* axes = Svc_Input_GetAxes();

    // Đóng gói thủ công 16-bit vào 32-bit (Bit-packing)
    flash_buffer[0] = ((uint32_t)axes[1].cal_min << 16) | axes[0].cal_min;
    flash_buffer[1] = ((uint32_t)axes[3].cal_min << 16) | axes[2].cal_min;
    flash_buffer[2] = ((uint32_t)axes[1].cal_max << 16) | axes[0].cal_max;
    flash_buffer[3] = ((uint32_t)axes[3].cal_max << 16) | axes[2].cal_max;
    flash_buffer[4] = ((uint32_t)axes[1].cal_mid << 16) | axes[0].cal_mid;
    flash_buffer[5] = ((uint32_t)axes[3].cal_mid << 16) | axes[2].cal_mid;
    flash_buffer[6] = sys_protocol;
    flash_buffer[7] = 0xDEADBEEF;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = FLASH_SECTOR_7;
    EraseInitStruct.NbSectors     = 1;

    taskENTER_CRITICAL();

    HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

    // Ghi thẳng mảng uint32_t an toàn tuyệt đối
    for (int i = 0; i < 8; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08060000 + (i * 4), flash_buffer[i]);
    }

    taskEXIT_CRITICAL();

    HAL_FLASH_Lock();
}
