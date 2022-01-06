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
 * flash_storage.h
 *
 *  Created on: 06.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_API_HAL_FLASH_STORAGE_H_
#define INC_API_HAL_FLASH_STORAGE_H_

#include "cfg/hal/flash_storage_cfg.h"
#include "types/hal/flash_storage_types.h"

extern void flash_storage_init(void);
extern uint8_t * flash_storage_get_config_base_address(void);
extern void flash_storage_write_config(uint8_t *buffer, uint32_t size);
extern uint8_t flash_storage_check_integrity(void);

#endif /* INC_API_HAL_FLASH_STORAGE_H_ */
