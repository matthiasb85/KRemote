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
 * kr_tx_types.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_APP_KR_TX_TYPES_H_
#define INC_TYPES_APP_KR_TX_TYPES_H_

#include "api/app/kr.h"
#include "api/hal/rc_input.h"

typedef enum
{
  KR_TX_MAP_DISABLED = 0,
  KR_TX_MAP_ANALOG,
  KR_TX_MAP_DIGITAL,
}kr_tx_mapping_type_t;

typedef enum
{
  KR_TX_COND_NONE = 0,
  KR_TX_COND_GE,
  KR_TX_COND_LE,
  KR_TX_COND_TGE,
  KR_TX_COND_TLE,
}kr_tx_condition_t;

typedef enum
{
  KR_TX_INV_FALSE = 0,
  KR_TX_INV_TRUE,
}kr_tx_direction_t;

typedef struct
{
  int16_t trim[RC_INPUT_MAX];
  uint16_t input_min[RC_INPUT_MAX];
  uint16_t input_max[RC_INPUT_MAX];
  kr_tx_mapping_type_t map_type[KR_CHANNEL_NUMBER];
  uint8_t map_channel[KR_CHANNEL_NUMBER];
  kr_tx_direction_t map_inverted[KR_CHANNEL_NUMBER];
  kr_ch_t map_output_min[KR_CHANNEL_NUMBER];
  kr_ch_t map_output_max[KR_CHANNEL_NUMBER];
  kr_tx_condition_t map_condition[KR_CHANNEL_NUMBER];
  uint16_t map_threshold[KR_CHANNEL_NUMBER];
  uint32_t main_thread_prio;
  uint32_t main_thread_period_ms;
  uint32_t loop_cmd_period_ms;
}kr_tx_config_t;

#endif /* INC_TYPES_APP_KR_TX_TYPES_H_ */
