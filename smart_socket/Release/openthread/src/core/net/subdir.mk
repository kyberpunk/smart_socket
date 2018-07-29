################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../openthread/src/core/net/dhcp6_client.cpp \
../openthread/src/core/net/dhcp6_server.cpp \
../openthread/src/core/net/dns_client.cpp \
../openthread/src/core/net/icmp6.cpp \
../openthread/src/core/net/ip6.cpp \
../openthread/src/core/net/ip6_address.cpp \
../openthread/src/core/net/ip6_filter.cpp \
../openthread/src/core/net/ip6_headers.cpp \
../openthread/src/core/net/ip6_mpl.cpp \
../openthread/src/core/net/ip6_routes.cpp \
../openthread/src/core/net/netif.cpp \
../openthread/src/core/net/udp6.cpp 

OBJS += \
./openthread/src/core/net/dhcp6_client.o \
./openthread/src/core/net/dhcp6_server.o \
./openthread/src/core/net/dns_client.o \
./openthread/src/core/net/icmp6.o \
./openthread/src/core/net/ip6.o \
./openthread/src/core/net/ip6_address.o \
./openthread/src/core/net/ip6_filter.o \
./openthread/src/core/net/ip6_headers.o \
./openthread/src/core/net/ip6_mpl.o \
./openthread/src/core/net/ip6_routes.o \
./openthread/src/core/net/netif.o \
./openthread/src/core/net/udp6.o 

CPP_DEPS += \
./openthread/src/core/net/dhcp6_client.d \
./openthread/src/core/net/dhcp6_server.d \
./openthread/src/core/net/dns_client.d \
./openthread/src/core/net/icmp6.d \
./openthread/src/core/net/ip6.d \
./openthread/src/core/net/ip6_address.d \
./openthread/src/core/net/ip6_filter.d \
./openthread/src/core/net/ip6_headers.d \
./openthread/src/core/net/ip6_mpl.d \
./openthread/src/core/net/ip6_routes.d \
./openthread/src/core/net/netif.d \
./openthread/src/core/net/udp6.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/src/core/net/%.o: ../openthread/src/core/net/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../freertos -I../frdmkw41z -I../utilities -I../startup -I../CMSIS -I../openthread/include -I../openthread/examples/platforms -I../openthread/examples/platforms/kw41z -I../openthread/third_party/mbedtls -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/src -I../openthread/src/core -I../source -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../metrology/include -I../metrology/drivers/inc -I../drivers -I../parson -I../metrology/tasks/inc -Os -fno-common -g -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


