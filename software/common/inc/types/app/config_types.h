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
 * config_types.h
 *
 *  Created on: 05.01.2022
 *      Author: matti
 */

#ifndef INC_TYPES_APP_CONFIG_TYPES_H_
#define INC_TYPES_APP_CONFIG_TYPES_H_

#include "ch.h"
#include "hal.h"

typedef struct _config_entry_mapping_t
{
  const char *const name;
  void (*parse)(char *,struct _config_entry_mapping_t *);
  void (*print)(BaseSequentialStream *, struct _config_entry_mapping_t *);
  void * payload;
}config_entry_mapping_t;

typedef struct
{
  uint8_t * config;
  config_entry_mapping_t * entry_mapping;
  void ** module_list;
}config_control_t;

#endif /* INC_TYPES_APP_CONFIG_TYPES_H_ */
