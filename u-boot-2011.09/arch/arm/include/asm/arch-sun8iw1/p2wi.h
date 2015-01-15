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

#ifndef __P2WI_I_H__
#define __P2WI_I_H__

#include "cpu.h"

#define P2WI_SCK			(2000000)
#define P2WI_SDAODLY		(1)

#define PMU_TRANS_BYTE_MAX  (8)
//register define
#define	P2WI_REG_BASE			(SUNXI_P2WI_BASE)
#define P2WI_REG_CTRL			(P2WI_REG_BASE + 0x00)
#define P2WI_REG_CCR			(P2WI_REG_BASE + 0x04)
#define P2WI_REG_INTE			(P2WI_REG_BASE + 0x08)
#define P2WI_REG_STAT			(P2WI_REG_BASE + 0x0c)
#define P2WI_REG_DADDR0			(P2WI_REG_BASE + 0x10)
#define P2WI_REG_DADDR1			(P2WI_REG_BASE + 0x14)
#define P2WI_REG_DLEN			(P2WI_REG_BASE + 0x18)
#define P2WI_REG_DATA0			(P2WI_REG_BASE + 0x1c)
#define P2WI_REG_DATA1			(P2WI_REG_BASE + 0x20)
#define P2WI_REG_LCR			(P2WI_REG_BASE + 0x24)
#define P2WI_REG_PMCR			(P2WI_REG_BASE + 0x28)

//p2wi control bit field
#define P2WI_SOFT_RST		(1U << 0)
#define P2WI_GLB_INTEN		(1U << 1)
#define P2WI_ABT_TRANS		(1U << 6)
#define P2WI_START_TRANS 	(1U << 7)

//p2wi state bit field
#define P2WI_TOVER_INT		(1U << 0)
#define P2WI_TERR_INT		(1U << 1)
#define P2WI_LBSY_INT		(1U << 2)

//p2wi pmu mode bit field
#define P2WI_PMU_INIT		(1U << 31)


#endif	//__P2WI_I_H__
