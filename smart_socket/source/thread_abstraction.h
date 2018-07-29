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
 *  The file contains Thread network API declaration.
 */

#ifndef THREAD_ABSTRACTION_H_
#define THREAD_ABSTRACTION_H_

#include <stdbool.h>

/**
 * @brief
 *   Thread module can be used for managing Thread network.
 *
 * @{
 *
 */

/**
 * This enumeration represents Thread API functions result.
 */
typedef enum {
	/**
	 * No error.
	 */
	THREAD_RESULT_OK = 0,
	/**
	 * Security operation failed.
	 */
	THREAD_RESULT_SECURITY,
	/**
	 * Target network not found.
	 */
	THREAD_RESULT_NOT_FOUND,
	/**
	 * Some error occurred.
	 */
	THREAD_RESULT_ERROR
} thread_result_t;

/**
 * This enumeration represents Thread device roles.
 */
typedef enum {
	THREAD_ROLE_DISABLED = 0,
	THREAD_ROLE_DETACHED,
	THREAD_ROLE_CHILD,
	THREAD_ROLE_ROUTER,
	THREAD_ROLE_LEADER
} thread_role_t;

/**
 * This function pointer is used as joiner process callback handler.
 *
 * @param[in]   eui64    Joiner process result.
 * @param[in]   data     A pointer to a Joiner context.
 *
 * @retval THREAD_RESULT_OK                   Joiner process succeeded.
 * @retval THREAD_RESULT_SECURITY    Security failure during the joiner process.
 * @retval THREAD_RESULT_NOT_FOUND   Network not found.
 */
typedef void (*thread_join_cb_t)(thread_result_t result, void *context);

/**
 * Function initializes Thread network.
 *
 * @retval THREAD_RESULT_OK       Successfully initialized.
 * @retval THREAD_RESULT_ERROR    Some error occurred.
 */
thread_result_t thread_init(void);

/**
 * Enable or disable IPv6 interface.
 *
 * @param[in]   value    Boolean value. True to enable, false to disable.
 *
 * @retval THREAD_RESULT_OK       State changed.
 * @retval THREAD_RESULT_ERROR    Some error occurred.
 */
thread_result_t thread_ip6_enable(bool value);

/**
 * Start Thread joiner process.
 *
 * @param[in]   pskd       A pointer to joiner PSKd key.
 * @param[in]   callback   A function pointer to joiner result callback handler.
 * @param[in]   context    A pointer to joiner process context.
 */
void thread_joiner_start(char *pskd, thread_join_cb_t callback, void *context);

/**
 * Start Thread networking.
 */
void thread_start(void);

/**
 * Stop Thread networking.
 */
void thread_stop(void);

/**
 * Get the device EUI-64 identifier string.
 *
 * @param[out]  buffer        A pointer to buffer.
 * @param[in]   buffer_size   Buffer size.
 */
void thread_get_eui64(char *buffer, size_t buffer_size);

/**
 * Perform factory reset and set Thread network functionality to default state.
 */
void thread_factory_reset(void);

/**
 * Enable or disable autostart mode.
 *
 * @param[in]   value    Boolean value. True to enable, false to disable.
 */
void thread_set_autostart(bool value);

/**
 * Get autostart mode state value.
 *
 * @return    Boolean value. True if enabled, false otherwise.
 */
bool thread_get_autostart(void);

/**
 * Set predefined network settings.
 *
 * @retval THREAD_RESULT_OK       Settings changed.
 * @retval THREAD_RESULT_ERROR    Some error occurred.
 */
thread_result_t thread_set_default_network(void);

/**
 * Get Thread device role.
 *
 * @return    Returns the device role.
 */
thread_role_t thread_get_role(void);

/**
 * Process single Thread network worker iteration.
 */
void thread_process(void);

/**
 * @}
 *
 */

#endif /* THREAD_ABSTRACTION_H_ */
