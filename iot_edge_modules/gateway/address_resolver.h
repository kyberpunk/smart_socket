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
 *  The file contains address resolver API functions declarations.
 */

#ifndef ADDRESS_RESOLVER_H_
#define ADDRESS_RESOLVER_H_

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>

/**
 * @brief
 *   Address resolver module provides functionality for linking EUI-64 device identifier with assigned IPv6 address.
 *
 * @{
 *
 */

/**
 * This enumeration represents result codes for address resolver component functions.
 */
typedef enum {
	/**
	 * No error.
	 */
	RESOLVER_OK=0,
	/**
	 * Some error occured.
	 */
	RESOLVER_ERROR,
	/**
	 * Item was not found.
	 */
	RESOLVER_NOT_FOUND
} resolver_error_t;

/**
 * Function initializes the resolver module.
 *
 * @retval RESOLVER_OK          Successfully initialized.
 * @retval RESOLVER_ERROR       Initialization error.
 */
resolver_error_t resolver_init(void);

/**
 * Assign an IPv6 address to the device. Input values are copied.
 *
 * @param[in]  eui64     A pointer to EUI-64 string.
 * @param[in]  lt        Expiry time in seconds.
 * @param[in]  in6       A pointer to device IPv6 address.
 *
 * @retval RESOLVER_OK          Successfully added.
 * @retval RESOLVER_ERROR       Memory allocation error.
 */
resolver_error_t resolver_add_address(char *eui64, int lt, struct in6_addr *in6);

/**
 * Remove cached device IPv6 address.
 *
 * @param[in]  eui64     A pointer to EUI-64 string.
 *
 * @retval RESOLVER_OK          Successfully initialized.
 * @retval RESOLVER_ERROR       Memory allocation error.
 * @retval RESOLVER_NOT_FOUND   Device EUI-64 not registered.
 */
resolver_error_t resolver_remove_address(char *eui64);

/**
 * Resolve IPv6 address assigned to the device.
 *
 * @param[in]   eui64     A pointer to EUI-64 string.
 * @param[out]  in6       A pointer to IPv6 address.
 *
 * @retval RESOLVER_OK          Device was found.
 * @retval RESOLVER_ERROR       Unexpected error.
 * @retval RESOLVER_NOT_FOUND   Device EUI-64 not registered.
 */
resolver_error_t resolver_resolve_eui64(char *eui64, struct in6_addr *in6);

/**
 * Resolve device EUI-64 identifier assigned to the IPv6 address.
 *
 * @param[in]   in6         A pointer to IPv6 address.
 * @param[out]  eui64_ptr   A pointer to EUI-64 string pointer.
 *
 * @retval RESOLVER_OK          Device was found.
 * @retval RESOLVER_ERROR       Unexpected error.
 * @retval RESOLVER_NOT_FOUND   IPv6 address not registered.
 */
resolver_error_t resolver_resolve_in6_addr(struct in6_addr* in6, char **eui64_ptr);

/**
 * Function frees resources allocated by the resolver module.
 */
void resolver_deinit(void);

/**
 * @}
 *
 */

#endif /* ADDRESS_RESOLVER_H_ */
