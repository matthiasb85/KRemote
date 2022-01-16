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
 * config.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_API_APP_CONFIG_H_
#define INC_API_APP_CONFIG_H_

#include "cfg/app/config_cfg.h"
#include "types/app/config_types.h"

extern void config_init(void);
extern void * config_get_module_config(uint32_t id);

extern CONFIG_PARSE_IF(int8_t);
extern CONFIG_PARSE_IF(int16_t);
extern CONFIG_PARSE_IF(int32_t);
extern CONFIG_PARSE_IF(int64_t);
extern CONFIG_PARSE_IF(uint8_t);
extern CONFIG_PARSE_IF(uint16_t);
extern CONFIG_PARSE_IF(uint32_t);
extern CONFIG_PARSE_IF(uint64_t);

extern CONFIG_PRINT_IF(dec,int8_t);
extern CONFIG_PRINT_IF(dec,int16_t);
extern CONFIG_PRINT_IF(dec,int32_t);
extern CONFIG_PRINT_IF(dec,int64_t);
extern CONFIG_PRINT_IF(dec,uint8_t);
extern CONFIG_PRINT_IF(dec,uint16_t);
extern CONFIG_PRINT_IF(dec,uint32_t);
extern CONFIG_PRINT_IF(dec,uint64_t);
extern CONFIG_PRINT_IF(hex,uint8_t);
extern CONFIG_PRINT_IF(hex,uint16_t);
extern CONFIG_PRINT_IF(hex,uint32_t);
extern CONFIG_PRINT_IF(hex,uint64_t);

#endif /* INC_API_APP_CONFIG_H_ */
