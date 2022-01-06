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
 * flash_storage_types.h
 *
 *  Created on: 06.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_HAL_FLASH_STORAGE_TYPES_H_
#define INC_TYPES_HAL_FLASH_STORAGE_TYPES_H_


typedef uint32_t crc_t;

typedef struct
{
  crc_t crc;
  uint32_t version;
  uint32_t config_size;
} flash_storage_header_t;

#endif /* INC_TYPES_HAL_FLASH_STORAGE_TYPES_H_ */
