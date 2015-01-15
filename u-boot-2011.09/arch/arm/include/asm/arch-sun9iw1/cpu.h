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

#ifndef _SUNXI_CPU_H
#define _SUNXI_CPU_H

#include "platform.h"


#define SUNXI_SRAM_D_BASE           (0x08100000)
/* AHB0 Module */
#define SUNXI_NFC_BASE				(REGS_AHB0_BASE + 0x3000)
#define SUNXI_TSC_BASE				(REGS_AHB0_BASE + 0x4000)

#define SUNXI_SID_BASE              (REGS_AHB0_BASE + 0x0e000)
#define SUNXI_MMC0_BASE				(REGS_AHB0_BASE + 0x0f000)
#define SUNXI_MMC1_BASE				(REGS_AHB0_BASE + 0x10000)
#define SUNXI_MMC2_BASE				(REGS_AHB0_BASE + 0x11000)
#define SUNXI_MMC3_BASE		    	(REGS_AHB0_BASE + 0x12000)
#define SUNXI_MMC_COMMON_BASE       (REGS_AHB0_BASE + 0x13000)

#define SUNXI_SPI0_BASE				(REGS_AHB0_BASE + 0x1A000)
#define SUNXI_SPI1_BASE				(REGS_AHB0_BASE + 0x1B000)
#define SUNXI_SPI2_BASE				(REGS_AHB0_BASE + 0x1C000)
#define SUNXI_SPI3_BASE				(REGS_AHB0_BASE + 0x1D000)

/* AHB1 Module */
#define SUNXI_DMA_BASE              (REGS_AHB1_BASE + 0x002000)
#define SUNXI_USBOTG_BASE			(REGS_AHB1_BASE + 0x100000)
#define SUNXI_USBEHCI0_BASE			(REGS_AHB1_BASE + 0x200000)
#define SUNXI_USBEHCI1_BASE			(REGS_AHB1_BASE + 0x201000)
#define SUNXI_USBEHCI2_BASE			(REGS_AHB1_BASE + 0x202000)

/* AHB2 Module */
#define SUNXI_DE_SYS_BASE         	(REGS_AHB2_BASE + 0x000000)
#define SUNXI_DISP_SYS_BASE        	(REGS_AHB2_BASE + 0x010000)
#define SUNXI_DE_FE0_BASE          	(REGS_AHB2_BASE + 0x100000)
#define SUNXI_DE_FE1_BASE      	   	(REGS_AHB2_BASE + 0x140000)
#define SUNXI_DE_FE2_BASE      	   	(REGS_AHB2_BASE + 0x180000)

#define SUNXI_DE_BE0_BASE      	   	(REGS_AHB2_BASE + 0x200000)
#define SUNXI_DE_BE1_BASE      	   	(REGS_AHB2_BASE + 0x240000)
#define SUNXI_DE_BE2_BASE      	   	(REGS_AHB2_BASE + 0x280000)

#define SUNXI_DE_DRC0_BASE    		(REGS_AHB2_BASE + 0x300000)
#define SUNXI_DE_DRC1_BASE    		(REGS_AHB2_BASE + 0x340000)
#define SUNXI_DE_DEU0_BASE    		(REGS_AHB2_BASE + 0x400000)
#define SUNXI_DE_DEU1_BASE    		(REGS_AHB2_BASE + 0x440000)

#define SUNXI_LCD0_BASE       	   	(REGS_AHB2_BASE + 0xC00000)
#define SUNXI_LCD1_BASE       	   	(REGS_AHB2_BASE + 0xC10000)
#define SUNXI_LCD2_BASE       	   	(REGS_AHB2_BASE + 0xC20000)
#define SUNXI_MIPI_DSI0_BASE       	(REGS_AHB2_BASE + 0xC40000)
#define SUNXI_HDMI_BASE       	    (REGS_AHB2_BASE + 0xD00000)

/* APB0 Module */
#define SUNXI_CCM_BASE      	   	(REGS_APB0_BASE + 0x0000)
#define SUNXI_CCMMODULE_BASE   	   	(REGS_APB0_BASE + 0x0400)
#define SUNXI_PIO_BASE         	   	(REGS_APB0_BASE + 0x0800)
#define SUNXI_R_PIO_BASE                (0x08002C00)
#define SUNXI_TIMER_BASE     	   	(REGS_APB0_BASE + 0x0C00)
#define SUNXI_PWM_BASE      	   	(REGS_APB0_BASE + 0x1400)
#define SUNXI_LRADC_BASE     	   	(REGS_APB0_BASE + 0x1800)

/* APB1 Module */
#define SUNXI_UART0_BASE			(REGS_APB1_BASE + 0x0000)
#define SUNXI_UART1_BASE			(REGS_APB1_BASE + 0x0400)
#define SUNXI_UART2_BASE			(REGS_APB1_BASE + 0x0800)
#define SUNXI_UART3_BASE			(REGS_APB1_BASE + 0x0C00)
#define SUNXI_UART4_BASE			(REGS_APB1_BASE + 0x1000)
#define SUNXI_UART5_BASE			(REGS_APB1_BASE + 0x1400)

/* RCPUS Module */
#define SUNXI_RPRCM_BASE            (REGS_RCPUS_BASE + 0x1400)

/* make the compiler happy */
#define SUNXI_MIPI_DSI0_DPHY_BASE  	0x01ca1000



#define SUNXI_BROM_BASE				0XFFFF0000		/* 32K */

#define SUNXI_CPU_CFG              (SUNXI_TIMER_BASE + 0x13c)

#ifndef __ASSEMBLY__
/* boot type */
typedef enum {
	SUNXI_BOOT_TYPE_NULL = -1,
	SUNXI_BOOT_TYPE_NAND = 0,
	SUNXI_BOOT_TYPE_MMC0 = 1,
	SUNXI_BOOT_TYPE_MMC2 = 2,
	SUNXI_BOOT_TYPE_SPI  = 3
} sunxi_boot_type_t;

sunxi_boot_type_t get_boot_type(void);
#endif /* __ASSEMBLY__ */

#define SUNXI_GET_BITS(value, start_bit, bits_num) ( (value >> start_bit) & \
													((1 << bits_num) - 1) )

#endif /* _CPU_H */
