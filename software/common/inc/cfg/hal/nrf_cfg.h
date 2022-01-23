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
 * nrf_cfg.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef COMMON_INC_CFG_HAL_NRF_CFG_H_
#define COMMON_INC_CFG_HAL_NRF_CFG_H_

#define NRF_EVENT_THREAD_STACK 256
#define NRF_EVENT_THREAD_PRIO  (NORMALPRIO + 2)
#define NRF_EVENT_LINE_EVENT   0

#define NRF_ADDR_LENGTH        5
#define NRF_MAX_PACKAGE_LENGTH 32

#define NRF_INTERFACE_LINE_CE   PAL_LINE(GPIOA, 9U)
#define NRF_INTERFACE_LINE_IRQ  PAL_LINE(GPIOA, 8U)
#define NRF_INTERFACE_LINE_MOSI PAL_LINE(GPIOB, 15U)
#define NRF_INTERFACE_LINE_MISO PAL_LINE(GPIOB, 14U)
#define NRF_INTERFACE_LINE_SCK  PAL_LINE(GPIOB, 13U)
#define NRF_INTERFACE_LINE_CSN  PAL_LINE(GPIOB, 12U)

#define NRF_SPI_DRIVER (&SPID2)


#endif /* COMMON_INC_CFG_HAL_NRF_CFG_H_ */
