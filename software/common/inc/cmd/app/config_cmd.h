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
 * config_cmd.h
 *
 *  Created on: 05.01.2022
 *      Author: matthiasb85
 */

#ifndef INC_CMD_APP_CONFIG_CMD_H_
#define INC_CMD_APP_CONFIG_CMD_H_

#if defined(USE_CMD_SHELL)
/*
 * Global definition of shell commands
 * for module config
 */
extern void config_load_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void config_store_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void config_get_entry_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void config_set_entry_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void config_show_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void config_export_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void config_invalidate_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module config
 */
// clang-format off
#define CONFIG_CMD_LIST \
            {"config-load",       config_load_sh}, \
            {"config-store",      config_store_sh}, \
            {"config-get",        config_get_entry_sh}, \
            {"config-set",        config_set_entry_sh}, \
            {"config-show",       config_show_sh}, \
            {"config-export",     config_export_sh}, \
            {"config-invalidate", config_invalidate_sh},

// clang-format on
#endif

#endif /* INC_CMD_APP_CONFIG_CMD_H_ */
