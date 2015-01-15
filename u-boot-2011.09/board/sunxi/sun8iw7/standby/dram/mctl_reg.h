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

#ifndef _MCTL_REG_H
#define _MCTL_REG_H

//DRAMC base address definition
#define MCTL_COM_BASE			0x01c62000
#define MCTL_CTL_BASE			0x01c63000
#define MCTL_PHY_BASE			0x01c65000

#define MC_CR					(MCTL_COM_BASE + 0x0)
#define MC_CCR 					(MCTL_COM_BASE + 0x4)
#define MC_DBGCR				(MCTL_COM_BASE + 0x8)
#define MC_MCR0(x)				(MCTL_COM_BASE + 0x10 + 0x8 * x)		    //x from 0 to 15
#define MC_MCR1(x)				(MCTL_COM_BASE + 0x14 + 0x8 * x)		    //x from 0 to 15
#define MC_BWCR					(MCTL_COM_BASE + 0x90)
#define MC_MAER					(MCTL_COM_BASE + 0x94)
#define MC_MCGCR				(MCTL_COM_BASE + 0x9C)
#define MC_BWCTR				(MCTL_COM_BASE + 0xA0)
#define MC_SWONR				(MCTL_COM_BASE + 0xA8)
#define MC_SWOFFR				(MCTL_COM_BASE + 0xAC)

#define MX_MSTR					(MCTL_CTL_BASE + 0x0)
#define MX_STATR				(MCTL_CTL_BASE + 0x4)
#define MX_MRCTRL0				(MCTL_CTL_BASE + 0x10)
#define MX_MRCTRL1				(MCTL_CTL_BASE + 0x14)
#define MX_MRSTATR				(MCTL_CTL_BASE + 0x18)
#define MX_DERATEEN				(MCTL_CTL_BASE + 0x20)
#define MX_DERATENINT			(MCTL_CTL_BASE + 0x24)
#define MX_PWRCTL				(MCTL_CTL_BASE + 0x30)
#define MX_PWRTMG				(MCTL_CTL_BASE + 0x34)
#define MX_RFSHCTL0				(MCTL_CTL_BASE + 0x50)
#define MX_RFSHCTL1				(MCTL_CTL_BASE + 0x54)
#define MX_RFSHCTL3				(MCTL_CTL_BASE + 0x60)
#define MX_RFSHTMG				(MCTL_CTL_BASE + 0x64)
#define MX_INIT0				(MCTL_CTL_BASE + 0xD0)
#define MX_INIT1				(MCTL_CTL_BASE + 0xD4)
#define MX_INIT2				(MCTL_CTL_BASE + 0xD8)
#define MX_INIT3				(MCTL_CTL_BASE + 0xDC)
#define MX_INIT4				(MCTL_CTL_BASE + 0xE0)
#define MX_INIT5				(MCTL_CTL_BASE + 0xE4)
#define MX_RANKCTL				(MCTL_CTL_BASE + 0xF4)
#define MX_DRAMTMG(x)			(MCTL_CTL_BASE + 0x100 + 0x4 * x)			// x from 0 to 8
#define MX_ZQCTL(x)				(MCTL_CTL_BASE + 0x180 + 0x4 * x)			// x from 0 to 2
#define MX_ZQSTAT				(MCTL_CTL_BASE + 0x18C)
#define MX_PITMG(x)				(MCTL_CTL_BASE + 0x190 + 0x4 * x)			// x from 0 to 1
#define MX_PLPCFG0				(MCTL_CTL_BASE + 0x198)
#define MX_UPD(x)				(MCTL_CTL_BASE + 0x1A0 + 0x4 * x)			// x from 0 to 3
#define MX_PIMISC         		(MCTL_CTL_BASE + 0x1B0)
#define MX_TRAINCTL(x)			(MCTL_CTL_BASE + 0x1D0 + 0x4 * x)			// x from 0 to 2
#define MX_TRAINSTAT			(MCTL_CTL_BASE + 0x1DC)
#define MX_ODTCFG				(MCTL_CTL_BASE + 0x240)
#define MX_ODTMAP				(MCTL_CTL_BASE + 0x244)
#define MX_SCHED				(MCTL_CTL_BASE + 0x250)
#define MX_PERFSHPR(x)			(MCTL_CTL_BASE + 0x258 + 0x4 * x)			// x from 0 to 1
#define MX_PERFLPR(x)			(MCTL_CTL_BASE + 0x260 + 0x4 * x)			// x from 0 to 1
#define MX_PERFWR(x)			(MCTL_CTL_BASE + 0x268 + 0x4 * x)			// x from 0 to 1

