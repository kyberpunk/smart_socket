################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../openthread/src/core/common/crc16.cpp \
../openthread/src/core/common/instance.cpp \
../openthread/src/core/common/locator.cpp \
../openthread/src/core/common/logging.cpp \
../openthread/src/core/common/message.cpp \
../openthread/src/core/common/notifier.cpp \
../openthread/src/core/common/tasklet.cpp \
../openthread/src/core/common/timer.cpp \
../openthread/src/core/common/tlvs.cpp \
../openthread/src/core/common/trickle_timer.cpp 

OBJS += \
./openthread/src/core/common/crc16.o \
./openthread/src/core/common/instance.o \
./openthread/src/core/common/locator.o \
./openthread/src/core/common/logging.o \
./openthread/src/core/common/message.o \
./openthread/src/core/common/notifier.o \
./openthread/src/core/common/tasklet.o \
./openthread/src/core/common/timer.o \
./openthread/src/core/common/tlvs.o \
./openthread/src/core/common/trickle_timer.o 

CPP_DEPS += \
./openthread/src/core/common/crc16.d \
./openthread/src/core/common/instance.d \
./openthread/src/core/common/locator.d \
./openthread/src/core/common/logging.d \
./openthread/src/core/common/message.d \
./openthread/src/core/common/notifier.d \
./openthread/src/core/common/tasklet.d \
./openthread/src/core/common/timer.d \
./openthread/src/core/common/tlvs.d \
./openthread/src/core/common/trickle_timer.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/src/core/common/%.o: ../openthread/src/core/common/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../freertos -I../frdmkw41z -I../utilities -I../startup -I../CMSIS -I../openthread/include -I../openthread/examples/platforms -I../openthread/examples/platforms/kw41z -I../openthread/third_party/mbedtls -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/src -I../openthread/src/core -I../source -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../metrology/include -I../metrology/drivers/inc -I../drivers -I../parson -I../metrology/tasks/inc -Os -fno-common -g -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


