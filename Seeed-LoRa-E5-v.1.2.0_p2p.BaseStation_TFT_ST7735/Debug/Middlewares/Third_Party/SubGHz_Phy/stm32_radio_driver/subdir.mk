################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.c \
../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.c \
../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.c 

OBJS += \
./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.o \
./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.o \
./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.o 

C_DEPS += \
./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.d \
./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.d \
./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/%.o Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/%.su: ../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/%.c Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../Core/Inc -I"E:/STM32WL - LoRa/project 10.1 IDE/Seeed-LoRa-E5-v.1.2.0_p2p.BaseStation_TFT_ST7735/Drivers/BSP/STM32WLxx_Nucleo" -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../Drivers/CMSIS/Include -I../Utilities/trace/adv_trace -I../Utilities/misc -I../Utilities/sequencer -I../Utilities/timer -I../Utilities/lpm/tiny_lpm -I../Middlewares/Third_Party/SubGHz_Phy -I../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../SubGHz_Phy/App -I../SubGHz_Phy/Target -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-SubGHz_Phy-2f-stm32_radio_driver

clean-Middlewares-2f-Third_Party-2f-SubGHz_Phy-2f-stm32_radio_driver:
	-$(RM) ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.d ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.o ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.su ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.d ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.o ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.su ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.d ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.o ./Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-SubGHz_Phy-2f-stm32_radio_driver

