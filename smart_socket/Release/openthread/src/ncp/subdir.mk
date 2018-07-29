################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../openthread/src/ncp/changed_props_set.cpp \
../openthread/src/ncp/example_vendor_hook.cpp \
../openthread/src/ncp/hdlc.cpp \
../openthread/src/ncp/ncp_base.cpp \
../openthread/src/ncp/ncp_base_ftd.cpp \
../openthread/src/ncp/ncp_base_mtd.cpp \
../openthread/src/ncp/ncp_base_radio.cpp \
../openthread/src/ncp/ncp_buffer.cpp \
../openthread/src/ncp/ncp_spi.cpp \
../openthread/src/ncp/ncp_uart.cpp \
../openthread/src/ncp/spinel_decoder.cpp \
../openthread/src/ncp/spinel_encoder.cpp 

C_SRCS += \
../openthread/src/ncp/spinel.c 

OBJS += \
./openthread/src/ncp/changed_props_set.o \
./openthread/src/ncp/example_vendor_hook.o \
./openthread/src/ncp/hdlc.o \
./openthread/src/ncp/ncp_base.o \
./openthread/src/ncp/ncp_base_ftd.o \
./openthread/src/ncp/ncp_base_mtd.o \
./openthread/src/ncp/ncp_base_radio.o \
./openthread/src/ncp/ncp_buffer.o \
./openthread/src/ncp/ncp_spi.o \
./openthread/src/ncp/ncp_uart.o \
./openthread/src/ncp/spinel.o \
./openthread/src/ncp/spinel_decoder.o \
./openthread/src/ncp/spinel_encoder.o 

CPP_DEPS += \
./openthread/src/ncp/changed_props_set.d \
./openthread/src/ncp/example_vendor_hook.d \
./openthread/src/ncp/hdlc.d \
./openthread/src/ncp/ncp_base.d \
./openthread/src/ncp/ncp_base_ftd.d \
./openthread/src/ncp/ncp_base_mtd.d \
./openthread/src/ncp/ncp_base_radio.d \
./openthread/src/ncp/ncp_buffer.d \
./openthread/src/ncp/ncp_spi.d \
./openthread/src/ncp/ncp_uart.d \
./openthread/src/ncp/spinel_decoder.d \
./openthread/src/ncp/spinel_encoder.d 

C_DEPS += \
./openthread/src/ncp/spinel.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/src/ncp/%.o: ../openthread/src/ncp/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../freertos -I../frdmkw41z -I../utilities -I../startup -I../CMSIS -I../openthread/include -I../openthread/examples/platforms -I../openthread/examples/platforms/kw41z -I../openthread/third_party/mbedtls -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/src -I../openthread/src/core -I../source -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../metrology/include -I../metrology/drivers/inc -I../drivers -I../parson -I../metrology/tasks/inc -Os -fno-common -g -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

openthread/src/ncp/%.o: ../openthread/src/ncp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../parson -I../drivers -I../metrology/drivers/inc -I../metrology/tasks/inc -I../metrology/include -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../openthread/src/core -I../openthread/src -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/third_party/mbedtls -I../openthread/examples/platforms/kw41z -I../openthread/examples/platforms -I../openthread/include -I../CMSIS -I../startup -I../utilities -I../frdmkw41z -I../freertos -I../source -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


