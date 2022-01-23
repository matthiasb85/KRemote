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
 * rc_output.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_API_HAL_RC_OUTPUT_H_
#define INC_API_HAL_RC_OUTPUT_H_

#include "cfg/hal/rc_output_cfg.h"
#include "types/hal/rc_output_types.h"
#include "api/app/config.h"

extern void rc_output_init(void);
extern void rc_output_set(rc_output_ch_t ch, uint16_t value);

#endif /* INC_API_HAL_RC_OUTPUT_H_ */
