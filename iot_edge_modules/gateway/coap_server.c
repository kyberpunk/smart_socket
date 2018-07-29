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
 *  The file contains CoAP server module implementation.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <coap/coap.h>
#include <coap/coap_dtls.h>

#include "config.h"
#include "coap_server.h"
#include "address_resolver.h"

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define INDEX "Azure IoT Edge Gateway"
#define CORE_PAYLOAD "[{\"href\":\"/time\",\"ct\":\"" STRINGIZE(COAP_MEDIATYPE_APPLICATION_JSON) "\"\
,\"title\":\"Gateway Clock\"},\
{\"href\":\"/events\",\"ct\":\"" STRINGIZE(COAP_MEDIATYPE_APPLICATION_JSON) "\",\
\"title\":\"Gateway Events\",\"rt\":\"Events\",\"if\":\"event\"},\
{\"href\":\"/well-known/core\",\"ct\":\"50\",\
\"title\":\"Core Resource\",\"rt\":\"Core\",\"if\":\"core\"}]"

static struct coap_resource_t *event_resource = NULL;
static struct coap_resource_t *time_resource = NULL;
static struct coap_resource_t *core_resource = NULL;
static coap_context_t *ctx = NULL;
static post_cb_t post_event_callback_handler = NULL;
static get_cb_t get_time_callback_handler = NULL;

static void address_to_string(char *addr_str, coap_address_t coap_address) {
	switch (coap_address.addr.sa.sa_family)
	{
	case AF_INET:
		inet_ntop(AF_INET, &coap_address.addr.sin.sin_addr, addr_str, INET_ADDRSTRLEN);
		break;
	case AF_INET6:
		inet_ntop(AF_INET6, &coap_address.addr.sin6.sin6_addr, addr_str, INET6_ADDRSTRLEN);
		break;
	default:
		sprintf(addr_str, "");
		break;
	}
}

static void log_remote_address(coap_address_t remote_address) {
	char remote_addr_str[INET6_ADDRSTRLEN + 1];
	address_to_string(remote_addr_str, remote_address);
	coap_log(LOG_INFO, "remote_address: [%s]\n", remote_addr_str);
}

/**
 * Parse CoAP request query parameter (CoAP option).
 *
 * @param[in]   search       A pointer to the parameter name string.
 * @param[in]   search_len   Parameter name length.
 * @param[in]   data         Query buffer.
 * @param[in]   data_len     Query buffer size.
 * @param[out]  result       Parameter value.
 *
 * @return Returns 1 if the query parameter was found or 0 otherwise.
 */
static int parse_param(unsigned char *search,
            size_t search_len,
            unsigned char *data,
            size_t data_len,
            str *result) {

  if (result)
    memset(result, 0, sizeof(str));

  if (!search_len)
    return 0;

  while (search_len <= data_len) {
    if (memcmp(search, data, search_len) == 0) {
      data += search_len;
      data_len -= search_len;

      if (!data_len || *data == '=' || *data == '&') {
        while (data_len && *data != '=') {
          ++data; --data_len;
        }

        if (data_len > 1 && result) {

          result->s = ++data;
          while (--data_len && *data != '&') {
            ++data; result->length++;
          }
        }

        return 1;
      }
    }

    while (--data_len && *data++ != '&')
      ;
  }

  return 0;
}

static uint8_t get_request_media_type(coap_pdu_t *request) {
	static coap_opt_iterator_t opt_it;
	coap_opt_t *media_type_option = coap_check_option(request, COAP_OPTION_CONTENT_FORMAT, &opt_it);
	if (media_type_option) {
		return *coap_opt_value(media_type_option);
	}
	return COAP_MEDIATYPE_ANY;
}

/**
 * Index resource GET method.
 */
