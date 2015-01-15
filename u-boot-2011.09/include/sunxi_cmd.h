/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef  _SUNXI_CMD_SYS_H
#define  _SUNXI_CMD_SYS_H

#include <command.h>

extern int do_boota (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_sunxi_flash(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);

extern int do_sunxi_display(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);
extern int do_sunxi_logo(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);

extern int do_fat_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern int do_fat_fsdown (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern int do_fat_ls (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern int do_fat_fsinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_save_user_data (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_bootelf (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_env_set(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern int do_boota (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_key_test (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_shutdown (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_sunxi_boot_signature(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);

extern int do_bootd (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

extern int do_burn_from_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#endif  /* _XILINX_H_ */
