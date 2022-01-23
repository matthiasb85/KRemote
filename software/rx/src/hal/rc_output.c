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
 * rc_output.c
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
#include "api/hal/rc_output.h"

/*
 * Include dependencies
 */
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "api/app/kr.h"
#include "api/app/config.h"
#include "api/app/config_entries.h"

/*
 * Static asserts
 */
static_assert(RC_OUTPUT_MAX == (RC_OUTPUT_AN_MAX + RC_OUTPUT_DIG_MAX),
              "Number of defined channels must match number of digital + analog outputs");

/*
 * Forward declarations of static functions
 */
static void _rc_output_init_hal(void);
static void _rc_output_init_module(void);
static void _rc_output_set_analog(uint8_t channel, uint16_t value);
static void _rc_output_set_digital(uint8_t channel, uint16_t value);

/*
 * Static variables
 */
static rc_output_config_t * _rc_output_config = NULL;
static rc_output_t _rc_output[RC_OUTPUT_MAX] = {
    { .line = RC_OUTPUT_AN_LINE0,   .type = RC_OUTPUT_CH_ANALOG,  .id.an = RC_OUTPUT_AN_OUT0},
    { .line = RC_OUTPUT_AN_LINE1,   .type = RC_OUTPUT_CH_ANALOG,  .id.an = RC_OUTPUT_AN_OUT1},
    { .line = RC_OUTPUT_AN_LINE2,   .type = RC_OUTPUT_CH_ANALOG,  .id.an = RC_OUTPUT_AN_OUT2},
    { .line = RC_OUTPUT_AN_LINE3,   .type = RC_OUTPUT_CH_ANALOG,  .id.an = RC_OUTPUT_AN_OUT3},
    { .line = RC_OUTPUT_DIG_LINE0,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT0},
    { .line = RC_OUTPUT_DIG_LINE1,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT1},
    { .line = RC_OUTPUT_DIG_LINE2,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT2},
    { .line = RC_OUTPUT_DIG_LINE3,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT3},
    { .line = RC_OUTPUT_DIG_LINE4,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT4},
    { .line = RC_OUTPUT_DIG_LINE5,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT5},
    { .line = RC_OUTPUT_DIG_LINE6,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT6},
    { .line = RC_OUTPUT_DIG_LINE7,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT7},
    { .line = RC_OUTPUT_DIG_LINE8,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT8},
    { .line = RC_OUTPUT_DIG_LINE9,  .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT9},
    { .line = RC_OUTPUT_DIG_LINE10, .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT10},
    { .line = RC_OUTPUT_DIG_LINE11, .type = RC_OUTPUT_CH_DIGITAL, .id.dig = RC_OUTPUT_DIG_OUT11},
};
static PWMConfig _rc_output_pwmd_cfg = {
    RC_OUTPUT_PWM_TIMER_FREQ,
    RC_OUTPUT_PWM_PERIOD_TICKS,
        NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL}
    },
    0,
    0,
#if STM32_PWM_USE_ADVANCED
    0
#endif
};
static uint32_t _rc_output_pwm_duty_cycles[RC_OUTPUT_AN_MAX];
static uint16_t _rc_output_dig_output_state = 0;

/*
 * Global variables
 */

/*
 * Tasks
 */


/*
 * Static helper functions
 */
static void _rc_output_init_hal(void)
{
  uint8_t i = 0;
  for(i=0; i < RC_OUTPUT_MAX; i++)
  {
    uint32_t line_mode;
    switch(_rc_output[i].type)
    {
      case RC_OUTPUT_CH_ANALOG: line_mode = PAL_MODE_STM32_ALTERNATE_PUSHPULL; break;
      case RC_OUTPUT_CH_DIGITAL:line_mode = _rc_output_config->digital_switch_mode[_rc_output[i].id.dig]; break;
    }
    palSetLineMode(_rc_output[i].line, line_mode);
  }

  pwmStart(RC_OUTPUT_PWM_TIMER_DRIVER, &_rc_output_pwmd_cfg);

  for(i=0; i < RC_OUTPUT_MAX; i++)
  {
    if(_rc_output[i].type == RC_OUTPUT_CH_ANALOG)
    {
      _rc_output_set_analog(i, KR_CHANNEL_MAX_VALUE/2);
    }
  }

}

