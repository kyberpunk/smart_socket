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
 *  The file contains real-time clock API declaration.
 */

#ifndef RTC_ABSTRACTION_H_
#define RTC_ABSTRACTION_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief
 *   Module provides functions for managing and accessing RTC module.
 *
 * @{
 *
 */

/**
 * This function pointer is used as timer interrupt handler.
 */
typedef void (*tick_cb_t)(void);

/**
 * Function initializes real-time clock peripheral.
 */
void rtc_init(void);

/**
 * Set RTC seconds interrupt handler.
 *
 * @param[in]  callback   A function pointer to RTC interrupt handler.
 */
void rtc_seconds_tick_handler(tick_cb_t callback);

/**
 * Set RTC seconds interrupt handler.
 *
 * @param[in]  callback   A function pointer to RTC interrupt handler.
 */
void rtc_alarm_handler(tick_cb_t callback);

/**
 * Set RTC alarm seconds.
 *
 * @param[in]  seconds   Number of seconds.
 */
bool rtc_set_alarm(uint32_t seconds);

/**
 * Set RTC time in seconds.
 *
 * @param[in]  seconds   Number of seconds from epoch start.
 */
void rtc_set_seconds(uint32_t seconds);

/**
 * Get RTC time in seconds.
 *
 * @return   Number of seconds from epoch start.
 */
uint32_t rtc_get_seconds(void);

/**
 * Function deinitializes real-time clock peripheral.
 */
void rtc_deinit(void);

/**
 * @}
 *
 */

#endif /* RTC_ABSTRACTION_H_ */
