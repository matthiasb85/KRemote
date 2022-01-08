/*
 * This file is part of The KRemote Project
 *
 * Copyright (c) 2021 Matthias Beckert
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
 * flash_storage.c
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

/*
 * Include ChibiOS & HAL
 */
// clang-format off
#include "ch.h"
#include "hal.h"
#include "hal_community.h"
#include "chprintf.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/hal/flash_storage.h"
#include "api/hal/usb.h"

/*
 * Include dependencies
 */
#include <string.h>
#include <stddef.h>

/*
 * Forward declarations of static functions
 */
static void _flash_storage_init_hal(void);
static void _flash_storage_init_module(void);
static uint32_t _flash_storage_get_crc(uint8_t * buffer);

/*
 * Static variables
 */
static uint8_t *_flash_storage_area = NULL;

/*
 * Global variables
 */

extern uint32_t __flash1_base__;

/*
 * Tasks
 */

/*
 * Static helper functions
 */
static void _flash_storage_init_hal(void)
{
  /*
   * Initialize flash driver
   */
  eflStart(&FLASH_STORAGE_DRIVER_HANDLE, NULL);

  /*
   * Enable and initialize CRC driver.
   * The hardware module uses CRC-32 (Ethernet)
   * polynomial: 0x4C11DB7
   */
  rccEnableCRC(true);
  crcStart(&FLASH_STORAGE_CRC_HANDLE, NULL);
}

static void _flash_storage_init_module(void)
{
  /*
   * Initialize _flash_storage_area to flash1 section
   */
  _flash_storage_area = (uint8_t *)&__flash1_base__;

}

static uint32_t _flash_storage_get_crc(uint8_t * buffer)
{
  flash_storage_header_t *header = (flash_storage_header_t *)buffer;

  /*
   * Use hardware CRC module to calculate flash CRC
   */
  crcResetI(&FLASH_STORAGE_CRC_HANDLE);
  return (!header->config_size || header->config_size > FLASH_STORAGE_SIZE) ?
      0 :
      crcCalcI(&FLASH_STORAGE_CRC_HANDLE, header->config_size - sizeof(crc_t),
                  &buffer[sizeof(crc_t)]);
}

/*
 * Callback functions
 */

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void flash_storage_info_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;

  if (argc != 0)
  {
    chprintf(chp, "Usage: fs-info name\r\n");
    return;
  }
  flash_storage_header_t *header = (flash_storage_header_t *)_flash_storage_area;
  uint32_t crc = _flash_storage_get_crc(_flash_storage_area);
  if (header->crc != crc)
  {
    chprintf(chp, "Warning CRC missmatch!\r\nActual CRC of flash partition is 0x%08x\r\n", crc);
  }
  chprintf(chp, "Flash partition starts at 0x%08p with size of %d bytes\r\n\r\n", header,
           FLASH_STORAGE_SIZE);

  chprintf(chp, "CRC           0x%08x\r\n", header->crc);
  chprintf(chp, "Version         %8d\r\n",  header->version);
  chprintf(chp, "Config size     %8d\r\n",  header->config_size);

  chprintf(chp, "\r\n");
}

#endif

/*
 * API functions
 */
void flash_storage_init(void)
{
  _flash_storage_init_hal();
  _flash_storage_init_module();
}

uint8_t * flash_storage_get_config_base_address(void)
{
  return _flash_storage_area;
}

void flash_storage_write_config(uint8_t *buffer)
{
  flash_storage_header_t *header = (flash_storage_header_t *)buffer;

  uint16_t start_sector = FLASH_STORAGE_START_SECTOR;
  uint16_t i = 0;
  uint32_t size = header->config_size;

  const flash_descriptor_t *desc = efl_lld_get_descriptor(&FLASH_STORAGE_DRIVER_HANDLE);
  flash_offset_t flash_offset = (flash_offset_t)_flash_storage_area - (flash_offset_t)desc->address;

  /*
   * Erase all flash sectors in flash1 section
   */
  for (i = start_sector; i <= FLASH_STORAGE_LAST_SECTOR; i++)
  {
    uint32_t wait_time = 0;
    efl_lld_start_erase_sector(&FLASH_STORAGE_DRIVER_HANDLE, (flash_sector_t)i);
    efl_lld_query_erase(&FLASH_STORAGE_DRIVER_HANDLE, &wait_time);
    chThdSleep(TIME_MS2I(wait_time));
  }

  /*
   * Set crc
   */
  header->crc = _flash_storage_get_crc(buffer);

  /*
   * Write config
   */
  efl_lld_program(&FLASH_STORAGE_DRIVER_HANDLE, flash_offset, size,
                  (const uint8_t *)buffer);

}

uint8_t flash_storage_check_integrity(uint32_t version)
{
 flash_storage_header_t *header = (flash_storage_header_t *)_flash_storage_area;
 return (header->crc == _flash_storage_get_crc(_flash_storage_area) && header->version == version);
}
