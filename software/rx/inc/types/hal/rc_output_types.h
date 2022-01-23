/*
 * This file is part of The KRemote Project
 *
 * Copyright (c) 2022 Matthias Beckert
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
 * rc_input_types.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_HAL_RC_OUTPUT_TYPES_H_
#define INC_TYPES_HAL_RC_OUTPUT_TYPES_H_

typedef enum
{
  RC_OUTPUT_CH0 = 0,
  RC_OUTPUT_CH1,
  RC_OUTPUT_CH2,
  RC_OUTPUT_CH3,
  RC_OUTPUT_CH4,
  RC_OUTPUT_CH5,
  RC_OUTPUT_CH6,
  RC_OUTPUT_CH7,
  RC_OUTPUT_CH8,
  RC_OUTPUT_CH9,
  RC_OUTPUT_CH10,
  RC_OUTPUT_CH11,
  RC_OUTPUT_CH12,
  RC_OUTPUT_CH13,
  RC_OUTPUT_CH14,
  RC_OUTPUT_CH15,
  RC_OUTPUT_MAX,
}rc_output_ch_t;

typedef enum
{
  RC_OUTPUT_AN_OUT0 = 0,
  RC_OUTPUT_AN_OUT1,
  RC_OUTPUT_AN_OUT2,
  RC_OUTPUT_AN_OUT3,
  RC_OUTPUT_AN_MAX
}rc_output_ch_an_t;

typedef enum
{
  RC_OUTPUT_DIG_OUT0 = 0,
  RC_OUTPUT_DIG_OUT1,
  RC_OUTPUT_DIG_OUT2,
  RC_OUTPUT_DIG_OUT3,
  RC_OUTPUT_DIG_OUT4,
  RC_OUTPUT_DIG_OUT5,
  RC_OUTPUT_DIG_OUT6,
  RC_OUTPUT_DIG_OUT7,
  RC_OUTPUT_DIG_OUT8,
  RC_OUTPUT_DIG_OUT9,
  RC_OUTPUT_DIG_OUT10,
  RC_OUTPUT_DIG_OUT11,
  RC_OUTPUT_DIG_MAX
}rc_output_ch_dig_t;

typedef enum
{
  RC_OUTPUT_CH_ANALOG = 0,
  RC_OUTPUT_CH_DIGITAL
}rc_output_ch_type_t;

typedef struct
{
  rc_output_ch_type_t type;
  uint32_t line;
  union
  {
    rc_output_ch_an_t  an;
    rc_output_ch_dig_t dig;
  }id;
} rc_output_t;

typedef struct
{
  uint32_t digital_switch_mode[RC_OUTPUT_DIG_MAX];
  uint32_t loop_cmd_period_ms;
}rc_output_config_t;

typedef struct
{
  const char *const name;
  uint32_t mode;
}rc_input_config_switch_mode_map_t;

#define RC_OUTPUT_PWM_MS_TO_TICKS(x)  ((RC_OUTPUT_PWM_TIMER_FREQ/1000)*(x))
#define RC_OUTPUT_PWM_PERIOD_TICKS    RC_OUTPUT_PWM_MS_TO_TICKS(RC_OUTPUT_PWM_P_MS)
#define RC_OUTPUT_PWM_MIN_DUTY_TICKS  RC_OUTPUT_PWM_MS_TO_TICKS(RC_OUTPUT_PWM_MIN_DUTY_MS)
#define RC_OUTPUT_PWM_MAX_DUTY_TICKS  RC_OUTPUT_PWM_MS_TO_TICKS(RC_OUTPUT_PWM_MAX_DUTY_MS)

#define RC_OUTPUT_PWM_INT_TO_TICKS(x) (((RC_OUTPUT_PWM_MAX_DUTY_TICKS-RC_OUTPUT_PWM_MIN_DUTY_TICKS)*(x))/RC_OUTPUT_PWM_MAX_VALUE)
#define RC_OUTPUT_PWM_TICKS_TO_PER(x) ((((x))*100)/(RC_OUTPUT_PWM_MAX_DUTY_TICKS-RC_OUTPUT_PWM_MIN_DUTY_TICKS))

#endif /* INC_TYPES_HAL_RC_OUTPUT_TYPES_H_ */
