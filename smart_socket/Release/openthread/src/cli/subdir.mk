################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../openthread/src/cli/cli.cpp \
../openthread/src/cli/cli_coap.cpp \
../openthread/src/cli/cli_console.cpp \
../openthread/src/cli/cli_dataset.cpp \
../openthread/src/cli/cli_instance.cpp \
../openthread/src/cli/cli_uart.cpp \
../openthread/src/cli/cli_udp.cpp \
../openthread/src/cli/cli_udp_example.cpp 

OBJS += \
./openthread/src/cli/cli.o \
./openthread/src/cli/cli_coap.o \
./openthread/src/cli/cli_console.o \
./openthread/src/cli/cli_dataset.o \
./openthread/src/cli/cli_instance.o \
./openthread/src/cli/cli_uart.o \
./openthread/src/cli/cli_udp.o \
./openthread/src/cli/cli_udp_example.o 

CPP_DEPS += \
./openthread/src/cli/cli.d \
./openthread/src/cli/cli_coap.d \
./openthread/src/cli/cli_console.d \
./openthread/src/cli/cli_dataset.d \
./openthread/src/cli/cli_instance.d \
./openthread/src/cli/cli_uart.d \
./openthread/src/cli/cli_udp.d \
./openthread/src/cli/cli_udp_example.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/src/cli/%.o: ../openthread/src/cli/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../freertos -I../frdmkw41z -I../utilities -I../startup -I../CMSIS -I../openthread/include -I../openthread/examples/platforms -I../openthread/examples/platforms/kw41z -I../openthread/third_party/mbedtls -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/src -I../openthread/src/core -I../source -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../metrology/include -I../metrology/drivers/inc -I../drivers -I../parson -I../metrology/tasks/inc -Os -fno-common -g -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


