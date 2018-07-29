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
 *  The file contains Thread network API implementation with OpenThread library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/link.h>
#include <openthread/joiner.h>
#include <openthread/types.h>
#include <openthread/thread.h>

#include <openthread/platform/logging.h>
#include <openthread/platform/uart.h>
#include <openthread/platform/alarm-milli.h>

#include "thread_abstraction.h"
#include "smart_socket_platform.h"
#include "smart_socket_config.h"
#include "shared_openthread.h"
#include "platform.h"

static thread_join_cb_t joiner_callback = NULL;
otInstance *thread_instance = NULL;

/**
 * Convert hexadecimal characters to byte array.
 *
 * @param[in] aHex         A pointer to hexadecimal char array.
 * @param[in] aBin         A pointer to byte array buffer.
 * @param[in] aBitLength   Buffer size.
 *
 * @return  Returns the length of value representation or -1 if some error occurred.
 */
static int hex_to_bin(const char *aHex, uint8_t *aBin, uint16_t aBinLength)
{
    size_t      hexLength = strlen(aHex);
    const char *hexEnd    = aHex + hexLength;
    uint8_t *   cur       = aBin;
    uint8_t     numChars  = hexLength & 1;
    uint8_t     byte      = 0;

    if ((hexLength + 1) / 2 > aBinLength)
    {
        return -1;
    }

    while (aHex < hexEnd)
    {
        if ('A' <= *aHex && *aHex <= 'F')
        {
            byte |= 10 + (*aHex - 'A');
        }
        else if ('a' <= *aHex && *aHex <= 'f')
        {
            byte |= 10 + (*aHex - 'a');
        }
        else if ('0' <= *aHex && *aHex <= '9')
        {
            byte |= *aHex - '0';
        }
        else
        {
            return -1;
        }

        aHex++;
        numChars++;

        if (numChars >= 2)
        {
            numChars = 0;
            *cur++   = byte;
            byte     = 0;
        }
        else
        {
            byte <<= 4;
        }
    }

    return (int)(cur - aBin);
}

uint32_t socket_get_millis()
{
	// Get milliseconds form OpenThread platform default implementation
	return otPlatAlarmMilliGetNow();
}

thread_result_t thread_init() {
	thread_instance = otInstanceInitSingle();
	// Set maximal transmit power
	otPlatRadioSetTransmitPower(thread_instance, 10);
	if (thread_instance == NULL) {
		return THREAD_RESULT_ERROR;
	}
	return THREAD_RESULT_OK;
}

thread_result_t thread_ip6_enable(bool value) {
	otError error = otIp6SetEnabled(thread_instance, value);
	return error == OT_ERROR_NONE ? THREAD_RESULT_OK : THREAD_RESULT_ERROR;
}

/**
 * Joiner result callback handler.
 */
static void handler_joiner_callback(otError aError, void *aContext)
{
	if (joiner_callback) {
		switch (aError) {
		case OT_ERROR_NONE:
			joiner_callback(THREAD_RESULT_OK, aContext);
			break;
		case OT_ERROR_SECURITY:
			joiner_callback(THREAD_RESULT_SECURITY, aContext);
			break;
		case OT_ERROR_NOT_FOUND:
			joiner_callback(THREAD_RESULT_NOT_FOUND, aContext);
			break;
		default:
			joiner_callback(THREAD_RESULT_ERROR, aContext);
			break;
		}
	}
}

void thread_joiner_start(char* pskd, thread_join_cb_t callback, void *context) {
	joiner_callback = callback;
	otJoinerStart(thread_instance, pskd, NULL, PACKAGE_NAME, OPENTHREAD_CONFIG_PLATFORM_INFO, PACKAGE_VERSION, NULL, handler_joiner_callback, context);
}

void thread_start() {
	otThreadSetEnabled(thread_instance, true);
}

void thread_stop() {
	otThreadSetEnabled(thread_instance, false);
}

