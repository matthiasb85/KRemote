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
 *      Author: matti
 */

#ifndef INC_API_APP_CONFIG_H_
#define INC_API_APP_CONFIG_H_

#include "cfg/app/config_cfg.h"
#include "types/app/config_types.h"

extern void config_init(void);
extern void * config_get_module_config(uint32_t id);

extern void config_parse_s08(char * value, config_entry_mapping_t * entry);
extern void config_parse_s16(char * value, config_entry_mapping_t * entry);
extern void config_parse_s32(char * value, config_entry_mapping_t * entry);
extern void config_parse_s64(char * value, config_entry_mapping_t * entry);
extern void config_parse_u08(char * value, config_entry_mapping_t * entry);
extern void config_parse_u16(char * value, config_entry_mapping_t * entry);
extern void config_parse_u32(char * value, config_entry_mapping_t * entry);
extern void config_parse_u64(char * value, config_entry_mapping_t * entry);

extern void config_print_ds08(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_ds16(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_ds32(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_ds64(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_du08(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_du16(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_du32(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_du64(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_x08(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_x16(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_x32(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);
extern void config_print_x64(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry);

#endif /* INC_API_APP_CONFIG_H_ */
