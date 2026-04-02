################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/audio.c \
../Core/Src/breakout.c \
../Core/Src/breakout_sfx.c \
../Core/Src/buttons.c \
../Core/Src/dac.c \
../Core/Src/display.c \
../Core/Src/dma.c \
../Core/Src/font.c \
../Core/Src/globals.c \
../Core/Src/gpio.c \
../Core/Src/input.c \
../Core/Src/loop.c \
../Core/Src/main.c \
../Core/Src/menu.c \
../Core/Src/menu_sfx.c \
../Core/Src/rng.c \
../Core/Src/setup.c \
../Core/Src/sfx.c \
../Core/Src/snake.c \
../Core/Src/snake_sfx.c \
../Core/Src/stm32g4xx_hal_msp.c \
../Core/Src/stm32g4xx_it.c \
../Core/Src/stm32g4xx_nucleo_bus.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32g4xx.c \
../Core/Src/tetris.c \
../Core/Src/tetris_audio.c \
../Core/Src/text.c \
../Core/Src/tim.c \
../Core/Src/timers.c 

OBJS += \
./Core/Src/audio.o \
./Core/Src/breakout.o \
./Core/Src/breakout_sfx.o \
./Core/Src/buttons.o \
./Core/Src/dac.o \
./Core/Src/display.o \
./Core/Src/dma.o \
./Core/Src/font.o \
./Core/Src/globals.o \
./Core/Src/gpio.o \
./Core/Src/input.o \
./Core/Src/loop.o \
./Core/Src/main.o \
./Core/Src/menu.o \
./Core/Src/menu_sfx.o \
./Core/Src/rng.o \
./Core/Src/setup.o \
./Core/Src/sfx.o \
./Core/Src/snake.o \
./Core/Src/snake_sfx.o \
./Core/Src/stm32g4xx_hal_msp.o \
./Core/Src/stm32g4xx_it.o \
./Core/Src/stm32g4xx_nucleo_bus.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32g4xx.o \
./Core/Src/tetris.o \
./Core/Src/tetris_audio.o \
./Core/Src/text.o \
./Core/Src/tim.o \
./Core/Src/timers.o 

C_DEPS += \
./Core/Src/audio.d \
./Core/Src/breakout.d \
./Core/Src/breakout_sfx.d \
./Core/Src/buttons.d \
./Core/Src/dac.d \
./Core/Src/display.d \
./Core/Src/dma.d \
./Core/Src/font.d \
./Core/Src/globals.d \
./Core/Src/gpio.d \
./Core/Src/input.d \
./Core/Src/loop.d \
./Core/Src/main.d \
./Core/Src/menu.d \
./Core/Src/menu_sfx.d \
./Core/Src/rng.d \
./Core/Src/setup.d \
./Core/Src/sfx.d \
./Core/Src/snake.d \
./Core/Src/snake_sfx.d \
./Core/Src/stm32g4xx_hal_msp.d \
./Core/Src/stm32g4xx_it.d \
./Core/Src/stm32g4xx_nucleo_bus.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32g4xx.d \
./Core/Src/tetris.d \
./Core/Src/tetris_audio.d \
./Core/Src/text.d \
./Core/Src/tim.d \
./Core/Src/timers.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32G4xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../DISPLAY/Target -I../Drivers/BSP/Components/ili9341 -I../Drivers/BSP/Components/Common -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/audio.cyclo ./Core/Src/audio.d ./Core/Src/audio.o ./Core/Src/audio.su ./Core/Src/breakout.cyclo ./Core/Src/breakout.d ./Core/Src/breakout.o ./Core/Src/breakout.su ./Core/Src/breakout_sfx.cyclo ./Core/Src/breakout_sfx.d ./Core/Src/breakout_sfx.o ./Core/Src/breakout_sfx.su ./Core/Src/buttons.cyclo ./Core/Src/buttons.d ./Core/Src/buttons.o ./Core/Src/buttons.su ./Core/Src/dac.cyclo ./Core/Src/dac.d ./Core/Src/dac.o ./Core/Src/dac.su ./Core/Src/display.cyclo ./Core/Src/display.d ./Core/Src/display.o ./Core/Src/display.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/font.cyclo ./Core/Src/font.d ./Core/Src/font.o ./Core/Src/font.su ./Core/Src/globals.cyclo ./Core/Src/globals.d ./Core/Src/globals.o ./Core/Src/globals.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/input.cyclo ./Core/Src/input.d ./Core/Src/input.o ./Core/Src/input.su ./Core/Src/loop.cyclo ./Core/Src/loop.d ./Core/Src/loop.o ./Core/Src/loop.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/menu.cyclo ./Core/Src/menu.d ./Core/Src/menu.o ./Core/Src/menu.su ./Core/Src/menu_sfx.cyclo ./Core/Src/menu_sfx.d ./Core/Src/menu_sfx.o ./Core/Src/menu_sfx.su ./Core/Src/rng.cyclo ./Core/Src/rng.d ./Core/Src/rng.o ./Core/Src/rng.su ./Core/Src/setup.cyclo ./Core/Src/setup.d ./Core/Src/setup.o ./Core/Src/setup.su ./Core/Src/sfx.cyclo ./Core/Src/sfx.d ./Core/Src/sfx.o ./Core/Src/sfx.su ./Core/Src/snake.cyclo ./Core/Src/snake.d ./Core/Src/snake.o ./Core/Src/snake.su ./Core/Src/snake_sfx.cyclo ./Core/Src/snake_sfx.d ./Core/Src/snake_sfx.o ./Core/Src/snake_sfx.su ./Core/Src/stm32g4xx_hal_msp.cyclo ./Core/Src/stm32g4xx_hal_msp.d ./Core/Src/stm32g4xx_hal_msp.o ./Core/Src/stm32g4xx_hal_msp.su ./Core/Src/stm32g4xx_it.cyclo ./Core/Src/stm32g4xx_it.d ./Core/Src/stm32g4xx_it.o ./Core/Src/stm32g4xx_it.su ./Core/Src/stm32g4xx_nucleo_bus.cyclo ./Core/Src/stm32g4xx_nucleo_bus.d ./Core/Src/stm32g4xx_nucleo_bus.o ./Core/Src/stm32g4xx_nucleo_bus.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32g4xx.cyclo ./Core/Src/system_stm32g4xx.d ./Core/Src/system_stm32g4xx.o ./Core/Src/system_stm32g4xx.su ./Core/Src/tetris.cyclo ./Core/Src/tetris.d ./Core/Src/tetris.o ./Core/Src/tetris.su ./Core/Src/tetris_audio.cyclo ./Core/Src/tetris_audio.d ./Core/Src/tetris_audio.o ./Core/Src/tetris_audio.su ./Core/Src/text.cyclo ./Core/Src/text.d ./Core/Src/text.o ./Core/Src/text.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/timers.cyclo ./Core/Src/timers.d ./Core/Src/timers.o ./Core/Src/timers.su

.PHONY: clean-Core-2f-Src

