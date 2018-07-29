/*
 *  Copyright (c) 2018, Vit Holasek.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @author Vit Holasek
 * @brief
 *  The file contains basic CoAP - IoT Hub gateway implementation.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include "coap_server.h"

#include "iothub_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/base64.h"
// This application currently only works when using MQTT protocol
#include "iothubtransportmqtt.h"
#include "serializer.h"
#include "config.h"
#include "module_log.h"

typedef struct
{
	IOTHUB_MESSAGE_HANDLE message_handle;
	unsigned char *message;
	size_t size;
} message_context_t;

static char *connection_string = NULL;
static IOTHUB_CLIENT_LL_HANDLE iothub_connection = NULL;

BEGIN_NAMESPACE(Hexade);
// Define event message model
DECLARE_MODEL(EventMessage,
WITH_DATA(ascii_char_ptr, deviceId),
WITH_DATA(int64_t, timestamp),
WITH_DATA(ascii_char_ptr_no_quotes, payload)
);

// Define time resource model
DECLARE_MODEL(TimeModel,
WITH_DATA(int64_t, milliseconds)
);
END_NAMESPACE(Hexade);

/**
 * Get time in milliseconds.
 */
static long long int get_timestamp_ms() {
	struct timeb timer_msec;
	long long int timestamp_msec;
	if (!ftime(&timer_msec)) {
		timestamp_msec = ((long long int) timer_msec.time) * 1000ll
				+ (long long int) timer_msec.millitm;
	} else {
		timestamp_msec = -1;
	}
	return timestamp_msec;
}

/**
 * Create IoT Hub message context.
 */
static message_context_t *create_message_context(unsigned char *data, size_t size) {
	message_context_t *context = (message_context_t*)malloc(sizeof(message_context_t));
	context->message = data;
	context->size = size;
	context->message_handle = NULL;
	return context;
}

/**
 * Destroy IoT Hub message context.
 */
static void destroy_message_context(message_context_t *context) {
	free(context->message);
	free(context);
}

/**
 * IoT Hub message confirmation callback.
 */
static void send_event_confirmation_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback) {
	message_context_t *context = (message_context_t*)userContextCallback;
	if (result != IOTHUB_CLIENT_CONFIRMATION_OK) {
		module_log(MODULE_LOG_WARN, "Confirmation failed with result = %d", result);
	}
	else {
		module_log(MODULE_LOG_INFO, "Confirmation OK");
	}
	// Destroy message context
	IoTHubMessage_Destroy(context->message_handle);
	destroy_message_context(context);
}

/**
 * Time resource GET method handler.
 */
static int coap_get_time_cb(char *eui, coap_server_data_t* data)
{
	module_log(MODULE_LOG_INFO, "GET time callback");
	TimeModel* model = CREATE_MODEL_INSTANCE(Hexade, TimeModel);
	if (!model) {
		module_log(MODULE_LOG_CRIT, "EventMessage model is NULL!");
		return 500;
	}
	model->milliseconds = get_timestamp_ms();
	// Serialize time resource JSON
	unsigned char* destination;
	size_t destination_size;
	if (SERIALIZE(&destination, &destination_size, model->milliseconds) != CODEFIRST_OK) {
		module_log(MODULE_LOG_CRIT, "Failed to serialize");
		return 500;
	}
	coap_server_set_cb_data(data, destination, destination_size);
	return 205;
}

/**
 * Event resource POST method handler.
 */
