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
 * kr_tx.c
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
#include "chprintf.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/app/kr_tx.h"

/*
 * Include dependencies
 */
#include "api/app/cmd_shell.h"
#include "api/app/config.h"
#include "api/hal/flash_storage.h"
#include "api/hal/nrf.h"
#include "api/hal/usb.h"
#include "api/hal/rc_input.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*
 * Static asserts
 */
//static_assert(ANYKEY_NUMBER_OF_KEYS == (KEYPAD_SW_COUNT),
//              "Number of keys does not match number used keypad switches");

/*
 * Forward declarations of static functions
 */
static void _kr_tx_init_hal(void);
static void _kr_tx_init_module(void);

/*
 * Static variables
 */
static THD_WORKING_AREA(_kr_tx_main_stack, KR_TX_MAIN_THREAD_STACK);

/*
 * Global variables
 */

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_kr_tx_main_thread, arg)
{
  (void)arg;
  systime_t time = 0;

  chRegSetThreadName("kr_tx_main_th");

  /*
   * Receive messages from rc_input module
   */
  while (true)
  {
    time = chVTGetSystemTimeX();
    // do something
    chThdSleepUntilWindowed(time, time + TIME_MS2I(1000));
  }
}


/*
 * Static helper functions
 */
static void _kr_tx_init_hal(void)
{
  // nothing to do
}

static void _kr_tx_init_module(void)
{
  /*
   * Create application main thread
   */
  chThdCreateStatic(_kr_tx_main_stack, sizeof(_kr_tx_main_stack), KR_TX_MAIN_THREAD_PRIO,
                    _kr_tx_main_thread, NULL);
}

/*
 * Callback functions
 */

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void kr_tx_foo_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc != 1)
  {
    chprintf(chp, "Usage:  kr_tx_foo address\r\n");
    return;
  }
  uint32_t *address = (uint32_t *)strtol(argv[0], NULL, 16);
  chprintf(chp, "STFU 0x%08p\r\n\r\n", address);
}

#endif

/*
 * API functions
 */
void kr_tx_init(void)
{
  /*
   * Toplevel hal init
   */
  _kr_tx_init_hal();

  /*
   * Initialize config
   */
  flash_storage_init();
  config_init();

  /*
   * Project specific hal initialization
   */
  //nrf_init();
  //rc_input_init();

#if defined(USE_CMD_SHELL)
  /*
   * Initialize command shell
   */
  usb_init();
  cmd_shell_init();
#endif

  /*
   * Initialize toplevel application
   */
  _kr_tx_init_module();

#if defined(USE_CMD_SHELL)
  /*
   * Start shell handling,
   * should never return
   */
  cmd_shell_loop();
#else
  /*
   * Nothing to do, kill initial thread
   */
  chThdExit(0);
#endif
  /*
   * If we reach this point,
   * something went horribly wrong
   */
}
