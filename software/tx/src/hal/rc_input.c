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
 * rc_input.c
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
#include "api/hal/rc_input.h"

/*
 * Include dependencies
 */
#include <string.h>
#include <assert.h>
#include "api/app/config.h"
#include "api/app/config_entries.h"

/*
 * Static asserts
 */
static_assert(RC_INPUT_MAX == (RC_INPUT_DIG_SW_COUNT + RC_INPUT_AN_IN_COUNT),
              "Number of defined channels must match number of digital + analog inputs");

/*
 * Forward declarations of static functions
 */
static void _rc_input_init_hal(void);
static void _rc_input_init_module(void);
static void _rc_input_set_dig_sw_state(rc_input_ch_t ch, rc_input_state_digital_t state);

/*
 * Static variables
 */
static THD_WORKING_AREA(_rc_input_poll_stack, RC_INPUT_POLL_THREAD_STACK);
static rc_input_config_t * _rc_input_config = NULL;
static rc_input_ch_states_t _rc_input_ch_states[RC_INPUT_MAX] = {
    { .type = RC_INPUT_CH_ANALOG,  .state.analog  = 0 },
    { .type = RC_INPUT_CH_ANALOG,  .state.analog  = 0 },
    { .type = RC_INPUT_CH_ANALOG,  .state.analog  = 0 },
    { .type = RC_INPUT_CH_ANALOG,  .state.analog  = 0 },
    { .type = RC_INPUT_CH_ANALOG,  .state.analog  = 0 },
    { .type = RC_INPUT_CH_ANALOG,  .state.analog  = 0 },
    { .type = RC_INPUT_CH_ANALOG,  .state.analog  = 0 },
    { .type = RC_INPUT_CH_ANALOG,  .state.analog  = 0 },
    { .type = RC_INPUT_CH_DIGITAL, .state.digital = RC_INPUT_DIG_SW_OFF },
    { .type = RC_INPUT_CH_DIGITAL, .state.digital = RC_INPUT_DIG_SW_OFF },
    { .type = RC_INPUT_CH_DIGITAL, .state.digital = RC_INPUT_DIG_SW_OFF },
    { .type = RC_INPUT_CH_DIGITAL, .state.digital = RC_INPUT_DIG_SW_OFF },
    { .type = RC_INPUT_CH_DIGITAL, .state.digital = RC_INPUT_DIG_SW_OFF },
    { .type = RC_INPUT_CH_DIGITAL, .state.digital = RC_INPUT_DIG_SW_OFF },
    { .type = RC_INPUT_CH_DIGITAL, .state.digital = RC_INPUT_DIG_SW_OFF },
    { .type = RC_INPUT_CH_DIGITAL, .state.digital = RC_INPUT_DIG_SW_OFF },
};
static rc_input_ch_digital_t _rc_input_dig_sw_list[RC_INPUT_DIG_SW_COUNT] = {
    {.line = RC_INPUT_DIG_LINE_SW0, .ch = RC_INPUT_CH8,  .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW1, .ch = RC_INPUT_CH9,  .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW2, .ch = RC_INPUT_CH10, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW3, .ch = RC_INPUT_CH11, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW4, .ch = RC_INPUT_CH12, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW5, .ch = RC_INPUT_CH13, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW6, .ch = RC_INPUT_CH14, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW7, .ch = RC_INPUT_CH15, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
};
static rc_input_ch_analog_t _rc_input_an_in_list[RC_INPUT_AN_IN_COUNT] = {
    { .line = RC_INPUT_AN_LINE_IN0, .ch = RC_INPUT_CH0 },
    { .line = RC_INPUT_AN_LINE_IN1, .ch = RC_INPUT_CH1 },
    { .line = RC_INPUT_AN_LINE_IN2, .ch = RC_INPUT_CH2 },
    { .line = RC_INPUT_AN_LINE_IN3, .ch = RC_INPUT_CH3 },
    { .line = RC_INPUT_AN_LINE_IN4, .ch = RC_INPUT_CH4 },
    { .line = RC_INPUT_AN_LINE_IN5, .ch = RC_INPUT_CH5 },
    { .line = RC_INPUT_AN_LINE_IN6, .ch = RC_INPUT_CH6 },
    { .line = RC_INPUT_AN_LINE_IN7, .ch = RC_INPUT_CH7 },
};

/*
 * Global variables
 */
event_source_t rc_input_event_handle;

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_rc_input_poll_thread, arg)
{
  (void)arg;
  systime_t time = 0;
  uint8_t sw_id = 0;
  uint32_t pin_state = 0;

  chRegSetThreadName("rc_input_poll_th");

  /*
   * Poll switches for each rc_input_POLL_MAIN_THREAD_P_MS
   */
  while (true)
  {
    time = chVTGetSystemTimeX();

    /*
     * Loop over all switches
     */
    for (sw_id = 0; sw_id < RC_INPUT_DIG_SW_COUNT; sw_id++)
    {
      /*
       * Check if delay is set for switch
       */
      if (_rc_input_dig_sw_list[sw_id].delay == 0)
      {
        /*
         * Read current switch state
         */
        pin_state = palReadLine(_rc_input_dig_sw_list[sw_id].line);
        /*
         * Switch state handling
         */
        switch (_rc_input_dig_sw_list[sw_id].state)
        {
          case RC_INPUT_SW_STATE_INIT:
            /*
             * Switch is in init state, switch to pressed state
             * if switch was set and use delay value for switch
             * debouncing.
             */
            if (pin_state == RC_INPUT_DIG_SW_HIGH)
            {
              _rc_input_dig_sw_list[sw_id].delay = RC_INPUT_DIG_DEBOUNCE_TIME_TICKS;
              _rc_input_dig_sw_list[sw_id].state = RC_INPUT_SW_STATE_PRESS;
              _rc_input_set_dig_sw_state(_rc_input_dig_sw_list[sw_id].ch, RC_INPUT_DIG_SW_ON);
            }
            break;
          case RC_INPUT_SW_STATE_PRESS:
            /*
             * Switch is in pressed state, switch to un-pressed state
             * if switch was released and use delay value for switch
             * debouncing.
             */
            if (pin_state == RC_INPUT_DIG_SW_LOW)
            {
              _rc_input_dig_sw_list[sw_id].delay = RC_INPUT_DIG_DEBOUNCE_TIME_TICKS;
              _rc_input_dig_sw_list[sw_id].state = RC_INPUT_SW_STATE_INIT;
              _rc_input_set_dig_sw_state(_rc_input_dig_sw_list[sw_id].ch, RC_INPUT_DIG_SW_OFF);
            }
            break;
          default:
            _rc_input_dig_sw_list[sw_id].delay = 0;
            _rc_input_dig_sw_list[sw_id].state = RC_INPUT_SW_STATE_INIT;
            _rc_input_set_dig_sw_state(_rc_input_dig_sw_list[sw_id].ch, RC_INPUT_DIG_SW_OFF);
            break;
        }
      }
      else
      {
        /*
         * Decrease delay value
         */
        _rc_input_dig_sw_list[sw_id].delay--;
      }
    }
    chThdSleepUntilWindowed(time, time + TIME_MS2I(_rc_input_config->poll_thread_period_ms));
  }
}

