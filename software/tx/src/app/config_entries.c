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
 *      Author: matti
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
    .kr_tx = 0,
    .rc_input = 0,
    .nrf = 0,
    .usb = 0
};

static const config_entry_mapping_t _config_entries_mapping[] = {
    { .name = "kr-tx config entries",    .parse = NULL, .print = NULL, .payload = NULL},
    { .name = "kr-tx",    .parse = config_parse_u32, .print = config_print_du32, .payload = &_config_entries_config.kr_tx},
    { .name = "rc-input config entries", .parse = NULL, .print = NULL, .payload = NULL},
    { .name = "rc-input", .parse = config_parse_u32, .print = config_print_du32, .payload = &_config_entries_config.rc_input},
    { .name = "nrf config entries",      .parse = NULL, .print = NULL, .payload = NULL},
    { .name = "nrf",      .parse = config_parse_u32, .print = config_print_x08,  .payload = &_config_entries_config.nrf},
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

