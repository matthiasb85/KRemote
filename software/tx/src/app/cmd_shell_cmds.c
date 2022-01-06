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
 * cmd_shell_cmds.c
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
#include "shell.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/app/cmd_shell_cmds.h"

/*
 * Include dependencies
 */
#include "cmd/app/kr_tx_cmd.h"
#include "cmd/app/config_cmd.h"
#include "cmd/hal/rc_input_cmd.h"
#include "cmd/hal/flash_storage_cmd.h"
#include "cmd/hal/nrf_cmd.h"
#include "cmd/hal/usb_cmd.h"

/*
 * Forward declarations of static functions
 */

/*
 * Static variables
 */

/*
 * Global variables
 */
#if defined(USE_CMD_SHELL)
// clang-format off
 const ShellCommand cmd_shell_cmds_list[] = {
  KR_TX_CMD_LIST
  CONFIG_CMD_LIST
  RC_INPUT_CMD_LIST
  NRF_CMD_LIST
  FLASH_STORAGE_CMD_LIST
  USB_CMD_LIST
  {NULL, NULL}
};
 // clang-format on
#endif
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
