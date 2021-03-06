// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

static void* real_malloc(size_t size)
{
    return malloc(size);
}

static void real_free(void* ptr)
{
    free(ptr);
}

#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/string_tokenizer.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/connection_string_parser.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/http_proxy_io.h"

#include "azure_uhttp_c/uhttp.h"

#include "provisioning_sc_enrollment.h"
#include "provisioning_sc_models_serializer.h"
#include "provisioning_sc_shared_helpers.h"

#undef ENABLE_MOCKS

#include "provisioning_service_client.h"

typedef enum {RESPONSE_ON, RESPONSE_OFF} response_switch;
typedef enum {CERT, NO_CERT} cert_flag;
typedef enum {TRACE, NO_TRACE} trace_flag;
typedef enum { PROXY, NO_PROXY } proxy_flag;

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static int g_uhttp_client_dowork_call_count;
static ON_HTTP_OPEN_COMPLETE_CALLBACK g_on_http_open;
static void* g_http_open_ctx;
static ON_HTTP_REQUEST_CALLBACK g_on_http_reply_recv;
static void* g_http_reply_recv_ctx;

static response_switch g_response_content_status;

static cert_flag g_cert;
static trace_flag g_trace;
static proxy_flag g_proxy;


#ifdef __cplusplus
extern "C"
{
#endif

    int STRING_sprintf(STRING_HANDLE handle, const char* format, ...);
    STRING_HANDLE STRING_construct_sprintf(const char* format, ...);

#ifdef __cplusplus
}
#endif

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

#ifdef __cplusplus
extern "C"
{
#endif
    int STRING_sprintf(STRING_HANDLE handle, const char* format, ...)
    {
        (void)handle;
        (void)format;
        return 0;
    }

    STRING_HANDLE STRING_construct_sprintf(const char* format, ...)
    {
        (void)format;
        return (STRING_HANDLE)real_malloc(1);
    }
#ifdef __cplusplus
}
#endif

//Control Parameters
#define IOTHUBHOSTNAME "HostName"
#define IOTHUBSHAREDACESSKEYNAME "SharedAccessKeyName"
#define IOTHUBSHAREDACESSKEY "SharedAccessKey"

static const MAP_HANDLE TEST_MAP_HANDLE = (MAP_HANDLE)0x11111111;
static const IO_INTERFACE_DESCRIPTION* TEST_INTERFACE_DESC = (IO_INTERFACE_DESCRIPTION*)0x11111112;
static const HTTP_CLIENT_HANDLE TEST_HTTP_CLIENT_HANDLE = (HTTP_CLIENT_HANDLE)0x11111113;
static INDIVIDUAL_ENROLLMENT_HANDLE TEST_INDIVIDUAL_ENROLLMENT_HANDLE = (INDIVIDUAL_ENROLLMENT_HANDLE)0x11111114;
static ATTESTATION_MECHANISM_HANDLE TEST_ATT_MECH_HANDLE = (ATTESTATION_MECHANISM_HANDLE)0x11111115;
static PROVISIONING_SERVICE_CLIENT_HANDLE TEST_PROVISIONING_SC_HANDLE = (PROVISIONING_SERVICE_CLIENT_HANDLE)0x11111116;
static ENROLLMENT_GROUP_HANDLE TEST_ENROLLMENT_GROUP_HANDLE = (ENROLLMENT_GROUP_HANDLE)0x11111117;
static IO_INTERFACE_DESCRIPTION* TEST_IO_INTERFACE_DESC = (IO_INTERFACE_DESCRIPTION*)0x11111118;
static const unsigned char* TEST_REPLY_JSON = (const unsigned char*)"{my-json-reply}";
static const char* TEST_ENROLLMENT_JSON = "{my-json-serialized-enrollment}";
static const char* TEST_CONNECTION_STRING = "my-connection-string";
static const char* TEST_STRING = "my-string";
static const char* TEST_EK = "my-ek";
static const char* TEST_REGID = "my-regid";
static const char* TEST_GROUPID = "my-groupid";
static const char* TEST_ETAG = "my-etag";
static const char* TEST_ETAG_STAR = "*";
static const char* TEST_HOSTNAME = "my-hostname";
static const char* TEST_SHARED_ACCESS_KEY = "my-shared-access-key";
static const char* TEST_SHARED_ACCESS_KEY_NAME = "my-shared-access-key-name";
static const char* TEST_TRUSTED_CERT = "my-trusted-cert";
static const char* TEST_PROXY_HOSTNAME = "my-proxy-hostname";
static const char* TEST_PROXY_USERNAME = "my-username";
static const char* TEST_PROXY_PASSWORD = "my-password";
static int TEST_PROXY_PORT = 123;
static size_t TEST_REPLY_JSON_LEN = 15;
static unsigned int STATUS_CODE_SUCCESS = 204;

typedef enum {ETAG, NO_ETAG} etag_flag;
typedef enum {RESPONSE, NO_RESPONSE} response_flag;


static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static HTTP_CLIENT_HANDLE my_uhttp_client_create(const IO_INTERFACE_DESCRIPTION* io_interface_desc, const void* xio_param, ON_HTTP_ERROR_CALLBACK on_http_error, void* callback_ctx)
{
    (void)io_interface_desc;
    (void)xio_param;
    (void)on_http_error;
    (void)callback_ctx;

    return (HTTP_CLIENT_HANDLE)real_malloc(1);
}

static void my_uhttp_client_destroy(HTTP_CLIENT_HANDLE handle)
{
    real_free(handle);
}

static HTTP_CLIENT_RESULT my_uhttp_client_open(HTTP_CLIENT_HANDLE handle, const char* host, int port_num, ON_HTTP_OPEN_COMPLETE_CALLBACK on_connect, void* callback_ctx)
{
    (void)handle;
    (void)host;
    (void)port_num;
    g_on_http_open = on_connect;
    g_http_open_ctx = callback_ctx; //prov_client

    //note that a real malloc does occur in this fn, but it can't be mocked since it's in a field of handle

    return HTTP_CLIENT_OK;
}

static void my_uhttp_client_close(HTTP_CLIENT_HANDLE handle, ON_HTTP_CLOSED_CALLBACK on_close_callback, void* callback_ctx)
{
    (void)handle;
    (void)on_close_callback;
    (void)callback_ctx;

    //note that a real free does occur in this fn, but it can't be mocked since it's in a field of handle
}

static HTTP_CLIENT_RESULT my_uhttp_client_execute_request(HTTP_CLIENT_HANDLE handle, HTTP_CLIENT_REQUEST_TYPE request_type, const char* relative_path,
    HTTP_HEADERS_HANDLE http_header_handle, const unsigned char* content, size_t content_len, ON_HTTP_REQUEST_CALLBACK on_request_callback, void* callback_ctx)
{
    (void)handle;
    (void)request_type;
    (void)relative_path;
    (void)http_header_handle;
    (void)content;
    (void)content_len;
    g_on_http_reply_recv = on_request_callback;
    g_http_reply_recv_ctx = callback_ctx;

    return HTTP_CLIENT_OK;
}

