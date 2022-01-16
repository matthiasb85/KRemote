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

#ifndef INC_TYPES_HAL_RC_INPUT_TYPES_H_
#define INC_TYPES_HAL_RC_INPUT_TYPES_H_

typedef enum
{
  RC_INPUT_CH0 = 0,
  RC_INPUT_CH1,
  RC_INPUT_CH2,
  RC_INPUT_CH3,
  RC_INPUT_CH4,
  RC_INPUT_CH5,
  RC_INPUT_CH6,
  RC_INPUT_CH7,
  RC_INPUT_CH8,
  RC_INPUT_CH9,
  RC_INPUT_CH10,
  RC_INPUT_CH11,
  RC_INPUT_CH12,
  RC_INPUT_CH13,
  RC_INPUT_CH14,
  RC_INPUT_CH15,
  RC_INPUT_MAX,
}rc_input_ch_t;

typedef enum
{
  RC_INPUT_AN_IN0 = 0,
  RC_INPUT_AN_IN1,
  RC_INPUT_AN_IN2,
  RC_INPUT_AN_IN3,
  RC_INPUT_AN_IN4,
  RC_INPUT_AN_IN5,
  RC_INPUT_AN_IN6,
  RC_INPUT_AN_IN7,
  RC_INPUT_AN_MAX
}rc_input_ch_an_t;

typedef enum
{
  RC_INPUT_DIG_IN0 = 0,
  RC_INPUT_DIG_IN1,
  RC_INPUT_DIG_IN2,
  RC_INPUT_DIG_IN3,
  RC_INPUT_DIG_IN4,
  RC_INPUT_DIG_IN5,
  RC_INPUT_DIG_IN6,
  RC_INPUT_DIG_IN7,
  RC_INPUT_DIG_MAX
}rc_input_ch_dig_t;


typedef enum
{
  RC_INPUT_CH_ANALOG = 0,
  RC_INPUT_CH_DIGITAL
}rc_input_ch_type_t;

typedef enum
{
  RC_INPUT_DIG_SW_OFF = 0,
  RC_INPUT_DIG_SW_ON
}rc_input_state_digital_t;

typedef uint16_t rc_input_state_analog_t;

typedef struct
{
  rc_input_ch_type_t type;
  union
  {
    rc_input_state_analog_t analog;
    rc_input_state_digital_t digital;
  }state;
}rc_input_ch_states_t;

typedef struct
{
  uint32_t line;
  rc_input_ch_t ch;
  uint32_t delay;
  enum
  {
    RC_INPUT_SW_STATE_INIT = 0,
    RC_INPUT_SW_STATE_PRESS
  } state;
} rc_input_ch_digital_t;

typedef struct
{
  uint32_t line;
  rc_input_ch_t ch;
} rc_input_ch_analog_t;

typedef struct
{
  uint32_t digital_switch_mode[RC_INPUT_DIG_MAX];
  uint32_t poll_thread_prio;
  uint32_t poll_thread_period_ms;
  uint32_t analog_conversion_period_ms;
  uint32_t analog_conversion_emph_old;
  uint32_t analog_conversion_emph_new;
  uint32_t loop_cmd_period_ms;
}rc_input_config_t;

#endif /* INC_TYPES_HAL_RC_INPUT_TYPES_H_ */
