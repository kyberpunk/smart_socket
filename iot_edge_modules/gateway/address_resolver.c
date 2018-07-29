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
 * @brief This file contains address resolver module implementation using linked list.
 */

#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include "address_resolver.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "config.h"

/**
 * Device list item.
 */
typedef struct {
	char *eui64;
	struct in6_addr in6;
	unsigned int timeout;
} device_interface_t;

static SINGLYLINKEDLIST_HANDLE list = NULL;

static bool match_expired(const void* item, const void* match_context, bool* continue_processing) {
	unsigned int *ts = (unsigned int*)match_context;
	*continue_processing = true;
	return ((device_interface_t*)item)->timeout < *ts;
}

/**
 * Timer callback handler.
 */
void timer_handler(int signum) {
   unsigned int ts = (unsigned int)time(NULL);
   // Remove expired items
   singlylinkedlist_remove_if(list, match_expired, &ts);
}

static bool eui64_is_equal(char *eui1, char *eui2) {
	return strcmp(eui1, eui2) == 0 ? true : false;
}

static bool match_eui64(LIST_ITEM_HANDLE list_item, const void* match_context) {
	char *eui1 = ((device_interface_t*)singlylinkedlist_item_get_value(list_item))->eui64;
	char *eui2 = (char *)match_context;
	return eui64_is_equal(eui1, eui2);
}

static bool match_in6_addr(LIST_ITEM_HANDLE list_item, const void* match_context) {
	unsigned char *in1 = (unsigned char *)&((device_interface_t*)singlylinkedlist_item_get_value(list_item))->in6;
	unsigned char *in2 = (unsigned char *)match_context;
	for (int i = 0; i < 16; i++) {
		if (in1[i] != in2[i]) {
			return false;
		}
	}
	return true;
}

resolver_error_t resolver_init() {
	struct sigaction sa;
	struct itimerval timer;

	// Init linked list
	list = singlylinkedlist_create();
	if (list == NULL) {
		return RESOLVER_ERROR;
	}

	// Set timer
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &timer_handler;
	sigaction(SIGALRM, &sa, NULL);

	timer.it_value.tv_sec = RESOLVER_EXPIRY_INTERVAL;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = RESOLVER_EXPIRY_INTERVAL;
	timer.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL, &timer, NULL);
	return RESOLVER_OK;
}

resolver_error_t resolver_add_address(char *eui64, int lt, struct in6_addr *addr) {
	LIST_ITEM_HANDLE item = singlylinkedlist_find(list, match_eui64, eui64);
	// If device already registered only change address value
	if (item != NULL) {
		device_interface_t* device = (device_interface_t*)singlylinkedlist_item_get_value(item);
		device->in6 = *addr;
		device->timeout = time(NULL) + lt;
		return RESOLVER_OK;
	}
	device_interface_t *device = (device_interface_t*)malloc(sizeof(device_interface_t));
	if (device == NULL) {
		return RESOLVER_ERROR;
	}
	device->in6 = *addr;
	device->timeout = time(NULL) + lt;
	device->eui64 = (char *)malloc((strlen(eui64) + 1) * sizeof(char));
	if (device->eui64 == NULL) {
		free(device);
		return RESOLVER_ERROR;
	}
	strcpy(device->eui64, eui64);
	if (singlylinkedlist_add(list, device) == NULL) {
		free(device);
		return RESOLVER_ERROR;
	}
	return RESOLVER_OK;
}

resolver_error_t resolver_remove_address(char *eui64) {
	LIST_ITEM_HANDLE item = singlylinkedlist_find(list, match_eui64, eui64);
	if (item == NULL) {
		return RESOLVER_NOT_FOUND;
	}
	device_interface_t *value = (device_interface_t*)singlylinkedlist_item_get_value(item);
	singlylinkedlist_remove(list, item);
	free(value->eui64);
	free(value);
	return RESOLVER_OK;
}
resolver_error_t resolver_resolve_eui64(char *eui64, struct in6_addr* in6_addr) {
	LIST_ITEM_HANDLE item = singlylinkedlist_find(list, match_eui64, eui64);
	if (item == NULL) {
		return RESOLVER_NOT_FOUND;
	}
	*in6_addr = ((device_interface_t*)singlylinkedlist_item_get_value(item))->in6;
	return RESOLVER_OK;
}

resolver_error_t resolver_resolve_in6_addr(struct in6_addr *in6_addr, char **eui64) {
	LIST_ITEM_HANDLE item = singlylinkedlist_find(list, match_in6_addr, in6_addr);
	if (item == NULL) {
		return RESOLVER_NOT_FOUND;
	}
	*eui64 = ((device_interface_t*)singlylinkedlist_item_get_value(item))->eui64;
	return RESOLVER_OK;
}

void resolver_deinit() {
	LIST_ITEM_HANDLE item = NULL;

	// Disable timer
	struct itimerval timer;
	memset(&timer, 0, sizeof(timer));
	setitimer(ITIMER_REAL, NULL, NULL);

	// Free all list items
	while ((item = singlylinkedlist_get_head_item(list)) != NULL) {
		device_interface_t *value = (device_interface_t*) singlylinkedlist_item_get_value(item);
		singlylinkedlist_remove(list, item);
		free(value->eui64);
		free(value);
	}
	singlylinkedlist_destroy(list);
}
