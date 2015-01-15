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

#ifndef __PLATFORM_H
#define __PLATFORM_H


/* base address of modules */
/* AHB BUS */
#define REGS_AHB0_BASE	     0x01C00000		//寄存器物理地址

#define SS_BASE 			(REGS_AHB0_BASE + 0x2000)
#define NFC_BASE			(REGS_AHB0_BASE + 0x3000)
#define TSC_BASE			(REGS_AHB0_BASE + 0x4000)

#define GTBUS_BASE          (REGS_AHB0_BASE + 0x09000)
#define SMC_BASE            (REGS_AHB0_BASE + 0x0b000)
#define SID_BASE            (REGS_AHB0_BASE + 0x0e000)

#define MMC0_BASE			(REGS_AHB0_BASE + 0x0f000)
#define MMC1_BASE			(REGS_AHB0_BASE + 0x10000)
#define MMC2_BASE			(REGS_AHB0_BASE + 0x11000)
#define MMC3_BASE			(REGS_AHB0_BASE + 0x12000)
#define MMC_COMMON_BASE     (REGS_AHB0_BASE + 0x13000)

#define SPI0_BASE			(REGS_AHB0_BASE + 0x1A000)
#define SPI1_BASE			(REGS_AHB0_BASE + 0x1B000)
#define SPI2_BASE			(REGS_AHB0_BASE + 0x1C000)
#define SPI3_BASE			(REGS_AHB0_BASE + 0x1D000)

#define GIC400_BASE			(REGS_AHB0_BASE + 0x40000)
#define ARMA9_GIC_BASE		(REGS_AHB0_BASE + 0x41000)
#define ARMA9_CPUIF_BASE	(REGS_AHB0_BASE + 0x42000)


#define REGS_AHB1_BASE	     0x00800000		//寄存器物理地址

#define DMAC_BASE           (REGS_AHB1_BASE + 0x002000)
#define USBOTG_BASE			(REGS_AHB1_BASE + 0x100000)
#define USBEHCI0_BASE		(REGS_AHB1_BASE + 0x200000)
#define USBEHCI1_BASE		(REGS_AHB1_BASE + 0x201000)
#define USBEHCI2_BASE		(REGS_AHB1_BASE + 0x202000)

#define REGS_AHB2_BASE	     0x03000000		//寄存器物理地址

#define FE0_BASE            (REGS_AHB2_BASE + 0x100000)
#define FE1_BASE            (REGS_AHB2_BASE + 0x140000)
#define FE2_BASE            (REGS_AHB2_BASE + 0x180000)

#define BE0_BASE            (REGS_AHB2_BASE + 0x200000)
#define BE1_BASE            (REGS_AHB2_BASE + 0x240000)
#define BE2_BASE            (REGS_AHB2_BASE + 0x280000)

#define DEU0_BASE            (REGS_AHB2_BASE + 0x300000)
#define DEU1_BASE            (REGS_AHB2_BASE + 0x340000)

#define DRC0_BASE            (REGS_AHB2_BASE + 0x400000)
#define DRC1_BASE            (REGS_AHB2_BASE + 0x440000)

#define LCD0_BASE           (REGS_AHB2_BASE + 0xC00000)
#define LCD1_BASE           (REGS_AHB2_BASE + 0xC10000)
#define LCD2_BASE           (REGS_AHB2_BASE + 0xC20000)

#define MIPI_DSI0_BASE		(REGS_AHB2_BASE + 0xC40000)
#define MIPI_DSI0_DPHY_BASE	(REGS_AHB2_BASE + 0xC40100)

#define HDMI_BASE			(REGS_AHB2_BASE + 0xd00000)

#define REGS_APB0_BASE	     0x06000000		//寄存器物理地址

#define CCMPLL_BASE         (REGS_APB0_BASE + 0x0000)
#define CCMMODULE_BASE      (REGS_APB0_BASE + 0x0400)
#define PIO_BASE            (REGS_APB0_BASE + 0x0800)
#define TIMER05_BASE        (REGS_APB0_BASE + 0x0C00)
#define PWM03_BASE			(REGS_APB0_BASE + 0x1400)
#define LRADC01_BASE        (REGS_APB0_BASE + 0x1800)

#define SMTA_BASE           (REGS_APB0_BASE + 0x3400)

#define REGS_APB1_BASE	     0x07000000		//寄存器物理地址

#define UART0_BASE			(REGS_APB1_BASE + 0x0000)
#define UART1_BASE			(REGS_APB1_BASE + 0x0400)
#define UART2_BASE			(REGS_APB1_BASE + 0x0800)
#define UART3_BASE			(REGS_APB1_BASE + 0x0C00)
#define UART4_BASE			(REGS_APB1_BASE + 0x1000)
#define UART5_BASE			(REGS_APB1_BASE + 0x1400)

#define TWI0_BASE			(REGS_APB1_BASE + 0x2800)
#define TWI1_BASE			(REGS_APB1_BASE + 0x2C00)
#define TWI2_BASE			(REGS_APB1_BASE + 0x3000)
#define TWI3_BASE			(REGS_APB1_BASE + 0x3400)
#define TWI4_BASE			(REGS_APB1_BASE + 0x3800)

#define REGS_RCPUS_BASE     0x08000000    //RCPU寄存器物理地址

#define RPRCM_BASE          (REGS_RCPUS_BASE + 0x1400)


#define CPUCFG_BASE			0x01700000


#endif
