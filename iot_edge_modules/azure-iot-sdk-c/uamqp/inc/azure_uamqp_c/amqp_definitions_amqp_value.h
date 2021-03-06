

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AMQP_DEFINITIONS_AMQP_VALUE_H
#define AMQP_DEFINITIONS_AMQP_VALUE_H


#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "azure_uamqp_c/amqpvalue.h"
#include "azure_c_shared_utility/umock_c_prod.h"


    typedef AMQP_VALUE amqp_value;

    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_amqp_value, AMQP_VALUE, value);
    #define amqp_value_clone amqpvalue_clone
    #define amqp_value_destroy amqpvalue_destroy

    MOCKABLE_FUNCTION(, bool, is_amqp_value_type_by_descriptor, AMQP_VALUE, descriptor);

    #define amqpvalue_get_amqp_value amqpvalue_get_*



#ifdef __cplusplus
}
#endif

#endif /* AMQP_DEFINITIONS_AMQP_VALUE_H */
