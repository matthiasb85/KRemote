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
 * kr_rx_types.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_APP_KR_RX_TYPES_H_
#define INC_TYPES_APP_KR_RX_TYPES_H_

#include "api/app/kr.h"

typedef struct
{
  uint8_t channel_mapping[KR_CHANNEL_NUMBER];
  kr_ch_t channel_failsafe[KR_CHANNEL_NUMBER];
  uint32_t loop_cmd_period_ms;
}kr_rx_config_t;

#endif /* INC_TYPES_APP_KR_RX_TYPES_H_ */
