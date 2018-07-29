################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../openthread/src/core/thread/address_resolver.cpp \
../openthread/src/core/thread/announce_begin_server.cpp \
../openthread/src/core/thread/data_poll_manager.cpp \
../openthread/src/core/thread/energy_scan_server.cpp \
../openthread/src/core/thread/key_manager.cpp \
../openthread/src/core/thread/link_quality.cpp \
../openthread/src/core/thread/lowpan.cpp \
../openthread/src/core/thread/mesh_forwarder.cpp \
../openthread/src/core/thread/mesh_forwarder_ftd.cpp \
../openthread/src/core/thread/mesh_forwarder_mtd.cpp \
../openthread/src/core/thread/mle.cpp \
../openthread/src/core/thread/mle_router.cpp \
../openthread/src/core/thread/network_data.cpp \
../openthread/src/core/thread/network_data_leader.cpp \
../openthread/src/core/thread/network_data_leader_ftd.cpp \
../openthread/src/core/thread/network_data_local.cpp \
../openthread/src/core/thread/network_diagnostic.cpp \
../openthread/src/core/thread/panid_query_server.cpp \
../openthread/src/core/thread/src_match_controller.cpp \
../openthread/src/core/thread/thread_netif.cpp \
../openthread/src/core/thread/tmf_proxy.cpp \
../openthread/src/core/thread/topology.cpp 

OBJS += \
./openthread/src/core/thread/address_resolver.o \
./openthread/src/core/thread/announce_begin_server.o \
./openthread/src/core/thread/data_poll_manager.o \
./openthread/src/core/thread/energy_scan_server.o \
./openthread/src/core/thread/key_manager.o \
./openthread/src/core/thread/link_quality.o \
./openthread/src/core/thread/lowpan.o \
./openthread/src/core/thread/mesh_forwarder.o \
./openthread/src/core/thread/mesh_forwarder_ftd.o \
./openthread/src/core/thread/mesh_forwarder_mtd.o \
./openthread/src/core/thread/mle.o \
./openthread/src/core/thread/mle_router.o \
./openthread/src/core/thread/network_data.o \
./openthread/src/core/thread/network_data_leader.o \
./openthread/src/core/thread/network_data_leader_ftd.o \
./openthread/src/core/thread/network_data_local.o \
./openthread/src/core/thread/network_diagnostic.o \
./openthread/src/core/thread/panid_query_server.o \
./openthread/src/core/thread/src_match_controller.o \
./openthread/src/core/thread/thread_netif.o \
./openthread/src/core/thread/tmf_proxy.o \
./openthread/src/core/thread/topology.o 

CPP_DEPS += \
./openthread/src/core/thread/address_resolver.d \
./openthread/src/core/thread/announce_begin_server.d \
./openthread/src/core/thread/data_poll_manager.d \
./openthread/src/core/thread/energy_scan_server.d \
./openthread/src/core/thread/key_manager.d \
./openthread/src/core/thread/link_quality.d \
./openthread/src/core/thread/lowpan.d \
./openthread/src/core/thread/mesh_forwarder.d \
./openthread/src/core/thread/mesh_forwarder_ftd.d \
./openthread/src/core/thread/mesh_forwarder_mtd.d \
./openthread/src/core/thread/mle.d \
./openthread/src/core/thread/mle_router.d \
./openthread/src/core/thread/network_data.d \
./openthread/src/core/thread/network_data_leader.d \
./openthread/src/core/thread/network_data_leader_ftd.d \
./openthread/src/core/thread/network_data_local.d \
./openthread/src/core/thread/network_diagnostic.d \
./openthread/src/core/thread/panid_query_server.d \
./openthread/src/core/thread/src_match_controller.d \
./openthread/src/core/thread/thread_netif.d \
./openthread/src/core/thread/tmf_proxy.d \
./openthread/src/core/thread/topology.d 


# Each subdirectory must supply rules for building sources it contributes
openthread/src/core/thread/%.o: ../openthread/src/core/thread/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -D__NEWLIB__ -DSDK_DEBUGCONSOLE_UART -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_FREE_RTOS -DCPU_MKW41Z512VHT4 -DCPU_MKW41Z512VHT4_cm0plus -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-kw41z-config.h"' -DMBEDTLS_CONFIG_FILE='"mbedtls-config.h"' -DOPENTHREAD_FTD=1 -DSPI_XFER_STPM3X -DSDK_DEBUGCONSOLE=0 -D__MTB_DISABLE -I../freertos -I../frdmkw41z -I../utilities -I../startup -I../CMSIS -I../openthread/include -I../openthread/examples/platforms -I../openthread/examples/platforms/kw41z -I../openthread/third_party/mbedtls -I../openthread/third_party/mbedtls/repo.patched/include -I../openthread/src -I../openthread/src/core -I../source -I../openthread/third_party/nxp/MKW41Z4/XCVR -I../metrology/include -I../metrology/drivers/inc -I../drivers -I../parson -I../metrology/tasks/inc -Os -fno-common -g -Wall -c -fmessage-length=0 -mcpu=cortex-m0plus -mthumb -D__NEWLIB__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


