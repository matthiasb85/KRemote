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
 * rc_input_cmd.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_CMD_HAL_RC_INPUT_CMD_H_
#define INC_CMD_HAL_RC_INPUT_CMD_H_

#if defined(USE_CMD_SHELL)
/*
 * Global definition of shell commands
 * for module anykey
 */
//extern void rc_input_foo_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module rc_input
 */
// clang-format off
#define RC_INPUT_CMD_LIST \
            /*{"rc-input-foo, rc_input_foo_sh},*/

// clang-format on
#endif

#endif /* INC_CMD_HAL_RC_INPUT_CMD_H_ */