static void hnd_get_index(coap_context_t *ctx UNUSED_PARAM,
		struct coap_resource_t *resource UNUSED_PARAM,
		coap_session_t *session UNUSED_PARAM, coap_pdu_t *request UNUSED_PARAM,
		str *token UNUSED_PARAM, str *query UNUSED_PARAM, coap_pdu_t *response) {
	unsigned char buf[3];

	response->code = COAP_RESPONSE_CODE(205);

	coap_add_option(response,
	COAP_OPTION_CONTENT_TYPE,
	coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

	coap_add_option(response,
	COAP_OPTION_MAXAGE, coap_encode_var_bytes(buf, 0x2ffff), buf);

	coap_add_data(response, strlen(INDEX), (unsigned char *) INDEX);
}

/**
 * Core resource GET method.
 */
static void hnd_get_core(coap_context_t *ctx, struct coap_resource_t *resource,
		coap_session_t *session, coap_pdu_t *request, str *token, str *query,
		coap_pdu_t *response) {
	coap_log(LOG_INFO, "GET .well-known/core/\n");
	(void) request;
	// Append core links
	char *payload = CORE_PAYLOAD;
	size_t size = sizeof(CORE_PAYLOAD);
	unsigned char buf[40];
	coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
			coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_JSON), buf);
	coap_add_data(response, size, (uint8_t*)payload);
	response->code = COAP_RESPONSE_CODE(205);
}

/**
 * Core resource POST method.
 */
static void hnd_post_core(coap_context_t *ctx UNUSED_PARAM,
		struct coap_resource_t *resource, coap_session_t *session,
		coap_pdu_t *request, str *token UNUSED_PARAM, str *query UNUSED_PARAM,
		coap_pdu_t *response) {
	coap_log(LOG_INFO, "POST .well-known/core/\n");
	log_remote_address(session->remote_addr);
	str lt = {0, NULL}, ep = {0, NULL};
	// Parse query
	if (query) {
		coap_log(LOG_INFO, "query: %s\n", query->s);
	    parse_param((unsigned char *)"ep", 2, query->s, query->length, &ep);
	    parse_param((unsigned char *)"lt", 2, query->s, query->length, &lt);
	}
	if (ep.s != NULL) {
		// Parse lt value
		intmax_t num = 86400;
		if (lt.s != NULL) {
			char *num_text = (char*) malloc((lt.length + 1) * sizeof(char));
			memcpy(num_text, lt.s, lt.length);
			num_text[lt.length] = '\0';
			intmax_t num = strtoimax(num_text, NULL, 10);
			free(num_text);
			if (num == INTMAX_MAX) {
				response->code = COAP_RESPONSE_CODE(400);
				return;
			}
		}
		char *eui = (char*)malloc((ep.length + 1) * sizeof(char));
		if (eui == NULL) {
			response->code = COAP_RESPONSE_CODE(500);
			return;
		}
		memcpy(eui, ep.s, ep.length);
		eui[ep.length] = '\0';
		// Register device
		if (resolver_add_address(eui, (int)num, &session->remote_addr.addr.sin6.sin6_addr) == RESOLVER_OK) {
			coap_log(LOG_INFO, "Address added. EUI64: %s, lt: %d\n", eui, (int)num);
			response->code = COAP_RESPONSE_CODE(201);
		} else {
			response->code = COAP_RESPONSE_CODE(500);
		}
		free(eui);
		return;
	}
	coap_log(LOG_INFO, "ep query parameter not present in the query\n");
	response->code = COAP_RESPONSE_CODE(400);
}

/**
 * Time resource GET method.
 */
static void hnd_get_time(coap_context_t *ctx, struct coap_resource_t *resource,
		coap_session_t *session, coap_pdu_t *request, str *token, str *query,
		coap_pdu_t *response) {
	coap_log(LOG_INFO, "GET time/\n");
	(void) request;
#ifdef CHECK_MEDIA_TYPE
	uint8_t media_type = get_request_media_type(request);
	if (media_type != MEDIA_TYPE && media_type != COAP_MEDIATYPE_ANY) {
		printf("Media type %d not supported\n", media_type);
		response->code = COAP_RESPONSE_CODE(415);
		return;
	}
#endif
	char *eui = NULL;
	// Resolve device EUI-64
	if (resolver_resolve_in6_addr(&session->remote_addr.addr.sin6.sin6_addr, &eui) != RESOLVER_OK) {
		response->code = COAP_RESPONSE_CODE(403);
		return;
	}
	// Set response data
	if (get_time_callback_handler) {
		coap_server_data_t data;
		int code = get_time_callback_handler(eui, &data);
		if (data.data) {
			unsigned char buf[40];
			coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, coap_encode_var_bytes(buf, data.media_type), buf);
			coap_add_data(response, data.size, data.data);
			coap_server_destroy_data(&data);
		}
		response->code = COAP_RESPONSE_CODE(code);
	} else {
		response->code = COAP_RESPONSE_CODE(501);
	}
}

/**
 * Time resource GET method.
 */
