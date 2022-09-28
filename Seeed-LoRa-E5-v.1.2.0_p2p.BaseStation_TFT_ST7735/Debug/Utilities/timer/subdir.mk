################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utilities/timer/stm32_timer.c 

OBJS += \
./Utilities/timer/stm32_timer.o 

C_DEPS += \
./Utilities/timer/stm32_timer.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/timer/%.o Utilities/timer/%.su: ../Utilities/timer/%.c Utilities/timer/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../Core/Inc -I"E:/STM32WL - LoRa/project 10.1 IDE/Seeed-LoRa-E5-v.1.2.0_p2p.BaseStation_TFT_ST7735/Drivers/BSP/STM32WLxx_Nucleo" -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../Drivers/CMSIS/Include -I../Utilities/trace/adv_trace -I../Utilities/misc -I../Utilities/sequencer -I../Utilities/timer -I../Utilities/lpm/tiny_lpm -I../Middlewares/Third_Party/SubGHz_Phy -I../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../SubGHz_Phy/App -I../SubGHz_Phy/Target -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Utilities-2f-timer

clean-Utilities-2f-timer:
	-$(RM) ./Utilities/timer/stm32_timer.d ./Utilities/timer/stm32_timer.o ./Utilities/timer/stm32_timer.su

.PHONY: clean-Utilities-2f-timer

