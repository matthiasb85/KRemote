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
static uint8_t _kr_tx_config_get_mapping_table(char * name, config_mode_map_t ** map);
static uint8_t _kr_tx_set_config_cb(config_entry_mapping_t * entry, uint8_t idx, char * arg);
static char * _kr_tx_get_config_cb(config_entry_mapping_t * entry, uint8_t idx);
static rc_input_ch_states_t _kr_tx_old_states[RC_INPUT_MAX];
/*
 * Static variables
 */
static THD_WORKING_AREA(_kr_tx_main_stack, KR_TX_MAIN_THREAD_STACK);
static kr_transmit_frame_t _kr_tx_frame;
static kr_tx_config_t * _kr_tx_config = NULL;
static const config_mode_map_t _kr_tx_mapping_type[] = {
  {"DIS", KR_TX_MAP_DISABLED},
  {"ANA", KR_TX_MAP_ANALOG},
  {"DIG", KR_TX_MAP_DIGITAL}
};
static const config_mode_map_t _kr_tx_condition[] = {
  {"NONE",KR_TX_COND_NONE},
  {"GE",  KR_TX_COND_GE},
  {"LE",  KR_TX_COND_LE},
  {"TGE", KR_TX_COND_TGE},
  {"TLE", KR_TX_COND_TLE}
};
static const config_mode_map_t _kr_tx_direction[] = {
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
    memcpy(_kr_tx_old_states, states, sizeof(_kr_tx_old_states));

    chThdSleepUntilWindowed(time, time + TIME_MS2I(_kr_tx_config->main_thread_period_ms));
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
  chThdCreateStatic(_kr_tx_main_stack, sizeof(_kr_tx_main_stack), _kr_tx_config->main_thread_prio,
                    _kr_tx_main_thread, NULL);

  /*
   * Initialize state channel state buffer
   */
  memset(_kr_tx_old_states, 0, sizeof(_kr_tx_old_states));
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
    uint8_t i_id = _kr_tx_config->map_channel[i];
    kr_tx_mapping_type_t type = _kr_tx_config->map_type[i];
    kr_tx_direction_t inverted = _kr_tx_config->map_inverted[i];
    kr_ch_t output_min = _kr_tx_config->map_output_min[i];
    kr_ch_t output_max = _kr_tx_config->map_output_max[i];
    kr_tx_condition_t condition = _kr_tx_config->map_condition[i];
    uint16_t threshold = _kr_tx_config->map_threshold[i];
    uint16_t i_channel = states[i_id].state.analog;
    uint16_t i_channel_old = _kr_tx_old_states[i_id].state.analog;

    switch(type)
    {
      case KR_TX_MAP_ANALOG:
        frame->channels[i] = output_min + (kr_ch_t)((float)i_channel*
            ((float)(output_max-output_min) / (float)(_kr_tx_config->input_max[i_id]-_kr_tx_config->input_min[i_id])));
        break;
      case KR_TX_MAP_DIGITAL:
        switch(condition)
        {
          case KR_TX_COND_GE: frame->channels[i] = (i_channel >= threshold) ? output_max : output_min; break;
          case KR_TX_COND_LE: frame->channels[i] = (i_channel <= threshold) ? output_max : output_min; break;
          case KR_TX_COND_TGE:
            if(i_channel != i_channel_old && i_channel >= threshold)
            {
                frame->channels[i] = (frame->channels[i] == output_max) ? output_min: output_max;
            }
            break;
          case KR_TX_COND_TLE:
            if(i_channel != i_channel_old && i_channel <= threshold)
            {
                frame->channels[i] = (frame->channels[i] == output_max) ? output_min: output_max;
            }
            break;
          default:
            break;
        }
        break;
      case KR_TX_MAP_DISABLED:
      default:
        break;
    }
    if(type)
    {
      frame->channels[i] = (inverted == KR_TX_INV_TRUE) ? KR_CHANNEL_MAX_VALUE - frame->channels[i] : frame->channels[i];
    }
  }
}