#define MP_PIR					(MCTL_PHY_BASE + 0x4)
#define MP_PGCR(x)				(MCTL_PHY_BASE + 0x8 + 0x4 * x)				// x from 0 to 1
#define MP_PGSR(x)				(MCTL_PHY_BASE + 0x10 + 0x4 * x)			// x from 0 to 1
#define MP_DLLGCR				(MCTL_PHY_BASE + 0x18)
#define MP_PTR(x)				(MCTL_PHY_BASE + 0x1C + 0x4 * x)			// x from 0 to 4
#define MP_ACMDLR				(MCTL_PHY_BASE + 0x30)
#define MP_ACBDLR				(MCTL_PHY_BASE + 0x34)
#define MP_ACIOCR				(MCTL_PHY_BASE + 0x38)
#define MP_DXCCR				(MCTL_PHY_BASE + 0x3C)
#define MP_DSGCR				(MCTL_PHY_BASE + 0x40)
#define MP_DCR					(MCTL_PHY_BASE + 0x44)
#define MP_DTPR(x)				(MCTL_PHY_BASE + 0x48 + 0x4 * x)			//x from 0 to 2
#define MP_MR(x)				(MCTL_PHY_BASE + 0x54 + 0x4 * x)			//x from 0 to 3
#define MP_ODTCR				(MCTL_PHY_BASE + 0x64)
#define MP_DTCR					(MCTL_PHY_BASE + 0x68)
#define MP_DTAR(x)				(MCTL_PHY_BASE + 0x6C + 0x4 * x)			//x from 0 to 3
#define MP_DTDR(x)				(MCTL_PHY_BASE + 0x7C + 0x4 * x)			//x from 0 to 1
#define MP_DTEDR(x)				(MCTL_PHY_BASE + 0x84 + 0x4 * x)			//x from 0 to 1
#define MP_PGCR2				(MCTL_PHY_BASE + 0x8C)
#define MP_BISTRR				(MCTL_PHY_BASE + 0x100)
#define MP_BISTWCR				(MCTL_PHY_BASE + 0x104)
#define MP_BISTMSKR(x)			(MCTL_PHY_BASE + 0x108 + 0x4 * x)			//x from 0 to 2
#define MP_BISTLSR				(MCTL_PHY_BASE + 0x114)
#define MP_BISTAR(x)			(MCTL_PHY_BASE + 0x118 + 0x4 * x)			//x from 0 to 2
#define MP_BISTUDPR				(MCTL_PHY_BASE + 0x124)
#define MP_BISTGSR				(MCTL_PHY_BASE + 0x128)
#define MP_BISTWER				(MCTL_PHY_BASE + 0x12C)
#define MP_BISTBER(x)			(MCTL_PHY_BASE + 0x130 + 0x4 * x)			//x from 0 to 3
#define MP_BISTWCSR				(MCTL_PHY_BASE + 0x140)
#define MP_BISTFWR(x)			(MCTL_PHY_BASE + 0x144 + 0x4 * x)			//x from 0 to 2
#define MP_ZQCR0(x)				(MCTL_PHY_BASE + 0x180 + 0x10 * x)			//x from 0 to 2
#define MP_ZQCR1(x)				(MCTL_PHY_BASE + 0x184 + 0x10 * x)			//x from 0 to 2
#define MP_ZQCR2				(MCTL_PHY_BASE + 0x190)
#define MP_ZQSR0(x)				(MCTL_PHY_BASE + 0x188 + 0x10 * x)			//x from 0 to 2
#define MP_ZQSR1(x)				(MCTL_PHY_BASE + 0x18C + 0x10 * x)			//x from 0 to 2
#define MP_DXGCR(x)				(MCTL_PHY_BASE + 0x1C0 + 0x40 * x)			//x from 0 to 3
#define MP_DXGSR0(x)			(MCTL_PHY_BASE + 0x1C4 + 0x40 * x)			//x from 0 to 3
#define MP_DXGSR1(x)			(MCTL_PHY_BASE + 0x1C8 + 0x40 * x)			//x from 0 to 3
#define MP_DXBDLR0(x)			(MCTL_PHY_BASE + 0x1CC + 0x40 * x)			//x from 0 to 3
#define MP_DXBDLR1(x)			(MCTL_PHY_BASE + 0x1D0 + 0x40 * x)			//x from 0 to 3
#define MP_DXBDLR2(x)			(MCTL_PHY_BASE + 0x1D4 + 0x40 * x)			//x from 0 to 3
#define MP_DXBDLR3(x)			(MCTL_PHY_BASE + 0x1D8 + 0x40 * x)			//x from 0 to 3
#define MP_DXBDLR4(x)			(MCTL_PHY_BASE + 0x1DC + 0x40 * x)			//x from 0 to 3
#define MP_DXLCDLR0(x)			(MCTL_PHY_BASE + 0x1E0 + 0x40 * x)			//x from 0 to 3
#define MP_DXLCDLR1(x)			(MCTL_PHY_BASE + 0x1E4 + 0x40 * x)			//x from 0 to 3
#define MP_DXLCDLR2(x)			(MCTL_PHY_BASE + 0x1E8 + 0x40 * x)			//x from 0 to 3
#define MP_DXMDLR(x)			(MCTL_PHY_BASE + 0x1EC + 0x40 * x)			//x from 0 to 3
#define MP_DXGTR(x)				(MCTL_PHY_BASE + 0x1F0 + 0x40 * x)			//x from 0 to 3
#define MP_DXGSR2(x)			(MCTL_PHY_BASE + 0x1F4 + 0x40 * x)			//x from 0 to 3

