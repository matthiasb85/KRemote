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
static config_entry_mapping_t * _config_get_entry_by_name(char * name);
static void _config_load_config(uint8_t * dest);
static uint8_t _config_parse_array_entries(char *arg, uint8_t idx, uint8_t length, config_entry_mapping_t * entry,
                                        config_value_type_t type, config_set_cb_t set_value_cb);
static uint8_t _config_parse_array_generic(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry,
                                           config_value_type_t type, uint8_t length, config_set_cb_t set_value_cb, config_mode_map_t * map, uint8_t map_len);
static void _config_print_array_generic(BaseSequentialStream * chp, config_entry_mapping_t * entry, config_value_type_t type, uint8_t length,
                                        config_print_type_t fmt, config_get_cb_t get_value_cb, uint8_t print_help);

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

static uint8_t _config_parse_array_entries(char *arg, uint8_t idx, uint8_t length, config_entry_mapping_t * entry,
                                        config_value_type_t type, config_set_cb_t set_value_cb)
{
  uint8_t i = 0;
  uint8_t ret = 1;
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
      case CONFIG_MAP:    ret = set_value_cb(entry, i, arg); break;
    }
  }
  return ret;
}

static uint8_t _config_parse_array_generic(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry,
                                           config_value_type_t type, uint8_t length, config_set_cb_t set_value_cb,
                                           config_mode_map_t * map, uint8_t map_len)
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
        error = (!_config_parse_array_entries(argv[2], idx, 1, entry, type, set_value_cb));
      }
      else if(idx == length)
      {
        error = (!_config_parse_array_entries(argv[2], 0, length, entry, type, set_value_cb));
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
        error = (!_config_parse_array_entries(argv[1+i], i, 1, entry, type, set_value_cb));
        if(error) break;
      }
    }
  }

  if(error)
  {
    chprintf(chp, "Input does not match, use: \r\n");
    if(argc==2)
    {
      chprintf(chp, " config-set %s value\r\n", entry->name,length-1);
    }
    else
    {
      chprintf(chp, " config-set %s [0..%d] value            (set single value)\r\n", entry->name,length-1);
      chprintf(chp, "               [%d] value               (set all values)\r\n", length);
      chprintf(chp, "               value0 value1 ...value%d (set all value individual)\r\n", length-1);
    }
    if(type==CONFIG_MAP)
    {
      chprintf(chp, " Valid values are: [");
      for(i=0; i<map_len; i++)
      {
        chprintf(chp, "%s|", (char *)map[i].name);
      }
      chprintf(chp, "]");
    }
    else
    {
      chprintf(chp, " Valid values are in range of: ");
      switch(type)
      {
        case CONFIG_UINT8:  chprintf(chp, "UINT8");  break;
        case CONFIG_UINT16: chprintf(chp, "UINT16");  break;
        case CONFIG_UINT32: chprintf(chp, "UINT32");  break;
        case CONFIG_UINT64: chprintf(chp, "UINT64");  break;
        case CONFIG_INT8:   chprintf(chp, "INT8");  break;
        case CONFIG_INT16:  chprintf(chp, "INT16");  break;
        case CONFIG_INT32:  chprintf(chp, "INT32");  break;
        case CONFIG_INT64:  chprintf(chp, "INT64");  break;
        default: break;
      }
    }
    chprintf(chp, "\r\n");
    return 0;
  }

  return 1;
}

static void _config_print_array_generic(BaseSequentialStream * chp, config_entry_mapping_t * entry, config_value_type_t type, uint8_t length,
                                        config_print_type_t fmt, config_get_cb_t get_value_cb, uint8_t print_help)
{
  const char *const fmt_str_dec = " %d";
  const char *const fmt_str_hex2 = " 0x%02x";
  const char *const fmt_str_hex4 = " 0x%04x";
  const char *const fmt_str_hex8 = " 0x%08x";
  const char *const fmt_str_hex16 = " 0x%016x";
  const char *const fmt_str_str = " %s";

  char * fmt_str = NULL;

  char buffer[CONFIG_PRINT_BUFFER_SIZE];
  memset(buffer, 0, CONFIG_PRINT_BUFFER_SIZE);

  uint8_t i = 0;

  switch(fmt)
  {
    case CONFIG_DEC:    fmt_str = (char *)fmt_str_dec; break;
    case CONFIG_STRING: fmt_str = (char *)fmt_str_str; break;
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
        default:
          return;
      }
      break;
  }
  for(i=0; i<length; i++)
  {
    uint8_t p_idx = strlen(buffer);
    uint8_t max_len = CONFIG_PRINT_BUFFER_SIZE - p_idx;
    switch(type)
    {
      case CONFIG_UINT8:  chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, ((uint8_t *) entry->payload)[i]); break;
      case CONFIG_UINT16: chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, ((uint16_t *) entry->payload)[i]); break;
      case CONFIG_UINT32: chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, ((uint32_t *) entry->payload)[i]); break;
      case CONFIG_UINT64: chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, ((uint64_t *) entry->payload)[i]); break;
      case CONFIG_INT8:   chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, ((int8_t *) entry->payload)[i]); break;
      case CONFIG_INT16:  chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, ((int16_t *) entry->payload)[i]); break;
      case CONFIG_INT32:  chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, ((int32_t *) entry->payload)[i]); break;
      case CONFIG_INT64:  chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, ((int64_t *) entry->payload)[i]); break;
      case CONFIG_MAP:    chsnprintf(&buffer[p_idx], max_len, (const char*)fmt_str, get_value_cb(entry, i)); break;
    }
  }
  chprintf(chp, "%-128s %s\r\n", buffer, (print_help) ? entry->help : "\0");
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
  if(entry->print)
  {
    chprintf(chp, " %s ",entry->name);
    entry->print(chp, entry, 0);
  }
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
      chprintf(chp, "  %-20s ",entry->name);
      entry->print(chp, entry, 1);
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
      chprintf(chp, " %s ",entry->name);
      entry->print(chp, entry, 0);
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

