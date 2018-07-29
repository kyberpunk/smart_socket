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
 *  This file contains smart socket main functionality implementation.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "smart_socket_platform.h"
#include "smart_socket_config.h"
#include "thread_abstraction.h"
#include "coap_abstraction.h"
#include "rtc_abstraction.h"
#include "button_abstraction.h"
#include "led_abstraction.h"
#include "parson.h"
#include "metroTask.h"

#define PRINTF printf

/**
 * This enumeration represents smart socket states.
 */
typedef enum {
	/**
	 * Initial state before initialization.
	 */
	SOCKET_STARTED = 0,
	/**
	 * Device is initialized and waiting for the user action.
	 */
	SOCKET_INITIALIZED,
	/**
	 * Commissioning process in progress.
	 */
	SOCKET_THREAD_JOINING,
	/**
	 * Waiting for the network start and connection establishment.
	 */
	SOCKET_THREAD_STARTING,
	/**
	 * No CoAP server found.
	 */
	SOCKET_NO_SERVER,
	/**
	 * Waiting for CoAP server response.
	 */
	SOCKET_FIND_SERVER,
	/**
	 * Time is not synchronized.
	 */
	SOCKET_NO_TIME,
	/**
	 * Waiting for time resource response.
	 */
	SOCKET_TIMESYNC,
	/**
	 * Everything is OK. Device sends events periodically.
	 */
	SOCKET_RUNNING
} socket_state_t;

#if !BYPASS_JOINER
static char *pskd = PSKD;
#endif

static socket_state_t state = SOCKET_STARTED;
static bool time_set = false;
static uint32_t alarm_seconds = 0;
static int32_t energy = 0;
static uint32_t last_timestamp = 0;
static uint32_t attemp_counter = 0;
static uint32_t latency_timestamp = 0;
static bool resend = false;

/**
 * Change the smart socket device state and update LED signalization.
 *
 * @param[in]  socket_state   New state.
 */
static void set_socket_state(socket_state_t socket_state) {
	char *state_text = NULL;
	switch (socket_state) {
	case SOCKET_STARTED:
		state_text = "STARTED";
		break;
	case SOCKET_INITIALIZED:
		led_set_state(LED_ON);
		state_text = "INITIALIZED";
		break;
	case SOCKET_THREAD_JOINING:
		led_set_state(LED_BLINK_FAST);
		state_text = "THREAD_JOINING";
		break;
	case SOCKET_THREAD_STARTING:
		led_set_state(LED_OFF);
		state_text = "THREAD_STARTING";
		break;
	case SOCKET_NO_SERVER:
		led_set_state(LED_BLINK_SLOW);
		state_text = "NO_SERVER";
		break;
	case SOCKET_FIND_SERVER:
		state_text = "FIND_SERVER";
		break;
	case SOCKET_NO_TIME:
		state_text = "NO_TIME";
		break;
	case SOCKET_TIMESYNC:
		state_text = "TIMESYNC";
		break;
	case SOCKET_RUNNING:
		led_set_state(LED_OFF);
		state_text = "RUNNING";
		break;
	default:
		state_text = "UNKNOWN";
		break;
	}
	socket_platform_log(SOCKET_LOG_INFO, "Socket state changed: %s", state_text);
	state = socket_state;
}

/**
 * Configure STPM32 chip registers.
 */
static void metrology_config() {
	Metro_Set_Current_gain(CHANNEL_1, X16);
	// Configure calibrations (0x800 is mid value as correction factor 0.875)
	Metro_Set_V_Calibration(CHANNEL_1, 0x800);
	Metro_Set_C_Calibration(CHANNEL_1, 0x800);
	// Enable current channel filter
	Metro_Set_Current_HP_Filter(CHANNEL_1, DEVICE_ENABLE);
	// Enable voltage channel filter
	Metro_Set_Voltage_HP_Filter(CHANNEL_1, DEVICE_ENABLE);
	// Set calculation factors (example for SmartPlug)
	uint32_t power_factor = 9800246;
	uint32_t energy_factor = 11422;
	uint32_t voltage_factor = 116274;
	uint32_t current_factor = 8428;
	Metro_Set_Hardware_Factors(CHANNEL_1, power_factor, energy_factor, voltage_factor, current_factor);
}

static void start_measurements() {
	alarm_seconds = rtc_get_seconds() + MEASURE_PERIOD;
	rtc_set_alarm(alarm_seconds);
}

/**
 * Event response callback handler.
 */
void send_measurement_cb(coap_result_t result) {
	if (result == COAP_RESULT_OK) {
		attemp_counter = 0;
		socket_platform_log(SOCKET_LOG_INFO, "Measurements successfully sent. Latency: %d.", socket_get_millis() - latency_timestamp);
		last_timestamp = rtc_get_seconds();
		energy = metroData.energyActive;
		resend = false;
		return;
	}
	resend = true;
	if (attemp_counter < COAP_ATTEMPTS) {
		attemp_counter++;
		socket_platform_log(SOCKET_LOG_WARN, "Failed to send measurements. Attempt %d.", attemp_counter);
		return;
	}
	attemp_counter = 0;
	socket_platform_log(SOCKET_LOG_CRIT, "Failed to send measurements.");
	set_socket_state(SOCKET_NO_SERVER);
}

