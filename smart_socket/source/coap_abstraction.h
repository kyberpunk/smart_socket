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
 *  The file contains gateway CoAP client API declaration.
 */

#ifndef COAP_ABSTRACTION_H_
#define COAP_ABSTRACTION_H_

/**
 * @brief
 *   CoAP module provides functionality for communication with gateway over CoAP protocol.
 *
 * @{
 *
 */

/**
 * This enumeration represents CoAP.
 */
typedef enum {
	/**
	 * No errors.
	 */
	COAP_RESULT_OK = 0,
	/**
	 * Some error occurred.
	 */
	COAP_RESULT_ERROR,
	/**
	 * Request timeout.
	 */
	COAP_RESULT_TIMEOUT,
	/**
	 * No gateway endpoint found.
	 */
	COAP_RESULT_NO_SERVER,
	/**
	 * Forbidden request result (device address not registered).
	 */
	COAP_RESULT_FORBIDDEN,
	/**
	 * Invalid resource.
	 */
	COAP_RESULT_INVALID_RESOURCE
} coap_result_t;

/**
 * This function pointer is used as CoAP generic request result handler.
 *
 * @param[in]  result   Request result.
 */
typedef void (*coap_result_cb_t)(coap_result_t result);

/**
 * This function pointer is used as CoAP GET request result handler.
 *
 * @param[in]  result   Request result.
 * @param[in]  data     A pointer to response data buffer.
 * @param[in]  size     Response data size.
 */
typedef void (*coap_get_cb_t)(coap_result_t result, char *data, size_t size);

/**
 * Initialize and start CoAP client and server.
 *
 * @retval COAP_RESULT_OK       Successfully initialized.
 * @retval COAP_RESULT_ERROR    Some error occurred.
 */
coap_result_t coap_start(void);

/**
 * Find any CoAP server (gateway) by multicast request.
 *
 * @param[in]  callback    A function pointer to request result callback handler.
 *
 * @retval COAP_RESULT_OK       Successfully initialized.
 * @retval COAP_RESULT_ERROR    Some error occurred.
 */
coap_result_t coap_find_server(coap_result_cb_t callback);

/**
 * Reset cached remote CoAP server information.
 *
 * @retval COAP_RESULT_OK       Successfully initialized.
 * @retval COAP_RESULT_ERROR    Some error occurred.
 */
coap_result_t coap_reset_server();

/**
 * Register device on the CoAP server (gateway).
 *
 * @param[in]  callback    A function pointer to request result callback handler.
 *
 * @retval COAP_RESULT_OK       Successfully initialized.
 * @retval COAP_RESULT_ERROR    Some error occurred.
 */
coap_result_t coap_register_device(coap_result_cb_t callback);

/**
 * Get time resource data.
 *
 * @param[in]  callback    A function pointer to GET request result callback handler.
 *
 * @retval COAP_RESULT_OK       Successfully initialized.
 * @retval COAP_RESULT_ERROR    Some error occurred.
 */
coap_result_t coap_get_time(coap_get_cb_t callback);

/**
 * Send event data to gateway.
 *
 * @param[in]  payload        A pointer to payload data buffer.
 * @param[in]  payload_size   Payload size.
 * @param[in]  callback       A function pointer to request result callback handler.
 *
 * @retval COAP_RESULT_OK       Successfully initialized.
 * @retval COAP_RESULT_ERROR    Some error occurred.
 */
coap_result_t coap_send_event(const char* payload, size_t payload_size, coap_result_cb_t callback);

/**
 * Stop CoAP client and server.
 *
 * @retval COAP_RESULT_OK       Successfully initialized.
 * @retval COAP_RESULT_ERROR    Some error occurred.
 */
coap_result_t coap_stop(void);

/**
 * @}
 *
 */

#endif /* COAP_ABSTRACTION_H_ */
