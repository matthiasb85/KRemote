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
 * kr_rx.c
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
#include "api/app/kr_rx.h"

/*
 * Include dependencies
 */
#include "api/app/kr.h"
#include "api/app/cmd_shell.h"
#include "api/app/config.h"
#include "api/app/config_entries.h"
#include "api/hal/flash_storage.h"
#include "api/hal/nrf.h"
#include "api/hal/usb.h"
#include "api/hal/rc_output.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*
 * Static asserts
 */

/*
 * Forward declarations of static functions
 */
static void _kr_rx_init_hal(void);
static void _kr_rx_init_module(void);
static void _kr_rx_set_channels(void);
static void _kr_rx_set_failsafe(void);
static void _kr_rx_connection_state_change_cb(nrf_connection_state_t state);

/*
 * Static variables
 */
static THD_WORKING_AREA(_kr_rx_main_stack, KR_RX_MAIN_THREAD_STACK);
static kr_transmit_frame_t _kr_rx_frame;
static kr_ch_t _kr_rx_channels[KR_CHANNEL_NUMBER];
static kr_rx_config_t * _kr_rx_config = NULL;

/*
 * Global variables
 */

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_kr_rx_main_thread, arg)
{
  (void)arg;
  uint8_t i = 0;
  event_listener_t  event_listener;

  chRegSetThreadName("kr_rx_main_th");

  chEvtRegister(&nrf_rx_event, &event_listener, NRF_EVENT_RX_EVENT);

  /*
   * Receive messages from rc_input module
   */
  while (true)
  {
    chEvtWaitAny(EVENT_MASK(NRF_EVENT_RX_EVENT));

    nrf_receive_payload(&_kr_rx_frame, sizeof(_kr_rx_frame));

    for(i=0; i < KR_CHANNEL_NUMBER; i++)
    {
        _kr_rx_channels[i] = _kr_rx_frame.channels[_kr_rx_config->channel_mapping[i]];
    }
    _kr_rx_set_channels();
  }
}


/*
 * Static helper functions
 */
static void _kr_rx_init_hal(void)
{
  // nothing to do
}

static void _kr_rx_init_module(void)
{
  _kr_rx_set_failsafe();
  nrf_register_connection_state_change_callback(_kr_rx_connection_state_change_cb);

  /*
   * Create application main thread
   */
  chThdCreateStatic(_kr_rx_main_stack, sizeof(_kr_rx_main_stack), KR_RX_MAIN_THREAD_PRIO,
                    _kr_rx_main_thread, NULL);
}

static void _kr_rx_set_channels(void)
{
  uint8_t i = 0;
  for(i=0; i < KR_CHANNEL_NUMBER; i++)
  {
      rc_output_set(i, _kr_rx_channels[i]);
  }
}

static void _kr_rx_set_failsafe(void)
{
  memcpy(_kr_rx_channels, _kr_rx_config->channel_failsafe, KR_CHANNEL_NUMBER);
  _kr_rx_set_channels();
}

/*
 * Callback functions
 */
static void _kr_rx_connection_state_change_cb(nrf_connection_state_t state)
{
  if(state == NRF_CONNECTION_LOST)
  {
    _kr_rx_set_failsafe();
  }
}

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void kr_rx_loop_channels_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 1)
  {
    chprintf(chp, "Usage:  kr-rx-loop-channels address\r\n");
    return;
  }
  systime_t time = 0;
  uint8_t i = 0;

  chprintf(chp, "  State");
  for(i=0; i < KR_CHANNEL_NUMBER; i++)
  {
    chprintf(chp, "  CH%02d", i);
  }
  chprintf(chp, "\r\n");

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
  {
      time = chVTGetSystemTimeX();

      chprintf(chp, "   %5s", (nrf_get_connection_state() ? "ESTAB" : "LOST"));

      for(i=0; i < KR_CHANNEL_NUMBER; i++)
      {
        chprintf(chp, "%7d",_kr_rx_channels[i]);
      }
      chprintf(chp, "\r");
      chThdSleepUntilWindowed(time, time + TIME_MS2I(_kr_rx_config->loop_cmd_period_ms));
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}

#endif

/*
 * API functions
 */
void kr_rx_init(void)
{
  /*
   * Toplevel hal init
   */
  _kr_rx_init_hal();

  /*
   * Initialize config
   */
  flash_storage_init();
  config_init();
  _kr_rx_config = (kr_rx_config_t *)config_get_module_config(CONFIG_ENTRY_KR_RX);

  /*
   * Project specific hal initialization
   */
  nrf_init();
  rc_output_init();

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
  _kr_rx_init_module();

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

void kr_rx_parse_array(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry)
{

  uint8_t error = 0;
  uint8_t bs = 0;

  if(argc < 3)
  {
    error = 1;
  }
  uint32_t idx = (uint16_t)strtol(argv[1], NULL, 0);

  if(!error)
  {
    bs = (strcmp(entry->name, "kr-rx-ch-map") == 0) ? 1 : 2;
    uint32_t value = (uint32_t)strtol(argv[2], NULL, 0);

    if(idx < KR_CHANNEL_NUMBER)
    {
      if(bs == 1)
      {
        uint8_t *ch_map = (uint8_t *)entry->payload;
        ch_map[idx] = (uint8_t)value;
      }
      else
      {
        uint16_t *failsafe_values = (uint16_t *)entry->payload;
        failsafe_values[idx] = value;
      }
    }
    else if(idx == KR_CHANNEL_NUMBER)
    {
      uint8_t i = 0;
      for(i=0; i<KR_CHANNEL_NUMBER; i++)
      {
        if(bs == 1)
        {
          uint8_t *ch_map = (uint8_t *)entry->payload;
          ch_map[i] = (uint8_t)value;
        }
        else
        {
          uint16_t *failsafe_values = (uint16_t *)entry->payload;
          failsafe_values[i] = value;
        }
      }
    }
    else
    {
      error = 1;
    }
  }
  if(error)
  {
    chprintf(chp, "Usage:  config-set variable idx value\r\n");
    chprintf(chp, "        idx=%d...%d to set single entries\r\n", 0, KR_CHANNEL_NUMBER - 1);
    chprintf(chp, "        idx=%d to set all entries\r\n", KR_CHANNEL_NUMBER);
    chprintf(chp, "        value=[0...%d]\r\n", KR_CHANNEL_MAX_VALUE);
  }
}

void kr_rx_print_array(BaseSequentialStream * chp, config_entry_mapping_t * entry)
{
  uint8_t i = 0;
  uint8_t bs = (strcmp(entry->name, "kr-rx-ch-map") == 0) ? 1 : 2;
  chprintf(chp, "  %-20s",entry->name);
  for(i=0; i<KR_CHANNEL_NUMBER; i++)
  {
      chprintf(chp, " %4d", (bs == 1) ?
          ((uint8_t *)entry->payload)[i] :
          ((uint16_t *)entry->payload)[i]);
  }
}
