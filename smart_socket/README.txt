This archive contains implementation of Smart Socket device firmware for KW41Z platform.

Following third-party sources are included:
- OpenThread (https://github.com/openthread/openthread)
- Parson (https://github.com/kgabis/parson)
- Kinetis SDK 2.0 (https://www.nxp.com/support/developer-resources/reference-designs/software-development-kit-for-kinetis-mcus:KINETIS-SDK)
- MCUXpresso IDE generated files

Main program sources are located in source folder.

#Requimerents:
- GNU Arm Embedded Toolchain (https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)

Installation on Ubuntu:
sudo apt-get install gcc-arm-none-eabi
sudo apt-get install libnewlib-arm-none-eabi

#Build:
It's recommended to update OpenThread and KSDK v2 sources first.

Firmware image can be builded using GNU make. Following commands can be used:
make
make clean

Compiled binary file smart_socket.axf is located in Release directory. For generating raw binary file use:
arm-none-eabi-objcopy -O smart_socket.axf smart_socket.bin
