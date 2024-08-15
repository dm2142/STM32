################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bsp/Src/tm1637.c 

OBJS += \
./Bsp/Src/tm1637.o 

C_DEPS += \
./Bsp/Src/tm1637.d 


# Each subdirectory must supply rules for building sources it contributes
Bsp/Src/%.o Bsp/Src/%.su Bsp/Src/%.cyclo: ../Bsp/Src/%.c Bsp/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I"C:/Users/DM/STM32CubeIDE/workspace_embedded_c/STM32F103C6T6/Bsp/Inc" -I"C:/Users/DM/STM32CubeIDE/workspace_embedded_c/STM32F103C6T6/Bsp/Src" -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Bsp-2f-Src

clean-Bsp-2f-Src:
	-$(RM) ./Bsp/Src/tm1637.cyclo ./Bsp/Src/tm1637.d ./Bsp/Src/tm1637.o ./Bsp/Src/tm1637.su

.PHONY: clean-Bsp-2f-Src

