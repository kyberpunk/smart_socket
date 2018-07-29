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
 *  The file contains gateway module configuration settings.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <coap/coap.h>
#include "config.h"

#ifndef COAP_LISTENER_ADDRESS
/**
 * @def COAP_LISTENER_ADDRESS
 *
 * IP address the CoAP server listens to
 *
 */
#define COAP_LISTENER_ADDRESS      "::"
#endif

#ifndef COAP_LISTENER_PORT
/**
 * @def COAP_LISTENER_PORT
 *
 * Network interface port number the CoAP server listens to
 *
 */
#define COAP_LISTENER_PORT         "5683"
#endif

#ifndef COAP_LOG_LEVEL
/**
 * @def COAP_LOG_LEVEL
 *
 * CoAP server log level
 *
 */
#define COAP_LOG_LEVEL             LOG_INFO
#endif

#ifndef MODULE_OUTPUT
/**
 * @def MODULE_OUTPUT
 *
 * Azure IoT Edge output name for gateway module
 *
 */
#define MODULE_OUTPUT              "coapOutput"
#endif

#ifndef MEDIA_TYPE
/**
 * @def MEDIA_TYPE
 *
 * Accepted CoAP content type
 *
 */
#define MEDIA_TYPE                 COAP_MEDIATYPE_APPLICATION_JSON
#endif

#ifndef RESOLVER_EXPIRY_INTERVAL
/**
 * @def RESOLVER_EXPIRY_INTERVAL
 *
 * Interval in seconds which the resolver module check devices expiration for
 *
 */
#define RESOLVER_EXPIRY_INTERVAL  60
#endif

#ifndef MODULE_LOG_LEVEL
/**
 * @def MODULE_LOG_LEVEL
 *
 * Module log level
 *
 */
#define MODULE_LOG_LEVEL          MODULE_LOG_INFO
#endif

/**
 * @def CHECK_MEDIA_TYPE
 *
 * Content type will be checked if defined
 *
 */
#define CHECK_MEDIA_TYPE

#endif /* CONFIG_H_ */
