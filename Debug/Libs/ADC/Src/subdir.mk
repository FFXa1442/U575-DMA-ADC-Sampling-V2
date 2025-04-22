################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libs/ADC/Src/signal.c 

OBJS += \
./Libs/ADC/Src/signal.o 

C_DEPS += \
./Libs/ADC/Src/signal.d 


# Each subdirectory must supply rules for building sources it contributes
Libs/ADC/Src/%.o Libs/ADC/Src/%.su Libs/ADC/Src/%.cyclo: ../Libs/ADC/Src/%.c Libs/ADC/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U575xx -c -I../Core/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../Libs/ADC/Inc -I../Libs/SPI/Inc -I../Libs/UART/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libs-2f-ADC-2f-Src

clean-Libs-2f-ADC-2f-Src:
	-$(RM) ./Libs/ADC/Src/signal.cyclo ./Libs/ADC/Src/signal.d ./Libs/ADC/Src/signal.o ./Libs/ADC/Src/signal.su

.PHONY: clean-Libs-2f-ADC-2f-Src

