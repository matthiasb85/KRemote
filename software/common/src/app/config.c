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
 * config.c
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
#include "chprintf.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/app/config.h"

/*
 * Include dependencies
 */
#include "api/app/config_entries.h"
#include "api/hal/flash_storage.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*
 * Static asserts
 */

/*
 * Forward declarations of static functions
 */
static void _config_init_hal(void);
static void _config_init_module(void);
static void _config_load_config(uint8_t * dest);
static config_entry_mapping_t * _config_get_entry_by_name(char * name);

/*
 * Static variables
 */
config_control_t * _config_control = NULL;

/*
 * Global variables
 */

/*
 * Tasks
 */

/*
 * Static helper functions
 */
static void _config_init_hal(void)
{
  // nothing to do
}

static void _config_init_module(void)
{
  _config_control = (config_control_t *)config_entries_get_control_struct();


  flash_storage_header_t *header = (flash_storage_header_t *)_config_control->config;
  if(!flash_storage_check_integrity(header->version))
  {
    /*
     * Flash corrupted, write default config
     */
    flash_storage_write_config(_config_control->config);
  }
  else
  {
    /*
     * Flash valid, load config
     */
      _config_load_config(_config_control->config);
  }
}

static void _config_load_config(uint8_t * dest)
{
  flash_storage_header_t *header = (flash_storage_header_t *)dest;
  memcpy(dest, flash_storage_get_config_base_address(), header->config_size);
}

config_entry_mapping_t * _config_get_entry_by_name(char * name)
{
  config_entry_mapping_t * entry = (config_entry_mapping_t *)&(_config_control->entry_mapping[0]);
  while(strlen(entry->name))
  {
    if(strcmp((char *)entry->name, name) == 0)
    {
      return entry;
    }
    else
    {
      entry++;
    }
  }
  return NULL;
}


/*
 * Callback functions
 */

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void config_load_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 0)
  {
    chprintf(chp, "Usage:  config-load %d\r\n",argc);
    return;
  }
  _config_load_config(_config_control->config);
  chprintf(chp, "Config loaded to 0x%08p\r\n\r\n", _config_control->config);
}

void config_store_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 0)
  {
    chprintf(chp, "Usage:  config-stored\r\n");
    return;
  }
  flash_storage_write_config(_config_control->config);
  chprintf(chp, "Config stored to flash\r\n\r\n");
}

void config_get_entry_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc != 1)
  {
    chprintf(chp, "Usage:  config-get variable\r\n");
    return;
  }
  config_entry_mapping_t * entry = _config_get_entry_by_name(argv[0]);
  if(entry == NULL)
  {
    chprintf(chp, "Cannot find entry for %s\r\n", argv[0]);
    return;
  }
  if(entry->print) entry->print(chp, entry);
}

void config_set_entry_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc < 2)
  {
    chprintf(chp, "Usage:  config-set variable value     (single value variables)\r\n");
    chprintf(chp, "        config-set variable idx value (array based variables)\r\n");
    chprintf(chp, "        config-set variable ... \r\n");
    return;
  }
  config_entry_mapping_t * entry = _config_get_entry_by_name(argv[0]);
  if(entry == NULL)
  {
    chprintf(chp, "Cannot find entry for %s\r\n", argv[0]);
    return;
  }
  if(entry->parse) entry->parse(chp, argc, argv, entry);
}

void config_show_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 0)
  {
    chprintf(chp, "Usage:  config-show\r\n");
    return;
  }
  config_entry_mapping_t * entry = (config_entry_mapping_t *)&(_config_control->entry_mapping[0]);
  while(strlen(entry->name))
  {
    if(entry->print)
    {
      entry->print(chp, entry);
      chprintf(chp, " %s\r\n",entry->help);
    }
    else
    {
      chprintf(chp, " %s\r\n",entry->name);
    }
    entry++;
  }
}

void config_export_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 0)
  {
    chprintf(chp, "Usage:  config-export\r\n");
    return;
  }
  config_entry_mapping_t * entry = (config_entry_mapping_t *)&(_config_control->entry_mapping[0]);
  while(strlen(entry->name))
  {
    if(entry->print)
    {
      chprintf(chp, "config-set");
      entry->print(chp, entry);
      chprintf(chp, "\r\n");
    }
    entry++;
  }
}

void config_invalidate_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 0)
  {
    chprintf(chp, "Usage:  config-invalidate\r\n");
    return;
  }
  flash_storage_header_t * header = (flash_storage_header_t *)_config_control->config;
  header->crc = 0xFFFFFFFF;

  flash_storage_write_config(_config_control->config);

  chprintf(chp, "Config invalidated and stored to flash.\r\n\r\n");
  chprintf(chp, "Default config will be loaded after restart\r\n\r\n");
}
#endif

/*
 * API functions
 */
void config_init(void)
{
  _config_init_hal();
  _config_init_module();
}

void * config_get_module_config(uint32_t id)
{
  return (void *)_config_control->module_list[id];
}

/*
 * Generic parser functions
 */
CONFIG_PARSE_IMPL(int8_t)
CONFIG_PARSE_IMPL(int16_t)
CONFIG_PARSE_IMPL(int32_t)
CONFIG_PARSE_IMPL(int64_t)
CONFIG_PARSE_IMPL(uint8_t)
CONFIG_PARSE_IMPL(uint16_t)
CONFIG_PARSE_IMPL(uint32_t)
CONFIG_PARSE_IMPL(uint64_t)

/*
 * Generic print functions
 */
CONFIG_PRINT_IMPL(dec, int8_t)
CONFIG_PRINT_IMPL(dec, int16_t)
CONFIG_PRINT_IMPL(dec, int32_t)
CONFIG_PRINT_IMPL(dec, int64_t)
CONFIG_PRINT_IMPL(dec, uint8_t)
CONFIG_PRINT_IMPL(dec, uint16_t)
CONFIG_PRINT_IMPL(dec, uint32_t)
CONFIG_PRINT_IMPL(dec, uint64_t)
CONFIG_PRINT_IF(hex, uint8_t)  { chprintf(chp, "  %-20s %012s0x%02x",entry->name,"\0",*((uint8_t *)entry->payload));}
CONFIG_PRINT_IF(hex, uint16_t) { chprintf(chp, "  %-20s %010s0x%04x",entry->name,"\0",*((uint16_t *)entry->payload));}
CONFIG_PRINT_IF(hex, uint32_t) { chprintf(chp, "  %-20s %010s0x%04x",entry->name,"\0",*((uint16_t *)entry->payload));}
CONFIG_PRINT_IF(hex, uint64_t) { chprintf(chp, "  %-20s 0x%016x",entry->name,*((uint64_t *)entry->payload));}
