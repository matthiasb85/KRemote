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
#include <stdlib.h>
#include "api/app/config.h"
#include "api/app/config_entries.h"

/*
 * Static asserts
 */
static_assert(RC_INPUT_MAX == (RC_INPUT_AN_MAX + RC_INPUT_DIG_MAX),
              "Number of defined channels must match number of digital + analog inputs");

/*
 * Forward declarations of static functions
 */
static void _rc_input_init_hal(void);
static void _rc_input_init_module(void);
static void _rc_input_set_dig_state(rc_input_ch_t ch, rc_input_state_digital_t state);
static void _rc_input_an_start_measurement (void);
static uint16_t _rc_input_get_an_state(rc_input_ch_an_t line, uint16_t old_value);
static void _rc_input_an_start_measurement_cb(void *arg);
static void _rc_input_an_finish_measurement_cb(ADCDriver *adcp);

/*
 * Static variables
 */
static THD_WORKING_AREA(_rc_input_poll_stack, RC_INPUT_POLL_THREAD_STACK);
static virtual_timer_t _rc_input_adc_start_measurement_vtp;
static adcsample_t _rc_input_an_samples[RC_INPUT_AN_MAX];

static rc_input_config_t * _rc_input_config = NULL;
static const config_mode_map_t _rc_input_config_switch_mode_map[] = {
    { "PU", PAL_MODE_INPUT_PULLUP },
    { "PD", PAL_MODE_INPUT_PULLDOWN }
};

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
static rc_input_ch_digital_t _rc_input_dig_sw_list[RC_INPUT_DIG_MAX] = {
    {.line = RC_INPUT_DIG_LINE_SW0, .ch = RC_INPUT_CH8,  .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW1, .ch = RC_INPUT_CH9,  .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW2, .ch = RC_INPUT_CH10, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW3, .ch = RC_INPUT_CH11, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW4, .ch = RC_INPUT_CH12, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW5, .ch = RC_INPUT_CH13, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW6, .ch = RC_INPUT_CH14, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
    {.line = RC_INPUT_DIG_LINE_SW7, .ch = RC_INPUT_CH15, .delay = 0, .state = RC_INPUT_SW_STATE_INIT },
};
static const ADCConversionGroup _rc_input_adc_cfg = {
  FALSE,
  RC_INPUT_AN_MAX,
  _rc_input_an_finish_measurement_cb,
  NULL,
  0,                                    /* CR1 */
  ADC_CR2_EXTSEL_SWSTART,               /* CR2 */
  0,                                    /* SMPR1 */
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_1P5) | ADC_SMPR2_SMP_AN1(ADC_SAMPLE_1P5) |      /* SMPR2 */
  ADC_SMPR2_SMP_AN2(ADC_SAMPLE_1P5) | ADC_SMPR2_SMP_AN3(ADC_SAMPLE_1P5) |      /* SMPR2 */
  ADC_SMPR2_SMP_AN4(ADC_SAMPLE_1P5) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_1P5) |      /* SMPR2 */
  ADC_SMPR2_SMP_AN6(ADC_SAMPLE_1P5) | ADC_SMPR2_SMP_AN7(ADC_SAMPLE_1P5),       /* SMPR2 */
  ADC_SQR1_NUM_CH(RC_INPUT_AN_MAX),   /* SQR1 */
  ADC_SQR2_SQ7_N(RC_INPUT_AN_IN6) | ADC_SQR2_SQ8_N(RC_INPUT_AN_IN7) ,   /* SQR2 */
  ADC_SQR3_SQ1_N(RC_INPUT_AN_IN0) | ADC_SQR3_SQ2_N(RC_INPUT_AN_IN1) | /* SQR3 */
  ADC_SQR3_SQ3_N(RC_INPUT_AN_IN2) | ADC_SQR3_SQ4_N(RC_INPUT_AN_IN3) | /* SQR3 */
  ADC_SQR3_SQ5_N(RC_INPUT_AN_IN4) | ADC_SQR3_SQ6_N(RC_INPUT_AN_IN5),  /* SQR3 */
};
static rc_input_ch_analog_t _rc_input_an_in_list[RC_INPUT_AN_MAX] = {
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
    for (sw_id = 0; sw_id < RC_INPUT_DIG_MAX; sw_id++)
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
              _rc_input_set_dig_state(_rc_input_dig_sw_list[sw_id].ch, RC_INPUT_DIG_SW_ON);
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
              _rc_input_set_dig_state(_rc_input_dig_sw_list[sw_id].ch, RC_INPUT_DIG_SW_OFF);
            }
            break;
          default:
            _rc_input_dig_sw_list[sw_id].delay = 0;
            _rc_input_dig_sw_list[sw_id].state = RC_INPUT_SW_STATE_INIT;
            _rc_input_set_dig_state(_rc_input_dig_sw_list[sw_id].ch, RC_INPUT_DIG_SW_OFF);
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
static void _rc_input_init_hal(void)
{
  uint8_t sw_id = 0;

  /*
   * Setup analog lines
   */
  for (sw_id = 0; sw_id < RC_INPUT_AN_MAX; sw_id++)
  {
    palSetLineMode(_rc_input_an_in_list[sw_id].line, PAL_MODE_INPUT_ANALOG);
  }
  adcStart(RC_INPUT_AN_DRIVER, NULL);

  /*
   * Setup digital lines
   */
  for (sw_id = 0; sw_id < RC_INPUT_DIG_MAX; sw_id++)
  {
    palSetLineMode(_rc_input_dig_sw_list[sw_id].line, _rc_input_config->digital_switch_mode[sw_id]);
  }
}

