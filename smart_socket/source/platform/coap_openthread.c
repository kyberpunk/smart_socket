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
 *  The file contains gateway CoAP client implementation using OpenThread library.
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <coap_abstraction.h>
#include <openthread/types.h>
#include <openthread/coap.h>

#include "smart_socket_platform.h"
#include "smart_socket_config.h"
#include "shared_openthread.h"
#include "thread_abstraction.h"

#define MEDIA_TYPE OT_COAP_OPTION_CONTENT_FORMAT_JSON
#define COAP_BUFFER_SIZE 1024

// Mesh-local multicast address to all FTDs and Border Routers
#define MULTICAST_IP "ff03::2"

typedef struct {
	coap_result_cb_t callback;
} coap_result_context_t;

typedef struct {
	coap_get_cb_t callback;
} coap_get_context_t;

static uint16_t counter = 0;
static char coap_buffer[COAP_BUFFER_SIZE];
static otIp6Address server_ip;
static bool server = false;

static uint16_t Swap16(uint16_t v)
{
    return (((v & 0x00ffU) << 8) & 0xff00) | (((v & 0xff00U) >> 8) & 0x00ff);
}

static uint16_t coap_new_id() {
	return counter++;
}

coap_result_t get_coap_result(otError aResult) {
	switch (aResult) {
	case OT_ERROR_NONE:
		return COAP_RESULT_OK;
	case OT_ERROR_ABORT:
		return COAP_RESULT_ERROR;
	case OT_ERROR_RESPONSE_TIMEOUT:
		return COAP_RESULT_TIMEOUT;
	default:
		return COAP_RESULT_ERROR;
	}
}

coap_result_t get_coap_code_result(otCoapCode code) {
	switch (code) {
	case OT_COAP_CODE_RESPONSE_MIN:
	case OT_COAP_CODE_CREATED:
	case OT_COAP_CODE_DELETED:
	case OT_COAP_CODE_VALID:
	case OT_COAP_CODE_CHANGED:
	case OT_COAP_CODE_CONTENT:
		return COAP_RESULT_OK;
	case OT_COAP_CODE_FORBIDDEN:
		return COAP_RESULT_FORBIDDEN;
	default:
		return COAP_RESULT_ERROR;
	}
}

coap_result_t coap_start() {
	otError error = otCoapStart(thread_instance, OT_DEFAULT_COAP_PORT);
	return error == OT_ERROR_NONE ? COAP_RESULT_OK : COAP_RESULT_ERROR;
}

static void find_server_response_handler(void *aContext, otCoapHeader *aHeader, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aResult) {
	coap_result_t result = get_coap_result(aResult);
	coap_result_context_t *context = (coap_result_context_t*)aContext;
	coap_result_cb_t callback = NULL;
	if (context) {
		callback = context->callback;
		free(context);
	}
	if (result != COAP_RESULT_OK) {
		server = false;
		if (callback) {
			callback(result);
		}
		return;
	}
	// Check request result code
	otCoapCode code = otCoapHeaderGetCode(aHeader);
	if (code != OT_COAP_CODE_CONTENT) {
		if (callback) {
			callback(COAP_RESULT_ERROR);
		}
		return;
	}
	// Set remote server address if found any.
	server_ip = aMessageInfo->mPeerAddr;
	// Log remote server address
	socket_platform_log(SOCKET_LOG_INFO,
			"Server IP addr: %x:%x:%x:%x:%x:%x:%x:%x",
			Swap16(server_ip.mFields.m16[0]), Swap16(server_ip.mFields.m16[1]),
			Swap16(server_ip.mFields.m16[2]), Swap16(server_ip.mFields.m16[3]),
			Swap16(server_ip.mFields.m16[4]), Swap16(server_ip.mFields.m16[5]),
			Swap16(server_ip.mFields.m16[6]), Swap16(server_ip.mFields.m16[7]));
	server = true;
	if (callback) {
		callback(COAP_RESULT_OK);
	}
}

