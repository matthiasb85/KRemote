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
 * rc_output_cfg.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_CFG_HAL_RC_OUTPUT_CFG_H_
#define INC_CFG_HAL_RC_OUTPUT_CFG_H_

#define RC_OUTPUT_LOOP_CMD_P_MS         10

#define RC_OUTPUT_DIG_LINE0  PAL_LINE(GPIOA, 4U)
#define RC_OUTPUT_DIG_LINE1  PAL_LINE(GPIOA, 5U)
#define RC_OUTPUT_DIG_LINE2  PAL_LINE(GPIOA, 6U)
#define RC_OUTPUT_DIG_LINE3  PAL_LINE(GPIOA, 7U)
#define RC_OUTPUT_DIG_LINE4  PAL_LINE(GPIOB, 2U)
#define RC_OUTPUT_DIG_LINE5  PAL_LINE(GPIOB, 3U)
#define RC_OUTPUT_DIG_LINE6  PAL_LINE(GPIOB, 4U)
#define RC_OUTPUT_DIG_LINE7  PAL_LINE(GPIOB, 5U)
#define RC_OUTPUT_DIG_LINE8  PAL_LINE(GPIOB, 6U)
#define RC_OUTPUT_DIG_LINE9  PAL_LINE(GPIOB, 7U)
#define RC_OUTPUT_DIG_LINE10 PAL_LINE(GPIOB, 10U)
#define RC_OUTPUT_DIG_LINE11 PAL_LINE(GPIOB, 11U)

#define RC_OUTPUT_AN_LINE0 PAL_LINE(GPIOA, 0U)
#define RC_OUTPUT_AN_LINE1 PAL_LINE(GPIOA, 1U)
#define RC_OUTPUT_AN_LINE2 PAL_LINE(GPIOA, 2U)
#define RC_OUTPUT_AN_LINE3 PAL_LINE(GPIOA, 3U)

#define RC_OUTPUT_PWM_TIMER_FREQ      200000                  // 20KHz timer speed
#define RC_OUTPUT_PWM_P_MS            20                      // 20ms period
#define RC_OUTPUT_PWM_MIN_DUTY_US     1000
#define RC_OUTPUT_PWM_MAX_DUTY_US     2000
#define RC_OUTPUT_PWM_TIMER_DRIVER    (&PWMD2)
#define RC_OUTPUT_PWM_MAX_VALUE       10000

#endif /* INC_CFG_HAL_RC_OUTPUT_CFG_H_ */