static int coap_post_event_cb(char *eui64, unsigned char *data, size_t size) {
	module_log(MODULE_LOG_INFO, "POST event callback");
	// Initialize event message model
	EventMessage* model = CREATE_MODEL_INSTANCE(Hexade, EventMessage);
	if (!model) {
		module_log(MODULE_LOG_CRIT, "EventMessage model is NULL!");
		return 500;
	}
	model->deviceId = eui64;
	model->timestamp = get_timestamp_ms();
	if((model->payload = (char*)malloc((size + 1) * sizeof(char))) == NULL) {
		module_log(MODULE_LOG_CRIT, "Cannot allocate space for payload");
		return 500;
	}
	memcpy(model->payload, data, size);
	model->payload[size] = '\0';

	// Serialize event message JSON
	unsigned char* destination;
	size_t destination_size;
	if (SERIALIZE(&destination, &destination_size, model->deviceId, model->timestamp, model->payload) != CODEFIRST_OK) {
		module_log(MODULE_LOG_CRIT, "Failed to serialize");
		free(model->payload);
		DESTROY_MODEL_INSTANCE(model);
		return 500;
	}
	free(model->payload);
	DESTROY_MODEL_INSTANCE(model);

	// Create IoT Hub message
	message_context_t *context = create_message_context(destination, destination_size);
	if ((context->message_handle = IoTHubMessage_CreateFromByteArray(context->message, context->size)) == NULL) {
		module_log(MODULE_LOG_CRIT, "Message handle is NULL!");
		return 500;
	}
	IoTHubMessage_SetMessageId(context->message_handle, "MSG_ID");
	IoTHubMessage_SetCorrelationId(context->message_handle, "CORE_ID");

	// Send event message to the IoT Hub service
	IOTHUB_CLIENT_RESULT client_result;
	client_result = IoTHubClient_LL_SendEventToOutputAsync(iothub_connection, context->message_handle, MODULE_OUTPUT, send_event_confirmation_callback, context);
	if (client_result != IOTHUB_CLIENT_OK) {
		module_log(MODULE_LOG_WARN, "Message refused by IoT Hub client, err=%d", client_result);
		destroy_message_context(context);
	} else {
		module_log(MODULE_LOG_INFO, "Message accepted by IoT Hub client");
	}

	module_log(MODULE_LOG_INFO, "Message payload: %.*s", context->size, context->message);
	IoTHubClient_LL_DoWork(iothub_connection);
	return 201;
}

/**
 * Initialize IoT Hub Connection
 */
static IOTHUB_CLIENT_LL_HANDLE iothub_initialize_connection() {
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = NULL;

    if (platform_init() != 0) {
    	module_log(MODULE_LOG_CRIT, "Failed to initialize the platform.");
        return NULL;
    }
	if (serializer_init(NULL) != SERIALIZER_OK) {
		module_log(MODULE_LOG_CRIT, "Failed to initialize the serializer");
		return NULL;
	}
	// Initialize connection using MQTT protocol
    else if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connection_string, MQTT_Protocol)) == NULL) {
    	module_log(MODULE_LOG_CRIT, "IoTHubClient_LL_CreateFromConnectionString(%s) failed", connection_string);
        return NULL;
    } else {
        bool trace = true;
        IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_LOG_TRACE, &trace);
    }
    return iotHubClientHandle;
}

/**
 * Deinitialize IoT Hub Connection
 */
static void iothub_deinitialize() {
	IoTHubClient_LL_Destroy(iothub_connection);
	platform_deinit();
	serializer_deinit();
}

/**
 * Main
 */
int main(int argc, char **argv, char **envp) {
	module_log(MODULE_LOG_INFO, "CoAP server module started");
	// Get IoT Hub connection string value from environment variables
    connection_string = getenv("EdgeHubConnectionString");
    if (connection_string) {
    	module_log(MODULE_LOG_INFO, "Connection string: %s", connection_string);
    } else {
    	module_log(MODULE_LOG_CRIT, "Connection string NULL");
    	return 1;
    }

    // Initialize IoT Hub connection
    if ((iothub_connection = iothub_initialize_connection()) == NULL) {
    	module_log(MODULE_LOG_CRIT, "IoT Hub connection initialization failed");
    	return 1;
    }

    // Initialize CoAP server
    if (coap_server_init() != COAP_OK) {
    	module_log(MODULE_LOG_CRIT, "CoAP server initialization failed");
    	return 1;
    }
    coap_server_register_event_cb(coap_post_event_cb);
    coap_server_register_time_cb(coap_get_time_cb);
    module_log(MODULE_LOG_INFO, "Gateway module initialized");

    while(1) {
    	// Process workers
		IoTHubClient_LL_DoWork(iothub_connection);
		ThreadAPI_Sleep(1);
    	coap_server_do_work(1);
    }
    coap_server_deinit();
    iothub_deinitialize();

	return 0;
}
