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
static void _nrf_init_transceiver(void);
void _nrf_flush_rx_tx_buffers(void);
static void _nrf_irq_line_cb(void *arg);

/*
 * Static variables
 */
static nrf_config_t * _nrf_config = NULL;
static uint8_t _nrf_tx_buffer[NRF_MAX_PACKAGE_LENGTH+1];
static uint8_t _nrf_rx_buffer[NRF_MAX_PACKAGE_LENGTH+1];
static uint8_t _nrf_payload[NRF_MAX_PACKAGE_LENGTH];
static const SPIConfig _nrf_spi_cfg = {
  false,                        /* no circular mode */
  NULL,                         /* no finish cb     */
  NRF_INTERFACE_LINE_CSN,       /* chip select line */
  SPI_CR1_BR_2,                 /* mode 0, ~8MHz    */
  0
};

/*
 * Global variables
 */

/*
 * Tasks
 */


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

  palSetLineCallback(NRF_INTERFACE_LINE_IRQ, _nrf_irq_line_cb, NULL);
}

static void _nrf_init_module(void)
{
  _nrf_init_transceiver();
}

static void _nrf_init_transceiver(void)
{
  spiStart(NRF_SPI_DRIVER, &_nrf_spi_cfg);
  palClearLine(NRF_INTERFACE_LINE_CE);
  // add init code
  palSetLine(NRF_INTERFACE_LINE_CE);
}

static void _nrf_execute_command(nrf_commands_t cmd, uint8_t * tx_buf, uint8_t * rx_buf, uint8_t size)
{

  switch(cmd)
  {
    case NRF_READ_REG:          tx_buf[0] &= 0x1f; break;
    case NRF_WRITE_REG:         tx_buf[0] |= 0x20; break;
    case NRF_READ_RX:           tx_buf[0] |= 0x61; break;
    case NRF_WRITE_TX:          tx_buf[0] |= 0xa0; break;
    case NRF_FLUSH_TX:          tx_buf[0] |= 0xe1; break;
    case NRF_FLUSH_RX:          tx_buf[0] |= 0xe2; break;
    case NRF_REUSE_TX_PL:       tx_buf[0] |= 0xe3; break;
    case NRF_READ_RX_LEN:       tx_buf[0] |= 0x60; break;
    case NRF_WRITE_ACK_PAYLOAD: tx_buf[0] |= 0xa8; break;
    case NRF_WRITE_TX_NO_ACK:   tx_buf[0] |= 0xb0; break;
    case NRF_NOP:               tx_buf[0] |= 0xff; break;
    default:                    return;
  }
  spiSelect(NRF_SPI_DRIVER);
  spiExchange(NRF_SPI_DRIVER, size, tx_buf, rx_buf);
  spiUnselect(NRF_SPI_DRIVER);
}

void _nrf_flush_rx_tx_buffers(void)
{
  memset(_nrf_rx_buffer, 0, sizeof(_nrf_rx_buffer));
  memset(_nrf_tx_buffer, 0, sizeof(_nrf_tx_buffer));
}

/*
 * Callback functions
 */
static void _nrf_irq_line_cb(void *arg)
{
  (void)arg;
  chSysLockFromISR();

  // do something

  chSysUnlockFromISR();
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

void nrf_send_payload(void * src, uint8_t size)
{
  if(size > NRF_MAX_PACKAGE_LENGTH) return;

  _nrf_flush_rx_tx_buffers();
  memcpy(&(_nrf_tx_buffer[1]), src , size);
  _nrf_execute_command(NRF_WRITE_TX, _nrf_tx_buffer, _nrf_rx_buffer, size+1);

}
