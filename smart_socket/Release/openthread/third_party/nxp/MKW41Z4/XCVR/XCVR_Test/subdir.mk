################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../openthread/third_party/nxp/MKW41Z4/XCVR/XCVR_Test/xcvr_test_fsk.c 

OBJS += \
./openthread/third_party/nxp/MKW41Z4/XCVR/XCVR_Test/xcvr_test_fsk.o 

C_DEPS += \
./openthread/third_party/nxp/MKW41Z4/XCVR/XCVR_Test/xcvr_test_fsk.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/third_party/nxp/MKW41Z4/XCVR/XCVR_Test/%.o: ../openthread/third_party/nxp/MKW41Z4/XCVR/XCVR_Test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../parson -I../drivers -I../metrology/drivers/inc -I../metrology/tasks/inc -I../metrology/include -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../openthread/src/core -I../openthread/src -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/third_party/mbedtls -I../openthread/examples/platforms/kw41z -I../openthread/examples/platforms -I../openthread/include -I../CMSIS -I../startup -I../utilities -I../frdmkw41z -I../freertos -I../source -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


