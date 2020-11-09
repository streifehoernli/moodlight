################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include service/subdir.mk
-include emlib/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/EFM32G/subdir.mk
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
all: MoodLight_Template.axf

# Tool invocations
MoodLight_Template.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m3 -mthumb -T "MoodLight_Template.ld" -Xlinker --gc-sections -Xlinker -Map="MoodLight_Template.map" --specs=nano.specs -o MoodLight_Template.axf "./BSP/bsp_bcc.o" "./BSP/bsp_stk.o" "./BSP/bsp_trace.o" "./CMSIS/EFM32G/startup_efm32g.o" "./CMSIS/EFM32G/system_efm32g.o" "./Drivers/segmentlcd.o" "./Drivers/vddcheck.o" "./emlib/em_acmp.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_lcd.o" "./emlib/em_leuart.o" "./emlib/em_rtc.o" "./emlib/em_system.o" "./emlib/em_timer.o" "./emlib/em_usart.o" "./emlib/em_vcmp.o" "./service/sl_sleeptimer.o" "./service/sl_sleeptimer_hal_rtc.o" "./src/communication.o" "./src/globals.o" "./src/main.o" "./src/powerLEDs.o" "./src/pushbuttons.o" "./src/signalLEDs.o" "./src/touchslider.o" "./src/userinterface.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: MoodLight_Template.hex'
	arm-none-eabi-objcopy -O ihex "MoodLight_Template.axf" "MoodLight_Template.hex"
	@echo ' '

	@echo 'Building bin file: MoodLight_Template.bin'
	arm-none-eabi-objcopy -O binary "MoodLight_Template.axf" "MoodLight_Template.bin"
	@echo ' '

	@echo 'Building s37 file: MoodLight_Template.s37'
	arm-none-eabi-objcopy -O srec "MoodLight_Template.axf" "MoodLight_Template.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "MoodLight_Template.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) MoodLight_Template.axf
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
