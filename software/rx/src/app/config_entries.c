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
    .kr_rx = {
      .channel_mapping = {
          0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
      },
      .channel_failsafe = {
          KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2,
          KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2,
          KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2,
          KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2, KR_CHANNEL_MAX_VALUE/2
      },
      .loop_cmd_period_ms = KR_RX_LOOP_CMD_P_MS
    },
    .rc_output = {
        .digital_output_mode = {
            PAL_MODE_OUTPUT_OPENDRAIN, PAL_MODE_OUTPUT_OPENDRAIN , PAL_MODE_OUTPUT_OPENDRAIN ,PAL_MODE_OUTPUT_OPENDRAIN,
            PAL_MODE_OUTPUT_OPENDRAIN, PAL_MODE_OUTPUT_OPENDRAIN , PAL_MODE_OUTPUT_OPENDRAIN ,PAL_MODE_OUTPUT_OPENDRAIN,
            PAL_MODE_OUTPUT_OPENDRAIN, PAL_MODE_OUTPUT_OPENDRAIN , PAL_MODE_OUTPUT_OPENDRAIN ,PAL_MODE_OUTPUT_OPENDRAIN
        },
        .loop_cmd_period_ms = RC_OUTPUT_LOOP_CMD_P_MS
    },
    .nrf = {
        .mode = NRF_MODE_RX,
        NRF_CONFIG_COMMON_DEFAULTS,
    },
    .usb = 0
};

static const config_entry_mapping_t _config_entries_mapping[] = {
    { CONFIG_SECTION_DIVIDER("kr-tx config entries") },
    { .name = "kr-rx-ch-map",   .parse = kr_rx_parse_array,           .print = kr_rx_print_array, .payload = &_config_entries_config.kr_rx, .help ="kr-rx value"},
    { .name = "kr-rx-failsafe", .parse = kr_rx_parse_array,           .print = kr_rx_print_array, .payload = &_config_entries_config.kr_rx, .help ="kr-rx value"},
    { .name = "kr-rx-loop-ms",  .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.kr_rx, .help ="kr-rx value"},
    { CONFIG_SECTION_DIVIDER("rc-output config entries") },
    { .name = "ro-dig-om",      .parse = rc_output_parse_dig_sm,      .print = rc_output_print_dig_sm,          .payload = &_config_entries_config.rc_output.digital_output_mode,   .help = "Digital output mode"},
    { .name = "ro-loop-ms",     .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.rc_output.loop_cmd_period_ms,    .help = "Period in ms for loop cmd"},
    { CONFIG_SECTION_DIVIDER("nrf config entries") },
    NRF_CONFIG_MAP_ENTRIES,
    { .name = "\0",       .parse = NULL,             .print = NULL,              .payload = NULL}
};

static void * _config_entries_module_list[] = {
    &_config_entries_config.kr_rx,
    &_config_entries_config.rc_output,
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