static uint8_t _kr_tx_config_get_mapping_table(char * name, config_mode_map_t ** map)
{
  uint8_t map_len = 0;
  if(strcmp(name, "kr-tx-map-type") == 0)      { *map = (config_mode_map_t *)_kr_tx_mapping_type; map_len = sizeof(_kr_tx_mapping_type); }
  else if(strcmp(name, "kr-tx-map-inv") == 0)  { *map = (config_mode_map_t *)_kr_tx_direction; map_len = sizeof(_kr_tx_direction); }
  else if(strcmp(name, "kr-tx-map-cond") == 0) { *map = (config_mode_map_t *)_kr_tx_condition; map_len = sizeof(_kr_tx_condition); }
  else return 0;
  return map_len/sizeof(config_mode_map_t);
}

/*
 * Callback functions
 */
static uint8_t _kr_tx_set_config_cb(config_entry_mapping_t * entry, uint8_t idx, char * arg)
{
  config_mode_map_t * map = NULL;
  uint8_t map_len = _kr_tx_config_get_mapping_table((char *)entry->name, &map);
  uint32_t * value = NULL;

  if(strcmp(entry->name, "kr-tx-map-type") == 0)    { value = (uint32_t *)(&((kr_tx_mapping_type_t*)(entry->payload))[idx]); }
  else if(strcmp(entry->name, "kr-tx-map-inv") == 0) { value = (uint32_t *)(&((kr_tx_direction_t *)(entry->payload))[idx]); }
  else if(strcmp(entry->name, "kr-tx-map-cond") == 0) { value = (uint32_t *)(&((kr_tx_condition_t *)(entry->payload))[idx]); }
  else return 0;

  return config_map_str_to_value(arg, value, map, map_len, CONFIG_UINT8);
}

static char * _kr_tx_get_config_cb(config_entry_mapping_t * entry, uint8_t idx)
{
  config_mode_map_t * map = NULL;
  uint8_t map_len = _kr_tx_config_get_mapping_table((char *)entry->name, &map);
  char *str = NULL;

  uint32_t value = 0;

  if(strcmp(entry->name, "kr-tx-map-type") == 0)    { value = (uint32_t)((kr_tx_mapping_type_t *)(entry->payload))[idx]; }
  else if(strcmp(entry->name, "kr-tx-map-inv") == 0) { value = (uint32_t)((kr_tx_direction_t *)(entry->payload))[idx]; }
  else if(strcmp(entry->name, "kr-tx-map-cond") == 0) { value = (uint32_t)((kr_tx_condition_t *)(entry->payload))[idx]; }
  else return NULL;

  config_map_value_to_str(value, &str, map, map_len);

  return str;
}

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void kr_tx_show_channel(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc != 1)
  {
    chprintf(chp, "Usage:  kr-tx-show-channel channel\r\n");
    return;
  }
  config_mode_map_t * map = NULL;
  uint8_t map_len = 0;
  uint32_t ch = (uint16_t)strtol(argv[0], NULL, 0);

  char * type = NULL;
  map_len = _kr_tx_config_get_mapping_table("kr-tx-map-type", &map);
  config_map_value_to_str(_kr_tx_config->map_type[ch], &type, map, map_len);

  uint8_t input_channel = _kr_tx_config->map_channel[ch];
  int16_t input_trim = _kr_tx_config->trim[input_channel];
  uint16_t input_min = _kr_tx_config->input_min[input_channel];
  uint16_t input_max = _kr_tx_config->input_max[input_channel];

  kr_ch_t output_min = _kr_tx_config->map_output_min[ch];
  kr_ch_t output_max = _kr_tx_config->map_output_max[ch];

  char * inverted = NULL;
  map_len = _kr_tx_config_get_mapping_table("kr-tx-map-inv", &map);
  config_map_value_to_str(_kr_tx_config->map_inverted[ch], &inverted, map, map_len);

  char * condition = NULL;
  map_len = _kr_tx_config_get_mapping_table("kr-tx-map-cond", &map);
  config_map_value_to_str(_kr_tx_config->map_condition[ch], &condition, map, map_len);

  uint16_t threshold = _kr_tx_config->map_threshold[ch];

  chprintf(chp, "Channel mapping for output channel %d\r\n", ch);
  chprintf(chp, " Mapping type:  %s\r\n", type);
  chprintf(chp, " Input channel: %d\r\n", input_channel);
  chprintf(chp, "     |-trim:    %d\r\n", input_trim);
  chprintf(chp, "     |-min:     %d\r\n", input_min);
  chprintf(chp, "     |-max:     %d\r\n", input_max);
  chprintf(chp, " Output min:    %d\r\n", output_min);
  chprintf(chp, "      |-max:    %d\r\n", output_max);
  chprintf(chp, " Inverted:      %s\r\n", inverted);
  chprintf(chp, " Condition:     %s\r\n", condition);
  chprintf(chp, " Threshold:     %d\r\n", threshold);

}