/**
 * Send measurements event.
 */
static void send_measurements() {
	// Latch STPM3x registers data
	METRO_Latch_Measures();
	// Read STPM3x registers data and update measurement calculations
	if (!resend) {
		if (METRO_Update_Measures() != METRO_OK) {
			socket_platform_log(SOCKET_LOG_CRIT, "Failed to get measurements");
			return;
		}
	}
	// Serialize measured data to JSON documetn
	double tmp = 0;
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	tmp = (double)(metroData.energyActive - energy);
	json_object_set_number(root_object, "activeEnergy", tmp == 0 ? 0 : tmp / 1000.0);
	json_object_set_number(root_object, "phase", (double)metroData.nbPhase);
	tmp = (double)metroData.rmscurrent;
	json_object_set_number(root_object, "rmsCurrent", tmp == 0 ? 0 : tmp / 1000.0);
	tmp = (double)metroData.rmsvoltage;
	json_object_set_number(root_object, "rmsVoltage", tmp == 0 ? 0 : tmp / 1000.0);
	json_object_set_number(root_object, "timestamp", (double)rtc_get_seconds());
	json_object_set_number(root_object, "period", (double)(rtc_get_seconds() - last_timestamp));
	serialized_string = json_serialize_to_string(root_value);
	if (serialized_string == NULL) {
		socket_platform_log(SOCKET_LOG_CRIT, "Failed to allocate memory for event data");
	}

	socket_platform_log(SOCKET_LOG_INFO, "Sending measurement data with payload: %s", serialized_string);
	latency_timestamp = socket_get_millis();
	// Send data
	coap_result_t result = coap_send_event(serialized_string, strlen(serialized_string), send_measurement_cb);
	if (result != COAP_RESULT_OK) {
		resend = true;
		if (attemp_counter < COAP_ATTEMPTS) {
			attemp_counter++;
			socket_platform_log(SOCKET_LOG_WARN, "Failed to send event coap request. Attempt %d.",	attemp_counter);
		} else {
			attemp_counter = 0;
			socket_platform_log(SOCKET_LOG_CRIT, "Failed to send event coap request.");
			set_socket_state(SOCKET_NO_SERVER);
		}
	}
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
}

/**
 * RTC alarm interrupt handler.
 */
static void alarm_handler() {
	if (state == SOCKET_RUNNING) {
		socket_platform_log(SOCKET_LOG_INFO, "Alarm fired");
		alarm_seconds += MEASURE_PERIOD;
		rtc_set_alarm(alarm_seconds);

		send_measurements();
	}
}

#if !BYPASS_JOINER
/**
 * Handle commissioning process result callback.
 */
static void joiner_handler_callback(thread_result_t result, void *context) {
	if (result == THREAD_RESULT_OK) {
		socket_platform_log(SOCKET_LOG_INFO, "Join success");
		thread_start();
		thread_set_autostart(true);
		set_socket_state(SOCKET_THREAD_STARTING);
	} else {
		socket_platform_log(SOCKET_LOG_INFO, "Join failed!");
		set_socket_state(SOCKET_INITIALIZED);
	}
}
#endif

/**
 * Short pushbutton press handler.
 */
static void short_press()
{
	socket_platform_log(SOCKET_LOG_INFO, "Short press fired");
#if BYPASS_JOINER
	// Start network with default settings
	socket_platform_log(SOCKET_LOG_INFO, "Bypass joiner. Start thread network.");
	if (state != SOCKET_INITIALIZED) {
		thread_set_autostart(false);
		coap_reset_server();
		coap_stop();
		thread_stop();
	}
	if (thread_set_default_network() != THREAD_RESULT_OK) {
		socket_platform_log(SOCKET_LOG_INFO, "Default network cannot be configured");
	}
	thread_start();
	thread_set_autostart(true);
	set_socket_state(SOCKET_THREAD_STARTING);
#else
	// Start joiner process
	socket_platform_log(SOCKET_LOG_INFO, "Start joiner");
	set_socket_state(SOCKET_THREAD_JOINING);
	if (state != SOCKET_INITIALIZED) {
		thread_set_autostart(false);
		coap_reset_server();
		coap_stop();
		thread_stop();
	}
	thread_joiner_start(pskd, joiner_handler_callback, NULL);
#endif
}

/**
 * Long pushbutton press handler.
 */
static void long_press()
{
	socket_platform_log(SOCKET_LOG_INFO, "Long press fired");
	// Factory reset
	socket_platform_log(SOCKET_LOG_INFO, "Factory reset");
	thread_set_autostart(false);
	thread_stop();
	thread_factory_reset();
	// Reset device
	socket_platform_reset();
}

static void register_handler_callback(coap_result_t result) {
	if (result != COAP_RESULT_OK) {
		socket_platform_log(SOCKET_LOG_INFO, "Device registration failed");
		set_socket_state(SOCKET_NO_SERVER);
		return;
	}
	socket_platform_log(SOCKET_LOG_INFO, "Device registration success");
	if (!time_set) {
		attemp_counter = 0;
		set_socket_state(SOCKET_NO_TIME);
	} else {
		// Start measurements when success
		set_socket_state(SOCKET_RUNNING);
		start_measurements();
	}
}

