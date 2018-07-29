################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../openthread/src/core/api/border_router_api.cpp \
../openthread/src/core/api/channel_manager_api.cpp \
../openthread/src/core/api/channel_monitor_api.cpp \
../openthread/src/core/api/child_supervision_api.cpp \
../openthread/src/core/api/coap_api.cpp \
../openthread/src/core/api/commissioner_api.cpp \
../openthread/src/core/api/crypto_api.cpp \
../openthread/src/core/api/dataset_api.cpp \
../openthread/src/core/api/dataset_ftd_api.cpp \
../openthread/src/core/api/dhcp6_api.cpp \
../openthread/src/core/api/dns_api.cpp \
../openthread/src/core/api/icmp6_api.cpp \
../openthread/src/core/api/instance_api.cpp \
../openthread/src/core/api/ip6_api.cpp \
../openthread/src/core/api/jam_detection_api.cpp \
../openthread/src/core/api/joiner_api.cpp \
../openthread/src/core/api/link_api.cpp \
../openthread/src/core/api/link_raw_api.cpp \
../openthread/src/core/api/message_api.cpp \
../openthread/src/core/api/netdata_api.cpp \
../openthread/src/core/api/server_api.cpp \
../openthread/src/core/api/tasklet_api.cpp \
../openthread/src/core/api/thread_api.cpp \
../openthread/src/core/api/thread_ftd_api.cpp \
../openthread/src/core/api/tmf_proxy_api.cpp \
../openthread/src/core/api/udp_api.cpp 

OBJS += \
./openthread/src/core/api/border_router_api.o \
./openthread/src/core/api/channel_manager_api.o \
./openthread/src/core/api/channel_monitor_api.o \
./openthread/src/core/api/child_supervision_api.o \
./openthread/src/core/api/coap_api.o \
./openthread/src/core/api/commissioner_api.o \
./openthread/src/core/api/crypto_api.o \
./openthread/src/core/api/dataset_api.o \
./openthread/src/core/api/dataset_ftd_api.o \
./openthread/src/core/api/dhcp6_api.o \
./openthread/src/core/api/dns_api.o \
./openthread/src/core/api/icmp6_api.o \
./openthread/src/core/api/instance_api.o \
./openthread/src/core/api/ip6_api.o \
./openthread/src/core/api/jam_detection_api.o \
./openthread/src/core/api/joiner_api.o \
./openthread/src/core/api/link_api.o \
./openthread/src/core/api/link_raw_api.o \
./openthread/src/core/api/message_api.o \
./openthread/src/core/api/netdata_api.o \
./openthread/src/core/api/server_api.o \
./openthread/src/core/api/tasklet_api.o \
./openthread/src/core/api/thread_api.o \
./openthread/src/core/api/thread_ftd_api.o \
./openthread/src/core/api/tmf_proxy_api.o \
./openthread/src/core/api/udp_api.o 

CPP_DEPS += \
./openthread/src/core/api/border_router_api.d \
./openthread/src/core/api/channel_manager_api.d \
./openthread/src/core/api/channel_monitor_api.d \
./openthread/src/core/api/child_supervision_api.d \
./openthread/src/core/api/coap_api.d \
./openthread/src/core/api/commissioner_api.d \
./openthread/src/core/api/crypto_api.d \
./openthread/src/core/api/dataset_api.d \
./openthread/src/core/api/dataset_ftd_api.d \
./openthread/src/core/api/dhcp6_api.d \
./openthread/src/core/api/dns_api.d \
./openthread/src/core/api/icmp6_api.d \
./openthread/src/core/api/instance_api.d \
./openthread/src/core/api/ip6_api.d \
./openthread/src/core/api/jam_detection_api.d \
./openthread/src/core/api/joiner_api.d \
./openthread/src/core/api/link_api.d \
./openthread/src/core/api/link_raw_api.d \
./openthread/src/core/api/message_api.d \
./openthread/src/core/api/netdata_api.d \
./openthread/src/core/api/server_api.d \
./openthread/src/core/api/tasklet_api.d \
./openthread/src/core/api/thread_api.d \
./openthread/src/core/api/thread_ftd_api.d \
./openthread/src/core/api/tmf_proxy_api.d \
./openthread/src/core/api/udp_api.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/src/core/api/%.o: ../openthread/src/core/api/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../freertos -I../frdmkw41z -I../utilities -I../startup -I../CMSIS -I../openthread/include -I../openthread/examples/platforms -I../openthread/examples/platforms/kw41z -I../openthread/third_party/mbedtls -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/src -I../openthread/src/core -I../source -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../metrology/include -I../metrology/drivers/inc -I../drivers -I../parson -I../metrology/tasks/inc -Os -fno-common -g -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


