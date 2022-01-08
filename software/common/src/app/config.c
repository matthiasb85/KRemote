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
 *      Author: matti
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

//#if defined(USE_CMD_SHELL)
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
  entry->print(chp, entry);
}

void config_set_entry_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc != 2)
  {
    chprintf(chp, "Usage:  config-set variable value\r\n");
    return;
  }
  config_entry_mapping_t * entry = _config_get_entry_by_name(argv[0]);
  if(entry == NULL)
  {
    chprintf(chp, "Cannot find entry for %s\r\n", argv[0]);
    return;
  }
  entry->parse(argv[1],entry);
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
    entry->print(chp, entry);
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
    chprintf(chp, "config-set");
    entry->print(chp, entry);
    entry++;
  }
}
//#endif

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
void config_parse_s08(char * value, config_entry_mapping_t * entry) { *((int8_t *)entry->payload)  = (int8_t)strtol(value, NULL, 0);}
void config_parse_s16(char * value, config_entry_mapping_t * entry) { *((int16_t *)entry->payload) = (int16_t)strtol(value, NULL, 0);}
void config_parse_s32(char * value, config_entry_mapping_t * entry) { *((int32_t *)entry->payload) = (int32_t)strtol(value, NULL, 0);}
void config_parse_s64(char * value, config_entry_mapping_t * entry) { *((int32_t *)entry->payload) = (int64_t)strtol(value, NULL, 0);}
void config_parse_u08(char * value, config_entry_mapping_t * entry) { *((uint8_t *)entry->payload)  = (uint8_t)strtol(value, NULL, 0);}
void config_parse_u16(char * value, config_entry_mapping_t * entry) { *((uint16_t *)entry->payload) = (uint16_t)strtoul(value, NULL, 0);}
void config_parse_u32(char * value, config_entry_mapping_t * entry) { *((uint32_t *)entry->payload) = (uint32_t)strtoul(value, NULL, 0);}
void config_parse_u64(char * value, config_entry_mapping_t * entry) { *((uint32_t *)entry->payload) = (uint64_t)strtoul(value, NULL, 0);}

/*
 * Generic print functions
 */
void config_print_ds08(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %16d\r\n",entry->name,*((int8_t *)entry->payload));}
void config_print_ds16(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %16d\r\n",entry->name,*((int16_t *)entry->payload));}
void config_print_ds32(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %16d\r\n",entry->name,*((int32_t *)entry->payload));}
void config_print_ds64(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %16d\r\n",entry->name,*((int64_t *)entry->payload));}
void config_print_du08(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %16d\r\n",entry->name,*((uint8_t *)entry->payload));}
void config_print_du16(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %16d\r\n",entry->name,*((uint16_t *)entry->payload));}
void config_print_du32(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %16d\r\n",entry->name,*((uint32_t *)entry->payload));}
void config_print_du64(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %16d\r\n",entry->name,*((uint64_t *)entry->payload));}
void config_print_x08(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %012s0x%02x\r\n",entry->name,"\0",*((uint8_t *)entry->payload));}
void config_print_x16(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %010s0x%04x\r\n",entry->name,"\0",*((uint16_t *)entry->payload));}
void config_print_x32(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s %06s0x%08x\r\n",entry->name,"\0",*((uint32_t *)entry->payload));}
void config_print_x64(BaseSequentialStream *chp, struct _config_entry_mapping_t *entry) { chprintf(chp, "  %-20s 0x%016x\r\n",entry->name,*((uint64_t *)entry->payload));}




