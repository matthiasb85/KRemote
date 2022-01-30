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
  NRF_CONNECTION_LOST = 0,
  NRF_CONNECTION_ESTABLISHED
}nrf_connection_state_t;

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
    NRF_AW_3BYTE = 1,
    NRF_AW_4BYTE,
    NRF_AW_5BYTE
} nrf_addr_width_t;

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

typedef struct
{
  uint8_t reg;
  uint8_t value;
} nrf_reg_write_read_cmd_t;

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
  NRF_REG_DUMMY,
  NRF_REG_DYNPD,
  NRF_REG_FEATURE
}nrf_registers_t;


typedef struct
{
  nrf_mode_t mode;
  nrf_pa_dbm_t pa_level;
  nrf_datarate_t datarate;
  uint8_t channel;
  nrf_addr_width_t address_width;
  uint8_t address[NRF_MAX_ADDR_LENGTH];
  uint32_t event_thread_prio;
  uint16_t rx_timeout;
  uint32_t boot_timeout;
}nrf_config_t;

typedef struct
{
  const char *const name;
  uint32_t mode;
}nrf_config_mode_map_t;

#define NRF_PIN_WAIT(X)     { \
                              uint16_t i = 0; \
                              for(i=0; i < (X); i++) \
                                    asm volatile("nop"); \
                            } \


#define NRF_BIT_MASK_RX_DR  6
#define NRF_BIT_MASK_TX_DS  5
#define NRF_BIT_MASK_MAX_RT 4
#define NRF_BIT_EN_CRC      3
#define NRF_BIT_CRCO        2
#define NRF_BIT_PWR_UP      1
#define NRF_BIT_PRIM_RX     0
#define NRF_BIT_ENAA_P5     5
#define NRF_BIT_ENAA_P4     4
#define NRF_BIT_ENAA_P3     3
#define NRF_BIT_ENAA_P2     2
#define NRF_BIT_ENAA_P1     1
#define NRF_BIT_ENAA_P0     0
#define NRF_BIT_ERX_P5      5
#define NRF_BIT_ERX_P4      4
#define NRF_BIT_ERX_P3      3
#define NRF_BIT_ERX_P2      2
#define NRF_BIT_ERX_P1      1
#define NRF_BIT_ERX_P0      0
#define NRF_BIT_AW          0
#define NRF_BIT_ARD         4
#define NRF_BIT_ARC         0
#define NRF_BIT_PLL_LOCK    4
#define NRF_BIT_CONT_WAVE   7
#define NRF_BIT_RF_DR       3
#define NRF_BIT_RF_PWR      1
#define NRF_BIT_RX_DR       6
#define NRF_BIT_TX_DS       5
#define NRF_BIT_MAX_RT      4
#define NRF_BIT_RX_P_NO     1
#define NRF_BIT_TX_FULL     0
#define NRF_BIT_PLOS_CNT    4
#define NRF_BIT_ARC_CNT     0
#define NRF_BIT_TX_REUSE    6
#define NRF_BIT_FIFO_FULL   5
#define NRF_BIT_TX_EMPTY    4
#define NRF_BIT_RX_FULL     1
#define NRF_BIT_RX_EMPTY    0
#define NRF_BIT_DPL_P5      5
#define NRF_BIT_DPL_P4      4
#define NRF_BIT_DPL_P3      3
#define NRF_BIT_DPL_P2      2
#define NRF_BIT_DPL_P1      1
#define NRF_BIT_DPL_P0      0
#define NRF_BIT_EN_DPL      2
#define NRF_BIT_EN_ACK_PAY  1
#define NRF_BIT_EN_DYN_ACK  0

#define NRF_CONFIG_COMMON_DEFAULTS \
    .pa_level = NRF_DEFAULT_PA, \
    .datarate = NRF_DEFAULT_DA, \
    .channel = NRF_DEFAULT_CH, \
    .address_width = NRF_DEFAULT_AW, \
    .address = NRF_DEFAULT_AD, \
    .event_thread_prio = NRF_EVENT_THREAD_PRIO, \
    .rx_timeout = NRF_DEFAULT_RXT , \
    .boot_timeout = 0

#define NRF_CONFIG_MAP_ENTRIES \
    { .name = "nrf-mode", .parse = nrf_parse_config, .print = nrf_print_config,  .payload = &_config_entries_config.nrf.mode, .help ="nrf operation mode"}, \
    { .name = "nrf-pa",   .parse = nrf_parse_config, .print = nrf_print_config,  .payload = &_config_entries_config.nrf.pa_level, .help ="nrf PA level"}, \
    { .name = "nrf-dr",   .parse = nrf_parse_config, .print = nrf_print_config,  .payload = &_config_entries_config.nrf.datarate, .help ="nrf datarate"}, \
    { .name = "nrf-aw",   .parse = nrf_parse_config, .print = nrf_print_config,  .payload = &_config_entries_config.nrf.address_width, .help ="nrf address width"}, \
    { .name = "nrf-ch",   .parse = CONFIG_PARSE_FUNC(uint8_t), .print = CONFIG_PRINT_FUNC(dec,uint8_t), .payload = &_config_entries_config.nrf.channel, .help ="nrf channel"}, \
    { .name = "nrf-rxt",  .parse = CONFIG_PARSE_FUNC(uint16_t), .print = CONFIG_PRINT_FUNC(dec,uint16_t), .payload = &_config_entries_config.nrf.rx_timeout,    .help ="nrf RX timeout *10ms"}, \
    { .name = "nrf-ad",   .parse = nrf_parse_config_ad,         .print = nrf_print_config_ad,             .payload = &_config_entries_config.nrf.address,       .help ="nrf address"}, \
    { .name = "nrf-bt",   .parse = CONFIG_PARSE_FUNC(uint32_t), .print = CONFIG_PRINT_FUNC(dec,uint32_t), .payload = &_config_entries_config.nrf.boot_timeout,  .help ="nrf boot timeout"}


#endif /* COMMON_INC_TYPES_HAL_NRF_TYPES_H_ */
