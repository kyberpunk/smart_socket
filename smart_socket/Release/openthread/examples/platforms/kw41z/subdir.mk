################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../openthread/examples/platforms/kw41z/alarm.c \
../openthread/examples/platforms/kw41z/diag.c \
../openthread/examples/platforms/kw41z/flash.c \
../openthread/examples/platforms/kw41z/logging.c \
../openthread/examples/platforms/kw41z/misc.c \
../openthread/examples/platforms/kw41z/platform.c \
../openthread/examples/platforms/kw41z/radio.c \
../openthread/examples/platforms/kw41z/random.c \
../openthread/examples/platforms/kw41z/uart.c 

OBJS += \
./openthread/examples/platforms/kw41z/alarm.o \
./openthread/examples/platforms/kw41z/diag.o \
./openthread/examples/platforms/kw41z/flash.o \
./openthread/examples/platforms/kw41z/logging.o \
./openthread/examples/platforms/kw41z/misc.o \
./openthread/examples/platforms/kw41z/platform.o \
./openthread/examples/platforms/kw41z/radio.o \
./openthread/examples/platforms/kw41z/random.o \
./openthread/examples/platforms/kw41z/uart.o 

C_DEPS += \
./openthread/examples/platforms/kw41z/alarm.d \
./openthread/examples/platforms/kw41z/diag.d \
./openthread/examples/platforms/kw41z/flash.d \
./openthread/examples/platforms/kw41z/logging.d \
./openthread/examples/platforms/kw41z/misc.d \
./openthread/examples/platforms/kw41z/platform.d \
./openthread/examples/platforms/kw41z/radio.d \
./openthread/examples/platforms/kw41z/random.d \
./openthread/examples/platforms/kw41z/uart.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/examples/platforms/kw41z/%.o: ../openthread/examples/platforms/kw41z/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../parson -I../drivers -I../metrology/drivers/inc -I../metrology/tasks/inc -I../metrology/include -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../openthread/src/core -I../openthread/src -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/third_party/mbedtls -I../openthread/examples/platforms/kw41z -I../openthread/examples/platforms -I../openthread/include -I../CMSIS -I../startup -I../utilities -I../frdmkw41z -I../freertos -I../source -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


