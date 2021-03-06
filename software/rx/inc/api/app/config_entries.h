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
 * config_entries.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_API_APP_CONFIG_ENTRIES_H_
#define INC_API_APP_CONFIG_ENTRIES_H_

#include "cfg/app/config_entries_cfg.h"
#include "types/app/config_entries_types.h"
#include "api/app/config.h"

extern const config_control_t * config_entries_get_control_struct(void);

#endif /* INC_API_APP_CONFIG_ENTRIES_H_ */
