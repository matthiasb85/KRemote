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
 * nrf.c
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
#include "api/hal/nrf.h"

/*
 * Include dependencies
 */
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "api/app/config.h"
#include "api/app/config_entries.h"

/*
 * Static asserts
 */

/*
 * Forward declarations of static functions
 */
static void _nrf_init_hal(void);
static void _nrf_init_module(void);
static uint8_t _nrf_init_transceiver(void);
static void _nrf_flush_rx_tx_buffers(void);
static uint8_t _nrf_execute_command(nrf_commands_t cmd, uint8_t * tx_buf, uint8_t * rx_buf, uint8_t size);
static void _nrf_set_connection_state(nrf_connection_state_t state);
static uint8_t _nrf_config_get_mapping_table(config_entry_mapping_t * entry, config_mode_map_t ** map);
static void _nrf_irq_line_cb(void *arg);
static void _nrf_rx_timeout_cb(void *arg);
static uint8_t _nrf_set_config_cb(config_entry_mapping_t * entry, uint8_t idx, char * arg);
static char * _nrf_get_config_cb(config_entry_mapping_t * entry, uint8_t idx);

/*
 * Static variables
 */
static THD_WORKING_AREA(_nrf_event_stack, NRF_EVENT_THREAD_STACK);
static event_source_t _nrf_line_event;
static nrf_config_t * _nrf_config = NULL;
static uint8_t _nrf_tx_buffer[NRF_MAX_PACKAGE_LENGTH+1];
static uint8_t _nrf_rx_buffer[NRF_MAX_PACKAGE_LENGTH+1];
static uint8_t _nrf_payload[NRF_MAX_PACKAGE_LENGTH];
static nrf_connection_state_t _nrf_connection_state = NRF_CONNECTION_LOST;
static nrf_connection_state_t _nrf_rx_timeout = NRF_CONNECTION_LOST;
static virtual_timer_t _nrf_rx_timeout_vtp;
static void(*_nrf_connection_state_cb)(nrf_connection_state_t) = NULL;
static const SPIConfig _nrf_spi_cfg = {
  false,                        /* no circular mode */
  NULL,                         /* no finish cb     */
  NRF_INTERFACE_LINE_CSN,       /* chip select line */
  SPI_CR1_BR_1,                 /* mode 0, ~5-6MHz  */
  0
};
static const config_mode_map_t _nrf_config_opmode_map[] = {
    {"RX", NRF_MODE_RX},
    {"TX", NRF_MODE_TX},
};
static const config_mode_map_t _nrf_config_palevel_map[] = {
    {"MIN",  NRF_PA_MIN},
    {"LOW",  NRF_PA_LOW},
    {"HIGH", NRF_PA_HIGH},
};
static const config_mode_map_t _nrf_config_datarate_map[] = {
    {"1MBPS",   NRF_1MBPS},
    {"2MBPS",   NRF_2MBPS},
    {"250KBPS", NRF_250KBPS},
};
static const config_mode_map_t _nrf_config_addresswidth_map[] = {
    {"3", NRF_AW_3BYTE},
    {"4", NRF_AW_4BYTE},
    {"5", NRF_AW_5BYTE},
};

/*
 * Global variables
 */