static void my_uhttp_client_dowork(HTTP_CLIENT_HANDLE handle)
{
    (void)handle;

    unsigned char* content;
    if (g_response_content_status == RESPONSE_ON)
        content = (unsigned char*)TEST_REPLY_JSON;
    else
        content = NULL;

    if (g_uhttp_client_dowork_call_count == 0)
        g_on_http_open(g_http_open_ctx, HTTP_CALLBACK_REASON_OK);
    else if (g_uhttp_client_dowork_call_count == 1)
    {
        g_on_http_reply_recv(g_http_reply_recv_ctx, HTTP_CALLBACK_REASON_OK, content, 1, STATUS_CODE_SUCCESS, NULL);
    }
    g_uhttp_client_dowork_call_count++;
}

static const char* my_Map_GetValueFromKey(MAP_HANDLE handle, const char* key)
{
    char* result = NULL;
    (void)handle;

    if (strcmp(key, IOTHUBHOSTNAME) == 0)
        result = (char*)TEST_HOSTNAME;
    else if (strcmp(key, IOTHUBSHAREDACESSKEY) == 0)
        result = (char*)TEST_SHARED_ACCESS_KEY;
    else if (strcmp(key, IOTHUBSHAREDACESSKEYNAME) == 0)
        result = (char*)TEST_SHARED_ACCESS_KEY_NAME;

    return result;
}

static int my_mallocAndStrcpy_s(char** destination, const char* source)
{
    (void)source;
    size_t src_len = strlen(source);
    *destination = (char*)real_malloc(src_len + 1);
    strcpy(*destination, source);
    return 0;
}

static int my_mallocAndStrcpy_overwrite(char** destination, const char* source)
{
    int result = 0;
    char* temp = NULL;

    if (destination == NULL || source == NULL)
    {
        LogError("Invalid input");
        result = __FAILURE__;
    }
    else if (my_mallocAndStrcpy_s(&temp, source) != 0)
    {
        LogError("Failed to copy value from source");
        result = __FAILURE__;
    }
    else
    {
        real_free(*destination);
        *destination = temp;
    }

    return result;
}

static HTTP_HEADERS_HANDLE my_HTTPHeaders_Alloc(void)
{
    return (HTTP_HEADERS_HANDLE)real_malloc(1);
}

static void my_HTTPHeaders_Free(HTTP_HEADERS_HANDLE handle)
{
    real_free(handle);
}

static STRING_HANDLE my_STRING_construct(const char* psz)
{
    (void)psz;
    return (STRING_HANDLE)real_malloc(1);
}

static void my_STRING_delete(STRING_HANDLE handle)
{
    real_free(handle);
}

static STRING_HANDLE my_SASToken_CreateString(const char* key, const char* scope, const char* keyName, size_t expiry) {
    (void)key;
    (void)scope;
    (void)keyName;
    (void)expiry;
    return (STRING_HANDLE)real_malloc(1);
}

static STRING_HANDLE my_URL_EncodeString(const char* textEncode)
{
    (void)textEncode;
    return (STRING_HANDLE)real_malloc(1);
}

static INDIVIDUAL_ENROLLMENT_HANDLE my_individualEnrollment_deserializeFromJson(const char* json_string)
{
    INDIVIDUAL_ENROLLMENT_HANDLE result;
    if (json_string != NULL)
        result = (INDIVIDUAL_ENROLLMENT_HANDLE)real_malloc(1);
    else
        result = NULL;
    return result;
}

static ENROLLMENT_GROUP_HANDLE my_enrollmentGroup_deserializeFromJson(const char* json_string)
{
    ENROLLMENT_GROUP_HANDLE result;
    if (json_string != NULL)
        result = (ENROLLMENT_GROUP_HANDLE)real_malloc(1);
    else
        result = NULL;
    return result;
}

static void my_individualEnrollment_destroy(INDIVIDUAL_ENROLLMENT_HANDLE handle)
{
    real_free(handle);
}

static void my_enrollmentGroup_destroy(ENROLLMENT_GROUP_HANDLE handle)
{
    real_free(handle);
}

static INDIVIDUAL_ENROLLMENT_HANDLE my_individualEnrollment_create(const char* reg_id, ATTESTATION_MECHANISM_HANDLE att_handle)
{
    (void)reg_id;
    (void)att_handle;
    return (INDIVIDUAL_ENROLLMENT_HANDLE)real_malloc(1);
}

static ENROLLMENT_GROUP_HANDLE my_enrollmentGroup_create(const char* group_id, ATTESTATION_MECHANISM_HANDLE att_handle)
{
    (void)group_id;
    (void)att_handle;
    return (ENROLLMENT_GROUP_HANDLE)real_malloc(1);
}

static const char* my_individualEnrollment_getRegistrationId(INDIVIDUAL_ENROLLMENT_HANDLE handle)
{
    const char* result = NULL;
    if (handle != NULL)
        result = TEST_REGID;
    return result;
}

static const char* my_enrollmentGroup_getGroupId(ENROLLMENT_GROUP_HANDLE handle)
{
    const char* result = NULL;
    if (handle != NULL)
        result = TEST_GROUPID;
    return result;
}

static const char* my_individualEnrollment_getEtag(INDIVIDUAL_ENROLLMENT_HANDLE handle)
{
    const char* result = NULL;
    if (handle != NULL)
        result = TEST_ETAG;
    return result;
}

static const char* my_enrollmentGroup_getEtag(ENROLLMENT_GROUP_HANDLE handle)
{
    const char* result = NULL;
    if (handle != NULL)
        result = TEST_ETAG;
    return result;
}

static char* my_individualEnrollment_serializeToJson(INDIVIDUAL_ENROLLMENT_HANDLE handle)
{
    (void)handle;
    char* result = NULL;
    size_t len = strlen(TEST_ENROLLMENT_JSON);
    result = (char*)real_malloc(len + 1);
    strncpy(result, TEST_ENROLLMENT_JSON, len + 1);
    return result;
}

static char* my_enrollmentGroup_serializeToJson(ENROLLMENT_GROUP_HANDLE handle)
{
    (void)handle;
    char* result = NULL;
    size_t len = strlen(TEST_ENROLLMENT_JSON);
    result = (char*)real_malloc(len + 1);
    strncpy(result, TEST_ENROLLMENT_JSON, len + 1);
    return result;
}

