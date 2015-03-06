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

#ifndef  __HD_SCRIPT_H_
#define  __HD_SCRIPT_H_


//#define   CONFIG_SUNXI_FPGA


#define readl(addr)    				( *((volatile unsigned int *)(addr)))
#define writel(value, addr)			((*((volatile unsigned int *)(addr))) = (value))
/*
*********************************************************************************************************
*   hardware registers base define
*********************************************************************************************************
*/
#define REGS_BASE	   0x01C00000		//寄存器物理地址

// 物理地址
#define CCMU_REGS_BASE         ( REGS_BASE + 0x20000 )    //clock manager unit
#define PIOC_REGS_BASE         ( REGS_BASE + 0x20800 )    //general perpose I/O
#define TMRC_REGS_BASE         ( REGS_BASE + 0x20c00 )    //timer

#define LRAC_REGS_BASE         ( REGS_BASE + 0x22800 )    //lradc
#define ADDA_REGS_BASE         ( REGS_BASE + 0x22c00 )    //AD/DA

#define UART0_REGS_BASE        ( REGS_BASE + 0x28000 )    //uart0 base
#define UART1_REGS_BASE        ( REGS_BASE + 0x28400 )    //uart1 base
#define UART2_REGS_BASE        ( REGS_BASE + 0x28800 )    //uart2 base
#define UART3_REGS_BASE        ( REGS_BASE + 0x28C00 )    //uart3 base
#define UART4_REGS_BASE        ( REGS_BASE + 0x29000 )    //uart4 base

#define TWIC0_REGS_BASE        ( REGS_BASE + 0x2AC00 )    //twi0
#define TWIC1_REGS_BASE        ( REGS_BASE + 0x2B000 )    //twi1
#define TWIC2_REGS_BASE        ( REGS_BASE + 0x2B400 )    //twi2

#define DMAC_REGS_BASE         ( REGS_BASE + 0x02000 )    //4k    DMA controller

#define SDMC0_REGS_BASE        ( REGS_BASE + 0x0f000 )    //sdmmc0 controller
#define SDMC1_REGS_BASE        ( REGS_BASE + 0x10000 )    //sdmmc1 controller
#define SDMC2_REGS_BASE        ( REGS_BASE + 0x11000 )    //sdmmc2 controller
//#define SDMC3_REGS_BASE        ( REGS_BASE + 0x12000 )              //sdmmc3 controller

#define ARMA9_GIC_BASE		   ( 0x01c81000)
#define ARMA9_CPUIF_BASE	   ( 0x01c82000)

#define R_PRCM_REGS_BASE       ( 0x01f01400)
#define RUART0_REGS_BASE	   ( 0x01f02800)
#define P2WI_REGS_BASE         ( 0x01F03400)


#define  DRAM_MEM_BASE         ( 0x40000000)

#endif // end of #ifndef __HD_SCRIPT_H_