event_source_t nrf_rx_event;

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_nrf_event_thread, arg)
{
  (void)arg;

  event_listener_t  event_listener;

  chRegSetThreadName("nrf_event");

  chEvtRegister(&_nrf_line_event, &event_listener, NRF_EVENT_LINE_EVENT);

  while (true)
  {
    chEvtWaitAny(EVENT_MASK(NRF_EVENT_LINE_EVENT));
    uint8_t status = 0;
    nrf_reg_write_read_cmd_t * cmd = (nrf_reg_write_read_cmd_t *)_nrf_tx_buffer;

    status = _nrf_execute_command(NRF_NOP, _nrf_tx_buffer, _nrf_rx_buffer, 1);

    if(status & (1 << NRF_BIT_TX_DS))
    {
      _nrf_set_connection_state(NRF_CONNECTION_ESTABLISHED);
    }

    if(status & (1 << NRF_BIT_MASK_MAX_RT))
    {
      _nrf_set_connection_state(NRF_CONNECTION_LOST);
    }

    if(status & (1 << NRF_BIT_RX_DR))
    {
      _nrf_set_connection_state(NRF_CONNECTION_ESTABLISHED);
      _nrf_flush_rx_tx_buffers();
      status = _nrf_execute_command(NRF_READ_RX, _nrf_tx_buffer, _nrf_rx_buffer, sizeof(_nrf_rx_buffer));

      chSysLock();
      memcpy(_nrf_payload, &(_nrf_rx_buffer[1]), sizeof(_nrf_payload));
      chSysUnlock();

      chEvtBroadcast(&nrf_rx_event);
    }

    status = _nrf_execute_command(NRF_FLUSH_RX, _nrf_tx_buffer, _nrf_rx_buffer, 1);
    status = _nrf_execute_command(NRF_FLUSH_TX, _nrf_tx_buffer, _nrf_rx_buffer, 1);

    cmd->reg = NRF_REG_STATUS;
    cmd->value = status & ((1 << NRF_BIT_TX_DS) | (1 << NRF_BIT_MASK_MAX_RT) | (1 << NRF_BIT_RX_DR));
    status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);
  }
}


/*
 * Static helper functions
 */
