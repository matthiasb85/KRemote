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
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_APP_CONFIG_TYPES_H_
#define INC_TYPES_APP_CONFIG_TYPES_H_

#include "ch.h"
#include "hal.h"

typedef struct _config_entry_mapping_t
{
  const char *const name;
  void (*parse)(BaseSequentialStream *, char **,struct _config_entry_mapping_t *);
  void (*print)(BaseSequentialStream *, struct _config_entry_mapping_t *);
  void * payload;
  const char *const help;
}config_entry_mapping_t;

typedef struct
{
  uint8_t * config;
  config_entry_mapping_t * entry_mapping;
  void ** module_list;
}config_control_t;

#define CONFIG_SECTION_DIVIDER(X) .name = (X), .parse = NULL, .print = NULL, .payload = NULL, .help = "\0"

#define CONFIG_PARSE_FUNC(X)      config_parse_##X
#define CONFIG_PARSE_IF(X)        void CONFIG_PARSE_FUNC(X) (BaseSequentialStream * chp, char ** values, config_entry_mapping_t * entry)
#define CONFIG_PARSE_IMPL(X)      \
                                  CONFIG_PARSE_IF(X) \
                                  { \
                                    (void)chp; \
                                    *((X *)entry->payload)  = (X)strtol(values[1], NULL, 0); \
                                  }


#define _CONFIG_PRINT_FUNC(A,B)   A##B
#define CONFIG_PRINT_FUNC(X,Y)    _CONFIG_PRINT_FUNC(config_print_##X, Y)
#define CONFIG_PRINT_IF(X,Y)      void CONFIG_PRINT_FUNC(X,Y) (BaseSequentialStream * chp, config_entry_mapping_t * entry)
#define CONFIG_PRINT_IMPL(X,Y)    \
                                  CONFIG_PRINT_IF(X,Y) \
                                  { \
                                    chprintf(chp, "  %-20s %16d",entry->name,*((Y *)entry->payload)); \
                                  }

#endif /* INC_TYPES_APP_CONFIG_TYPES_H_ */
