################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../openthread/src/core/utils/channel_manager.cpp \
../openthread/src/core/utils/channel_monitor.cpp \
../openthread/src/core/utils/child_supervision.cpp \
../openthread/src/core/utils/jam_detector.cpp \
../openthread/src/core/utils/slaac_address.cpp 

C_SRCS += \
../openthread/src/core/utils/missing_strlcat.c \
../openthread/src/core/utils/missing_strlcpy.c \
../openthread/src/core/utils/missing_strnlen.c 

OBJS += \
./openthread/src/core/utils/channel_manager.o \
./openthread/src/core/utils/channel_monitor.o \
./openthread/src/core/utils/child_supervision.o \
./openthread/src/core/utils/jam_detector.o \
./openthread/src/core/utils/missing_strlcat.o \
./openthread/src/core/utils/missing_strlcpy.o \
./openthread/src/core/utils/missing_strnlen.o \
./openthread/src/core/utils/slaac_address.o 

CPP_DEPS += \
./openthread/src/core/utils/channel_manager.d \
./openthread/src/core/utils/channel_monitor.d \
./openthread/src/core/utils/child_supervision.d \
./openthread/src/core/utils/jam_detector.d \
./openthread/src/core/utils/slaac_address.d 

C_DEPS += \
./openthread/src/core/utils/missing_strlcat.d \
./openthread/src/core/utils/missing_strlcpy.d \
./openthread/src/core/utils/missing_strnlen.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/src/core/utils/%.o: ../openthread/src/core/utils/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../freertos -I../frdmkw41z -I../utilities -I../startup -I../CMSIS -I../openthread/include -I../openthread/examples/platforms -I../openthread/examples/platforms/kw41z -I../openthread/third_party/mbedtls -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/src -I../openthread/src/core -I../source -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../metrology/include -I../metrology/drivers/inc -I../drivers -I../parson -I../metrology/tasks/inc -Os -fno-common -g -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

openthread/src/core/utils/%.o: ../openthread/src/core/utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../parson -I../drivers -I../metrology/drivers/inc -I../metrology/tasks/inc -I../metrology/include -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../openthread/src/core -I../openthread/src -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/third_party/mbedtls -I../openthread/examples/platforms/kw41z -I../openthread/examples/platforms -I../openthread/include -I../CMSIS -I../startup -I../utilities -I../frdmkw41z -I../freertos -I../source -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


