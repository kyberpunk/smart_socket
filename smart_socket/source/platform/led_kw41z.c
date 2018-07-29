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
 *  The file contains LED indicator implementation for KW41Z platform.
 */

#include <stdbool.h>
#include "board.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_tpm.h"
#include "led_abstraction.h"
#include "smart_socket_platform.h"

#define BOARD_TPM_BASEADDR TPM2
#define BOARD_TPM_CHANNEL 1U
#define TPM_CHANNEL_INTERRUPT_ENABLE kTPM_Chnl1InterruptEnable
#define TPM_CHANNEL_FLAG kTPM_Chnl1Flag
#define TPM_INTERRUPT_NUMBER TPM2_IRQn
#define TPM_LED_HANDLER TPM2_IRQHandler
#define TPM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_McgInternalRefClk)
#define PIN19_IDX 19u
#define SOPT4_TPM2CH0SRC_TPM 0x00u

static bool initialized = false;

void led_init(void) {
	// Set TPM timer on pin PTA19
	CLOCK_EnableClock(kCLOCK_PortA);
	PORT_SetPinMux(PORTA, PIN19_IDX, kPORT_MuxAlt5);
	SIM->SOPT4 = ((SIM->SOPT4 & (~(SIM_SOPT4_TPM2CH0SRC_MASK)))
	| SIM_SOPT4_TPM2CH0SRC(SOPT4_TPM2CH0SRC_TPM)
	);
	// Set 32.768 kHz clock source
	CLOCK_SetTpmClock(3U);
	tpm_config_t tpm_info;
	TPM_GetDefaultConfig(&tpm_info);
	// 128x clock source prescale
	tpm_info.prescale = kTPM_Prescale_Divide_128;
	// Initialize TPM driver
	TPM_Init(BOARD_TPM_BASEADDR, &tpm_info);
	initialized = true;
}

void led_set_state(led_state_t state) {
	if (!initialized) {
		socket_platform_log(SOCKET_LOG_CRIT, "Led module not initialized.");
		return;
	}
	TPM_StopTimer(BOARD_TPM_BASEADDR);
	if (state == LED_OFF) {
		return;
	}
	tpm_chnl_pwm_signal_param_t tpm_param;
	tpm_pwm_level_select_t pwm_level = kTPM_LowTrue;

	tpm_param.chnlNumber = (tpm_chnl_t)BOARD_TPM_CHANNEL;
	tpm_param.level = pwm_level;
	uint32_t freq = 1U;
	// Set duty cycle based on requested state
	switch (state) {
	case LED_BLINK_SLOW:
		freq = 1U;
		tpm_param.dutyCyclePercent = 10U;
		break;
	case LED_BLINK_FAST:
		freq = 4U;
		tpm_param.dutyCyclePercent = 40U;
		break;
	case LED_ON:
		freq = 1U;
		tpm_param.dutyCyclePercent = 100U;
		break;
	default:
		return;
	}
	TPM_SetupPwm(BOARD_TPM_BASEADDR, &tpm_param, 1U, kTPM_CenterAlignedPwm, freq, TPM_SOURCE_CLOCK);
	TPM_StartTimer(BOARD_TPM_BASEADDR, kTPM_SystemClock);
}

void led_deinit(void) {
	// Deinitialize TPM driver
	TPM_Deinit(BOARD_TPM_BASEADDR);
	initialized = false;
}