static void hnd_get_event(coap_context_t *ctx, struct coap_resource_t *resource,
		coap_session_t *session, coap_pdu_t *request, str *token, str *query,
		coap_pdu_t *response) {
	(void) request;
#ifdef CHECK_MEDIA_TYPE
	uint8_t media_type = get_request_media_type(request);
	if (media_type != MEDIA_TYPE && media_type != COAP_MEDIATYPE_ANY) {
		printf("Media type %d not supported\n", media_type);
		response->code = COAP_RESPONSE_CODE(415);
		return;
	}
#endif
	response->code = COAP_RESPONSE_CODE(405);
}

/**
 * Event resource POST method.
 */
static void hnd_post_event(coap_context_t *ctx UNUSED_PARAM,
		struct coap_resource_t *resource, coap_session_t *session,
		coap_pdu_t *request, str *token UNUSED_PARAM, str *query UNUSED_PARAM,
		coap_pdu_t *response) {
	coap_log(LOG_INFO, "POST events/\n");
	log_remote_address(session->remote_addr);
#ifdef CHECK_MEDIA_TYPE
	uint8_t media_type = get_request_media_type(request);
	if (media_type != MEDIA_TYPE) {
		printf("Media type %d not supported\n", media_type);
		response->code = COAP_RESPONSE_CODE(415);
		return;
	}
#endif
	response->code = COAP_RESPONSE_CODE(201);
	char *eui = NULL;
	if (resolver_resolve_in6_addr(&session->remote_addr.addr.sin6.sin6_addr, &eui) != RESOLVER_OK) {
		response->code = COAP_RESPONSE_CODE(403);
		return;
	}
	// Callback
	if (post_event_callback_handler)
	{
		size_t size;
		unsigned char *data;
		coap_resource_set_dirty(resource, NULL);
		coap_get_data(request, &size, &data);
		int code = post_event_callback_handler(eui, data, size);
		response->code = COAP_RESPONSE_CODE(code);
	} else {
		response->code = COAP_RESPONSE_CODE(501);
	}
}

/**
 * Initialize CoAP resources.
 */
static void init_resources(coap_context_t *ctx) {
	coap_resource_t *r;

	// Init index resource (only for testing purposes)
	coap_log(LOG_INFO, "index resource init\n");
	r = coap_resource_init(NULL, 0, 0);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

	coap_add_attr(r, (unsigned char *) "ct", 2, (unsigned char *) "0", 1, 0);
	coap_add_attr(r, (unsigned char *) "title", 5,
			(unsigned char *) "\"General Info\"", 14, 0);
	coap_add_resource(ctx, r);

	// Init core resource
	coap_log(LOG_INFO, "core resource init\n");
	r = coap_resource_init((unsigned char *) ".well-known/core", 16, 0);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_core);
	coap_register_handler(r, COAP_REQUEST_POST, hnd_post_core);

	coap_add_attr(r, (unsigned char *) "rt", 2, (unsigned char *) "Core", 4, 0);
	coap_add_attr(r, (unsigned char *) "ct", 2, (unsigned char *) STRINGIZE(COAP_MEDIATYPE_APPLICATION_JSON), 2, 0);
	coap_add_attr(r, (unsigned char *) "title", 5,
			(unsigned char *) "\"Core Resource\"", 15, 0);
	coap_add_attr(r, (unsigned char *) "if", 2, (unsigned char *) "\"resources\"",
				11, 0);
	coap_add_resource(ctx, r);
	core_resource = r;

	// Init time resource
	coap_log(LOG_INFO, "time resource init\n");
	r = coap_resource_init((unsigned char *) "time", 4, 0);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_time);

	coap_add_attr(r, (unsigned char *) "ct", 2, (unsigned char *) STRINGIZE(COAP_MEDIATYPE_APPLICATION_JSON), 2, 0);
	coap_add_attr(r, (unsigned char *) "title", 5,
			(unsigned char *) "\"Gateway Clock\"", 15, 0);
	coap_add_resource(ctx, r);
	time_resource = r;

	// Init events resource
	coap_log(LOG_INFO, "events resource init\n");
	r = coap_resource_init((unsigned char *) "events", 6,
			COAP_RESOURCE_FLAGS_NOTIFY_CON);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_event);
	coap_register_handler(r, COAP_REQUEST_POST, hnd_post_event);

	coap_add_attr(r, (unsigned char *) "ct", 2, (unsigned char *) STRINGIZE(COAP_MEDIATYPE_APPLICATION_JSON), 2, 0);
	coap_add_attr(r, (unsigned char *) "title", 5,
			(unsigned char *) "\"Gateway Events\"", 16, 0);
	coap_add_attr(r, (unsigned char *) "rt", 2, (unsigned char *) "\"Events\"",
			8, 0);
	r->observable = 1;
	coap_add_attr(r, (unsigned char *) "if", 2, (unsigned char *) "\"event\"",
			9, 0);

	coap_add_resource(ctx, r);
	event_resource = r;
}

