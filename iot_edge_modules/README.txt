This archive contains implementation of the Azure IoT Edge module. The module provides protocol translation between Thread network (UDP CoAP protocol) and Azure IoT Hub service. It is designed to use as Docker container with Azure IoT Edge v2 service. For more information see: https://docs.microsoft.com/en-us/azure/iot-edge/how-iot-edge-works.

The project consists of three parts:
- Azure SDK for C - Thirt party library for communication with Azure IoT Hub service. Latest version can be obtained from: https://github.com/Azure/azure-iot-sdk-c/. For IoT Edge modules is required the modules-preview branch.
- libcoap - Third party library implementing CoAP protocol. Latest version can be obtained from https://github.com/obgm/libcoap.
- gateway - Protocol translation gateway module implementation for communication between Thread network and Azure IoT Hub service. It can be used with Smart Socket devices.

#Requirements:
- Docker with Linux containers (https://docs.docker.com/install/)
- Internet connection

#Docker container image build:
Execute following commands in this folder root for building docker image with 'gateway' tag: 

sudo docker build -t baseimage -f Dockerfile.base .
sudo docker build -t coap -f Dockerfile.coap .
sudo docker build -t gateway -f Dockerfile.opaque .

Deploy Docker container with this create options:
{
  "NetworkMode": "host",
  "HostConfig": {
    "PortBindings": {
      "5683/udp": [
        {
          "HostPort": "5683/udp"
        }
      ]
    }
  }
}

Use following routes configuration:
{
  "routes": {
    "coapToCloud": "FROM /messages/modules/gateway/* INTO $upstream"
  }
}