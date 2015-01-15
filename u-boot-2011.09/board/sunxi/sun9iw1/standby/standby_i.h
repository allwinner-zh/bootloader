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
#ifndef  __STANDBY_I_H__
#define  __STANDBY_I_H__


#include  <common.h>
#include  <asm/io.h>
#include  "dram/dram_i.h"

extern void boot_store_sp(void);
extern void boot_restore_sp(void);
extern void boot_set_sp(void);
extern void boot_halt(void);

extern int standby_axp_output_control(int onoff);
extern int standby_axp_store_int_status(void);
extern int standby_axp_restore_int_status(void);
extern int standby_axp_int_query(__u8 *int_status);
extern int standby_axp_probe_power_exist(void);
extern int standby_axp_probe_key(void);
extern int standby_axp_probe_usb(void);

extern int  standby_int_disable(void);
extern int  standby_int_enable(void);
extern void standby_gic_store(void);
extern void standby_gic_restore(void);
extern void standby_gic_clear_pengding(void);


extern int  standby_axp_i2c_read(unsigned char chip, unsigned char addr, unsigned char *buffer);
extern int  standby_axp_i2c_write(unsigned char chip, unsigned char addr, unsigned char data);

extern void standby_timer_delay(unsigned int ms);

extern int  standby_clock_to_24M(void);
extern int  standby_clock_to_pll1(void);
extern void standby_clock_plldisable(void);
extern void standby_clock_pllenable(void);

extern void standby_serial_putc(char c);

extern int dram_power_save_process(void);
extern int dram_power_up_process(void);


#endif  // __STANDBY_I_H__


