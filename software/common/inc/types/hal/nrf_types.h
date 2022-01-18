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
 * nrf_types.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef COMMON_INC_TYPES_HAL_NRF_TYPES_H_
#define COMMON_INC_TYPES_HAL_NRF_TYPES_H_

typedef enum
{
  NRF_MODE_TX = 0,
  NRF_MODE_RX
}nrf_mode_t;

typedef enum
{
    NRF_PA_MIN = 0,
    NRF_PA_LOW,
    NRF_PA_HIGH,
    NRF_PA_MAX,
} nrf_pa_dbm_t;

typedef enum
{
    NRF_1MBPS = 0,
    NRF_2MBPS,
    NRF_250KBPS
} nrf_datarate_t;

typedef enum
{
  NRF_READ_REG = 0,
  NRF_WRITE_REG,
  NRF_READ_RX,
  NRF_WRITE_TX,
  NRF_FLUSH_TX,
  NRF_FLUSH_RX,
  NRF_REUSE_TX_PL,
  NRF_READ_RX_LEN,
  NRF_WRITE_ACK_PAYLOAD,
  NRF_WRITE_TX_NO_ACK,
  NRF_NOP,
} nrf_commands_t;

typedef enum
{
  NRF_REG_CONFIG = 0,
  NRF_REG_EN_AA,
  NRF_REG_EN_RXADDR,
  NRF_REG_SETUP_AW,
  NRF_REG_SETUP_RETR,
  NRF_REG_RF_CH,
  NRF_REG_RF_SETUP,
  NRF_REG_STATUS,
  NRF_REG_OBSERVE_TX,
  NRF_REG_RPD,
  NRF_REG_RX_ADDR_P0,
  NRF_REG_RX_ADDR_P1,
  NRF_REG_RX_ADDR_P2,
  NRF_REG_RX_ADDR_P3,
  NRF_REG_RX_ADDR_P4,
  NRF_REG_RX_ADDR_P5,
  NRF_REG_TX_ADDR,
  NRF_REG_RX_PW_P0,
  NRF_REG_RX_PW_P1,
  NRF_REG_RX_PW_P2,
  NRF_REG_RX_PW_P3,
  NRF_REG_RX_PW_P4,
  NRF_REG_RX_PW_P5,
  NRF_REG_FIFO_STATUS,
  NRF_REG_NA_ACK_PLD,
  NRF_REG_NA_TX_PLD,
  NRF_REG_NA_RX_PLD,
  NRF_REG_DYNPD,
  NRF_REG_FEATURE
}nrf_registers_t;


typedef struct
{
  nrf_mode_t mode;
  nrf_pa_dbm_t pa_level;
  nrf_datarate_t datarate;
  uint8_t channel;
  uint8_t address[NRF_ADDR_LENGTH];
}nrf_config_t;


#endif /* COMMON_INC_TYPES_HAL_NRF_TYPES_H_ */