uint8_t config_map_str_to_value(char * str, void * dest, const config_mode_map_t * map, uint8_t map_len, config_value_type_t type)
{
  uint8_t i = 0;
  for(i=0; i < map_len; i++)
  {
    if(strcmp(str, (char *)map[i].name)==0)
    {
      switch(type)
      {
        case CONFIG_UINT8:  *((uint8_t *)dest)  = (uint8_t) map[i].mode;  break;
        case CONFIG_INT8:   *((int8_t *)dest)   = (int8_t) map[i].mode;   break;
        case CONFIG_UINT16: *((uint16_t *)dest) = (uint16_t) map[i].mode; break;
        case CONFIG_INT16:  *((int16_t *)dest)  = (int16_t) map[i].mode;  break;
        case CONFIG_UINT32: *((uint32_t *)dest) = (uint32_t) map[i].mode; break;
        case CONFIG_INT32:  *((int32_t *)dest)  = (int32_t) map[i].mode;  break;
        case CONFIG_UINT64: *((uint64_t *)dest) = (uint64_t) map[i].mode; break;
        case CONFIG_INT64:  *((int64_t *)dest)  = (int64_t) map[i].mode;  break;
        default:
          return 0;
      }
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
  return _config_parse_array_generic(chp, argc, argv, entry, type, length, NULL, NULL, 0);
}

void config_print_array(BaseSequentialStream * chp, config_entry_mapping_t * entry, config_value_type_t type, uint8_t length, config_print_type_t fmt, uint8_t print_help)
{
  _config_print_array_generic(chp, entry, type, length, fmt, NULL, print_help);
}

uint8_t config_parse_array_map(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry,
                               uint8_t length, config_set_cb_t set_value_cb, config_mode_map_t * map, uint8_t map_len)
{
  return _config_parse_array_generic(chp, argc, argv, entry, CONFIG_MAP, length, set_value_cb, map, map_len);
}

void config_print_array_map(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t length, config_get_cb_t get_value_cb, uint8_t print_help)
{
  _config_print_array_generic(chp, entry, CONFIG_MAP, length, CONFIG_STRING, get_value_cb, print_help);
}

/*
 * Generic parser functions
 */
CONFIG_PARSE_IF(int8_t)   { config_parse_array(chp, argc, argv, entry, CONFIG_INT8,   1); }
CONFIG_PARSE_IF(int16_t)  { config_parse_array(chp, argc, argv, entry, CONFIG_INT16,  1); }
CONFIG_PARSE_IF(int32_t)  { config_parse_array(chp, argc, argv, entry, CONFIG_INT32,  1); }
CONFIG_PARSE_IF(int64_t)  { config_parse_array(chp, argc, argv, entry, CONFIG_INT64,  1); }
CONFIG_PARSE_IF(uint8_t)  { config_parse_array(chp, argc, argv, entry, CONFIG_UINT8,  1); }
CONFIG_PARSE_IF(uint16_t) { config_parse_array(chp, argc, argv, entry, CONFIG_UINT16, 1); }
CONFIG_PARSE_IF(uint32_t) { config_parse_array(chp, argc, argv, entry, CONFIG_UINT32, 1); }
CONFIG_PARSE_IF(uint64_t) { config_parse_array(chp, argc, argv, entry, CONFIG_UINT64, 1); }

/*
 * Generic print functions
 */
CONFIG_PRINT_IF(dec, int8_t)   { config_print_array(chp,entry, CONFIG_INT8,   1, CONFIG_DEC, print_help); }
CONFIG_PRINT_IF(dec, int16_t)  { config_print_array(chp,entry, CONFIG_INT16,  1, CONFIG_DEC, print_help); }
CONFIG_PRINT_IF(dec, int32_t)  { config_print_array(chp,entry, CONFIG_INT32,  1, CONFIG_DEC, print_help); }
CONFIG_PRINT_IF(dec, int64_t)  { config_print_array(chp,entry, CONFIG_INT64,  1, CONFIG_DEC, print_help); }
CONFIG_PRINT_IF(dec, uint8_t)  { config_print_array(chp,entry, CONFIG_UINT8,  1, CONFIG_DEC, print_help); }
CONFIG_PRINT_IF(dec, uint16_t) { config_print_array(chp,entry, CONFIG_UINT16, 1, CONFIG_DEC, print_help); }
CONFIG_PRINT_IF(dec, uint32_t) { config_print_array(chp,entry, CONFIG_UINT32, 1, CONFIG_DEC, print_help); }
CONFIG_PRINT_IF(dec, uint64_t) { config_print_array(chp,entry, CONFIG_UINT64, 1, CONFIG_DEC, print_help); }
CONFIG_PRINT_IF(hex, uint8_t)  { config_print_array(chp,entry, CONFIG_UINT8,  1, CONFIG_HEX, print_help); }
CONFIG_PRINT_IF(hex, uint16_t) { config_print_array(chp,entry, CONFIG_UINT16, 1, CONFIG_HEX, print_help); }
CONFIG_PRINT_IF(hex, uint32_t) { config_print_array(chp,entry, CONFIG_UINT32, 1, CONFIG_HEX, print_help); }
CONFIG_PRINT_IF(hex, uint64_t) { config_print_array(chp,entry, CONFIG_UINT64, 1, CONFIG_HEX, print_help); }
