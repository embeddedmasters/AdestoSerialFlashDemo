################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/efm32lg/subdir.mk
-include BSP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: AdestoSerialFlashDemo.axf

# Tool invocations
AdestoSerialFlashDemo.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m3 -mthumb -T "AdestoSerialFlashDemo.ld" -Xlinker --gc-sections -Xlinker -Map="AdestoSerialFlashDemo.map" --specs=nano.specs -o AdestoSerialFlashDemo.axf "./src/buffer.o" "./src/button.o" "./src/delay.o" "./src/demo_serial.o" "./src/fatal.o" "./src/gpio.o" "./src/hex_dump.o" "./src/lcd_scroll.o" "./src/lcdtest.o" "./src/led.o" "./src/low_power.o" "./src/main.o" "./src/oneshot.o" "./src/serial.o" "./src/spi.o" "./src/spiflash.o" "./emlib/em_acmp.o" "./emlib/em_lesense.o" "./emlib/em_leuart.o" "./emlib/em_usart.o" "./Drivers/caplesense.o" "./Drivers/retargetio.o" "./Drivers/rtcdriver.o" "./Drivers/segmentlcd.o" "./Drivers/vddcheck.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_int.o" "./emlib/em_lcd.o" "./emlib/em_rtc.o" "./emlib/em_system.o" "./emlib/em_vcmp.o" "./CMSIS/efm32lg/startup_efm32lg.o" "./CMSIS/efm32lg/system_efm32lg.o" "./BSP/bsp_trace.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: AdestoSerialFlashDemo.hex'
	arm-none-eabi-objcopy -O ihex "AdestoSerialFlashDemo.axf" "AdestoSerialFlashDemo.hex"
	@echo ' '

	@echo 'Building bin file: AdestoSerialFlashDemo.bin'
	arm-none-eabi-objcopy -O binary "AdestoSerialFlashDemo.axf" "AdestoSerialFlashDemo.bin"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "AdestoSerialFlashDemo.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) AdestoSerialFlashDemo.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