/**
 * Initialize CoAP context.
 */
static coap_context_t * get_context(const char *node, const char *port) {
	coap_context_t *ctx = NULL;
	int s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	ctx = coap_new_context(NULL);
	if (!ctx) {
		return NULL;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM; // use UDP protocol
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

	s = getaddrinfo(node, port, &hints, &result);
	if (s != 0) {
		coap_free_context(ctx);
		return NULL;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		coap_address_t addr, addrs;
		coap_endpoint_t *ep_udp = NULL, *ep_dtls = NULL, *ep_tcp = NULL,
				*ep_tls = NULL;

		if (rp->ai_addrlen <= sizeof(addr.addr)) {
			coap_address_init(&addr);
			addr.size = rp->ai_addrlen;
			memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);
			addrs = addr;
			if (addr.addr.sa.sa_family == AF_INET) {
				addrs.addr.sin.sin_port = htons(
						ntohs(addr.addr.sin.sin_port) + 1);
			} else if (addr.addr.sa.sa_family == AF_INET6) {
				addrs.addr.sin6.sin6_port = htons(
						ntohs(addr.addr.sin6.sin6_port) + 1);
			} else {
				goto finish;
			}

			ep_udp = coap_new_endpoint(ctx, &addr, COAP_PROTO_UDP);
			if (ep_udp) {
				;
			} else {
				coap_log(LOG_CRIT, "cannot create UDP endpoint\n");
				continue;
			}
			ep_tcp = coap_new_endpoint(ctx, &addr, COAP_PROTO_TCP);
			if (ep_tcp) {
				;
			} else {
				coap_log(LOG_CRIT, "cannot create TCP endpoint\n");
			}
			if (ep_udp)
				goto finish;
		}
	}

	fprintf(stderr, "no context available for interface '%s'\n", node);

	finish: freeaddrinfo(result);
	return ctx;
}

coap_server_result_t coap_server_init()
{
	if (resolver_init() != RESOLVER_OK) {
		return COAP_ERROR;
	}
	char addr_str[NI_MAXHOST] = COAP_LISTENER_ADDRESS;
	char port_str[NI_MAXSERV] = COAP_LISTENER_PORT;
	coap_log_t log_level = COAP_LOG_LEVEL;

	coap_startup();
	coap_dtls_set_log_level(log_level);
	coap_set_log_level(log_level);

	ctx = get_context(addr_str, port_str);
	if (!ctx) {
		resolver_deinit();
		return COAP_ERROR;
	}

	init_resources(ctx);
	return COAP_OK;
}

void coap_server_register_event_cb(post_cb_t callback)
{
	post_event_callback_handler = callback;
}

void coap_server_register_time_cb(get_cb_t callback)
{
	get_time_callback_handler = callback;
}

coap_server_result_t coap_server_do_work(unsigned wait_ms)
{
	coap_server_result_t err = COAP_ERROR;
	int result = coap_run_once(ctx, wait_ms);
	if (result < 0) {
		return result;
	} else if ((unsigned) result < wait_ms) {
		err = COAP_PROCESSED;
	} else {
		if (event_resource) {
			coap_resource_set_dirty(event_resource, NULL);
		}
		err = COAP_TIMEOUT;
	}
	coap_check_notify(ctx);
	return err;
}

void coap_server_deinit(void)
{
	coap_free_context(ctx);
	coap_cleanup();
	resolver_deinit();
}

void coap_server_set_cb_data(coap_server_data_t* data, const unsigned char *buf, size_t size) {
	if (data == NULL) return;
	data->data = (unsigned char *)malloc((size)*sizeof(unsigned char));
	if (data->data) {
		memcpy(data->data, buf, size);
		data->media_type = MEDIA_TYPE;
		data->size = size;
	}
}

void coap_server_destroy_data(coap_server_data_t* data) {
	free(data->data);
	data->media_type = 0;
	data->size = 0;
}
