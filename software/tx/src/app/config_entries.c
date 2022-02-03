/*
 * This file is part of The KRemote Project
 *
 * Copyright (c) 2021 Matthias Beckert
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * config_entries.c
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

/*
 * Include ChibiOS & HAL
 */
// clang-format off
#include "ch.h"
#include "hal.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/app/config_entries.h"

/*
 * Include dependencies
 */
#include "api/app/config.h"

/*
 * Forward declarations of static functions
 */

/*
 * Static variables
 */
static config_entries_t _config_entries_config = {
    .header = {
      .crc = 0,
      .version = CONFIG_ENTRIES_CONFIG_VERSION,
      .config_size = sizeof(config_entries_t),
    },
    .kr_tx = {
        .trim = {
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
        },
        .input_min = {
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
        },
        .input_max = {
            RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE,
            RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE,
            RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE,
            RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE, RC_INPUT_AN_MAX_VALUE
        },
        .map_type = {
            KR_TX_MAP_ANALOG, KR_TX_MAP_ANALOG, KR_TX_MAP_DISABLED, KR_TX_MAP_DISABLED,
            KR_TX_MAP_DIGITAL, KR_TX_MAP_DIGITAL, KR_TX_MAP_DISABLED, KR_TX_MAP_DISABLED,
            KR_TX_MAP_DISABLED, KR_TX_MAP_DISABLED, KR_TX_MAP_DISABLED, KR_TX_MAP_DISABLED,
            KR_TX_MAP_DISABLED, KR_TX_MAP_DISABLED, KR_TX_MAP_DISABLED, KR_TX_MAP_DISABLED
        },
        .map_channel = {
            0, 1, 0, 0, 1, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        .map_inverted = {
            KR_TX_INV_FALSE, KR_TX_INV_FALSE, KR_TX_INV_FALSE, KR_TX_INV_FALSE,
            KR_TX_INV_FALSE, KR_TX_INV_FALSE, KR_TX_INV_FALSE, KR_TX_INV_FALSE,
            KR_TX_INV_FALSE, KR_TX_INV_FALSE, KR_TX_INV_FALSE, KR_TX_INV_FALSE,
            KR_TX_INV_FALSE, KR_TX_INV_FALSE, KR_TX_INV_FALSE, KR_TX_INV_FALSE
        },
        .map_output_min = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        .map_output_max = {
            KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE,
            KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE,
            KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE,
            KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE, KR_CHANNEL_MAX_VALUE
        },
        .map_condition = {
            KR_TX_COND_NONE, KR_TX_COND_NONE, KR_TX_COND_NONE, KR_TX_COND_NONE,
            KR_TX_COND_LE, KR_TX_COND_GE, KR_TX_COND_NONE, KR_TX_COND_NONE,
            KR_TX_COND_NONE, KR_TX_COND_NONE, KR_TX_COND_NONE, KR_TX_COND_NONE,
            KR_TX_COND_NONE, KR_TX_COND_NONE, KR_TX_COND_NONE, KR_TX_COND_NONE
        },
        .map_threshold = {
            0, 0, 0, 0, 1900, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        .main_thread_prio = KR_TX_MAIN_THREAD_PRIO,
        .main_thread_period_ms = KR_TX_MAIN_THREAD_PERIOD,
    },
    .rc_input = {
        .digital_switch_mode = {
            PAL_MODE_INPUT_PULLDOWN, PAL_MODE_INPUT_PULLDOWN , PAL_MODE_INPUT_PULLDOWN ,PAL_MODE_INPUT_PULLDOWN,
            PAL_MODE_INPUT_PULLDOWN, PAL_MODE_INPUT_PULLDOWN , PAL_MODE_INPUT_PULLDOWN ,PAL_MODE_INPUT_PULLDOWN
        },
        .analog_conversion_period_ms = 10,
        .analog_conversion_emph_old = 5,
        .analog_conversion_emph_new = 1,
        .poll_thread_prio = RC_INPUT_POLL_THREAD_STACK,
        .poll_thread_period_ms = RC_INPUT_POLL_MAIN_THREAD_P_MS,
        .loop_cmd_period_ms = RC_INPUT_LOOP_CMD_P_MS
    },
    .nrf = {
        .mode = NRF_MODE_TX,
        NRF_CONFIG_COMMON_DEFAULTS,
    },
    .usb = 0
};

static const config_entry_mapping_t _config_entries_mapping[] = {
    { CONFIG_SECTION_DIVIDER("kr-tx config entries") },
    { .name = "kr-tx-main-p",    .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.kr_tx.main_thread_prio,            .help = "Main thread priority"},
    { .name = "kr-tx-main-ms",   .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.kr_tx.main_thread_period_ms,       .help = "Period in ms for main thread"},
    { .name = "kr-tx-trim",      .parse = kr_tx_parse_config,     .print = kr_tx_print_config,     .payload = &_config_entries_config.kr_tx.trim,           .help ="kr-tx trim values"},
    { .name = "kr-tx-input-min", .parse = kr_tx_parse_config,     .print = kr_tx_print_config,     .payload = &_config_entries_config.kr_tx.input_min,      .help ="kr-tx input min values"},
    { .name = "kr-tx-input-min", .parse = kr_tx_parse_config,     .print = kr_tx_print_config,     .payload = &_config_entries_config.kr_tx.input_max,      .help ="kr-tx input max values"},
    { .name = "kr-tx-map-type",  .parse = kr_tx_parse_config_map, .print = kr_tx_print_config_map, .payload = &_config_entries_config.kr_tx.map_type,       .help ="kr-tx channel map type"},
    { .name = "kr-tx-map-ch",    .parse = kr_tx_parse_config,     .print = kr_tx_print_config,     .payload = &_config_entries_config.kr_tx.map_channel,    .help ="kr-tx channel map"},
    { .name = "kr-tx-map-inv",   .parse = kr_tx_parse_config_map, .print = kr_tx_print_config_map, .payload = &_config_entries_config.kr_tx.map_inverted,   .help ="kr-tx channel invert"},
    { .name = "kr-tx-map-omin",  .parse = kr_tx_parse_config,     .print = kr_tx_print_config,     .payload = &_config_entries_config.kr_tx.map_output_min, .help ="kr-tx channel min value"},
    { .name = "kr-tx-map-omax",  .parse = kr_tx_parse_config,     .print = kr_tx_print_config,     .payload = &_config_entries_config.kr_tx.map_output_max, .help ="kr-tx channel max value"},
    { .name = "kr-tx-map-cond",  .parse = kr_tx_parse_config_map, .print = kr_tx_print_config_map, .payload = &_config_entries_config.kr_tx.map_condition,  .help ="kr-tx channel digital switch condition"},
    { .name = "kr-tx-map-thresh",.parse = kr_tx_parse_config,     .print = kr_tx_print_config,     .payload = &_config_entries_config.kr_tx.map_threshold,  .help ="kr-tx channel digital switch threshold"},
    { CONFIG_SECTION_DIVIDER("rc-input config entries") },
    { .name = "ri-dig-sm",      .parse = rc_input_parse_dig_sm,       .print = rc_input_print_dig_sm,           .payload = &_config_entries_config.rc_input.digital_switch_mode,         .help = "Digital switch mode"},
    { .name = "ri-poll-p",      .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.rc_input.poll_thread_prio,            .help = "Poll thread priority"},
    { .name = "ri-poll-ms",     .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.rc_input.poll_thread_period_ms,       .help = "Period in ms for poll thread"},
    { .name = "ri-vtp-ms",      .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.rc_input.analog_conversion_period_ms, .help = "Period in ms for virtual timer"},
    { .name = "ri-an-emph-old", .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.rc_input.analog_conversion_emph_old,  .help = "Emphasis for old analog value"},
    { .name = "ri-an-emph-new", .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.rc_input.analog_conversion_emph_new,  .help = "Emphasis for new analog value"},
    { .name = "ri-loop-ms",     .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.rc_input.loop_cmd_period_ms,    .help = "Period in ms for loop cmd"},
    { CONFIG_SECTION_DIVIDER("nrf config entries") },
    NRF_CONFIG_MAP_ENTRIES,
    { .name = "\0",       .parse = NULL,             .print = NULL,              .payload = NULL}
};

static void * _config_entries_module_list[] = {
    &_config_entries_config.kr_tx,
    &_config_entries_config.rc_input,
    &_config_entries_config.nrf,
    &_config_entries_config.usb
};

static const config_control_t _config_entries_control = {
    .config = (uint8_t *)&_config_entries_config,
    .entry_mapping = (config_entry_mapping_t *)&_config_entries_mapping,
    .module_list = _config_entries_module_list
};

/*
 * Global variables
 */

/*
 * Tasks
 */

/*
 * Static helper functions
 */

/*
 * Callback functions
 */

/*
 * Shell functions
 */

/*
 * API functions
 */
const config_control_t * config_entries_get_control_struct(void)
{
  return &_config_entries_control;
}

