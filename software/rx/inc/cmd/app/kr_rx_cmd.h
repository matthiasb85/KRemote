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
 * kr_rx_cmd.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_CMD_APP_KR_RX_CMD_H_
#define INC_CMD_APP_KR_RX_CMD_H_

#if defined(USE_CMD_SHELL)
/*
 * Global definition of shell commands
 * for module kr_rx
 */
//extern void kr_rx_foo_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module kr_rx
 */
// clang-format off
#define KR_RX_CMD_LIST \
            /*{"kr-rx-foo, kr_rx_foo_sh},*/

// clang-format on
#endif

#endif /* INC_CMD_APP_KR_RX_CMD_H_ */
