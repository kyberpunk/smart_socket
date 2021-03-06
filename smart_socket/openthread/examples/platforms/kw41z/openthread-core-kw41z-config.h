/*
 *  Copyright (c) 2017, The OpenThread Authors.
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
 *   This file includes kw41z compile-time configuration constants
 *   for OpenThread.
 */

#ifndef OPENTHREAD_CORE_KW41Z_CONFIG_H_
#define OPENTHREAD_CORE_KW41Z_CONFIG_H_

/**
 * @def OPENTHREAD_CONFIG_LOG_OUTPUT
 *
 * The emsk platform provides an otPlatLog() function.
 */
#ifndef OPENTHREAD_CONFIG_LOG_OUTPUT /* allow command line override */
//#define OPENTHREAD_CONFIG_LOG_OUTPUT  OPENTHREAD_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_APP
#endif

/**
 * @def OPENTHREAD_CONFIG_PLATFORM_INFO
 *
 * The platform-specific string to insert into the OpenThread version string.
 *
 */
#define OPENTHREAD_CONFIG_PLATFORM_INFO                         "KW41Z"

/**
 * @def SETTINGS_CONFIG_BASE_ADDRESS
 *
 * The base address of settings.
 *
 */
#define SETTINGS_CONFIG_BASE_ADDRESS                            0x40000

/**
 * @def SETTINGS_CONFIG_PAGE_SIZE
 *
 * The page size of settings.
 *
 */
#define SETTINGS_CONFIG_PAGE_SIZE                               0x800

/**
 * @def SETTINGS_CONFIG_PAGE_NUM
 *
 * The page number of settings.
 *
 */
#define SETTINGS_CONFIG_PAGE_NUM                                2

/**
 * @def RADIO_CONFIG_SRC_MATCH_ENTRY_NUM
 *
 * The number of source address table entries.
 *
 */
#define RADIO_CONFIG_SRC_MATCH_ENTRY_NUM                        128

/**
 * @def OPENTHREAD_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
 *
 * Define to 1 if you want to enable software retransmission logic.
 *
 */
#define OPENTHREAD_CONFIG_ENABLE_SOFTWARE_RETRANSMIT            1

#define OPENTHREAD_CONFIG_LOG_LEVEL                             OT_LOG_LEVEL_WARN

#define OPENTHREAD_CONFIG_LOG_MLE                               1

#endif  // OPENTHREAD_CORE_KW41Z_CONFIG_H_
