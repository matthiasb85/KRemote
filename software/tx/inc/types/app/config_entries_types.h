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
 * config_entries_types.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_APP_CONFIG_ENTRIES_TYPES_H_
#define INC_TYPES_APP_CONFIG_ENTRIES_TYPES_H_

#include "api/app/kr_tx.h"
#include "api/hal/rc_input.h"
#include "api/hal/flash_storage.h"
#include "api/hal/nrf.h"
#include "api/hal/usb.h"

typedef enum
{
  CONFIG_ENTRY_KR_TX = 0,
  CONFIG_ENTRY_RC_INPUT,
  CONFIG_ENTRY_NRF,
  CONFIG_ENTRY_USB
}config_entries_lookup_t;

typedef struct
{
  flash_storage_header_t header;
  kr_tx_config_t kr_tx;
  rc_input_config_t rc_input;
  nrf_config_t nrf;
  uint32_t usb;
}config_entries_t;

#endif /* INC_TYPES_APP_CONFIG_ENTRIES_TYPES_H_ */
