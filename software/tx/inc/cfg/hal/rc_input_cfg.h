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
 * rc_input_cfg.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_CFG_HAL_RC_INPUT_CFG_H_
#define INC_CFG_HAL_RC_INPUT_CFG_H_

#define RC_INPUT_POLL_THREAD_STACK     128
#define RC_INPUT_POLL_THREAD_PRIO      (NORMALPRIO + 1)
#define RC_INPUT_POLL_MAIN_THREAD_P_MS 10
#define RC_INPUT_LOOP_CMD_P_MS         10

#define RC_INPUT_DIG_SW_HIGH 1
#define RC_INPUT_DIG_SW_LOW  0

#define RC_INPUT_DIG_SW_COUNT 8
#define RC_INPUT_DIG_LINE_SW0 PAL_LINE(GPIOB, 2U)
#define RC_INPUT_DIG_LINE_SW1 PAL_LINE(GPIOB, 3U)
#define RC_INPUT_DIG_LINE_SW2 PAL_LINE(GPIOB, 4U)
#define RC_INPUT_DIG_LINE_SW3 PAL_LINE(GPIOB, 5U)
#define RC_INPUT_DIG_LINE_SW4 PAL_LINE(GPIOB, 6U)
#define RC_INPUT_DIG_LINE_SW5 PAL_LINE(GPIOB, 7U)
#define RC_INPUT_DIG_LINE_SW6 PAL_LINE(GPIOA, 10U)
#define RC_INPUT_DIG_LINE_SW7 PAL_LINE(GPIOB, 11U)

#define RC_INPUT_DIG_DEBOUNCE_TIME_MS 100

#define RC_INPUT_AN_IN_COUNT 8
#define RC_INPUT_AN_LINE_IN0 PAL_LINE(GPIOA, 0U)
#define RC_INPUT_AN_LINE_IN1 PAL_LINE(GPIOA, 1U)
#define RC_INPUT_AN_LINE_IN2 PAL_LINE(GPIOA, 2U)
#define RC_INPUT_AN_LINE_IN3 PAL_LINE(GPIOA, 3U)
#define RC_INPUT_AN_LINE_IN4 PAL_LINE(GPIOA, 4U)
#define RC_INPUT_AN_LINE_IN5 PAL_LINE(GPIOA, 5U)
#define RC_INPUT_AN_LINE_IN6 PAL_LINE(GPIOA, 6U)
#define RC_INPUT_AN_LINE_IN7 PAL_LINE(GPIOA, 7U)

/*
 * Derived configuration
 */
#define RC_INPUT_DIG_DEBOUNCE_TIME_TICKS (RC_INPUT_DIG_DEBOUNCE_TIME_MS / _rc_input_config->poll_thread_period_ms)

#endif /* INC_CFG_HAL_RC_INPUT_CFG_H_ */
