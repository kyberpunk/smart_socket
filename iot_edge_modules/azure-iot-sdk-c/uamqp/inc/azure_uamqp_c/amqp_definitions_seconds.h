

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AMQP_DEFINITIONS_SECONDS_H
#define AMQP_DEFINITIONS_SECONDS_H


#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "azure_uamqp_c/amqpvalue.h"
#include "azure_c_shared_utility/umock_c_prod.h"


    typedef uint32_t seconds;

    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_seconds, seconds, value);


    #define amqpvalue_get_seconds amqpvalue_get_uint



#ifdef __cplusplus
}
#endif

#endif /* AMQP_DEFINITIONS_SECONDS_H */
