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
#include "api/app/kr.h"
#include "api/app/cmd_shell.h"
#include "api/app/config.h"
#include "api/app/config_entries.h"
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

/*
 * Forward declarations of static functions
 */
static void _kr_tx_init_hal(void);
static void _kr_tx_init_module(void);
static void _kr_tx_trim_and_limit(rc_input_ch_states_t * states, uint8_t channels);
static void _kr_tx_map_channels(kr_transmit_frame_t * frame, rc_input_ch_states_t * states, uint8_t channels);

/*
 * Static variables
 */
static THD_WORKING_AREA(_kr_tx_main_stack, KR_TX_MAIN_THREAD_STACK);
static kr_transmit_frame_t _kr_tx_frame;
static kr_tx_config_t * _kr_tx_config = NULL;
static const config_mode_map_t kr_tx_mapping_type[] = {
  {"DIS", KR_TX_MAP_DISABLED},
  {"ANA", KR_TX_MAP_ANALOG},
  {"DIG", KR_TX_MAP_DIGITAL}
};
static const config_mode_map_t kr_tx_condition[] = {
  {"NONE",KR_TX_COND_NONE},
  {"GE", KR_TX_COND_GE},
  {"LE", KR_TX_COND_LE}
};
static const config_mode_map_t kr_tx_direction[] = {
  {"FALSE",KR_TX_INV_FALSE},
  {"TRUE", KR_TX_INV_TRUE},
};

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
  rc_input_ch_states_t states[RC_INPUT_MAX];

  chRegSetThreadName("kr_tx_main_th");

  /*
   * Receive messages from rc_input module
   */
  while (true)
  {
    time = chVTGetSystemTimeX();

    rc_input_get_channel_states(states);
    _kr_tx_trim_and_limit(states, RC_INPUT_MAX);
    _kr_tx_map_channels(&_kr_tx_frame, states, KR_CHANNEL_NUMBER);
    nrf_send_payload(&_kr_tx_frame, sizeof(_kr_tx_frame));

    chThdSleepUntilWindowed(time, time + TIME_MS2I(10));
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


static void _kr_tx_trim_and_limit(rc_input_ch_states_t * states, uint8_t channels)
{
  uint8_t i = 0;
  for(i=0; i<channels; i++)
  {
    if(states[i].type == RC_INPUT_CH_DIGITAL)
    {
      states[i].type = RC_INPUT_CH_ANALOG;
      states[i].state.analog = (states[i].state.digital == RC_INPUT_DIG_SW_ON) ?
          _kr_tx_config->input_max[i] : _kr_tx_config->input_min[i];
    }
    else
    {
      states[i].state.analog += _kr_tx_config->trim[i];
      states[i].state.analog = (states[i].state.analog > _kr_tx_config->input_max[i]) ?
          _kr_tx_config->input_max[i] : (
          (states[i].state.analog < _kr_tx_config->input_min[i]) ?
              _kr_tx_config->input_min[i] :
              states[i].state.analog);
    }
  }
}

static void _kr_tx_map_channels(kr_transmit_frame_t * frame, rc_input_ch_states_t * states, uint8_t channels)
{
  uint8_t i = 0;
  for(i=0; i<channels; i++)
  {
    uint8_t i_id = _kr_tx_config->mapping[i].channel;
    uint16_t i_channel = states[i_id].state.analog;
    kr_tx_mapping_t * mapping = &_kr_tx_config->mapping[i];
    switch(mapping->type)
    {
      case KR_TX_MAP_ANALOG:
        frame->channels[i] = mapping->output_min + (kr_ch_t)((float)i_channel*
            ((float)(mapping->output_max-mapping->output_min) / (float)(_kr_tx_config->input_max[i_id]-_kr_tx_config->input_min[i_id])));
        break;
      case KR_TX_MAP_DIGITAL:
        switch(mapping->condition)
        {
          case KR_TX_COND_GE:
            frame->channels[i] = (i_channel >= mapping->threshold) ?
                mapping->output_max : mapping->output_min;
            break;
          case KR_TX_COND_LE:
            frame->channels[i] = (i_channel <= mapping->threshold) ?
                mapping->output_max : mapping->output_min;
            break;
          default:
            break;
        }
        break;
      case KR_TX_MAP_DISABLED:
      default:
        break;
    }
    if(mapping->type)
    {
      frame->channels[i] = (mapping->inverted == KR_TX_INV_TRUE) ?
          KR_CHANNEL_MAX_VALUE - frame->channels[i] : frame->channels[i];
    }
  }
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
  _kr_tx_config = (kr_tx_config_t *)config_get_module_config(CONFIG_ENTRY_KR_TX);

  /*
   * Project specific hal initialization
   */
  nrf_init();
  rc_input_init();

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

void kr_tx_parse_array(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry)
{
  if(strcmp(entry->name, "kr-tx-trim") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_INT16, KR_CHANNEL_NUMBER);
  else if(strcmp(entry->name, "kr-tx-input-min") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER);
  else if(strcmp(entry->name, "kr-tx-input-max") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER);
}

void kr_tx_print_array(BaseSequentialStream * chp, config_entry_mapping_t * entry)
{
  if(strcmp(entry->name, "kr-tx-trim") == 0)
    config_print_array(chp, entry, CONFIG_INT16, KR_CHANNEL_NUMBER, CONFIG_DEC);
  else if(strcmp(entry->name, "kr-tx-input-min") == 0)
    config_print_array(chp, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER, CONFIG_DEC);
  else if(strcmp(entry->name, "kr-tx-input-max") == 0)
    config_print_array(chp, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER, CONFIG_DEC);
}
