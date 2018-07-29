################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../framework/Lists/GenericList.c 

OBJS += \
./framework/Lists/GenericList.o 

C_DEPS += \
./framework/Lists/GenericList.d 


# Each subdirectory must supply rules for building sources it contributes
framework/Lists/%.o: ../framework/Lists/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DSDK_DEBUGCONSOLE=0 -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\source" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\freertos" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\frdmkw41z" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\drivers" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\utilities" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\startup" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\CMSIS" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\openthread\include" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\openthread\examples\platforms" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\openthread\examples\platforms\kw41z" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\openthread\third_party\mbedtls" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\openthread\third_party\mbedtls\repo.patched\include" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\openthread\src" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\openthread\src\core" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\framework\XCVR\MKW41Z4" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\framework\common" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\framework\OSAbstraction\Interface" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\framework\Lists" -I"C:\MCUXpressoIDE\workspace\KW41Z_smart_socket\framework\Panic\Interface" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


