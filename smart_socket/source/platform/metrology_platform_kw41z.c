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
 *  The file contains platform specific functionality implementation of STPM3x metrology driver for KW41Z platform.
 */

#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include "metrology_platform.h"
#include "system_MKW41Z4.h"
#include "metrology_kw41z.h"

void delay_microseconds(volatile uint32_t us);

void delay_microseconds(volatile uint32_t us) {
	// Busy delay for short intervals
	uint32_t one_tick = SystemCoreClock / 1000000U;
	us = (one_tick * us) / 10U;
	while (us > 0) {
		__asm("NOP");
		us--;
	}
}

void metrology_platform_init() {
	;
}

void metrology_platform_uart_config(struct uart_handle_data_tag *uart_handle, uint32_t in_baudrate) {
	;
}

void metrology_platform_spi_config(struct spi_handle_data_tag *spi_handle) {
	if (spi_handle != NULL) {
		if (spi_handle->spi->initialized == 1) {
			DSPI_Deinit(spi_handle->spi->spi_type);
			spi_handle->spi->initialized = 0;
		}

		// Config SPI - mode 3, cpol=1, cpha=1
		dspi_master_config_t masterConfig;
		memset(&masterConfig, 0, sizeof(masterConfig));
		masterConfig.whichCtar = kDSPI_Ctar0;
		masterConfig.ctarConfig.baudRate = SPI_PORT_SPEED;
		masterConfig.ctarConfig.bitsPerFrame = 8;
		masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveLow;
		masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseSecondEdge;
		masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
		masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 0;
		masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 0;
		masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 0;

		uint32_t srcClock_Hz = CLOCK_GetFreq(DSPI0_CLK_SRC);
		DSPI_MasterInit(spi_handle->spi->spi_type, &masterConfig, srcClock_Hz);
		metrology_platform_log(MET_LOG_INFO, "SPI initialized");
		spi_handle->spi->initialized = 1;
	}
}

void metrology_platform_wait_microseconds(uint32_t time) {
	delay_microseconds(time);
}

void metrology_platform_gpio_write(struct pin_handle_data_tag *pin_handle, MET_PORT_Pin_t pin, MET_GPIO_PIN_State_t state) {
	if (pin_handle != NULL) {
		uint32_t gpio_pin;
		GPIO_Type *type;
		switch (pin) {
		case MET_PORT_CS:
			gpio_pin = pin_handle->cs_pin;
			type = pin_handle->cs_pin_type;
			break;
		case MET_PORT_EN:
			gpio_pin = pin_handle->en_pin;
			type = pin_handle->en_pin_type;
			break;
		case MET_PORT_SYN:
			gpio_pin = pin_handle->syn_pin;
			type = pin_handle->syn_pin_type;
			break;
		default:
			return;
		}
		GPIO_WritePinOutput(type, gpio_pin, (uint8_t)state);
	}
}

uint32_t metrology_platform_uart_transmit(struct uart_handle_data_tag *uart_handle, uint8_t* data, uint32_t size, uint16_t timeout) {
	return 0;
}

uint32_t metrology_platform_uart_receive(struct uart_handle_data_tag *uart_handle, uint8_t* data, uint32_t size, uint16_t timeout) {
	return 0;
}

uint32_t metrology_platform_spi_transmit_receive(struct spi_handle_data_tag *spi_handle, uint8_t* data_out, uint8_t* data_in, uint32_t size, uint16_t timeout) {
	if (spi_handle != NULL) {
		dspi_transfer_t masterXfer;
		memset(&masterXfer, 0, sizeof(masterXfer));
		masterXfer.txData = data_out;
		masterXfer.rxData = data_in;
		masterXfer.dataSize = size;
		masterXfer.configFlags = kDSPI_MasterCtar0;
		status_t result = DSPI_MasterTransferBlocking(spi_handle->spi->spi_type, &masterXfer);
		if (result != kStatus_Success || masterXfer.dataSize == 0) {
			metrology_platform_log(MET_LOG_ERROR, "Read 0 bytes!");
		}
		return masterXfer.dataSize;
	}
	return 0;
}

void metrology_platform_log(MET_LogLevel_t level, const char * format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	printf("\r\n");
	va_end(args);
}

static void init_pin_handle_data(pin_handle_data *pin_handle) {
	metrology_platform_log(MET_LOG_INFO, "init_pin_handle_data");
	gpio_pin_config_t pin_config = { kGPIO_DigitalOutput, 0 };
	GPIO_PinInit(pin_handle->cs_pin_type, pin_handle->cs_pin, &pin_config);
	GPIO_PinInit(pin_handle->en_pin_type, pin_handle->en_pin, &pin_config);
	GPIO_PinInit(pin_handle->syn_pin_type, pin_handle->syn_pin, &pin_config);
}

void metrology_kw41z_init_spi(METRO_NB_Device_t device_id, spi_handle_data *spi_handle, pin_handle_data *pin_handle) {
	if (spi_handle == NULL || pin_handle == NULL || device_id >= NB_MAX_DEVICE) {
		return;
	}
	metrology_platform_log(MET_LOG_INFO, "metrology_kw41z_init_spi");
	init_pin_handle_data(pin_handle);
	Metro_Set_Spi_Handle(device_id, spi_handle);
	Metro_Set_Pin_Handle(device_id, pin_handle);
}
