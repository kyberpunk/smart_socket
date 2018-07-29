cmake_minimum_required(VERSION 2.8.11)

set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS} -O2 -D_GNU_SOURCE -DWITH_POSIX")

set(server_c_files coap_server.c address_resolver.c standard_output_log.c)
set(server_h_files coap_server.h address_resolver.h config.h module_log.h)

set(AZUREIOT_INC_FOLDER ".." "/usr/local/include/azureiot" "/usr/local/include/azureiot/inc")
set(LIBCOAP_INC_FOLDER "/usr/local/include/" "/usr/local/include/coap/")
set(LIBCOAP_LIB_FOLDER "/usr/local/lib")
set(LINK_LIBRARIES
    serializer
    iothub_client
    iothub_client_mqtt_transport
    aziotsharedutil
    parson
    umqtt
    pthread
    curl
    ssl
    crypto
    m
    uuid
	${LIBCOAP_LIB_FOLDER}/libcoap-1.a)

include_directories(${AZUREIOT_INC_FOLDER} ${LIBCOAP_INC_FOLDER})

add_executable(gateway gateway.c ${server_c_files} ${server_h_files})

target_link_libraries(gateway ${LINK_LIBRARIES})

