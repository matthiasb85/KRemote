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
 * kr_tx.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_API_APP_KR_TX_H_
#define INC_API_APP_KR_TX_H_

#include "cfg/app/kr_tx_cfg.h"
#include "types/app/kr_tx_types.h"

extern void kr_tx_init(void);
extern void kr_tx_parse_config(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry);
extern void kr_tx_print_config(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t print_help);
extern void kr_tx_parse_config_map(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry);
extern void kr_tx_print_config_map(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t print_help);

#endif /* INC_API_APP_KR_TX_H_ */