void kr_tx_trim_channel(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc != 1)
  {
    chprintf(chp, "Usage:  kr-tx-trim-channel channel\r\n");
    return;
  }
  uint32_t ch = (uint16_t)strtol(argv[0], NULL, 0);

  int16_t old = _kr_tx_config->trim[ch];

  chprintf(chp, "Modify trim value via +/-, press ENTER to save value or a/A to abort!\r\n");
  while (true)
  {
    chprintf(chp, "CH%d %5d\r",ch, _kr_tx_config->trim[ch]);

    uint8_t input=0;

    if(streamRead(chp,&input,1))
    {
      switch(input)
      {
        case '+': _kr_tx_config->trim[ch] += 10; break;
        case '-': _kr_tx_config->trim[ch] -= 10; break;
        case 0x0d:
          chprintf(chp, "\r\nSaved trim value %d for input channel %d. Use config-store to persist data!\r\n", _kr_tx_config->trim[ch], ch);
          return;
        case 'a':
        case 'A':
          _kr_tx_config->trim[ch] = old;
          chprintf(chp, "Abort!\r\n");
          return;
        default:
          break;;
      }
    }
    chThdSleep(TIME_MS2I(10));
  }
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

void kr_tx_parse_config(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry)
{
  if(strcmp(entry->name, "kr-tx-trim") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_INT16, RC_INPUT_MAX);
  else if(strcmp(entry->name, "kr-tx-input-min") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_UINT16, RC_INPUT_MAX);
  else if(strcmp(entry->name, "kr-tx-input-max") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_UINT16, RC_INPUT_MAX);
  else if(strcmp(entry->name, "kr-tx-map-ch") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_UINT8, KR_CHANNEL_NUMBER);
  else if(strcmp(entry->name, "kr-tx-map-omin") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER);
  else if(strcmp(entry->name, "kr-tx-map-omax") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER);
  else if(strcmp(entry->name, "kr-tx-map-thresh") == 0)
    config_parse_array(chp, argc, argv, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER);
}

void kr_tx_print_config(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t print_help)
{
  if(strcmp(entry->name, "kr-tx-trim") == 0)
    config_print_array(chp, entry, CONFIG_INT16, RC_INPUT_MAX, CONFIG_DEC, print_help);
  else if(strcmp(entry->name, "kr-tx-input-min") == 0)
    config_print_array(chp, entry, CONFIG_UINT16, RC_INPUT_MAX, CONFIG_DEC, print_help);
  else if(strcmp(entry->name, "kr-tx-input-max") == 0)
    config_print_array(chp, entry, CONFIG_UINT16, RC_INPUT_MAX, CONFIG_DEC, print_help);
  else if(strcmp(entry->name, "kr-tx-map-ch") == 0)
    config_print_array(chp, entry, CONFIG_UINT8, KR_CHANNEL_NUMBER, CONFIG_DEC, print_help);
  else if(strcmp(entry->name, "kr-tx-map-omin") == 0)
    config_print_array(chp, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER, CONFIG_DEC, print_help);
  else if(strcmp(entry->name, "kr-tx-map-omax") == 0)
    config_print_array(chp, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER, CONFIG_DEC, print_help);
  else if(strcmp(entry->name, "kr-tx-map-thresh") == 0)
    config_print_array(chp, entry, CONFIG_UINT16, KR_CHANNEL_NUMBER, CONFIG_DEC, print_help);
}

void kr_tx_parse_config_map(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry)
{
  config_mode_map_t * map = NULL;
  uint8_t map_len = _kr_tx_config_get_mapping_table((char *)entry->name, &map);

  config_parse_array_map(chp, argc, argv, entry, KR_CHANNEL_NUMBER, _kr_tx_set_config_cb,map,map_len);
}

void kr_tx_print_config_map(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t print_help)
{
  config_print_array_map(chp, entry, KR_CHANNEL_NUMBER, _kr_tx_get_config_cb, print_help);

}