static void register_global_mock_hooks()
{
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, real_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, real_free);

    REGISTER_GLOBAL_MOCK_HOOK(Map_GetValueFromKey, my_Map_GetValueFromKey);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(Map_GetValueFromKey, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_s, my_mallocAndStrcpy_s);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(mallocAndStrcpy_s, __FAILURE__);

    REGISTER_GLOBAL_MOCK_HOOK(mallocAndStrcpy_overwrite, my_mallocAndStrcpy_overwrite);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(mallocAndStrcpy_overwrite, __FAILURE__);

    REGISTER_GLOBAL_MOCK_HOOK(STRING_construct, my_STRING_construct);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_construct, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(STRING_delete, my_STRING_delete);

    REGISTER_GLOBAL_MOCK_HOOK(SASToken_CreateString, my_SASToken_CreateString);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(SASToken_CreateString, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(URL_EncodeString, my_URL_EncodeString);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(URL_EncodeString, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(uhttp_client_dowork, my_uhttp_client_dowork);

    REGISTER_GLOBAL_MOCK_HOOK(uhttp_client_create, my_uhttp_client_create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(uhttp_client_create, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(uhttp_client_open, my_uhttp_client_open);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(uhttp_client_open, HTTP_CLIENT_ERROR);

    REGISTER_GLOBAL_MOCK_HOOK(uhttp_client_close, my_uhttp_client_close);

    REGISTER_GLOBAL_MOCK_HOOK(uhttp_client_destroy, my_uhttp_client_destroy);

    REGISTER_GLOBAL_MOCK_HOOK(uhttp_client_execute_request, my_uhttp_client_execute_request);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(uhttp_client_execute_request, HTTP_CLIENT_ERROR);

    REGISTER_GLOBAL_MOCK_HOOK(HTTPHeaders_Alloc, my_HTTPHeaders_Alloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(HTTPHeaders_Alloc, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(HTTPHeaders_Free, my_HTTPHeaders_Free);

    REGISTER_GLOBAL_MOCK_HOOK(individualEnrollment_deserializeFromJson, my_individualEnrollment_deserializeFromJson);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(individualEnrollment_deserializeFromJson, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(individualEnrollment_create, my_individualEnrollment_create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(individualEnrollment_create, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(individualEnrollment_destroy, my_individualEnrollment_destroy);

    REGISTER_GLOBAL_MOCK_HOOK(individualEnrollment_getRegistrationId, my_individualEnrollment_getRegistrationId);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(individualEnrollment_getRegistrationId, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(individualEnrollment_getEtag, my_individualEnrollment_getEtag);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(individualEnrollment_getEtag, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(enrollmentGroup_deserializeFromJson, my_enrollmentGroup_deserializeFromJson);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(enrollmentGroup_deserializeFromJson, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(enrollmentGroup_create, my_enrollmentGroup_create);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(enrollmentGroup_create, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(enrollmentGroup_destroy, my_enrollmentGroup_destroy);

    REGISTER_GLOBAL_MOCK_HOOK(enrollmentGroup_getGroupId, my_enrollmentGroup_getGroupId);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(enrollmentGroup_getGroupId, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(enrollmentGroup_getEtag, my_enrollmentGroup_getEtag);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(enrollmentGroup_getEtag, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(individualEnrollment_serializeToJson, my_individualEnrollment_serializeToJson);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(individualEnrollment_serializeToJson, NULL);

    REGISTER_GLOBAL_MOCK_HOOK(enrollmentGroup_serializeToJson, my_enrollmentGroup_serializeToJson);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(enrollmentGroup_serializeToJson, NULL);
}

static void register_global_mock_returns()
{
    REGISTER_GLOBAL_MOCK_RETURN(connectionstringparser_parse, TEST_MAP_HANDLE);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(connectionstringparser_parse, NULL);

    REGISTER_GLOBAL_MOCK_RETURN(HTTPHeaders_AddHeaderNameValuePair, HTTP_HEADERS_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(HTTPHeaders_AddHeaderNameValuePair, HTTP_HEADERS_ERROR);

    REGISTER_GLOBAL_MOCK_RETURN(platform_get_default_tlsio, TEST_INTERFACE_DESC);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(platform_get_default_tlsio, NULL);

    REGISTER_GLOBAL_MOCK_RETURN(uhttp_client_set_trusted_cert, HTTP_CLIENT_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(uhttp_client_set_trusted_cert, HTTP_CLIENT_ERROR);

    REGISTER_GLOBAL_MOCK_RETURN(uhttp_client_set_trace, HTTP_CLIENT_OK);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(uhttp_client_set_trace, HTTP_CLIENT_INVALID_ARG);

    REGISTER_GLOBAL_MOCK_RETURN(STRING_c_str, TEST_STRING);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_c_str, NULL);

    REGISTER_GLOBAL_MOCK_RETURN(http_proxy_io_get_interface_description, TEST_IO_INTERFACE_DESC);
}

static void register_global_mock_alias_types()
{
    REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(MAP_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ATTESTATION_MECHANISM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(INDIVIDUAL_ENROLLMENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ENROLLMENT_GROUP_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(const INDIVIDUAL_ENROLLMENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(const ENROLLMENT_GROUP_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTP_HEADERS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_HTTP_ERROR_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_HTTP_OPEN_COMPLETE_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_HTTP_REQUEST_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(ON_HTTP_CLOSED_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTP_CLIENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTP_CLIENT_REQUEST_TYPE, int);
}

BEGIN_TEST_SUITE(provisioning_service_client_ut)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);
    umocktypes_bool_register_types();

    register_global_mock_alias_types();
    register_global_mock_hooks();
    register_global_mock_returns();

}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    g_on_http_open = NULL;
    g_http_open_ctx = NULL;
    g_on_http_reply_recv = NULL;
    g_http_reply_recv_ctx = NULL;
    g_uhttp_client_dowork_call_count = 0;
    g_response_content_status = RESPONSE_ON;

    g_cert = NO_CERT;
    g_trace = NO_TRACE;
    g_proxy = NO_PROXY;

    umock_c_negative_tests_deinit();
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

static int should_skip_index(size_t current_index, const size_t skip_array[], size_t length)
{
    int result = 0;
    for (size_t index = 0; index < length; index++)
    {
        if (current_index == skip_array[index])
        {
            result = __FAILURE__;
            break;
        }
    }
    return result;
}

static void set_response_status(response_switch response)
{
    g_response_content_status = response;
}

static void expected_calls_mallocAndStrcpy_overwrite()
{
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
}

static void expected_calls_construct_registration_path()
{
    STRICT_EXPECTED_CALL(URL_EncodeString(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail
}

static void expected_calls_construct_http_headers(etag_flag etag_flag, HTTP_CLIENT_REQUEST_TYPE request)
{
    STRICT_EXPECTED_CALL(HTTPHeaders_Alloc());
    STRICT_EXPECTED_CALL(get_time(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(SASToken_CreateString(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    if (request != HTTP_CLIENT_REQUEST_DELETE)
    {
        STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    }
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    if (etag_flag == ETAG)
        STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail
}

static void expected_calls_connect_to_service()
{
    if (g_proxy == PROXY)
        STRICT_EXPECTED_CALL(http_proxy_io_get_interface_description()); //does not fail
    STRICT_EXPECTED_CALL(platform_get_default_tlsio());
    STRICT_EXPECTED_CALL(uhttp_client_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    if (g_cert == CERT)
        STRICT_EXPECTED_CALL(uhttp_client_set_trusted_cert(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    if (g_trace == TRACE)
        STRICT_EXPECTED_CALL(uhttp_client_set_trace(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uhttp_client_open(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
}

static void expected_calls_rest_call(HTTP_CLIENT_REQUEST_TYPE request_type, response_flag response_flag)
{
    expected_calls_connect_to_service();
    STRICT_EXPECTED_CALL(uhttp_client_dowork(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(uhttp_client_execute_request(IGNORED_PTR_ARG, request_type, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uhttp_client_dowork(IGNORED_PTR_ARG)); //does not fail
    if (response_flag == RESPONSE)
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); //this is in a callback for on_http_reply_recv
    STRICT_EXPECTED_CALL(uhttp_client_close(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(uhttp_client_destroy(IGNORED_PTR_ARG)); //does not fail

}

/* UNIT TESTS BEGIN */

/* Tests_PROVISIONING_SERVICE_CLIENT_22_001: [ If conn_string is NULL prov_sc_create_from_connection_string shall fail and return NULL ] */
TEST_FUNCTION(prov_sc_create_from_connection_string_ERROR_INPUT_NULL)
{
    //arrange

    //act
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(NULL);

    //assert
    ASSERT_IS_NULL(sc);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_002: [ conn_string shall be parsed and its information will populate a new PROVISIONING_SERVICE_CLIENT_HANDLE ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_004: [ Upon successful creation of the new PROVISIONING_SERVICE_CLIENT_HANDLE, prov_sc_create_from_connection_string shall return it ] */
TEST_FUNCTION(prov_sc_create_from_connection_string_GOLDEN)
{
    //arrange
    STRICT_EXPECTED_CALL(STRING_construct(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(connectionstringparser_parse(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_GetValueFromKey(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_GetValueFromKey(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_GetValueFromKey(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));

    //act
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    //assert
    ASSERT_IS_NOT_NULL(sc);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_SRS_PROVISIONING_SERVICE_CLIENT_22_003: [ If the new PROVISIONING_SERVICE_CLIENT_HANDLE is not correctly populated prov_sc_create_from_connection_string shall fail and return NULL ] */
TEST_FUNCTION(prov_sc_create_from_connection_string_FAIL)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    STRICT_EXPECTED_CALL(STRING_construct(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(connectionstringparser_parse(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_GetValueFromKey(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_GetValueFromKey(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_GetValueFromKey(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mallocAndStrcpy_s(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Map_Destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 9, 10 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_create_from_connection_string failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);

        //assert
        ASSERT_IS_NULL_WITH_MSG(sc, tmp_msg);

        //cleanup
        prov_sc_destroy(sc);
    }

    //cleanup
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_005: [ prov_sc_destroy shall free all the memory contained inside prov_client ] */
TEST_FUNCTION(prov_sc_destroy_INPUT_NULL)
{
    //arrange

    //act
    prov_sc_destroy(NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_005: [ prov_sc_destroy shall free all the memory contained inside prov_client ] */
TEST_FUNCTION(prov_sc_destroy_GOLDEN)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    //act
    prov_sc_destroy(sc);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_068: [ If prov_client is NULL, prov_sc_trace_on shall do nothing ] */
TEST_FUNCTION(prov_sc_set_trace_INPUT_NULL)
{
    //arrange

    //act
    prov_sc_set_trace(NULL, TRACING_STATUS_ON);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());


    //cleanup

}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_069: [ HTTP tracing for communications using prov_client will be set to status ] */
TEST_FUNCTION(prov_sc_trace_GOLDEN)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    //act
    prov_sc_set_trace(sc, TRACING_STATUS_ON);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    //can't really test that the value was set since it's abstracted behind a handle, so this is a pretty useless test for now
    
    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_058: [ If prov_client is NULL, prov_sc_set_certificate shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_set_certificate_ERROR_NULL_HANDLE)
{
    //arrange

    //act
    int result = prov_sc_set_certificate(NULL, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, result, 0);

    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_059: [ If certificate is NULL, any previously set trusted certificate will be cleared ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_062: [ Upon success, prov_sc_set_certficiate shall return 0 ]*/
TEST_FUNCTION(prov_sc_set_certificate_NULL_CERTIFICATE)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    //act
    int result = prov_sc_set_certificate(sc, NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, result, 0);

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_060: [ If certificate is not NULL, it will be set as the trusted certificate for prov_client ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_062: [ Upon success, prov_sc_set_certficiate shall return 0 ]*/
TEST_FUNCTION(prov_sc_set_certificate_GOLDEN)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_overwrite(IGNORED_PTR_ARG, TEST_TRUSTED_CERT));

    //act
    int result = prov_sc_set_certificate(sc, TEST_TRUSTED_CERT);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, result, 0);

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_061: [ If allocating the trusted certificate fails, prov_sc_set_certificate shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_set_certificate_FAIL)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(mallocAndStrcpy_overwrite(IGNORED_PTR_ARG, TEST_TRUSTED_CERT));

    umock_c_negative_tests_snapshot();

    //size_t calls_cannot_fail[] = { };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = 0; //sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);
    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, NULL, num_cannot_fail) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_set_certificate failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_set_certificate(sc, TEST_TRUSTED_CERT);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);
    }

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_063: [ If prov_client or proxy_options are NULL, prov_sc_set_proxy shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_set_proxy_ERROR_INPUT_NULL1)
{
    //arrange
    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_PROXY_HOSTNAME;
    proxy_options.port = TEST_PROXY_PORT;
    proxy_options.username = TEST_PROXY_USERNAME;
    proxy_options.password = TEST_PROXY_PASSWORD;

    //act
    int result = prov_sc_set_proxy(NULL, &proxy_options);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, result, 0);

    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_063: [ If prov_client or proxy_options are NULL, prov_sc_set_proxy shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_set_proxy_ERROR_INPUT_NULL2)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    //act
    int result = prov_sc_set_proxy(sc, NULL);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, result, 0);

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_064: [ If the host address is NULL in proxy_options, prov_sc_set_proxy shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_set_proxy_ERROR_NO_HOST)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = NULL;
    proxy_options.port = TEST_PROXY_PORT;
    proxy_options.username = TEST_PROXY_USERNAME;
    proxy_options.password = TEST_PROXY_PASSWORD;

    umock_c_reset_all_calls();

    //act
    int result = prov_sc_set_proxy(sc, &proxy_options);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, result, 0);

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_065: [ If only the username, or only the password is NULL in proxy_options, prov_sc_set_proxy shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_set_proxy_ERROR_ONLY_PASSWORD)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_PROXY_HOSTNAME;
    proxy_options.port = TEST_PROXY_PORT;
    proxy_options.username = NULL;
    proxy_options.password = TEST_PROXY_PASSWORD;

    umock_c_reset_all_calls();

    //act
    int result = prov_sc_set_proxy(sc, &proxy_options);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, result, 0);

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_065: [ If only the username, or only the password is NULL in proxy_options, prov_sc_set_proxy shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_set_proxy_ERROR_ONLY_USERNAME)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_PROXY_HOSTNAME;
    proxy_options.port = TEST_PROXY_PORT;
    proxy_options.username = TEST_PROXY_USERNAME;
    proxy_options.password = NULL;

    umock_c_reset_all_calls();

    //act
    int result = prov_sc_set_proxy(sc, &proxy_options);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, result, 0);

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_066: [ The proxy settings specified in proxy_options will be set for use by prov_client ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_067: [ Upon success, prov_sc_set_proxy shall return 0 ] */
TEST_FUNCTION(prov_sc_set_proxy_GOLDEN_NO_LOGIN)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_PROXY_HOSTNAME;
    proxy_options.port = TEST_PROXY_PORT;
    proxy_options.username = NULL;
    proxy_options.password = NULL;

    umock_c_reset_all_calls();

    //act
    int result = prov_sc_set_proxy(sc, &proxy_options);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, result, 0);

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_066: [ The proxy settings specified in proxy_options will be set for use by prov_client ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_067: [ Upon success, prov_sc_set_proxy shall return 0 ] */
TEST_FUNCTION(prov_sc_set_proxy_GOLDEN_FULL_PROXY)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_PROXY_HOSTNAME;
    proxy_options.port = TEST_PROXY_PORT;
    proxy_options.username = TEST_PROXY_USERNAME;
    proxy_options.password = TEST_PROXY_PASSWORD;

    umock_c_reset_all_calls();

    //act
    int result = prov_sc_set_proxy(sc, &proxy_options);

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, result, 0);

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_006: [ If prov_client or enrollment_ptr are NULL, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_ERROR_INPUT_NULL_SC_HANDLE)
{
    //arrange
    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);
    umock_c_reset_all_calls();

    //act
    int result = prov_sc_create_or_update_individual_enrollment(NULL, &ie);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, result, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    individualEnrollment_destroy(ie);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_006: [ If prov_client or enrollment_ptr are NULL, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_ERROR_INPUT_NULL_IE_HANDLE)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    //act
    int result = prov_sc_create_or_update_individual_enrollment(sc, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, result, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_006: [ If prov_client or enrollment_ptr are NULL, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_ERROR_INPUT_NULL_IE_HANDLE2)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = NULL;
    umock_c_reset_all_calls();

    //act
    int result = prov_sc_create_or_update_individual_enrollment(sc, &ie);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, result, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_007: [ A 'PUT' REST call shall be issued to create/update the enrollment record of a device on the Provisioning Service, using data contained in enrollment_ptr ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_044: [ The data in enrollment_ptr will be updated to reflect new information added by the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_009: [ Upon a successful create or update, prov_sc_create_or_update_individual_enrollment shall return 0 ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_GOLDEN)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);
    INDIVIDUAL_ENROLLMENT_HANDLE old_ie = ie;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(individualEnrollment_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG)).SetReturn(NULL);
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(individualEnrollment_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail

    //act
    int res = prov_sc_create_or_update_individual_enrollment(sc, &ie);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_IS_TRUE(old_ie != ie); //ie was changed by the function
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_007: [ A 'PUT' REST call shall be issued to create/update the enrollment record of a device on the Provisioning Service, using data contained in enrollment_ptr ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_044: [ The data in enrollment_ptr will be updated to reflect new information added by the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_009: [ Upon a successful create or update, prov_sc_create_or_update_individual_enrollment shall return 0 ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_GOLDEN_w_etag)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);
    INDIVIDUAL_ENROLLMENT_HANDLE old_ie = ie;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(individualEnrollment_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG));
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(individualEnrollment_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail

    //act
    int res = prov_sc_create_or_update_individual_enrollment(sc, &ie);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_IS_TRUE(old_ie != ie); //ie was changed by the function
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_007: [ A 'PUT' REST call shall be issued to create/update the enrollment record of a device on the Provisioning Service, using data contained in enrollment_ptr ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_044: [ The data in enrollment_ptr will be updated to reflect new information added by the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_009: [ Upon a successful create or update, prov_sc_create_or_update_individual_enrollment shall return 0 ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_GOLDEN_ALL_HTTP_OPTIONS)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_PROXY_HOSTNAME;
    proxy_options.port = TEST_PROXY_PORT;
    proxy_options.username = TEST_PROXY_USERNAME;
    proxy_options.password = TEST_PROXY_PASSWORD;

    prov_sc_set_proxy(sc, &proxy_options);
    prov_sc_set_certificate(sc, TEST_TRUSTED_CERT);
    prov_sc_set_trace(sc, TRACING_STATUS_ON);

    g_proxy = PROXY;
    g_cert = CERT;
    g_trace = TRACE;

    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);
    INDIVIDUAL_ENROLLMENT_HANDLE old_ie = ie;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(individualEnrollment_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG)).SetReturn(NULL);
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(individualEnrollment_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail

    //act
    int res = prov_sc_create_or_update_individual_enrollment(sc, &ie);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_IS_TRUE(old_ie != ie); //ie was changed by the function
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_008: [ If the 'PUT' REST call fails, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_045: [ If receiving the response from the Provisioning Service fails, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value. ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_FAIL_null_etag)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(individualEnrollment_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG)).SetReturn(NULL); //cannot fail
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(individualEnrollment_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_destroy(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 3, 4, 5, 7, 12, 14, 15, 19, 20, 21, 23, 24, 26, 27, 28, 29, 30 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_create_or_update_individual_enrollment failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_create_or_update_individual_enrollment(sc, &ie);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_008: [ If the 'PUT' REST call fails, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_045: [ If receiving the response from the Provisioning Service fails, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value. ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_FAIL_w_etag)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(individualEnrollment_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG)); //cannot fail
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(individualEnrollment_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_destroy(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 3, 4, 5, 7, 12, 15, 16, 20, 21, 22, 24, 25, 27, 28, 29, 30, 31 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_create_or_update_individual_enrollment failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_create_or_update_individual_enrollment(sc, &ie);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_008: [ If the 'PUT' REST call fails, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_045: [ If receiving the response from the Provisioning Service fails, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value. ] */
TEST_FUNCTION(prov_sc_create_or_update_individual_enrollment_FAIL_ALL_HTTP_OPTIONS)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);

    HTTP_PROXY_OPTIONS proxy_options;
    proxy_options.host_address = TEST_PROXY_HOSTNAME;
    proxy_options.port = TEST_PROXY_PORT;
    proxy_options.username = TEST_PROXY_USERNAME;
    proxy_options.password = TEST_PROXY_PASSWORD;

    prov_sc_set_proxy(sc, &proxy_options);
    prov_sc_set_certificate(sc, TEST_TRUSTED_CERT);
    prov_sc_set_trace(sc, TRACING_STATUS_ON);

    g_proxy = PROXY;
    g_cert = CERT;
    g_trace = TRACE;

    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(individualEnrollment_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG)).SetReturn(NULL); //cannot fail
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(individualEnrollment_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_destroy(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 3, 4, 5,  7, 12, 14, 15, 16, 22, 24, 26, 27, 29, 30, 31, 32, 33 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_create_or_update_individual_enrollment failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_create_or_update_individual_enrollment(sc, &ie);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_010: [ If prov_client or enrollment are NULL, prov_sc_delete_individual_enrollment shall fail and return return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_ERROR_INPUT_NULL_PROV_SC)
{
    //arrange
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));

    //act
    int res = prov_sc_delete_individual_enrollment(NULL, TEST_INDIVIDUAL_ENROLLMENT_HANDLE);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    
    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_010: [ If prov_client or enrollment are NULL, prov_sc_delete_individual_enrollment shall fail and return return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_ERROR_INPUT_NULL_ENROLLMENT)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));

    //act
    int res = prov_sc_delete_individual_enrollment(sc, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_011: [ A 'DELETE' REST call shall be issued to delete the individual enrollment record on the Provisioning Service that matches enrollment based on registration id and etag ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_013: [ Upon a successful delete, prov_sc_delete_individual_enrollment shall return 0 ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_GOLDEN_NO_ETAG)
{
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);
    set_response_status(RESPONSE_OFF);
    umock_c_reset_all_calls();

    //arrange
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG)).SetReturn(NULL); //no etag
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_delete_individual_enrollment(sc, ie);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_011: [ A 'DELETE' REST call shall be issued to delete the individual enrollment record on the Provisioning Service that matches enrollment based on registration id and etag ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_013: [ Upon a successful delete, prov_sc_delete_individual_enrollment shall return 0 ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_GOLDEN_WITH_ETAG)
{
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);
    set_response_status(RESPONSE_OFF);
    umock_c_reset_all_calls();

    //arrange
    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_delete_individual_enrollment(sc, ie);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_012: [ If the 'DELETE' REST call fails, prov_sc_delete_individual_enrollment shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_FAIL)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = individualEnrollment_create(TEST_REGID, TEST_ATT_MECH_HANDLE);
    set_response_status(RESPONSE_OFF);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(individualEnrollment_getEtag(IGNORED_PTR_ARG)); //can fail, but for sake of this example, cannot
    STRICT_EXPECTED_CALL(individualEnrollment_getRegistrationId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 0, 3, 4, 6, 10, 13, 14, 18, 20, 21, 22, 23, 24, 25 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_delete_individual_enrollment failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_delete_individual_enrollment(sc, ie);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_046: [ If prov_client or reg_id are NULL, prov_sc_delete_individual_enrollment_by_param shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_by_param_NULL_PROV_SC)
{
    //arrange

    //act
    int res = prov_sc_delete_individual_enrollment_by_param(NULL, TEST_REGID, TEST_ETAG);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_046: [ If prov_client or reg_id are NULL, prov_sc_delete_individual_enrollment_by_param shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_by_param_NULL_REG_ID)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    //act
    int res = prov_sc_delete_individual_enrollment_by_param(sc, NULL, TEST_ETAG);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_047: [ If etag is given as NULL, it shall be ignored ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_048: [A 'DELETE' REST call shall be issued to delete the individual enrollment record of a device with ID reg_id, and optionally, etag from the Provisioning Service] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_050: [ Upon a successful delete, prov_sc_delete_individual_enrollment_by_param shall return 0 ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_by_param_GOLDEN_NO_ETAG)
{
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    set_response_status(RESPONSE_OFF);
    umock_c_reset_all_calls();

    //arrange
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_DELETE); // <- this NO_ETAG flag is what proves Requirement 047 - if etag wasn't ignored, actual calls will not line up with expected
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_delete_individual_enrollment_by_param(sc, TEST_REGID, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_048: [A 'DELETE' REST call shall be issued to delete the individual enrollment record of a device with ID reg_id, and optionally, etag from the Provisioning Service] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_050: [ Upon a successful delete, prov_sc_delete_individual_enrollment_by_param shall return 0 ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_by_param_GOLDEN_WITH_ETAG)
{
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    set_response_status(RESPONSE_OFF);
    umock_c_reset_all_calls();

    //arrange
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_delete_individual_enrollment_by_param(sc, TEST_REGID, TEST_ETAG_STAR);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_049: [ If the 'DELETE' REST call fails, prov_sc_delete_individual_enrollment_by_param shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_individual_enrollment_by_param_FAIL)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    set_response_status(RESPONSE_OFF);

    umock_c_reset_all_calls();

    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 1, 2, 4, 8, 11, 12, 16, 18, 19, 20, 21, 22, 23 };

    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_delete_individual_enrollment_by_param failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_delete_individual_enrollment_by_param(sc, TEST_REGID, TEST_ETAG_STAR);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_014: [ If prov_client, reg_id or enrollment_ptr are NULL, prov_sc_get_individual_enrollment shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_get_individual_enrollment_ERROR_INPUT_NULL_PROV_SC)
{
    //arrange
    INDIVIDUAL_ENROLLMENT_HANDLE ie;

    //act
    int res = prov_sc_get_individual_enrollment(NULL, TEST_REGID, &ie);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_014: [ If prov_client, reg_id or enrollment_ptr are NULL, prov_sc_get_individual_enrollment shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_get_individual_enrollment_ERROR_INPUT_NULL_REG_ID)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie;
    umock_c_reset_all_calls();

    //act
    int res = prov_sc_get_individual_enrollment(sc, NULL, &ie);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_014: [ If prov_client, reg_id or enrollment_ptr are NULL, prov_sc_get_individual_enrollment shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_get_individual_enrollment_ERROR_INPUT_NULL_ENROLLMENT)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    //act
    int res = prov_sc_get_individual_enrollment(sc, TEST_REGID, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_015: [ A 'GET' REST call shall be issued to retrieve the enrollment record of a device with ID reg_id from the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_017: [ The data from the retrieved device enrollment record shall populate enrollment_ptr ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_019: [ Upon successful population of enrollment_ptr with the retrieved device enrollment record data, prov_sc_get_individual_enrollment shall return 0 ] */
TEST_FUNCTION(prov_sc_get_individual_enrollment_GOLDEN)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = NULL;
    umock_c_reset_all_calls();

    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_GET);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_GET, RESPONSE);
    STRICT_EXPECTED_CALL(individualEnrollment_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_get_individual_enrollment(sc, TEST_REGID, &ie);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(ie);
    
    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_016: [If the 'GET' REST call fails, prov_sc_get_individual_enrollment shall fail and return a non - zero value] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_018: [ If populating enrollment_ptr with retrieved data fails, prov_sc_get_individual_enrollment shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_get_individual_enrollment_FAIL)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    INDIVIDUAL_ENROLLMENT_HANDLE ie = NULL;
    umock_c_reset_all_calls();

    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_GET);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_GET, RESPONSE);
    STRICT_EXPECTED_CALL(individualEnrollment_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 1, 2, 4, 9, 11, 12, 16, 18, 20, 21, 23, 24, 25};
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_get_individual_enrollment failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_get_individual_enrollment(sc, TEST_REGID, &ie);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    individualEnrollment_destroy(ie);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_030: [ If prov_client or enrollment_ptr are NULL, prov_sc_create_or_update_enrollment_group shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_create_or_update_enrollment_group_ERROR_INPUT_NULL_SC_HANDLE)
{
    //arrange
    ENROLLMENT_GROUP_HANDLE eg = enrollmentGroup_create(TEST_GROUPID, TEST_ATT_MECH_HANDLE);
    umock_c_reset_all_calls();

    //act
    int result = prov_sc_create_or_update_enrollment_group(NULL, &eg);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, result, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    enrollmentGroup_destroy(eg);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_030: [ If prov_client or enrollment_ptr are NULL, prov_sc_create_or_update_enrollment_group shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_create_or_update_enrollment_group_ERROR_INPUT_NULL_EG_HANDLE)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    //act
    int result = prov_sc_create_or_update_enrollment_group(sc, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, result, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_030: [ If prov_client or enrollment_ptr are NULL, prov_sc_create_or_update_enrollment_group shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_create_or_update_enrollment_group_ERROR_INPUT_NULL_EG_HANDLE2)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = NULL;
    umock_c_reset_all_calls();

    //act
    int result = prov_sc_create_or_update_enrollment_group(sc, &eg);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, result, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_031: [ A 'PUT' REST call shall be issued to create/update the device enrollment group on the Provisioning Service, using data contained in enrollment_ptr ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_051: [ The data in enrollment_ptr will be updated to reflect new information added by the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_033: [ Upon a successful create or update, prov_sc_create_or_update_enrollment_group shall return 0 ] */
TEST_FUNCTION(prov_sc_create_or_update_enrollment_group_GOLDEN_no_etag)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = enrollmentGroup_create(TEST_GROUPID, TEST_ATT_MECH_HANDLE);
    ENROLLMENT_GROUP_HANDLE old_eg = eg;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(enrollmentGroup_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG)).SetReturn(NULL); //cannot fail
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(enrollmentGroup_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_create_or_update_enrollment_group(sc, &eg);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_IS_TRUE(old_eg != eg); //eg was changed by the function
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_031: [ A 'PUT' REST call shall be issued to create/update the device enrollment group on the Provisioning Service, using data contained in enrollment_ptr ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_051: [ The data in enrollment_ptr will be updated to reflect new information added by the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_033: [ Upon a successful create or update, prov_sc_create_or_update_enrollment_group shall return 0 ] */
TEST_FUNCTION(prov_sc_create_or_update_enrollment_group_GOLDEN_w_etag)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = enrollmentGroup_create(TEST_GROUPID, TEST_ATT_MECH_HANDLE);
    ENROLLMENT_GROUP_HANDLE old_eg = eg;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(enrollmentGroup_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG)); //cannot fail
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG));
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(enrollmentGroup_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //cannot fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_create_or_update_enrollment_group(sc, &eg);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_IS_TRUE(old_eg != eg); //eg was changed by the function
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_008: [ If the 'PUT' REST call fails, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_052: [ If receiving the response from the Provisioning Service fails, prov_sc_create_or_update_enrollment_group shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_create_or_update_enrollment_group_FAIL_no_etag)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = enrollmentGroup_create(TEST_GROUPID, TEST_ATT_MECH_HANDLE);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(enrollmentGroup_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG)).SetReturn(NULL); //cannot fail
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(enrollmentGroup_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_destroy(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 3, 4, 5, 7, 12, 14, 15, 19, 20, 21, 23, 24, 26, 27, 28, 29, 30 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_create_or_update_enrollment_group failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_create_or_update_enrollment_group(sc, &eg);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_008: [ If the 'PUT' REST call fails, prov_sc_create_or_update_individual_enrollment shall fail and return a non-zero value ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_052: [ If receiving the response from the Provisioning Service fails, prov_sc_create_or_update_enrollment_group shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_create_or_update_enrollment_group_FAIL_w_etag)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = enrollmentGroup_create(TEST_GROUPID, TEST_ATT_MECH_HANDLE);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(enrollmentGroup_serializeToJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG)); //cannot fail
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_PUT);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_PUT, RESPONSE);
    STRICT_EXPECTED_CALL(enrollmentGroup_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_destroy(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 3, 4, 5, 7, 12, 15, 16, 20, 21, 22, 24, 25, 27, 28, 29, 30, 31 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_create_or_update_enrollment_group failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_create_or_update_enrollment_group(sc, &eg);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_034: [ If prov_client or id are NULL, prov_sc_delete_enrollment_group shall return return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_ERROR_INPUT_NULL_PROV_SC)
{
    //arrange
    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));

    //act
    int res = prov_sc_delete_enrollment_group(NULL, TEST_ENROLLMENT_GROUP_HANDLE);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_034: [ If prov_client or id are NULL, prov_sc_delete_enrollment_group shall return return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_ERROR_INPUT_NULL_ENROLLMENT)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));

    //act
    int res = prov_sc_delete_enrollment_group(sc, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_035: [ A 'DELETE' REST call shall be issued to delete the device enrollment group that matches enrollment from the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_037: [ Upon a successful delete, prov_sc_delete_enrollment_group shall return 0 ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_GOLDEN_NO_ETAG)
{
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = enrollmentGroup_create(TEST_GROUPID, TEST_ATT_MECH_HANDLE);
    set_response_status(RESPONSE_OFF);
    umock_c_reset_all_calls();

    //arrange
    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG)).SetReturn(NULL); //no etag
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_delete_enrollment_group(sc, eg);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_035: [ A 'DELETE' REST call shall be issued to delete the device enrollment group that matches enrollment from the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_037: [ Upon a successful delete, prov_sc_delete_enrollment_group shall return 0 ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_GOLDEN_WITH_ETAG)
{
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = enrollmentGroup_create(TEST_GROUPID, TEST_ATT_MECH_HANDLE);
    set_response_status(RESPONSE_OFF);
    umock_c_reset_all_calls();

    //arrange
    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_delete_enrollment_group(sc, eg);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_036: [ If the 'DELETE' REST call fails, prov_sc_delete_enrollment_group shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_FAIL)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = enrollmentGroup_create(TEST_GROUPID, TEST_ATT_MECH_HANDLE);
    set_response_status(RESPONSE_OFF);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(enrollmentGroup_getEtag(IGNORED_PTR_ARG)); //can fail, but for sake of this example, cannot
    STRICT_EXPECTED_CALL(enrollmentGroup_getGroupId(IGNORED_PTR_ARG));
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 0, 3, 4, 6, 10, 13, 14, 18, 20, 21, 22, 23, 24, 25 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_delete_enrollment_group failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_delete_enrollment_group(sc, eg);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_053: [ If prov_client or group_id are NULL, prov_sc_delete_enrollment_group_by_param shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_by_param_NULL_PROV_SC)
{
    //arrange

    //act
    int res = prov_sc_delete_enrollment_group_by_param(NULL, TEST_GROUPID, TEST_ETAG);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_053: [ If prov_client or group_id are NULL, prov_sc_delete_enrollment_group_by_param shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_by_param_NULL_REG_ID)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    //act
    int res = prov_sc_delete_enrollment_group_by_param(sc, NULL, TEST_ETAG);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_054: [ If etag is given as NULL, it shall be ignored ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_055: [ A 'DELETE' REST call shall be issued to delete the enrollment group record with ID group_id, and optionally, etag from the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_057: [ Upon a successful delete, prov_sc_delete_enrollment_group_by_param shall return 0 ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_by_param_GOLDEN_NO_ETAG)
{
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    set_response_status(RESPONSE_OFF);
    umock_c_reset_all_calls();

    //arrange
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_DELETE); // <- this NO_ETAG flag is what proves Requirement 054 - if etag wasn't ignored, actual calls will not line up with expected
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_delete_enrollment_group_by_param(sc, TEST_REGID, NULL);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_055: [ A 'DELETE' REST call shall be issued to delete the enrollment group record with ID group_id, and optionally, etag from the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_057: [ Upon a successful delete, prov_sc_delete_enrollment_group_by_param shall return 0 ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_by_param_GOLDEN_WITH_ETAG)
{
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    set_response_status(RESPONSE_OFF);
    umock_c_reset_all_calls();

    //arrange
    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_delete_enrollment_group_by_param(sc, TEST_REGID, TEST_ETAG_STAR);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_056: [ If the 'DELETE' REST call fails, prov_sc_delete_enrollment_group_by_param shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_delete_enrollment_group_by_param_FAIL)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    set_response_status(RESPONSE_OFF);

    umock_c_reset_all_calls();

    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(ETAG, HTTP_CLIENT_REQUEST_DELETE);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_DELETE, NO_RESPONSE);
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 1, 2, 4, 8, 11, 12, 16, 18, 19, 20, 21, 22, 23 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_delete_enrollment_group_by_param failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_delete_enrollment_group_by_param(sc, TEST_GROUPID, TEST_ETAG_STAR);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    umock_c_negative_tests_deinit();
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_038: [ If prov_client, group_id or enrollment_ptr are NULL, prov_sc_get_enrollment_group shall return return a non-zero value ] */
TEST_FUNCTION(prov_sc_get_enrollment_group_ERROR_INPUT_NULL_PROV_SC)
{
    //arrange
    ENROLLMENT_GROUP_HANDLE eg;

    //act
    int res = prov_sc_get_enrollment_group(NULL, TEST_GROUPID, &eg);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_038: [ If prov_client, group_id or enrollment_ptr are NULL, prov_sc_get_enrollment_group shall return return a non-zero value ] */
TEST_FUNCTION(prov_sc_get_enrollment_group_ERROR_INPUT_NULL_REG_ID)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg;
    umock_c_reset_all_calls();

    //act
    int res = prov_sc_get_enrollment_group(sc, NULL, &eg);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_038: [ If prov_client, group_id or enrollment_ptr are NULL, prov_sc_get_enrollment_group shall return return a non-zero value ] */
TEST_FUNCTION(prov_sc_get_enrollment_group_ERROR_INPUT_NULL_ENROLLMENT)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    umock_c_reset_all_calls();

    //act
    int res = prov_sc_get_enrollment_group(sc, TEST_GROUPID, NULL);

    //assert
    ASSERT_ARE_NOT_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //cleanup
    prov_sc_destroy(sc);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_039: [ A 'GET' REST call shall be issued to retrieve the device enrollment group with ID group_id from the Provisioning Service ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_041: [ The data from the retrieved device enrollment group shall populate enrollment_ptr ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_043: [ Upon successful population of enrollment_ptr with the retrieved device enrollment group data, prov_sc_get_enrollment_group shall return 0 ] */
TEST_FUNCTION(prov_sc_get_enrollment_group_GOLDEN)
{
    //arrange
    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = NULL;
    umock_c_reset_all_calls();

    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_GET);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_GET, RESPONSE);
    STRICT_EXPECTED_CALL(enrollmentGroup_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    //act
    int res = prov_sc_get_enrollment_group(sc, TEST_GROUPID, &eg);

    //assert
    ASSERT_ARE_EQUAL(int, res, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(eg);

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
}

/* Tests_PROVISIONING_SERVICE_CLIENT_22_040: [ If the 'GET' REST call fails, prov_sc_get_enrollment_group shall fail and return a non-zero value ] */
/* Tests_PROVISIONING_SERVICE_CLIENT_22_042: [ If populating enrollment_ptr with retrieved data fails, prov_sc_get_enrollment_group shall fail and return a non-zero value ] */
TEST_FUNCTION(prov_sc_get_enrollment_group_FAIL)
{
    //arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    PROVISIONING_SERVICE_CLIENT_HANDLE sc = prov_sc_create_from_connection_string(TEST_CONNECTION_STRING);
    ENROLLMENT_GROUP_HANDLE eg = NULL;
    umock_c_reset_all_calls();

    expected_calls_construct_registration_path();
    expected_calls_construct_http_headers(NO_ETAG, HTTP_CLIENT_REQUEST_GET);
    STRICT_EXPECTED_CALL(STRING_c_str(IGNORED_PTR_ARG)); //does not fail
    expected_calls_rest_call(HTTP_CLIENT_REQUEST_GET, RESPONSE);
    STRICT_EXPECTED_CALL(enrollmentGroup_deserializeFromJson(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG)); //does not fail
    STRICT_EXPECTED_CALL(STRING_delete(IGNORED_PTR_ARG)); //does not fail

    umock_c_negative_tests_snapshot();

    size_t calls_cannot_fail[] = { 1, 2, 4, 9, 11, 12, 16, 18, 20, 21, 23, 24, 25 };
    size_t count = umock_c_negative_tests_call_count();
    size_t num_cannot_fail = sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0]);

    size_t test_num = 0;
    size_t test_max = count - num_cannot_fail;

    for (size_t index = 0; index < count; index++)
    {
        if (should_skip_index(index, calls_cannot_fail, sizeof(calls_cannot_fail) / sizeof(calls_cannot_fail[0])) != 0)
            continue;
        test_num++;

        char tmp_msg[128];
        sprintf(tmp_msg, "prov_sc_get_enrollment_group failure in test %zu/%zu", test_num, test_max);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        //act
        int res = prov_sc_get_enrollment_group(sc, TEST_REGID, &eg);

        //assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, res, 0, tmp_msg);

        g_uhttp_client_dowork_call_count = 0;
    }

    //cleanup
    prov_sc_destroy(sc);
    enrollmentGroup_destroy(eg);
    umock_c_negative_tests_deinit();
}

END_TEST_SUITE(provisioning_service_client_ut);