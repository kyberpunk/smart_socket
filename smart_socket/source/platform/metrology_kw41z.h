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
 *  The file contains the declaration of KW41Z metrology driver platform configruation functions and structures.
 */

#ifndef INCLUDE_METROLOGY_KW41Z_H_
#define INCLUDE_METROLOGY_KW41Z_H_

#include "fsl_gpio.h"
#include "fsl_dspi.h"
#include "metrology.h"

#define UART_TX_TIMEOUT 100
#define SPI_PORT_SPEED 1000000

/**
 * This structure contains SPI interface configuration for STPM3x devices.
 */
typedef struct {
	SPI_Type *spi_type;
	clock_name_t clock;
	uint8_t initialized;
} SPI_Instance_t;

/**
 * This structure contains SPI interface instance pointer used for specific STPM3x device.
 */
typedef struct spi_handle_data_tag {
	SPI_Instance_t *spi;
} spi_handle_data;

/**
 * This structure contains control pins configuration for specific STPM3x device.
 */
typedef struct pin_handle_data_tag {
	uint32_t cs_pin;
	GPIO_Type *cs_pin_type;
	uint32_t en_pin;
	GPIO_Type *en_pin_type;
	uint32_t syn_pin;
	GPIO_Type *syn_pin_type;
} pin_handle_data;

/**
 * Configure SPI interface for the specific STPM3x device.
 *
 * @param[in]  result   Request result.
 */
void metrology_kw41z_init_spi(METRO_NB_Device_t device_id, spi_handle_data *spi_handle, pin_handle_data *pin_handle);

#endif /* INCLUDE_METROLOGY_KW41Z_H_ */
