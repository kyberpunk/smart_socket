################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../openthread/src/core/crypto/aes_ccm.cpp \
../openthread/src/core/crypto/aes_ecb.cpp \
../openthread/src/core/crypto/heap.cpp \
../openthread/src/core/crypto/hmac_sha256.cpp \
../openthread/src/core/crypto/mbedtls.cpp \
../openthread/src/core/crypto/pbkdf2_cmac.cpp \
../openthread/src/core/crypto/sha256.cpp 

OBJS += \
./openthread/src/core/crypto/aes_ccm.o \
./openthread/src/core/crypto/aes_ecb.o \
./openthread/src/core/crypto/heap.o \
./openthread/src/core/crypto/hmac_sha256.o \
./openthread/src/core/crypto/mbedtls.o \
./openthread/src/core/crypto/pbkdf2_cmac.o \
./openthread/src/core/crypto/sha256.o 

CPP_DEPS += \
./openthread/src/core/crypto/aes_ccm.d \
./openthread/src/core/crypto/aes_ecb.d \
./openthread/src/core/crypto/heap.d \
./openthread/src/core/crypto/hmac_sha256.d \
./openthread/src/core/crypto/mbedtls.d \
./openthread/src/core/crypto/pbkdf2_cmac.d \
./openthread/src/core/crypto/sha256.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/src/core/crypto/%.o: ../openthread/src/core/crypto/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../freertos -I../frdmkw41z -I../utilities -I../startup -I../CMSIS -I../openthread/include -I../openthread/examples/platforms -I../openthread/examples/platforms/kw41z -I../openthread/third_party/mbedtls -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/src -I../openthread/src/core -I../source -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../metrology/include -I../metrology/drivers/inc -I../drivers -I../parson -I../metrology/tasks/inc -Os -fno-common -g -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


