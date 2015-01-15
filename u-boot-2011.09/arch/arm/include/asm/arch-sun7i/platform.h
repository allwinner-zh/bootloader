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
/*
#define SRAM_A1_CPU12_BASE	(0x0)
#define SRAM_A1_CPU0_BASE	(0x8000)
#define SRAM_A1_SIZE		(0x8000)
#define SRAM_A2_CPU12_BASE	(0x40000)
#define SRAM_A2_CPU0_BASE	(0x0)
#define SRAM_A2_SIZE		(0x8000)
#define SRAM_C_BASE			(0x01D00000)
*/
//////////////////////////////////////////
#define SRAM_A1_BASE		(0x0)
#define SRAM_A1_SIZE		(0x4000)//16k
#define SRAM_A2_BASE		(0x4000)
#define SRAM_A2_SIZE		(0x4000)//16k
#define SRAM_A3_BASE		(0x8000)
#define SRAM_A3_SIZE		(0x3400)//13k
#define SRAM_A4_BASE		(0xb400)
#define SRAM_A4_SIZE		(0x0c00)//3k
//2K SRAM NAND
#define SRAM_NAND_BASE		(0x0c00)//2K
#define SRAM_D_BASE			(0x010000)//4K
#define SRAM_B_BASE			(0x020000)//64K

/* base address of modules */
#define SRAMC_BASE			(0x01c00000)//4K
#define DRAMC_BASE			(0x01c01000)//4K
#define DMA_BASE			(0x01c02000)//4K
#define NFC_BASE			(0x01c03000)//4K
#define TSC_BASE			(0x01c04000)//4K
#define SPI0_BASE			(0x01c05000)//4K
#define SPI1_BASE			(0x01c06000)//4K
#define MS_BASE				(0x01c07000)//4K
#define TVD_BASE			(0x01c08000)//4K
#define CSI0_BASE			(0x01c09000)//4K
#define TVE0_BASE			(0x01c9a000)//4K
#define EMAC_BASE			(0x01c0b000)//4K
#define LCD0_BASE			(0x01c0c000)//4K
#define LCD1_BASE			(0x01c0d000)//4K
#define VE_BASE				(0x01c0e000)//4K
#define MMC0_BASE			(0x01c0f000)//4K
#define MMC1_BASE			(0x01c10000)//4K
#define MMC2_BASE			(0x01c11000)//4K
#define MMC3_BASE			(0x01c12000)//4K
#define USBOTG_BASE			(0x01c13000)//4K
#define USB1_BASE			(0x01c14000)//4K
#define SS_BASE				(0x01c15000)//4K
#define HDMI_BASE			(0x01c16000)//4K
#define SPI2_BASE			(0x01c17000)//4K
#define SATA_BASE			(0x01c18000)//4K
#define PATA_BASE			(0x01c19000)//4K
#define ACE_BASE			(0x01c1a000)//4K
#define TVE1_BASE			(0x01c1b000)//4K
#define USB2_BASE			(0x01c1c000)//4K
#define CSI1_BASE			(0x01c1d000)//4K
#define OHCI2_BASE			(0x01c1e000)//4K
//4K
#define SPI3_BASE			(0x01c1f000)//4K
#define CCM_BASE			(0x01c20000)//1K
#define INT_BASE			(0x01c20400)//1K
#define PIO_BASE			(0x01c20800)//1K
#define PWM03_BASE			(0x01c20c00)//1K //TIMER_BASE
#define SPDIF_BASE			(0x01c21000)//1K
#define AC97_BASE			(0x01c21400)//1K
#define IR0_BASE			(0x01c21800)//1K
#define IR1_BASE			(0x01c21C00)//1K
#define I2S1_BASE			(0x01c22000)//1K
#define I2S0_BASE			(0x01c22400)//1K
#define LRADC_BASE			(0x01c22800)//1K
#define ADDA_BASE			(0x01c22c00)//1K
#define KP_BASE				(0x01c23000)//1K
//1K
#define SID_BASE			(0x01c23800)//1K
#define SJTAG_BASE			(0x01c23c00)//1K
//1K
#define I2S2_BASE			(0x01c24400)//1K
//1K
//1K
#define TP_BASE				(0x01c25000)//1K
#define PMU_BASE			(0x01c25400)//1K
//1K
#define CPUC_BASE			(0x01c25C00)//1K






#define UART0_BASE			(0x01c28000)//1K
#define UART1_BASE			(0x01c28400)//1K
#define UART2_BASE			(0x01c28800)//1K
#define UART3_BASE			(0x01c28c00)//1K
#define UART4_BASE			(0x01c29000)//1K
#define UART5_BASE			(0x01c29400)//1K
#define UART6_BASE			(0x01c29800)//1K
#define UART7_BASE			(0x01c29C00)//1K
#define PS20_BASE			(0x01c2a000)//1K
#define PS21_BASE			(0x01c2a400)//1K

#define TWI0_BASE			(0x01c2ac00)//1K
#define TWI1_BASE			(0x01c2b000)//1K
#define TWI2_BASE			(0x01c2b400)//1K
#define TWI3_BASE			(0x01c2b800)//1K
#define CAN_BASE			(0x01c2bc00)//1K
#define TWI4_BASE			(0x01c2c000)//1K
#define SCR_BASE			(0x01c2c400)//1K
#define GPS_BASE			(0x01c30000)//64K
#define MALI400_BASE		(0x01c40000)//64K
#define GMAC_BASE			(0x01c50000)//64K
#define HSTMR_BASE			(0x01c60000)//4k

#define ARMA9_SCU_BASE		(0x01c80000)
#define ARMA9_GIC_BASE		(0x01c81000)
#define ARMA9_CPUIF_BASE	(0x01c82000)
#define ARMA9_CPUBIST_BASE	(0x01c88000)//according boot1.0 Hd_script.h


#define HDMI1_BASE			(0x01ce0000)//128K
#define CPUBIST_BASE		(0x3f50C000)//4K
#define SRAM_C_BASE			(0x01d00000)//Module sram

#define DEFE0_BASE			(0x01e00000)//128K
#define DEFE1_BASE			(0x01e20000)//128K
#define DEBE0_BASE			(0x01e60000)//128K
#define DEBE1_BASE			(0x01e40000)//128K
#define MP_BASE				(0x01e80000)//128k
#define AVG_BASE			(0x01ea0000)//128k
#define CSDM_BASE			(0x03f50000)//64K
#define DDR_BASE			(0x40000000)//2G
#define BROM_BASE			(0xffff0000)//32K

#endif
