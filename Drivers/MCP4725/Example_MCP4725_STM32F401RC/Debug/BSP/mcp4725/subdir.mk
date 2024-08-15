################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSP/mcp4725/mcp4725.c 

OBJS += \
./BSP/mcp4725/mcp4725.o 

C_DEPS += \
./BSP/mcp4725/mcp4725.d 


# Each subdirectory must supply rules for building sources it contributes
BSP/mcp4725/%.o BSP/mcp4725/%.su BSP/mcp4725/%.cyclo: ../BSP/mcp4725/%.c BSP/mcp4725/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I"C:/Users/DM/STM32CubeIDE/workspace_embedded_c/SMT32_ModulesDrivers/BSP/mcp4725" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSP-2f-mcp4725

clean-BSP-2f-mcp4725:
	-$(RM) ./BSP/mcp4725/mcp4725.cyclo ./BSP/mcp4725/mcp4725.d ./BSP/mcp4725/mcp4725.o ./BSP/mcp4725/mcp4725.su

.PHONY: clean-BSP-2f-mcp4725

