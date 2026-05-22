################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Driver/drv_adc.c \
../Src/Driver/drv_encoder.c \
../Src/Driver/drv_st7789.c 

C_DEPS += \
./Src/Driver/drv_adc.d \
./Src/Driver/drv_encoder.d \
./Src/Driver/drv_st7789.d 

OBJS += \
./Src/Driver/drv_adc.o \
./Src/Driver/drv_encoder.o \
./Src/Driver/drv_st7789.o 


# Each subdirectory must supply rules for building sources it contributes
Src/Driver/%.o Src/Driver/%.su Src/Driver/%.cyclo: ../Src/Driver/%.c Src/Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/App" -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/Common" -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/Driver" -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/Service" -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/Lib" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-Driver

clean-Src-2f-Driver:
	-$(RM) ./Src/Driver/drv_adc.cyclo ./Src/Driver/drv_adc.d ./Src/Driver/drv_adc.o ./Src/Driver/drv_adc.su ./Src/Driver/drv_encoder.cyclo ./Src/Driver/drv_encoder.d ./Src/Driver/drv_encoder.o ./Src/Driver/drv_encoder.su ./Src/Driver/drv_st7789.cyclo ./Src/Driver/drv_st7789.d ./Src/Driver/drv_st7789.o ./Src/Driver/drv_st7789.su

.PHONY: clean-Src-2f-Driver