static void find_server_handler_callback(coap_result_t result) {
	if (result == COAP_RESULT_OK) {
		socket_platform_log(SOCKET_LOG_INFO, "Find server success");
		// Try to register device if any server was found
		coap_register_device(register_handler_callback);
	} else {
		socket_platform_log(SOCKET_LOG_WARN, "Find server error");
		set_socket_state(SOCKET_NO_SERVER);
	}
}

static void get_time_response_handler_callback(coap_result_t result, char *data, size_t size) {
	if (result == COAP_RESULT_OK) {
		attemp_counter = 0;
		// Deserialize time resource JSON
		JSON_Value *root_value = json_parse_string(data);
		JSON_Object *root_object = json_value_get_object(root_value);
		if (json_object_has_value(root_object, "milliseconds")) {
			double ms = json_object_get_number(root_object, "milliseconds");
			uint32_t timestamp = (uint32_t)(ms / 1000);
			socket_platform_log(SOCKET_LOG_INFO, "Seconds: %u", timestamp);
			// Set RTC time.
			rtc_set_seconds(timestamp);
			socket_platform_log(SOCKET_LOG_INFO, "Time sync success");
			set_socket_state(SOCKET_RUNNING);
			start_measurements();
		}
		json_value_free(root_value);
		return;
	}
	if (attemp_counter < COAP_ATTEMPTS) {
		attemp_counter++;
		socket_platform_log(SOCKET_LOG_WARN, "Time sync error. Attempt %d.", attemp_counter);
		return;
	}
	attemp_counter = 0;
	socket_platform_log(SOCKET_LOG_WARN, "Time sync error");
	set_socket_state(SOCKET_NO_SERVER);
}

static void print_thread_role(thread_role_t role) {
	char *role_text = NULL;
	switch (role) {
	case THREAD_ROLE_CHILD:
		role_text = "CHILD";
		break;
	case THREAD_ROLE_LEADER:
		role_text = "LEADER";
		break;
	case THREAD_ROLE_ROUTER:
		role_text = "ROUTER";
		break;
	case THREAD_ROLE_DETACHED:
		role_text = "DETACHED";
		break;
	case THREAD_ROLE_DISABLED:
		role_text = "DISABLED";
		break;
	default:
		role_text = "UNKNOWN";
		break;
	}
	socket_platform_log(SOCKET_LOG_INFO, "Thread role: %s", role_text);
}

/**
 * Long pushbutton press handler.
 */
static void smart_socket_worker(void) {
	thread_role_t role = THREAD_ROLE_DISABLED;
	// Handle device states
	switch (state) {
	case SOCKET_STARTED:
		return;
	case SOCKET_INITIALIZED:
		return;
	case SOCKET_THREAD_STARTING:
		role = thread_get_role();
		if (role == THREAD_ROLE_CHILD || role == THREAD_ROLE_LEADER || role == THREAD_ROLE_ROUTER) {
			coap_start();
			print_thread_role(role);
			set_socket_state(SOCKET_NO_SERVER);
		}
		return;
	case SOCKET_THREAD_JOINING:
		return;
	case SOCKET_NO_SERVER:
		set_socket_state(SOCKET_FIND_SERVER);
		coap_find_server(find_server_handler_callback);
		return;
	case SOCKET_FIND_SERVER:
		return;
	case SOCKET_NO_TIME:
		coap_get_time(get_time_response_handler_callback);
		return;
	case SOCKET_TIMESYNC:
		return;
	default:
		return;
	}
}

int main(int argc, char *argv[])
{
	// Initialize peripherals
	socket_platform_init();
    button_init();
    rtc_init();
    led_init();

    // Initialize metrology device
    metrology_platform_log(MET_LOG_INFO, "Test");
	if (METRO_Init() != METRO_OK) {
		socket_platform_log(SOCKET_LOG_CRIT, "Failed to init metrology device");
		return 1;
	}
	// Configure metrology device
	metrology_config();
	set_socket_state(SOCKET_INITIALIZED);

	// Set pushbutton handlers
    button_short_press_handler(short_press);
    button_long_press_handler(long_press);

    last_timestamp = rtc_get_seconds();
    // Initialize Thread network
    thread_init();
    // Enable IPv6 interface
	if (thread_ip6_enable(true) != THREAD_RESULT_OK) {
		socket_platform_log(SOCKET_LOG_INFO, "IPv6 enable failed");
	}
    socket_platform_log(SOCKET_LOG_INFO, "Smart socket initialized");

    // Start network immediately if autostart enabled
    if (thread_get_autostart()) {
    	set_socket_state(SOCKET_THREAD_STARTING);
    }

    // Set RTC handlers
    rtc_seconds_tick_handler(smart_socket_worker);
    rtc_alarm_handler(alarm_handler);

    while (1)
    {
    	// Process Thread worker
        thread_process();
    }
    return 0;
}