coap_result_t coap_find_server(coap_result_cb_t callback) {
	otError error = OT_ERROR_NONE;
	otCoapHeader header;
	otIp6Address destination_ip;
	otIp6AddressFromString(MULTICAST_IP, &destination_ip);

	// Send GET request to .well-known/core resource to multicast address
	otCoapType type = OT_COAP_TYPE_NON_CONFIRMABLE;
	otCoapCode code = OT_COAP_CODE_GET;
	otCoapHeaderInit(&header, type, code);
	otCoapHeaderGenerateToken(&header, 2);

	error = otCoapHeaderAppendUriPathOptions(&header, ".well-known/core");

	otCoapHeaderSetMessageId(&header, coap_new_id());
	otMessage *message = otCoapNewMessage(thread_instance, &header);
	if (message == NULL) {
		return COAP_RESULT_ERROR;
	}
	otMessageInfo message_info;
	memset(&message_info, 0, sizeof(message_info));
	message_info.mPeerAddr    = destination_ip;
	message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
	message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;

	coap_result_context_t *context = (coap_result_context_t*)malloc(sizeof(coap_result_context_t));
	context->callback = callback;
	error = otCoapSendRequest(thread_instance, message, &message_info, find_server_response_handler, context);
	return error == OT_ERROR_NONE ? COAP_RESULT_OK : COAP_RESULT_ERROR;
}

coap_result_t coap_reset_server() {
	server = false;
	return COAP_RESULT_OK;
}

static void register_device_response_handler(void *aContext, otCoapHeader *aHeader, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aResult) {
	coap_result_t result = get_coap_result(aResult);
	coap_result_context_t *context = (coap_result_context_t*)aContext;
	coap_result_cb_t callback = NULL;
	if (context) {
		callback = context->callback;
		free(context);
	}
	if (result != COAP_RESULT_OK) {
		server = false;
		if (callback) {
			callback(result);
		}
		return;
	}
	otCoapCode code = otCoapHeaderGetCode(aHeader);
	if (callback) {
		callback(get_coap_code_result(code));
	}
}

coap_result_t coap_register_device(coap_result_cb_t callback) {
	if (!server) {
		return COAP_RESULT_NO_SERVER;
	}
	otError error = OT_ERROR_NONE;
	otCoapHeader header;

	// Send POST request to .well-know/core resource with device EUI-64
	otCoapType type = OT_COAP_TYPE_NON_CONFIRMABLE;
	otCoapCode code = OT_COAP_CODE_POST;
	otCoapHeaderInit(&header, type, code);
	otCoapHeaderGenerateToken(&header, 2);

	error = otCoapHeaderAppendUriPathOptions(&header, ".well-known/core");
	if (error != OT_ERROR_NONE) {
		return COAP_RESULT_ERROR;
	}
	// Set device EUI-64 parameter
	char eui64[20];
	thread_get_eui64(eui64, sizeof(eui64));
	sprintf(coap_buffer, "ep=%s", eui64);
	error = otCoapHeaderAppendUriQueryOption(&header, coap_buffer);
	if (error != OT_ERROR_NONE) {
		return COAP_RESULT_ERROR;
	}
	// Set registration timeout parameter
	sprintf(coap_buffer, "lt=%d", ADDRESS_LT);
	error = otCoapHeaderAppendUriQueryOption(&header, coap_buffer);
	if (error != OT_ERROR_NONE) {
		return COAP_RESULT_ERROR;
	}

	otCoapHeaderSetMessageId(&header, coap_new_id());
	otMessage *message = otCoapNewMessage(thread_instance, &header);
	if (message == NULL) {
		return COAP_RESULT_ERROR;
	}
	otMessageInfo message_info;
	memset(&message_info, 0, sizeof(message_info));
	message_info.mPeerAddr    = server_ip;
	message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
	message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;

	coap_result_context_t *context = (coap_result_context_t*)malloc(sizeof(coap_result_context_t));
	context->callback = callback;
	// Send request to remote server address
	error = otCoapSendRequest(thread_instance, message, &message_info, register_device_response_handler, context);
	return error == OT_ERROR_NONE ? COAP_RESULT_OK : COAP_RESULT_ERROR;
}

static void get_response_handler(void *aContext, otCoapHeader *aHeader, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aResult) {
	coap_result_t result = get_coap_result(aResult);
	coap_get_context_t *context = (coap_get_context_t*)aContext;
	coap_get_cb_t callback = NULL;
	if (context) {
		callback = context->callback;
		free(context);
	}
	if (result != COAP_RESULT_OK) {
		server = false;
		if (callback) {
			callback(result, NULL, 0);
		}
		return;
	}
	otCoapCode code = otCoapHeaderGetCode(aHeader);
	result = get_coap_code_result(code);
	if (callback) {
		if (result == COAP_RESULT_OK) {
			// Read response payload
			size_t length = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
			length = COAP_BUFFER_SIZE - 1 < length ? COAP_BUFFER_SIZE - 1 : length;

			if (length > 0) {
				otMessageRead(aMessage, otMessageGetOffset(aMessage),
						coap_buffer, length);
				coap_buffer[length] = '\0';
				callback(COAP_RESULT_OK, coap_buffer, length);
				return;
			}
		}
		callback(result, NULL, 0);
	}
}

