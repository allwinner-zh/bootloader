/*
 * (C) Copyright 2012
 *     wangflord@allwinnertech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */
#ifndef  __COMMON_OP_H__
#define  __COMMON_OP_H__



extern void bias_calibration(void);

extern void timer_init(void);
extern void __msdelay(unsigned int ms);
extern void timer_exit(void);

extern void pll_reset( void );

extern void cpu_init_s(void);

extern unsigned int  get_fel_flag(void);
extern void show_rtc_reg(void);
extern void  clear_fel_flag(void);

#endif    //__COMMON_OP_H__
