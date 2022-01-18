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
 * rc_input.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_API_HAL_RC_INPUT_H_
#define INC_API_HAL_RC_INPUT_H_

#include "cfg/hal/rc_input_cfg.h"
#include "types/hal/rc_input_types.h"
#include "api/app/config.h"

extern void rc_input_init(void);
extern void rc_input_get_channel_states(rc_input_ch_states_t *dest);
extern void rc_input_parse_dig_sm(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry);
extern void rc_input_print_dig_sm(BaseSequentialStream * chp, config_entry_mapping_t * entry);

#endif /* INC_API_HAL_RC_INPUT_H_ */
