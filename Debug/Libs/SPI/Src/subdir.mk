################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libs/SPI/Src/spi_adc.c 

OBJS += \
./Libs/SPI/Src/spi_adc.o 

C_DEPS += \
./Libs/SPI/Src/spi_adc.d 


# Each subdirectory must supply rules for building sources it contributes
Libs/SPI/Src/%.o Libs/SPI/Src/%.su Libs/SPI/Src/%.cyclo: ../Libs/SPI/Src/%.c Libs/SPI/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U575xx -c -I../Core/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I../Libs/ADC/Inc -I../Libs/SPI/Inc -I../Libs/UART/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libs-2f-SPI-2f-Src

clean-Libs-2f-SPI-2f-Src:
	-$(RM) ./Libs/SPI/Src/spi_adc.cyclo ./Libs/SPI/Src/spi_adc.d ./Libs/SPI/Src/spi_adc.o ./Libs/SPI/Src/spi_adc.su

.PHONY: clean-Libs-2f-SPI-2f-Src

