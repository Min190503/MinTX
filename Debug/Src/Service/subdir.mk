################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Service/svc_input.c \
../Src/Service/svc_rf.c \
../Src/Service/svc_storage.c 

C_DEPS += \
./Src/Service/svc_input.d \
./Src/Service/svc_rf.d \
./Src/Service/svc_storage.d 

OBJS += \
./Src/Service/svc_input.o \
./Src/Service/svc_rf.o \
./Src/Service/svc_storage.o 


# Each subdirectory must supply rules for building sources it contributes
Src/Service/%.o Src/Service/%.su Src/Service/%.cyclo: ../Src/Service/%.c Src/Service/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/App" -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/Common" -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/Driver" -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/Service" -I"/home/min/FC_mamab_4_H743_V2/MinTX/Src/Lib" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-Service

clean-Src-2f-Service:
	-$(RM) ./Src/Service/svc_input.cyclo ./Src/Service/svc_input.d ./Src/Service/svc_input.o ./Src/Service/svc_input.su ./Src/Service/svc_rf.cyclo ./Src/Service/svc_rf.d ./Src/Service/svc_rf.o ./Src/Service/svc_rf.su ./Src/Service/svc_storage.cyclo ./Src/Service/svc_storage.d ./Src/Service/svc_storage.o ./Src/Service/svc_storage.su

.PHONY: clean-Src-2f-Service

