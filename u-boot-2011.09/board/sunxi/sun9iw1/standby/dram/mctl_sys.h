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
//*****************************************************************************
//	Allwinner Technology, All Right Reserved. 2006-2010 Copyright (c)
//
//	File: 				mctl_sys.h
//
//	Description:  This file implements basic functions for AW1633 DRAM controller
//
//	History:
//              2012/02/06      Berg Xing       0.10    Initial version
//              2012/02/24      Berg Xing       0.20    Support 2 channel
//              2012/02/27      Berg Xing       0.30    modify mode register access
//				2012/03/01		Berg Xing       0.40    add LPDDR2
//				2012/03/10		Berg Xing       0.50    add mctl_dll_init() function
//				2012/04/26		Berg Xing       0.60    add deep sleep
//				2012/06/19		Berg Xing       0.70    add 2T mode
//				2012/11/07		CPL				0.80	FPGA version based on berg's code
//				2012/11/14		CPL				0.90	add SID and regulate the parameters order
//				2012/11/21		CPL				0.91	modify parameters error
//				2012/11/25		CPL				0.92	modify for IC test
//				2012/11/27		CPL				0.93	add master configuration
//				2012/11/28		CPL				0.94	modify for boot and burn interface compatible
//				2012/11/29		CPL				0.95	modify lock parameters configuration
//				2012/12/3		CPL				0.96	add dll&pll delay and simple test
//*****************************************************************************
#ifndef MCTL_SYS_H_
#define MCTL_SYS_H_

#include "asm/arch/dram.h"

extern void mctl_self_refresh_entry(unsigned int ch_index);
extern void mctl_self_refresh_exit(unsigned int ch_index);
extern unsigned int mctl_selfrefesh_test(void);
extern unsigned int mctl_deep_sleep_test(void);
extern void mctl_deep_sleep_entry(void);
extern void mctl_deep_sleep_exit(__dram_para_t *para);

#endif