static void _nrf_init_hal(void)
{
  palSetLineMode(NRF_INTERFACE_LINE_SCK, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetLineMode(NRF_INTERFACE_LINE_MOSI, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetLineMode(NRF_INTERFACE_LINE_MISO, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetLineMode(NRF_INTERFACE_LINE_CE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(NRF_INTERFACE_LINE_CSN, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(NRF_INTERFACE_LINE_IRQ, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(NRF_STATE_LED, PAL_MODE_INPUT_PULLUP);
}

static void _nrf_init_module(void)
{
  chEvtObjectInit(&_nrf_line_event);
  chEvtObjectInit(&nrf_rx_event);

  chThdCreateStatic(_nrf_event_stack, sizeof(_nrf_event_stack), _nrf_config->event_thread_prio,
                    _nrf_event_thread, NULL);

  _nrf_init_transceiver();
  _nrf_set_connection_state(NRF_CONNECTION_LOST);

  if(_nrf_config->mode == NRF_MODE_RX)
  {
      _nrf_rx_timeout = _nrf_config->rx_timeout + 1;
      _nrf_rx_timeout_cb(NULL);
  }

}

static uint8_t _nrf_init_transceiver(void)
{
  spiStart(NRF_SPI_DRIVER, &_nrf_spi_cfg);

  nrf_reg_write_read_cmd_t * cmd = (nrf_reg_write_read_cmd_t *)_nrf_tx_buffer;
  uint8_t status = 0;
  (void)status;

  _nrf_flush_rx_tx_buffers();

  palClearLine(NRF_INTERFACE_LINE_CE);
  NRF_PIN_WAIT(100);

  /* Set 1500uS timeout and 5 retries */
  cmd->reg = NRF_REG_SETUP_RETR;
  cmd->value = (5 << NRF_BIT_ARC) | (15 << NRF_BIT_ARD);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Set LNA and baudrate */
  cmd->reg = NRF_REG_RF_SETUP;
  cmd->value = (_nrf_config->datarate << NRF_BIT_RF_DR) | (_nrf_config->pa_level << NRF_BIT_RF_PWR);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* No features used */
  cmd->reg = NRF_REG_FEATURE;
  cmd->value = 0;
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* No dynamic payload*/
  cmd->reg = NRF_REG_DYNPD;
  cmd->value = 0;
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Enable auto-ack on all pipes */
  cmd->reg = NRF_REG_EN_AA;
  cmd->value = (1 << NRF_BIT_ENAA_P0) | (1 << NRF_BIT_ENAA_P1) | (1 << NRF_BIT_ENAA_P2) |
      (1 << NRF_BIT_ENAA_P3) | (1 << NRF_BIT_ENAA_P4) | (1 << NRF_BIT_ENAA_P5);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Open RX pipes 0 & 1 */
  cmd->reg = NRF_REG_EN_RXADDR;
  cmd->value = (1 << NRF_BIT_ERX_P0) | (1 << NRF_BIT_ERX_P1);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Set payload size to 32 (PW_P0 & PW_P1*/
  cmd->reg = NRF_REG_RX_PW_P0;
  cmd->value = 32;
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);
  cmd->reg = NRF_REG_RX_PW_P1;
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Set addresss width */
  cmd->reg = NRF_REG_SETUP_AW;
  cmd->value = (_nrf_config->address_width << NRF_BIT_AW);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Set rf channel */
  cmd->reg = NRF_REG_RF_CH;
  cmd->value = (_nrf_config->channel & 0x7f);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Write address to TX and RX register*/
  cmd->reg = NRF_REG_TX_ADDR;
  memcpy(&(cmd->value),_nrf_config->address,_nrf_config->address_width + 2);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, _nrf_config->address_width + 2 + 1);
  cmd->reg = NRF_REG_RX_ADDR_P0;
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, _nrf_config->address_width + 2 + 1);

  /* Reset flags in status regiser*/
  cmd->reg = NRF_REG_STATUS;
  cmd->value = (1 << NRF_BIT_MASK_MAX_RT) | (1 << NRF_BIT_TX_DS) | (1 << NRF_BIT_RX_DR);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Flush buffers */
  status = _nrf_execute_command(NRF_FLUSH_RX, _nrf_tx_buffer, _nrf_rx_buffer, 1);
  status = _nrf_execute_command(NRF_FLUSH_TX, _nrf_tx_buffer, _nrf_rx_buffer, 1);

  /* Write config register*/
  cmd->reg = NRF_REG_CONFIG;
  cmd->value = (1 << NRF_BIT_EN_CRC) | (1 << NRF_BIT_CRCO) | (_nrf_config->mode << NRF_BIT_PRIM_RX);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  /* Enable line event handling for IRQ pin */
  palSetLineCallback(NRF_INTERFACE_LINE_IRQ, _nrf_irq_line_cb, NULL);
  palEnableLineEvent(NRF_INTERFACE_LINE_IRQ, PAL_EVENT_MODE_FALLING_EDGE);

  /* Power up*/
  cmd->reg = NRF_REG_CONFIG;
  status = _nrf_execute_command(NRF_READ_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);
  cmd->value = _nrf_rx_buffer[1] | (1 << NRF_BIT_PWR_UP);
  status = _nrf_execute_command(NRF_WRITE_REG, _nrf_tx_buffer, _nrf_rx_buffer, 2);

  chThdSleepMilliseconds(5);

  status = _nrf_execute_command(NRF_NOP, _nrf_tx_buffer, _nrf_rx_buffer, 1);

  palSetLine(NRF_INTERFACE_LINE_CE);
  NRF_PIN_WAIT(1000);

  return status;
}

static uint8_t _nrf_execute_command(nrf_commands_t cmd, uint8_t * tx_buf, uint8_t * rx_buf, uint8_t size)
{

  switch(cmd)
  {
    case NRF_READ_REG:          tx_buf[0] &= 0x1f; break;
    case NRF_WRITE_REG:         tx_buf[0] |= 0x20; break;
    case NRF_READ_RX:           tx_buf[0] =  0x61; break;
    case NRF_WRITE_TX:          tx_buf[0] =  0xa0; break;
    case NRF_FLUSH_TX:          tx_buf[0] =  0xe1; break;
    case NRF_FLUSH_RX:          tx_buf[0] =  0xe2; break;
    case NRF_REUSE_TX_PL:       tx_buf[0] =  0xe3; break;
    case NRF_READ_RX_LEN:       tx_buf[0] =  0x60; break;
    case NRF_WRITE_ACK_PAYLOAD: tx_buf[0] |= 0xa8; break;
    case NRF_WRITE_TX_NO_ACK:   tx_buf[0] =  0xb0; break;
    case NRF_NOP:               tx_buf[0] =  0xff; break;
    default:                    return 0;
  }
  spiSelect(NRF_SPI_DRIVER);
  spiExchange(NRF_SPI_DRIVER, size, tx_buf, rx_buf);
  spiUnselect(NRF_SPI_DRIVER);
  return rx_buf[0];
}

static void _nrf_flush_rx_tx_buffers(void)
{
  memset(_nrf_rx_buffer, 0, sizeof(_nrf_rx_buffer));
  memset(_nrf_tx_buffer, 0, sizeof(_nrf_tx_buffer));
}

static void _nrf_set_connection_state(nrf_connection_state_t state)
{
  chSysLockFromISR();
  _nrf_connection_state = state;
  chSysUnlockFromISR();
  switch(state)
  {
    case NRF_CONNECTION_LOST:
      palClearLine(NRF_STATE_LED);
      break;
    case NRF_CONNECTION_ESTABLISHED:
      chSysLockFromISR();
      _nrf_rx_timeout = _nrf_config->rx_timeout;
      chSysUnlockFromISR();
      palSetLine(NRF_STATE_LED);
      break;
  }
  if(_nrf_connection_state_cb)
  {
    _nrf_connection_state_cb(state);
  }
}

static uint8_t _nrf_config_get_mapping_table(config_entry_mapping_t * entry, config_mode_map_t ** map)
{
  uint8_t map_len = 0;
  if(strcmp(entry->name, "nrf-mode") == 0)    { *map = (config_mode_map_t *)_nrf_config_opmode_map; map_len = sizeof(_nrf_config_opmode_map); }
  else if(strcmp(entry->name, "nrf-pa") == 0) { *map = (config_mode_map_t *)_nrf_config_palevel_map; map_len = sizeof(_nrf_config_palevel_map); }
  else if(strcmp(entry->name, "nrf-dr") == 0) { *map = (config_mode_map_t *)_nrf_config_datarate_map; map_len = sizeof(_nrf_config_datarate_map); }
  else if(strcmp(entry->name, "nrf-aw") == 0) { *map = (config_mode_map_t *)_nrf_config_addresswidth_map; map_len = sizeof(_nrf_config_addresswidth_map); }
  else return 0;
  return map_len/sizeof(config_mode_map_t);
}

/*
 * Callback functions
 */
static void _nrf_irq_line_cb(void *arg)
{
  (void)arg;
  chEvtBroadcastI(&_nrf_line_event);
}

static void _nrf_rx_timeout_cb(void *arg)
{
  (void)arg;
  chVTSetI(&_nrf_rx_timeout_vtp,
           TIME_MS2I(10),
           _nrf_rx_timeout_cb,
           (void *)(NULL));
  chSysLockFromISR();

  if(_nrf_connection_state)
  {
    _nrf_rx_timeout--;

    chSysUnlockFromISR();
    if(_nrf_rx_timeout == 0)
    {
      _nrf_set_connection_state(NRF_CONNECTION_LOST);
    }
  }
}

static uint8_t _nrf_set_config_cb(config_entry_mapping_t * entry, uint8_t idx, char * arg)
{
  config_mode_map_t * map = NULL;
  uint8_t map_len = _nrf_config_get_mapping_table(entry, &map);
  uint32_t * value = NULL;

  if(strcmp(entry->name, "nrf-mode") == 0)    { value = (uint32_t *)(&((nrf_mode_t *)(entry->payload))[idx]); }
  else if(strcmp(entry->name, "nrf-pa") == 0) { value = (uint32_t *)(&((nrf_pa_dbm_t *)(entry->payload))[idx]); }
  else if(strcmp(entry->name, "nrf-dr") == 0) { value = (uint32_t *)(&((nrf_datarate_t *)(entry->payload))[idx]); }
  else if(strcmp(entry->name, "nrf-aw") == 0) { value = (uint32_t *)(&((nrf_addr_width_t *)(entry->payload))[idx]); }
  else return 0;

  return config_map_str_to_value(arg, value, map, map_len, CONFIG_UINT8);
}

static char * _nrf_get_config_cb(config_entry_mapping_t * entry, uint8_t idx)
{
  config_mode_map_t * map = NULL;
  uint8_t map_len = _nrf_config_get_mapping_table(entry, &map);
  char *str = NULL;

  uint32_t value = 0;

  if(strcmp(entry->name, "nrf-mode") == 0)    { value = (uint32_t)((nrf_mode_t *)(entry->payload))[idx]; }
  else if(strcmp(entry->name, "nrf-pa") == 0) { value = (uint32_t)((nrf_pa_dbm_t *)(entry->payload))[idx]; }
  else if(strcmp(entry->name, "nrf-dr") == 0) { value = (uint32_t)((nrf_datarate_t *)(entry->payload))[idx]; }
  else if(strcmp(entry->name, "nrf-aw") == 0) { value = (uint32_t)((nrf_addr_width_t *)(entry->payload))[idx]; }
  else return NULL;

  config_map_value_to_str(value, &str, map, map_len);

  return str;
}

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void nrf_foo_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0)
  {
    chprintf(chp, "Usage: nrf-foo\r\n");
    return;
  }

  systime_t time = 0;


  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
  {
      time = chVTGetSystemTimeX();

      chprintf(chp, "\r");
      chThdSleepUntilWindowed(time, time + TIME_MS2I(1000));
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}
#endif

/*
 * API functions
 */
void nrf_init(void)
{
  _nrf_config = (nrf_config_t *)config_get_module_config(CONFIG_ENTRY_NRF);
  chThdSleep(TIME_MS2I(_nrf_config->boot_timeout));
  _nrf_init_hal();
  _nrf_init_module();
}

void nrf_receive_payload(void * dest, uint8_t size)
{
  if(size > NRF_MAX_PACKAGE_LENGTH) return;

  chSysLock();
  memcpy(dest,_nrf_payload, size);
  chSysUnlock();
}

uint8_t nrf_send_payload(void * src, uint8_t size)
{
  if(size > NRF_MAX_PACKAGE_LENGTH) return 0;

  _nrf_flush_rx_tx_buffers();
  _nrf_execute_command(NRF_NOP, _nrf_tx_buffer, _nrf_rx_buffer, 1);

  memcpy(&(_nrf_tx_buffer[1]), src , size);
  uint8_t status = _nrf_execute_command(NRF_WRITE_TX, _nrf_tx_buffer, _nrf_rx_buffer, size+1);

  return status;
}

nrf_connection_state_t nrf_get_connection_state(void)
{
  chSysLock();
  nrf_connection_state_t ret = _nrf_connection_state;
  chSysUnlock();
  return ret;
}

void nrf_register_connection_state_change_callback(void(*cb)(nrf_connection_state_t))
{
  chSysLock();
  _nrf_connection_state_cb = cb;
  chSysUnlock();
}

void nrf_parse_config(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry)
{
  config_mode_map_t * map = NULL;
  uint8_t map_len = _nrf_config_get_mapping_table(entry, &map);

  config_parse_array_map(chp, argc, argv, entry, 1, _nrf_set_config_cb,map,map_len);
}

void nrf_print_config(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t print_help)
{
  config_print_array_map(chp, entry, 1, _nrf_get_config_cb, print_help);
}

void nrf_parse_config_ad(BaseSequentialStream * chp, int argc, char ** argv, config_entry_mapping_t * entry)
{
  config_parse_array(chp, argc, argv, entry, CONFIG_UINT8,  _nrf_config->address_width + 2);
}

void nrf_print_config_ad(BaseSequentialStream * chp, config_entry_mapping_t * entry, uint8_t print_help)
{
  config_print_array(chp,entry, CONFIG_UINT8,  _nrf_config->address_width + 2, CONFIG_HEX, print_help);
}
