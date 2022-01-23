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
 * rc_output_cmd.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_CMD_HAL_RC_OUTPUT_CMD_H_
#define INC_CMD_HAL_RC_OUTPUT_CMD_H_

#if defined(USE_CMD_SHELL)
/*
 * Global definition of shell commands
 * for module rc_output
 */
extern void rc_output_loop_channels_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void rc_output_set_channel_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module rc_output
 */
// clang-format off
#define RC_OUTPUT_CMD_LIST \
            {"rc-output-loop", rc_output_loop_channels_sh}, \
            {"rc-output-set",  rc_output_set_channel_sh},

// clang-format on
#endif

#endif /* INC_CMD_HAL_RC_OUTPUT_CMD_H_ */