/*
 * Static helper functions
 */
static void _rc_input_set_dig_sw_state(rc_input_ch_t ch, rc_input_state_digital_t state)
{
  /*
   * Use critical section to provide
   * consistent data
   */
  chSysLock();
  _rc_input_ch_states[ch].state.digital = state;
  chSysUnlock();
}

static void _rc_input_init_hal(void)
{
  uint8_t sw_id = 0;

  /*
   * Setup analog lines
   */
  for (sw_id = 0; sw_id < RC_INPUT_DIG_SW_COUNT; sw_id++)
  {
    palSetLineMode(_rc_input_an_in_list[sw_id].line, PAL_MODE_INPUT_ANALOG);
  }
  /*
   * Setup digital lines
   */
  for (sw_id = 0; sw_id < RC_INPUT_DIG_SW_COUNT; sw_id++)
  {
    palSetLineMode(_rc_input_dig_sw_list[sw_id].line, _rc_input_config->digital_switch_mode[sw_id]);
  }
}

static void _rc_input_init_module(void)
{

  /*
   * Create rc_input polling task
   */
  chThdCreateStatic(_rc_input_poll_stack, sizeof(_rc_input_poll_stack), _rc_input_config->poll_thread_prio,
                    _rc_input_poll_thread, NULL);
}

/*
 * Callback functions
 */

//#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void rc_input_loop_channels_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0)
  {
    chprintf(chp, "Usage: rc-input-loop\r\n");
    return;
  }

  rc_input_ch_t ch = 0;
  systime_t time = 0;

  for(ch=RC_INPUT_CH0; ch < RC_INPUT_MAX; ch++)
  {
    chprintf(chp, "  CH%02d%s",ch,
             ((_rc_input_ch_states[ch].type == RC_INPUT_CH_ANALOG) ?
                 "A" : "D"));
  }
  chprintf(chp, "\r\n");

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
  {
      time = chVTGetSystemTimeX();

      for(ch=RC_INPUT_CH0; ch < RC_INPUT_MAX; ch++)
      {
        chprintf(chp, "%7d",((_rc_input_ch_states[ch].type == RC_INPUT_CH_ANALOG) ?
            _rc_input_ch_states[ch].state.analog :
            _rc_input_ch_states[ch].state.digital));
      }
      chprintf(chp, "\r");
      chThdSleepUntilWindowed(time, time + TIME_MS2I(_rc_input_config->poll_thread_period_ms));
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}
//#endif

/*
 * API functions
 */
void rc_input_init(void)
{
  _rc_input_config = (rc_input_config_t *)config_get_module_config(CONFIG_ENTRY_RC_INPUT);
  _rc_input_init_hal();
  _rc_input_init_module();
}

void rc_input_get_channel_states(rc_input_ch_states_t *dest)
{
  /*
   * Use critical section to provide
   * consistent data
   */
  chSysLock();
  memcpy(dest, _rc_input_ch_states, sizeof(rc_input_ch_states_t) * RC_INPUT_MAX);
  chSysUnlock();
}