static void _rc_output_init_module(void)
{
  // nothing to do
}

static void _rc_output_set_analog(uint8_t channel, uint16_t value)
{
  rc_output_ch_an_t ch = _rc_output[channel].id.an;
  uint32_t duty_cycle = (value*RC_OUTPUT_PWM_MAX_VALUE)/KR_CHANNEL_MAX_VALUE;
  pwmEnableChannel(RC_OUTPUT_PWM_TIMER_DRIVER, ch, RC_OUTPUT_PWM_MIN_DUTY_TICKS + RC_OUTPUT_PWM_INT_TO_TICKS(duty_cycle));
  _rc_output_pwm_duty_cycles[ch] = duty_cycle;
}

static void _rc_output_set_digital(uint8_t channel, uint16_t value)
{
  rc_output_ch_dig_t ch = _rc_output[channel].id.dig;
  uint32_t line = _rc_output[channel].line;
  if(value == KR_CHANNEL_MAX_VALUE)
  {
    palSetLine(line);
    _rc_output_dig_output_state |= (1 << ch);
  }
  else
  {
    palClearLine(line);
    _rc_output_dig_output_state &= ~(1 << ch);
  }
}

/*
 * Callback functions
 */

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void rc_output_loop_channels_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0)
  {
    chprintf(chp, "Usage: rc-output-loop\r\n");
    return;
  }

  rc_output_ch_t ch = 0;
  systime_t time = 0;

  for(ch=RC_OUTPUT_CH0; ch < RC_OUTPUT_MAX; ch++)
  {
    chprintf(chp, "  CH%02d%s",ch,
             ((_rc_output[ch].type == RC_OUTPUT_CH_ANALOG) ?
                 "A" : "D"));
  }
  chprintf(chp, "\r\n");

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
  {
      time = chVTGetSystemTimeX();

      for(ch=RC_OUTPUT_CH0; ch < RC_OUTPUT_MAX; ch++)
      {
        chprintf(chp, "%7d",((_rc_output[ch].type == RC_OUTPUT_CH_ANALOG) ?
            _rc_output_pwm_duty_cycles[_rc_output[ch].id.an] :
            (_rc_output_dig_output_state & (1 << _rc_output[ch].id.dig))));
      }
      chprintf(chp, "\r");
      chThdSleepUntilWindowed(time, time + TIME_MS2I(_rc_output_config->loop_cmd_period_ms));
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}

void rc_output_set_channel_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 2)
  {
    chprintf(chp, "Usage: rc-output-set channel value\r\n");
    return;
  }
  uint16_t channel = strtol(argv[0], NULL, 0);
  uint16_t value = strtol(argv[1], NULL, 0);

  if(channel > RC_OUTPUT_MAX)
  {
    chprintf(chp, "Channel out of bound! Supported channels are %d...%d)",
             RC_OUTPUT_CH0,RC_OUTPUT_MAX-1);
  }

  if(value > KR_CHANNEL_MAX_VALUE)
  {
    chprintf(chp, "Value out of bound! Supported values are 0...%d)",
             KR_CHANNEL_MAX_VALUE-1);
  }

  rc_output_set(channel, value);
}
#endif

/*
 * API functions
 */
void rc_output_init(void)
{
  _rc_output_config = (rc_output_config_t *)config_get_module_config(CONFIG_ENTRY_RC_OUTPUT);
  _rc_output_init_hal();
  _rc_output_init_module();
}

void rc_output_set(rc_output_ch_t ch, uint16_t value)
{
  if(ch >= RC_OUTPUT_MAX)
    return;

  switch(_rc_output[ch].type)
  {
    case RC_OUTPUT_CH_ANALOG:  _rc_output_set_analog(ch, value); break;
    case RC_OUTPUT_CH_DIGITAL: _rc_output_set_digital(ch, value); break;
  }
}
