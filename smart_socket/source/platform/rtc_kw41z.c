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
 *  The file real-time clock functionality implementation for KW41Z platform.
 */

#include "rtc_abstraction.h"
#include "fsl_rtc.h"
#include "board.h"

static RTC_Type *rtc_base = NULL;
static tick_cb_t alarm_callback = NULL;
static tick_cb_t seconds_callback = NULL;

/**
 * RTC alarm interrupt handler.
 */
void RTC_IRQHandler(void) {
    if (RTC_GetStatusFlags(rtc_base) & kRTC_AlarmFlag) {
        RTC_ClearStatusFlags(rtc_base, kRTC_AlarmInterruptEnable);
        if (alarm_callback) {
        	alarm_callback();
        }
    }
}

/**
 * RTC seconds interrupt handler.
 */
void RTC_Seconds_IRQHandler(void) {
	if (seconds_callback) {
		seconds_callback();
	}
}

void rtc_init(void) {
	rtc_base = RTC;
	rtc_config_t rtcConfig;
	RTC_GetDefaultConfig(&rtcConfig);
	RTC_Init(rtc_base, &rtcConfig);
	// Set RTC clock source
	rtc_base->CR |= RTC_CR_OSCE_MASK;
	// Enable interrupts
	RTC_EnableInterrupts(rtc_base, kRTC_AlarmInterruptEnable | kRTC_SecondsInterruptEnable);
	EnableIRQ(RTC_IRQn);
	EnableIRQ(RTC_Seconds_IRQn);
	// Set stime to 0
	rtc_set_seconds(0);
}

void rtc_seconds_tick_handler(tick_cb_t callback) {
	seconds_callback = callback;
}

void rtc_alarm_handler(tick_cb_t callback) {
	alarm_callback = callback;
}

bool rtc_set_alarm(uint32_t seconds) {
	uint32_t curr_seconds = rtc_base->TSR;

	if (seconds <= curr_seconds) {
		return false;
	}

	rtc_base->TAR = seconds;
	return true;
}

void rtc_set_seconds(uint32_t seconds) {
	if (rtc_base != NULL) {
		RTC_StopTimer(rtc_base);
		rtc_base->TSR = seconds;
		RTC_StartTimer(rtc_base);
	}
}

uint32_t rtc_get_seconds(void) {
	if (rtc_base != NULL) {
		return rtc_base->TSR;
	}
	return 0;
}

void rtc_deinit() {
	if (rtc_base != NULL) {
		RTC_Deinit(rtc_base);
		rtc_base = NULL;
	}
}
