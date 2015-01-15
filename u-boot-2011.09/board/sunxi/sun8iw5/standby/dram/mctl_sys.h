/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * CPL <cplanxy@allwinnertech.com>
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

#ifndef MCTL_SYS_H_
#define MCTL_SYS_H_

extern int dram_power_save_process(void);
extern int dram_power_up_process(void);
extern void mctl_self_refresh_entry(void);
extern void mctl_self_refresh_exit(void);
//extern int soft_switch_freq(__dram_para_t *dram_para);
//extern void mctl_deep_sleep_entry(__dram_para_t *para);

#endif