static void _rc_input_init_module(void)
{

  /*
   * Create rc_input polling task and adc conversion timer
   */
  chThdCreateStatic(_rc_input_poll_stack, sizeof(_rc_input_poll_stack), _rc_input_config->poll_thread_prio,
                    _rc_input_poll_thread, NULL);

  chVTObjectInit(&_rc_input_adc_start_measurement_vtp);
  _rc_input_an_start_measurement();
}

static void _rc_input_set_dig_state(rc_input_ch_t ch, rc_input_state_digital_t state)
{
  /*
   * Use critical section to provide
   * consistent data
   */
  chSysLock();
  _rc_input_ch_states[ch].state.digital = state;
  chSysUnlock();
}

static void _rc_input_an_start_measurement (void)
{
  adcStartConversionI(RC_INPUT_AN_DRIVER, &_rc_input_adc_cfg, _rc_input_an_samples, 1);

  chVTSetI(&_rc_input_adc_start_measurement_vtp,
           TIME_MS2I(_rc_input_config->analog_conversion_period_ms),
           _rc_input_an_start_measurement_cb,
           (void *)(NULL));
}

static uint16_t _rc_input_get_an_state(rc_input_ch_an_t line, uint16_t old_value)
{
  return (_rc_input_an_samples[line]*_rc_input_config->analog_conversion_emph_new + old_value*_rc_input_config->analog_conversion_emph_old)
      /(_rc_input_config->analog_conversion_emph_new + _rc_input_config->analog_conversion_emph_old);
}

/*
 * Callback functions
 */
static void _rc_input_an_start_measurement_cb(void *arg)
{
  (void)arg;
  _rc_input_an_start_measurement();
}

static void _rc_input_an_finish_measurement_cb(ADCDriver *adcp)
{
  (void)adcp;
  uint8_t ch = 0;
  for(ch = 0; ch < RC_INPUT_AN_MAX; ch++)
  {
    chSysLockFromISR();
    rc_input_state_analog_t old = _rc_input_ch_states[_rc_input_an_in_list[ch].ch].state.analog;
    _rc_input_ch_states[_rc_input_an_in_list[ch].ch].state.analog = _rc_input_get_an_state(ch, old);
    chSysUnlockFromISR();
  }
}

#if defined(USE_CMD_SHELL)
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
      chThdSleepUntilWindowed(time, time + TIME_MS2I(_rc_input_config->loop_cmd_period_ms));
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}
#endif

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

void rc_input_parse_dig_sm(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry)
{
  uint32_t * digital_switch_mode = entry->payload;
  uint32_t idx = (uint16_t)strtol(argv[1], NULL, 0);
  uint8_t error = 0;
  if(argc != 3 && argc != RC_INPUT_DIG_MAX+1)
  {
    error = 1;
  }
  if(!error)
  {
    if(argc == 3)
    {
      if(idx < RC_INPUT_DIG_MAX)
      {
        if(!config_map_str_to_value(argv[1], &digital_switch_mode[idx], _rc_input_config_switch_mode_map,
            sizeof(_rc_input_config_switch_mode_map)/sizeof(config_mode_map_t)))
        {
          error =  3;
        }
      }
      else if(idx == RC_INPUT_DIG_MAX)
      {
        uint8_t i = 0;
        for(i=0; i<RC_INPUT_DIG_MAX; i++)
        {
          if(!config_map_str_to_value(argv[1], &digital_switch_mode[i], _rc_input_config_switch_mode_map,
              sizeof(_rc_input_config_switch_mode_map)/sizeof(config_mode_map_t)))
          {
            error =  3;
          }
          if(error) break;
        }
      }
      else
      {
        error = 1;
      }
    }
    else
    {
      uint8_t i = 0;
      for(i=0; i<RC_INPUT_DIG_MAX; i++)
      {
        if(!config_map_str_to_value(argv[i+1], &digital_switch_mode[i], _rc_input_config_switch_mode_map,
            sizeof(_rc_input_config_switch_mode_map)/sizeof(config_mode_map_t)))
        {
          error =  3;
        }
        if(error) break;
      }
    }
  }
  if(error)
  {
    chprintf(chp, "Input does not match, use: [0..%d] value            (set single value)\r\n", RC_INPUT_DIG_IN7);
    chprintf(chp, "                           [%d] value               (set all values)\r\n", RC_INPUT_DIG_MAX);
    chprintf(chp, "                           value0 value1 ...value%d (set all value individual)\r\n", RC_INPUT_DIG_MAX-1);
    chprintf(chp, "                           value=[PD|PU]\r\n");
  }
}

void rc_input_print_dig_sm(BaseSequentialStream * chp, config_entry_mapping_t * entry)
{
  uint8_t i = 0;
  uint32_t * digital_switch_mode = entry->payload;
  chprintf(chp, "  %-20s",entry->name);
  char * str = NULL;
  for(i=0; i<RC_INPUT_DIG_MAX; i++)
  {
      config_map_value_to_str(digital_switch_mode[i], &str, _rc_input_config_switch_mode_map,
                              sizeof(_rc_input_config_switch_mode_map)/sizeof(config_mode_map_t));
      chprintf(chp, " %s", str);
  }
}