//other relevant registers
#ifndef CCM_BASE
	#define CCM_BASE			(0x01c20000)
#endif

#ifndef CCM_PLL_CPUX_CTRL
	#define CCM_PLL_CPUX_CTRL  	(CCM_BASE+0x000)
#endif

#ifndef CCM_PLL_DDR_CTRL
	#define CCM_PLL_DDR_CTRL  	(CCM_BASE+0x020)
#endif

#ifndef CCM_PLL_PERH_CTRL
	#define CCM_PLL_PERH_CTRL  	(CCM_BASE+0x028)
#endif

#ifndef CCM_CPU_AXI_CFG
	#define CCM_CPU_AXI_CFG		(CCM_BASE+0x050)
#endif

#ifndef CCM_AHB1_APB1_CFG
	#define CCM_AHB1_APB1_CFG	(CCM_BASE+0x054)
#endif

#ifndef CCM_AHB1_GATE0_REG
	#define CCM_AHB1_GATE0_REG	(CCM_BASE+0x060)
#endif

#ifndef CCM_DRAM_CFG_REG
	#define CCM_DRAM_CFG_REG	(CCM_BASE+0x0f4)
#endif

#ifndef CCM_DRAM_GATE_REG
	#define CCM_DRAM_GATE_REG	(CCM_BASE+0x0100)
#endif

#ifndef CCM_AHB1_RST_REG1
	#define CCM_AHB1_RST_REG0	(CCM_BASE+0x02c0)
#endif

#ifndef PRCM_BASE
	#define PRCM_BASE			(0x01F01400)
#endif

#ifndef VDD_SYS_PWROFF_GATING
	#define VDD_SYS_PWROFF_GATING	(PRCM_BASE+0x110)
#endif

//========================================================================================
#define mctl_read_w(n)   		(*((volatile unsigned int *)(n)))
#define mctl_write_w(n,c) 		(*((volatile unsigned int *)(n)) = (c))
//========================================================================================

#endif  //_MCTL_REG_H
