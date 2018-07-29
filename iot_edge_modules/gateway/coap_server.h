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
 *  The file contains CoAP server API functions declarations.
 */

#ifndef COAP_SERVER_H_
#define COAP_SERVER_H_

/**
 * @brief
 *   CoAP module provides functionality for Thread devices communication over CoAP protocol.
 *
 * @{
 *
 */

/**
 * This enumeration represents result codes for address resolver component functions.
 */
typedef enum coap_server_result_t {
	/**
	 * No error.
	 */
	COAP_OK = 0,
	/**
	 * Some error occured.
	 */
	COAP_ERROR,
	/**
	 * CoAP message timeout.
	 */
	COAP_TIMEOUT,
	/**
	 * Operation is being processed.
	 */
	COAP_PROCESSED
} coap_server_result_t;

/**
 * This structure represents an CoAP server resource data.
 */
typedef struct {
	unsigned char *data;
	size_t size;
	uint8_t media_type;
} coap_server_data_t;

/**
 * This function pointer is used as POST request handler.
 *
 * @param[in]  eui64    A pointer to EUI-64 identifier string.
 * @param[in]  data     A pointer to data buffer.
 * @param[in]  size     Data bugger size.
 *
 * @return Returns CoAP response result code.
 */
typedef int (*post_cb_t)(char *eui64, unsigned char *data, size_t size);

/**
 * This function pointer is used as GET request handler. Response data are set when called.
 *
 * @param[in]   eui64    A pointer to EUI-64 identifier string.
 * @param[out]  data     A pointer to allocated response data model.
 *
 * @return Returns CoAP response result code.
 */
typedef int (*get_cb_t)(char *eui, coap_server_data_t* data);

/**
 * Function initializes the CoAP server module.
 *
 * @retval COAP_OK          Successfully initialized.
 * @retval COAP_ERROR       Initialization error.
 */
coap_server_result_t coap_server_init(void);

/**
 * Register POST event resource request callback handler.
 *
 * @param[in]  callback   A pointer to the POST request callback function.
 */
void coap_server_register_event_cb(post_cb_t callback);

/**
 * Register GET time resource request callback handler.
 *
 * @param[in]  callback   A pointer to the GET request callback function.
 */
void coap_server_register_time_cb(get_cb_t callback);

/**
 * Process the single CoAP worker iteration.
 *
 * @param[in]  callback   A pointer to the GET request callback function.
 *
 * @retval COAP_OK          Successfully processed.
 * @retval COAP_ERROR       Unexpected error.
 * @retval COAP_PROCESSED   Worker is being processed now.
 * @retval COAP_TIMEOUT     Worker operation timeout.
 */
coap_server_result_t coap_server_do_work(unsigned wait_ms);

/**
 * Function frees resources allocated by the CoAP server module.
 */
void coap_server_deinit(void);

/**
 * Function sets CoAP response data.
 *
 * @param[out]  data   A pointer to an allocated response data model.
 * @param[in]   buf    A pointer to a data buffer.
 * @param[in]   size   Buffer size.
 */
void coap_server_set_cb_data(coap_server_data_t* data, const unsigned char *buf, size_t size);

/**
 * Deallocate response data model.
 *
 * @param[in]  data   A pointer to response data model.
 */
void coap_server_destroy_data(coap_server_data_t* data);

/**
 * @}
 *
 */

#endif /* COAP_SERVER_H_ */
