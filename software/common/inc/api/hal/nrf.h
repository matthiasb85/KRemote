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
 * nrf.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef COMMON_INC_API_HAL_NRF_H_
#define COMMON_INC_API_HAL_NRF_H_

#include "cfg/hal/nrf_cfg.h"
#include "types/hal/nrf_types.h"
#include "api/app/config.h"

extern event_source_t nrf_rx_event;

extern void nrf_init(void);
extern void nrf_receive_payload(void * dest, uint8_t size);
extern uint8_t nrf_send_payload(void * src, uint8_t size);
extern nrf_connection_state_t nrf_get_connection_state(void);
extern void nrf_register_connection_state_change_callback(void(*cb)(nrf_connection_state_t));
extern void nrf_parse_config(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry);
extern void nrf_print_config(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t print_help);
extern void nrf_parse_config_ad(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry);
extern void nrf_print_config_ad(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t print_help);

#endif /* COMMON_INC_API_HAL_NRF_H_ */
