################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0/platform/Device/SiliconLabs/EFM32G/Source/system_efm32g.c 

S_UPPER_SRCS += \
C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0/platform/Device/SiliconLabs/EFM32G/Source/GCC/startup_efm32g.S 

OBJS += \
./CMSIS/EFM32G/startup_efm32g.o \
./CMSIS/EFM32G/system_efm32g.o 

C_DEPS += \
./CMSIS/EFM32G/system_efm32g.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS/EFM32G/startup_efm32g.o: C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0/platform/Device/SiliconLabs/EFM32G/Source/GCC/startup_efm32g.S
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Assembler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m3 -mthumb -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/config" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/src" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/common/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//app/mcu_example/EFM32_Gxxx_STK/emlcd" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/EFM32_Gxxx_STK/config" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/CMSIS/Include" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/emlib/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/bsp" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/drivers" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/Device/SiliconLabs/EFM32G/Include" '-DEFM32G890F128=1' -c -x assembler-with-cpp -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

CMSIS/EFM32G/system_efm32g.o: C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0/platform/Device/SiliconLabs/EFM32G/Source/system_efm32g.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m3 -mthumb -std=c99 '-DEFM32G890F128=1' -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/config" -I"C:\Users\eand\SimplicityStudio\v5_workspace\MoodLight_Template\src\inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/service/sleeptimer/src" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/common/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//app/mcu_example/EFM32_Gxxx_STK/emlcd" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/EFM32_Gxxx_STK/config" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/CMSIS/Include" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/emlib/inc" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/bsp" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//hardware/kit/common/drivers" -I"C:/SiliconLabs/SimplicityStudio/v5/developer/sdks/gecko_sdk_suite/v3.0//platform/Device/SiliconLabs/EFM32G/Include" -O0 -Wall -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -c -fmessage-length=0 -MMD -MP -MF"CMSIS/EFM32G/system_efm32g.d" -MT"CMSIS/EFM32G/system_efm32g.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