void thread_get_eui64(char *buffer, size_t buffer_size) {
	otExtAddress ext_address;
	otLinkGetFactoryAssignedIeeeEui64(thread_instance, &ext_address);
	char* buf = buffer;
	if (buffer_size - 1 < OT_EXT_ADDRESS_SIZE * 2) {
		return;
	}
	for (uint32_t i = 0; i < OT_EXT_ADDRESS_SIZE; i++) {
		sprintf(&buf[i*2], "%02x", ext_address.m8[i]);
	}
	buf[OT_EXT_ADDRESS_SIZE*2] = '\0';
}

void thread_factory_reset(void) {
	thread_set_autostart(false);
	otInstanceFactoryReset(thread_instance);
}

void thread_set_autostart(bool value) {
	otThreadSetAutoStart(thread_instance, value);
}

bool thread_get_autostart() {
	return otThreadGetAutoStart(thread_instance);
}

thread_result_t thread_set_default_network(void) {
	otError error = otThreadSetNetworkName(thread_instance, NETWORK_NAME);
	if (error != OT_ERROR_NONE) {
		return THREAD_RESULT_ERROR;
	}

	char *endptr;
	long panid = strtol(PANID, &endptr, 0);
	error = otLinkSetPanId(thread_instance, (otPanId)panid);
	if (error != OT_ERROR_NONE) {
		return THREAD_RESULT_ERROR;
	}

	uint8_t ext_panid[8];
	int result = hex_to_bin(EXTPANID, ext_panid, sizeof(ext_panid));
	if (result < 0) {
		return THREAD_RESULT_ERROR;
	}
	error = otThreadSetExtendedPanId(thread_instance, ext_panid);
	if (error != OT_ERROR_NONE) {
		return THREAD_RESULT_ERROR;
	}

	error = otLinkSetChannel(thread_instance, DEFAULT_CHANNEL);
	if (error != OT_ERROR_NONE) {
		return THREAD_RESULT_ERROR;
	}

	otMasterKey key;
    result = hex_to_bin(MASTER_KEY, key.m8, sizeof(key.m8));
    if (result != OT_MASTER_KEY_SIZE) {
		return THREAD_RESULT_ERROR;
	}
    error = otThreadSetMasterKey(thread_instance, &key);
	if (error != OT_ERROR_NONE) {
		return THREAD_RESULT_ERROR;
	}
	return THREAD_RESULT_OK;
}

thread_role_t thread_get_role(void) {
	otDeviceRole role = otThreadGetDeviceRole(thread_instance);
	switch (role) {
	case OT_DEVICE_ROLE_DISABLED:
		return THREAD_ROLE_DISABLED;
	case OT_DEVICE_ROLE_DETACHED:
		return THREAD_ROLE_DETACHED;
	case OT_DEVICE_ROLE_CHILD:
		return THREAD_ROLE_CHILD;
	case OT_DEVICE_ROLE_ROUTER:
		return THREAD_ROLE_ROUTER;
	case OT_DEVICE_ROLE_LEADER:
		return THREAD_ROLE_LEADER;
	default:
		return THREAD_ROLE_DISABLED;
	}
}

void thread_process() {
	// Process OT tasklets
    otTaskletsProcess(thread_instance);
    // Process platform specific tasks
    PlatformProcessDrivers(thread_instance);
}

void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
}

#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogRegion);

    socket_log_level_t level = SOCKET_LOG_NONE;
    switch(aLogLevel) {
    case OT_LOG_LEVEL_NONE:
    	level = SOCKET_LOG_NONE;
    	break;
    case OT_LOG_LEVEL_CRIT:
    	level = SOCKET_LOG_CRIT;
    	break;
    case OT_LOG_LEVEL_WARN:
    	level = SOCKET_LOG_WARN;
    	break;
    default:
    	level = SOCKET_LOG_INFO;
    }

    va_list ap;
    va_start(ap, aFormat);
    socket_platform_log(level, aFormat, ap);
    va_end(ap);
}
#endif


