################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../third_party/parson/parson.c 

OBJS += \
./third_party/parson/parson.o 

C_DEPS += \
./third_party/parson/parson.d 


# Each subdirectory must supply rules for building sources it contributes
third_party/parson/%.o: ../third_party/parson/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -I"C:\MCUXpressoIDE\workspace\smart_socket\source" -I"C:\MCUXpressoIDE\workspace\smart_socket\freertos" -I"C:\MCUXpressoIDE\workspace\smart_socket\frdmkw41z" -I"C:\MCUXpressoIDE\workspace\smart_socket\drivers" -I"C:\MCUXpressoIDE\workspace\smart_socket\utilities" -I"C:\MCUXpressoIDE\workspace\smart_socket\startup" -I"C:\MCUXpressoIDE\workspace\smart_socket\CMSIS" -I"C:\MCUXpressoIDE\workspace\smart_socket\openthread\include" -I"C:\MCUXpressoIDE\workspace\smart_socket\openthread\examples\platforms" -I"C:\MCUXpressoIDE\workspace\smart_socket\openthread\examples\platforms\kw41z" -I"C:\MCUXpressoIDE\workspace\smart_socket\openthread\third_party\mbedtls" -I"C:\MCUXpressoIDE\workspace\smart_socket\openthread\third_party\mbedtls\repo.patched\include" -I"C:\MCUXpressoIDE\workspace\smart_socket\openthread\src" -I"C:\MCUXpressoIDE\workspace\smart_socket\openthread\src\core" -I"C:\MCUXpressoIDE\workspace\smart_socket\openthread\third_party\nxp\MKW41Z4\XCVR" -I"C:\MCUXpressoIDE\workspace\smart_socket" -I"C:\MCUXpressoIDE\workspace\smart_socket\third_party\parson" -I"C:\MCUXpressoIDE\workspace\smart_socket\metrology\include" -I"C:\MCUXpressoIDE\workspace\smart_socket\metrology\tasks\inc" -I"C:\MCUXpressoIDE\workspace\smart_socket\metrology\drivers\inc" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


