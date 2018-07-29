################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utilities/subdir.mk
-include startup/subdir.mk
-include source/platform/subdir.mk
-include source/subdir.mk
-include parson/subdir.mk
-include openthread/third_party/nxp/MKW41Z4/XCVR/cfgs_kw4x_3x_2x/subdir.mk
-include openthread/third_party/nxp/MKW41Z4/XCVR/XCVR_Test/subdir.mk
-include openthread/third_party/nxp/MKW41Z4/XCVR/subdir.mk
-include openthread/third_party/mbedtls/repo.patched/library/subdir.mk
-include openthread/src/ncp/subdir.mk
-include openthread/src/diag/subdir.mk
-include openthread/src/core/utils/subdir.mk
-include openthread/src/core/thread/subdir.mk
-include openthread/src/core/net/subdir.mk
-include openthread/src/core/meshcop/subdir.mk
-include openthread/src/core/mac/subdir.mk
-include openthread/src/core/crypto/subdir.mk
-include openthread/src/core/common/subdir.mk
-include openthread/src/core/coap/subdir.mk
-include openthread/src/core/api/subdir.mk
-include openthread/src/cli/subdir.mk
-include openthread/examples/platforms/utils/subdir.mk
-include openthread/examples/platforms/kw41z/subdir.mk
-include metrology/tasks/src/subdir.mk
-include metrology/drivers/src/subdir.mk
-include freertos/subdir.mk
-include frdmkw41z/subdir.mk
-include drivers/subdir.mk
-include CMSIS/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: smart_socket.axf

# Tool invocations
smart_socket.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU C++ Linker'
	arm-none-eabi-c++ -nostdlib -Xlinker -Map="smart_socket.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -mcpu=cortex-m0plus -mthumb -u _printf_float -u _scanf_float -T smart_socket_Release.ld -o "smart_socket.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) smart_socket.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "smart_socket.axf"; # arm-none-eabi-objcopy -v -O binary "smart_socket.axf" "smart_socket.bin" ; # checksum -p MKW41Z512xxx4 -d "smart_socket.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
