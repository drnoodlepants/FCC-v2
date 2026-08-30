################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/can_driver.c \
../App/Src/control_system.c \
../App/Src/fan_control.c \
../App/Src/pot_fan.c \
../App/Src/sensor_inputs.c 

OBJS += \
./App/Src/can_driver.o \
./App/Src/control_system.o \
./App/Src/fan_control.o \
./App/Src/pot_fan.o \
./App/Src/sensor_inputs.o 

C_DEPS += \
./App/Src/can_driver.d \
./App/Src/control_system.d \
./App/Src/fan_control.d \
./App/Src/pot_fan.d \
./App/Src/sensor_inputs.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/%.o App/Src/%.su App/Src/%.cyclo: ../App/Src/%.c App/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Rx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Src

clean-App-2f-Src:
	-$(RM) ./App/Src/can_driver.cyclo ./App/Src/can_driver.d ./App/Src/can_driver.o ./App/Src/can_driver.su ./App/Src/control_system.cyclo ./App/Src/control_system.d ./App/Src/control_system.o ./App/Src/control_system.su ./App/Src/fan_control.cyclo ./App/Src/fan_control.d ./App/Src/fan_control.o ./App/Src/fan_control.su ./App/Src/pot_fan.cyclo ./App/Src/pot_fan.d ./App/Src/pot_fan.o ./App/Src/pot_fan.su ./App/Src/sensor_inputs.cyclo ./App/Src/sensor_inputs.d ./App/Src/sensor_inputs.o ./App/Src/sensor_inputs.su

.PHONY: clean-App-2f-Src

