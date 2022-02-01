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
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "api/app/config_entries.h"
#include "api/hal/flash_storage.h"

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

static config_entry_mapping_t * _config_get_entry_by_name(char * name)
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

static void _config_parse_array_entries(char *arg, uint8_t idx, uint8_t length, config_entry_mapping_t * entry, config_value_type_t type)
{
  uint8_t i = 0;
  for(i=idx; i<(idx+length); i++)
  {
    switch(type)
    {
      case CONFIG_UINT8:  ((uint8_t *) entry->payload)[i] = (uint8_t)strtol(arg, NULL, 0);  break;
      case CONFIG_UINT16: ((uint16_t *)entry->payload)[i] = (uint16_t)strtol(arg, NULL, 0); break;
      case CONFIG_UINT32: ((uint32_t *)entry->payload)[i] = (uint32_t)strtol(arg, NULL, 0); break;
      case CONFIG_UINT64: ((uint64_t *)entry->payload)[i] = (uint64_t)strtol(arg, NULL, 0); break;
      case CONFIG_INT8:   ((int8_t *)  entry->payload)[i] = (int8_t)strtol(arg, NULL, 0);   break;
      case CONFIG_INT16:  ((int16_t *) entry->payload)[i] = (int16_t)strtol(arg, NULL, 0);  break;
      case CONFIG_INT32:  ((int32_t *) entry->payload)[i] = (int32_t)strtol(arg, NULL, 0);  break;
      case CONFIG_INT64:  ((int64_t *) entry->payload)[i] = (int64_t)strtol(arg, NULL, 0);  break;
    }
  }
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

uint8_t config_map_str_to_value(char * str, uint32_t * dest, const config_mode_map_t * map, uint8_t map_len)
{
  uint8_t i = 0;
  for(i=0; i < map_len; i++)
  {
    if(strcmp(str, (char *)map[i].name)==0)
    {
      *dest = map[i].mode;
      return 1;
    }
  }
  return 0;
}

uint8_t config_map_value_to_str(uint32_t value, char ** dest, const config_mode_map_t * map, uint8_t map_len)
{
  uint8_t i = 0;
  for(i=0; i < map_len; i++)
  {
    if(value == map[i].mode)
    {
      *dest = (char *)map[i].name;
      return 1;
    }
  }
  return 0;
}

uint8_t config_parse_array(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry, config_value_type_t type, uint8_t length)
{
  uint8_t error = 0;
  uint8_t i = 0;

  if(argc != 3 && argc != length + 1)
  {
    error = 1;
  }

  if(!error)
  {
    if(argc == 3)
    {
      uint32_t idx = (uint16_t)strtol(argv[1], NULL, 0);
      if(idx < length)
      {
        _config_parse_array_entries(argv[2], idx, 1, entry, type);
      }
      else if(idx == length)
      {
        _config_parse_array_entries(argv[2], 0, length, entry, type);
      }
      else
      {
        error = 1;
      }
    }
    else
    {
      for(i=0; i<length; i++)
      {
        _config_parse_array_entries(argv[1+i], i, 1, entry, type);
      }
    }
  }

  if(error)
  {
    chprintf(chp, "Input does not match, use: [0..%d] value            (set single value)\r\n", length-1);
    chprintf(chp, "                           [%d] value               (set all values)\r\n", length);
    chprintf(chp, "                           value0 value1 ...value%d (set all value individual)\r\n", length-1);
    return 0;
  }

  return 1;
}

void config_print_array(BaseSequentialStream * chp, config_entry_mapping_t * entry, config_value_type_t type, uint8_t length, config_print_type_t fmt)
{
  const char *const fmt_str_dec = " %d";
  const char *const fmt_str_hex2 = " 0x%02x";
  const char *const fmt_str_hex4 = " 0x%04x";
  const char *const fmt_str_hex8 = " 0x%08x";
  const char *const fmt_str_hex16 = " 0x%016x";

  char * fmt_str = NULL;

  uint8_t i = 0;

  chprintf(chp, "  %-20s ", entry->name);
  switch(fmt)
  {
    case CONFIG_DEC: fmt_str = (char *)fmt_str_dec; break;
    case CONFIG_HEX:
      switch(type)
      {
        case CONFIG_UINT8:
        case CONFIG_INT8:
          fmt_str = (char *)fmt_str_hex2; break;
        case CONFIG_UINT16:
        case CONFIG_INT16:
          fmt_str = (char *)fmt_str_hex4; break;
        case CONFIG_UINT32:
        case CONFIG_INT32:
          fmt_str = (char *)fmt_str_hex8; break;
        case CONFIG_UINT64:
        case CONFIG_INT64:
          fmt_str = (char *)fmt_str_hex16; break;
      }
      break;
  }
  for(i=0; i<length; i++)
    {
      switch(type)
      {
        case CONFIG_UINT8:  chprintf(chp, (const char*)fmt_str, ((uint8_t *) entry->payload)[i]); break;
        case CONFIG_UINT16: chprintf(chp, (const char*)fmt_str, ((uint16_t *) entry->payload)[i]); break;
        case CONFIG_UINT32: chprintf(chp, (const char*)fmt_str, ((uint32_t *) entry->payload)[i]); break;
        case CONFIG_UINT64: chprintf(chp, (const char*)fmt_str, ((uint64_t *) entry->payload)[i]); break;
        case CONFIG_INT8:   chprintf(chp, (const char*)fmt_str, ((int8_t *) entry->payload)[i]); break;
        case CONFIG_INT16:  chprintf(chp, (const char*)fmt_str, ((int16_t *) entry->payload)[i]); break;
        case CONFIG_INT32:  chprintf(chp, (const char*)fmt_str, ((int32_t *) entry->payload)[i]); break;
        case CONFIG_INT64:  chprintf(chp, (const char*)fmt_str, ((int64_t *) entry->payload)[i]); break;
      }
    }
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
CONFIG_PRINT_IF(hex, uint32_t) { chprintf(chp, "  %-20s %06s0x%08x", entry->name,"\0",*((uint32_t *)entry->payload));}
CONFIG_PRINT_IF(hex, uint64_t) { chprintf(chp, "  %-20s 0x%016x",entry->name,*((uint64_t *)entry->payload));}