coap_result_t coap_get_time(coap_get_cb_t callback) {
	otError error = OT_ERROR_NONE;
	otCoapHeader header;

	// Send GET request to time resource
	otCoapType type = OT_COAP_TYPE_NON_CONFIRMABLE;
	otCoapCode code = OT_COAP_CODE_GET;
	otCoapHeaderInit(&header, type, code);
	otCoapHeaderGenerateToken(&header, 2);


	error = otCoapHeaderAppendUriPathOptions(&header, "time");
	if (error != OT_ERROR_NONE) {
		return COAP_RESULT_ERROR;
	}
	error = otCoapHeaderAppendUintOption(&header, OT_COAP_OPTION_ACCEPT, MEDIA_TYPE);
	if (error != OT_ERROR_NONE) {
		return COAP_RESULT_ERROR;
	}

	otCoapHeaderSetMessageId(&header, coap_new_id());
	otMessage *message = otCoapNewMessage(thread_instance, &header);
	if (message == NULL) {
		return COAP_RESULT_ERROR;
	}
	otMessageInfo message_info;
	memset(&message_info, 0, sizeof(message_info));
	message_info.mPeerAddr    = server_ip;
	message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
	message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;

	coap_get_context_t *context = (coap_get_context_t*)malloc(sizeof(coap_get_context_t));
	context->callback = callback;
	// Send request to remote server address
	error = otCoapSendRequest(thread_instance, message, &message_info, get_response_handler, context);
	return error == OT_ERROR_NONE ? COAP_RESULT_OK : COAP_RESULT_ERROR;
}

static void send_event_response_handler(void *aContext, otCoapHeader *aHeader, otMessage *aMessage, const otMessageInfo *aMessageInfo, otError aResult) {
	coap_result_t result = get_coap_result(aResult);
	coap_result_context_t *context = (coap_result_context_t*)aContext;
	coap_result_cb_t callback = NULL;
	if (context) {
		callback = context->callback;
		free(context);
	}
	if (result != COAP_RESULT_OK) {
		server = false;
		if (callback) {
			callback(result);
		}
		return;
	}
	otCoapCode code = otCoapHeaderGetCode(aHeader);
	if (callback) {
		callback(get_coap_code_result(code));
	}
}

coap_result_t coap_send_event(const char* payload, size_t payload_size, coap_result_cb_t callback) {
	if (!server) {
		return COAP_RESULT_NO_SERVER;
	}
	otError error = OT_ERROR_NONE;
	otCoapHeader header;

	// Send event data int POST request to events resource
	otCoapType type = OT_COAP_TYPE_NON_CONFIRMABLE;
	otCoapCode code = OT_COAP_CODE_POST;
	otCoapHeaderInit(&header, type, code);
	otCoapHeaderGenerateToken(&header, 2);

	error = otCoapHeaderAppendUriPathOptions(&header, "events");
	if (error != OT_ERROR_NONE) {
		return COAP_RESULT_ERROR;
	}
	error = otCoapHeaderAppendContentFormatOption(&header, MEDIA_TYPE);
	if (error != OT_ERROR_NONE) {
		return COAP_RESULT_ERROR;
	}
	otCoapHeaderSetMessageId(&header, coap_new_id());

	if (payload != NULL) {
		otCoapHeaderSetPayloadMarker(&header);
	}
	otMessage *message = otCoapNewMessage(thread_instance, &header);
	if (message == NULL) {
		return COAP_RESULT_ERROR;
	}
	// Append event payload
	if (payload != NULL) {
		error = otMessageAppend(message, payload, payload_size);
		if (error != OT_ERROR_NONE) {
			return COAP_RESULT_ERROR;
		}
	}

	otMessageInfo message_info;
	memset(&message_info, 0, sizeof(message_info));
	message_info.mPeerAddr = server_ip;
	message_info.mPeerPort = OT_DEFAULT_COAP_PORT;
	message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;

	coap_result_context_t *context = (coap_result_context_t*) malloc(
			sizeof(coap_result_context_t));
	context->callback = callback;
	// Send request to remote server address
	error = otCoapSendRequest(thread_instance, message, &message_info,
			send_event_response_handler, context);
	return error == OT_ERROR_NONE ? COAP_RESULT_OK : COAP_RESULT_ERROR;
}

coap_result_t coap_stop() {
	otError error = otCoapStop(thread_instance);
	return error == OT_ERROR_NONE ? COAP_RESULT_OK : COAP_RESULT_ERROR;
}
