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
#ifndef  __STANDBY_H__
#define  __STANDBY_H__

extern  int standby_int_init(void);
extern  int standby_int_exit(void);
extern  int standby_int_query(void);

extern  int  standby_clock_store(void);
extern  int  standby_clock_restore(void);
extern  int  standby_clock_to_source(int clock_source);
extern  void standby_clock_plldisable(void);
extern  void standby_clock_divsetto0(void);
extern  void standby_clock_divsetback(void);
extern  void standby_clock_drampll_ouput(int op);

extern  void standby_clock_apb1_to_source(int clock);
extern  void standby_clock_24m_op(int op);

extern  void  boot_store_sp(void);
extern  void  boot_restore_sp(void);
extern  void  boot_set_sp(void);
extern  void  boot_halt(void);


extern  int  dram_power_save_process(void);
extern  int  dram_power_up_process(void);

extern  void standby_tmr_enable_watchdog(void);
extern  void standby_tmr_disable_watchdog(void);


#endif  // __STANDBY_I_H__


