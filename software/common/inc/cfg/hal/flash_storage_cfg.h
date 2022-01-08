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
 * flash_storage_cfg.h
 *
 *  Created on: 06.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_CFG_HAL_FLASH_STORAGE_CFG_H_
#define INC_CFG_HAL_FLASH_STORAGE_CFG_H_

#include "cfg/hal/linker_layout_cfg.h"

#define FLASH_STORAGE_SECTOR_SIZE    STM32_FLASH_SECTOR_SIZE
#define FLASH_STORAGE_SIZE           (LINKER_LAYOUT_FLASH1_SIZE)
#define FLASH_STORAGE_START_SECTOR   (LINKER_LAYOUT_CODE_SIZE / FLASH_STORAGE_SECTOR_SIZE)
#define FLASH_STORAGE_LAST_SECTOR    ((LINKER_LAYOUT_FLASH_SIZE / FLASH_STORAGE_SECTOR_SIZE) - 1)
#define FLASH_STORAGE_LINKER_SECTION ".flash1"
#define FLASH_STORAGE_DRIVER_HANDLE  EFLD1
#define FLASH_STORAGE_CRC_HANDLE     CRCD1

#endif /* INC_CFG_HAL_FLASH_STORAGE_CFG_H_ */
