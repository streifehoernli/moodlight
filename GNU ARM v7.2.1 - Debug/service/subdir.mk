################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../service/sl_sleeptimer.c \
../service/sl_sleeptimer_hal_rtc.c 

OBJS += \
./service/sl_sleeptimer.o \
./service/sl_sleeptimer_hal_rtc.o 

C_DEPS += \
./service/sl_sleeptimer.d \
./service/sl_sleeptimer_hal_rtc.d 


# Each subdirectory must supply rules for building sources it contributes
service/sl_sleeptimer.o: ../service/sl_sleeptimer.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m3 -mthumb -std=c99 '-DEFM32G890F128=1' -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/config" -I"C:\gitrepo\moodlight\src\inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/src" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/common/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//app/mcu_example/EFM32_Gxxx_STK/emlcd" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/EFM32_Gxxx_STK/config" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/CMSIS/Include" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/emlib/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/bsp" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/drivers" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/Device/SiliconLabs/EFM32G/Include" -O3 -Wall -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -c -fmessage-length=0 -MMD -MP -MF"service/sl_sleeptimer.d" -MT"service/sl_sleeptimer.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

service/sl_sleeptimer_hal_rtc.o: ../service/sl_sleeptimer_hal_rtc.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m3 -mthumb -std=c99 '-DEFM32G890F128=1' -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/config" -I"C:\gitrepo\moodlight\src\inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/src" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/common/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//app/mcu_example/EFM32_Gxxx_STK/emlcd" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/EFM32_Gxxx_STK/config" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/CMSIS/Include" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/emlib/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/bsp" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/drivers" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/Device/SiliconLabs/EFM32G/Include" -O3 -Wall -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -c -fmessage-length=0 -MMD -MP -MF"service/sl_sleeptimer_hal_rtc.d" -MT"service/sl_sleeptimer_hal_rtc.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


