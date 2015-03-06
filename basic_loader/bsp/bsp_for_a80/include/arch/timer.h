/*
 * (C) Copyright 2007-2015
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
#ifndef _TIMER_H_
#define _TIMER_H_


#include "hd_scrpt.h"


#define TMRC_AVS_CTRL			readl(TMRC_REGS_BASE + 0x80)
#define TMRC_AVS_COUNT0			readl(TMRC_REGS_BASE + 0x84)
#define TMRC_AVS_COUNT1			readl(TMRC_REGS_BASE + 0x88)
#define TMRC_AVS_DIVISOR		readl(TMRC_REGS_BASE + 0x8C)


#define WATCHDOG1_CTRL       	readl(TMRC_REGS_BASE + 0xB0)
#define WATCHDOG1_CFG       	readl(TMRC_REGS_BASE + 0xB4)
#define WATCHDOG1_MODE       	readl(TMRC_REGS_BASE + 0xB8)



#endif  /* _TMRC_H_ */

