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
#ifndef _DMAC_H_
#define _DMAC_H_



/*
*********************************************************************************************************
*   DMA Controller define          < maintained by Steve >
*********************************************************************************************************
*/



#if 1 //AW1650 DMAC Configuration
/* register define */
#define DMA_BASE					(DMAC_REGS_BASE)

#define DMA_IRQ_EN_REG				(DMA_BASE + 0x00)

#define DMA_IRQ_PEND_REG			(DMA_BASE + 0x10)

#define DMA_CHAN_STA_REG			(DMA_BASE + 0x30)

#define DMA_ENABLE_REG(ch)			(DMA_BASE + 0x0100 + ((ch) << 6) + 0x00)
#define DMA_PAUSE_REG(ch)			(DMA_BASE + 0x0100 + ((ch) << 6) + 0x04)
#define DMA_DESADDR_REG(ch)			(DMA_BASE + 0x0100 + ((ch) << 6) + 0x08)
#define DMA_CONFIGR_REG(ch)			(DMA_BASE + 0x0100 + ((ch) << 6) + 0x0C)
#define DMA_CUR_SADDR_REG(ch)		(DMA_BASE + 0x0100 + ((ch) << 6) + 0x10)
#define DMA_CUR_DADDR_REG(ch)		(DMA_BASE + 0x0100 + ((ch) << 6) + 0x14)
#define DMA_LEFTCNT_REG(ch)			(DMA_BASE + 0x0100 + ((ch) << 6) + 0x18)
#define DMA_PARAM_REG(ch)			(DMA_BASE + 0x0100 + ((ch) << 6) + 0x1C)

#define DMA_CHANN_NUM				(8)
/* dma descriptor */
#define STATIC_ALLOC_DMA_DES
struct dma_des {
	u32 config;
	u32 saddr;
	u32 daddr;
	u32 bcnt;
	u32 param;
	struct dma_des *next;
};
extern struct dma_des nfc_desc;

enum {DMA_CFG_BST1, DMA_CFG_BST4, DMA_CFG_BST8};
enum {DMA_CFG_WID8, DMA_CFG_WID16, DMA_CFG_WID32};
enum {DMA_LINEAR, DMA_IO};
enum {DMA_SECURITY, DMA_NONSECURITY};
#define DMA_DES_NULL			(0xfffff800)

/* DMA drq type */
#define DMA_TYPE_SRAM						(0 )      /* port 00 */
#define DMA_TYPE_SDRAM          (1 )      /* port 01 */
#define DMA_TYPE_SPDIF          (2 )      /* port 02 */
#define DMA_TYPE_IIS0           (3 )      /* port 03 */
#define DMA_TYPE_IIS1           (4 )      /* port 04 */
#define DMA_TYPE_NAND0          (5 )      /* port 05 */
#define DMA_TYPE_UART0          (6 )      /* port 06 */
#define DMA_TYPE_UART1          (7 )      /* port 07 */
#define DMA_TYPE_UART2          (8 )      /* port 08 */
#define DMA_TYPE_UART3          (9 )      /* port 09 */
#define DMA_TYPE_UART4          (10)      /* port 10 */
#define DMA_TYPE_TCON0          (11)      /* port 11 */
#define DMA_TYPE_TCON1          (12)      /* port 12 */
#define DMA_TYPE_HDMIDDC        (13)      /* port 13 */
#define DMA_TYPE_HDMIAUDIO      (14)      /* port 14 */
#define DMA_TYPE_CODEC          (15)      /* port 15 */
#define DMA_TYPE_SS             (16)      /* port 16 */
#define DMA_TYPE_OTG_EP1        (17)      /* port 17 */
#define DMA_TYPE_OTG_EP2        (18)      /* port 18 */
#define DMA_TYPE_OTG_EP3        (19)      /* port 19 */
#define DMA_TYPE_OTG_EP4        (20)      /* port 20 */
#define DMA_TYPE_OTG_EP5        (21)      /* port 21 */
#define DMA_TYPE_UART5          (22)      /* port 22 */
#define DMA_TYPE_SPI0           (23)      /* port 23 */
#define DMA_TYPE_SPI1           (24)      /* port 24 */
#define DMA_TYPE_SPI2           (25)      /* port 25 */
#define DMA_TYPE_SPI3           (26)      /* port 26 */
#define DMA_TYPE_TP           	(27)      /* port 27 */
#define DMA_TYPE_NAND1          (28)      /* port 28 */
#define DMA_TYPE_MTCACC         (29)      /* port 29 */
#define DMA_TYPE_DIGMIC         (30)      /* port 30 */
/* DMA source drq config */
#define DMA_CFG_SRC_DRQ_SRAM		(DMA_TYPE_SRAM		<< 0)
#define DMA_CFG_SRC_DRQ_SDRAM		(DMA_TYPE_SDRAM     << 0)
#define DMA_CFG_SRC_DRQ_SPDIF		(DMA_TYPE_SPDIF     << 0)
#define DMA_CFG_SRC_DRQ_IIS0		(DMA_TYPE_IIS0      << 0)
#define DMA_CFG_SRC_DRQ_IIS1		(DMA_TYPE_IIS1      << 0)
#define DMA_CFG_SRC_DRQ_NAND0		(DMA_TYPE_NAND0     << 0)
#define DMA_CFG_SRC_DRQ_UART0		(DMA_TYPE_UART0     << 0)
#define DMA_CFG_SRC_DRQ_UART1		(DMA_TYPE_UART1     << 0)
#define DMA_CFG_SRC_DRQ_UART2		(DMA_TYPE_UART2     << 0)
#define DMA_CFG_SRC_DRQ_UART3		(DMA_TYPE_UART3     << 0)
#define DMA_CFG_SRC_DRQ_UART4		(DMA_TYPE_UART4     << 0)
#define DMA_CFG_SRC_DRQ_TCON0		(DMA_TYPE_TCON0     << 0)
#define DMA_CFG_SRC_DRQ_TCON1		(DMA_TYPE_TCON1     << 0)
#define DMA_CFG_SRC_DRQ_HDMIDDC		(DMA_TYPE_HDMIDDC   << 0)
#define DMA_CFG_SRC_DRQ_HDMIAUDIO	(DMA_TYPE_HDMIAUDIO << 0)
#define DMA_CFG_SRC_DRQ_CODEC		(DMA_TYPE_CODEC     << 0)
#define DMA_CFG_SRC_DRQ_SS			(DMA_TYPE_SS        << 0)
#define DMA_CFG_SRC_DRQ_OTG_EP1		(DMA_TYPE_OTG_EP1   << 0)
#define DMA_CFG_SRC_DRQ_OTG_EP2		(DMA_TYPE_OTG_EP2   << 0)
#define DMA_CFG_SRC_DRQ_OTG_EP3		(DMA_TYPE_OTG_EP3   << 0)
#define DMA_CFG_SRC_DRQ_OTG_EP4		(DMA_TYPE_OTG_EP4   << 0)
#define DMA_CFG_SRC_DRQ_OTG_EP5		(DMA_TYPE_OTG_EP5   << 0)
#define DMA_CFG_SRC_DRQ_UART5		(DMA_TYPE_UART5     << 0)
#define DMA_CFG_SRC_DRQ_SPI0 		(DMA_TYPE_SPI0      << 0)
#define DMA_CFG_SRC_DRQ_SPI1 		(DMA_TYPE_SPI1      << 0)
#define DMA_CFG_SRC_DRQ_SPI2 		(DMA_TYPE_SPI2      << 0)
#define DMA_CFG_SRC_DRQ_SPI3 		(DMA_TYPE_SPI3      << 0)
#define DMA_CFG_SRC_DRQ_TP   		(DMA_TYPE_TP        << 0)
#define DMA_CFG_SRC_DRQ_NAND1		(DMA_TYPE_NAND1     << 0)
#define DMA_CFG_SRC_DRQ_MTCACC		(DMA_TYPE_MTCACC    << 0)
#define DMA_CFG_SRC_DRQ_DIGMIC		(DMA_TYPE_DIGMIC    << 0)
/* DMA destination drq config */
#define DMA_CFG_DST_DRQ_SRAM		(DMA_TYPE_SRAM		<< 16)
#define DMA_CFG_DST_DRQ_SDRAM		(DMA_TYPE_SDRAM     << 16)
#define DMA_CFG_DST_DRQ_SPDIF		(DMA_TYPE_SPDIF     << 16)
#define DMA_CFG_DST_DRQ_IIS0		(DMA_TYPE_IIS0      << 16)
#define DMA_CFG_DST_DRQ_IIS1		(DMA_TYPE_IIS1      << 16)
#define DMA_CFG_DST_DRQ_NAND0		(DMA_TYPE_NAND0     << 16)
#define DMA_CFG_DST_DRQ_UART0		(DMA_TYPE_UART0     << 16)
#define DMA_CFG_DST_DRQ_UART1		(DMA_TYPE_UART1     << 16)
#define DMA_CFG_DST_DRQ_UART2		(DMA_TYPE_UART2     << 16)
#define DMA_CFG_DST_DRQ_UART3		(DMA_TYPE_UART3     << 16)
#define DMA_CFG_DST_DRQ_UART4		(DMA_TYPE_UART4     << 16)
#define DMA_CFG_DST_DRQ_TCON0		(DMA_TYPE_TCON0     << 16)
#define DMA_CFG_DST_DRQ_TCON1		(DMA_TYPE_TCON1     << 16)
#define DMA_CFG_DST_DRQ_HDMIDDC		(DMA_TYPE_HDMIDDC   << 16)
#define DMA_CFG_DST_DRQ_HDMIAUDIO	(DMA_TYPE_HDMIAUDIO << 16)
#define DMA_CFG_DST_DRQ_CODEC		(DMA_TYPE_CODEC     << 16)
#define DMA_CFG_DST_DRQ_SS			(DMA_TYPE_SS        << 16)
#define DMA_CFG_DST_DRQ_OTG_EP1		(DMA_TYPE_OTG_EP1   << 16)
#define DMA_CFG_DST_DRQ_OTG_EP2		(DMA_TYPE_OTG_EP2   << 16)
#define DMA_CFG_DST_DRQ_OTG_EP3		(DMA_TYPE_OTG_EP3   << 16)
#define DMA_CFG_DST_DRQ_OTG_EP4		(DMA_TYPE_OTG_EP4   << 16)
#define DMA_CFG_DST_DRQ_OTG_EP5		(DMA_TYPE_OTG_EP5   << 16)
#define DMA_CFG_DST_DRQ_UART5		(DMA_TYPE_UART5     << 16)
#define DMA_CFG_DST_DRQ_SPI0 		(DMA_TYPE_SPI0      << 16)
#define DMA_CFG_DST_DRQ_SPI1 		(DMA_TYPE_SPI1      << 16)
#define DMA_CFG_DST_DRQ_SPI2 		(DMA_TYPE_SPI2      << 16)
#define DMA_CFG_DST_DRQ_SPI3 		(DMA_TYPE_SPI3      << 16)
#define DMA_CFG_DST_DRQ_TP   		(DMA_TYPE_TP        << 16)
#define DMA_CFG_DST_DRQ_NAND1		(DMA_TYPE_NAND1     << 16)
#define DMA_CFG_DST_DRQ_MTCACC		(DMA_TYPE_MTCACC    << 16)
#define DMA_CFG_DST_DRQ_DIGMIC		(DMA_TYPE_DIGMIC    << 16)
/* DMA source burst length and width */
#define DMA_CFG_SRC_BST1_WIDTH8     ((DMA_CFG_BST1 << 7) | (DMA_CFG_WID8 << 9))
#define DMA_CFG_SRC_BST1_WIDTH16    ((DMA_CFG_BST1 << 7) | (DMA_CFG_WID16<< 9))
#define DMA_CFG_SRC_BST1_WIDTH32    ((DMA_CFG_BST1 << 7) | (DMA_CFG_WID32<< 9))
#define DMA_CFG_SRC_BST4_WIDTH8     ((DMA_CFG_BST4 << 7) | (DMA_CFG_WID8 << 9))
#define DMA_CFG_SRC_BST4_WIDTH16    ((DMA_CFG_BST4 << 7) | (DMA_CFG_WID16<< 9))
#define DMA_CFG_SRC_BST4_WIDTH32    ((DMA_CFG_BST4 << 7) | (DMA_CFG_WID32<< 9))
#define DMA_CFG_SRC_BST8_WIDTH8     ((DMA_CFG_BST8 << 7) | (DMA_CFG_WID8 << 9))
#define DMA_CFG_SRC_BST8_WIDTH16    ((DMA_CFG_BST8 << 7) | (DMA_CFG_WID16<< 9))
#define DMA_CFG_SRC_BST8_WIDTH32    ((DMA_CFG_BST8 << 7) | (DMA_CFG_WID32<< 9))
/* DMA destination burst length and width */
#define DMA_CFG_DST_BST1_WIDTH8     ((DMA_CFG_BST1 << 23) | (DMA_CFG_WID8 << 25))
#define DMA_CFG_DST_BST1_WIDTH16    ((DMA_CFG_BST1 << 23) | (DMA_CFG_WID16<< 25))
#define DMA_CFG_DST_BST1_WIDTH32    ((DMA_CFG_BST1 << 23) | (DMA_CFG_WID32<< 25))
#define DMA_CFG_DST_BST4_WIDTH8     ((DMA_CFG_BST4 << 23) | (DMA_CFG_WID8 << 25))
#define DMA_CFG_DST_BST4_WIDTH16    ((DMA_CFG_BST4 << 23) | (DMA_CFG_WID16<< 25))
#define DMA_CFG_DST_BST4_WIDTH32    ((DMA_CFG_BST4 << 23) | (DMA_CFG_WID32<< 25))
#define DMA_CFG_DST_BST8_WIDTH8     ((DMA_CFG_BST8 << 23) | (DMA_CFG_WID8 << 25))
#define DMA_CFG_DST_BST8_WIDTH16    ((DMA_CFG_BST8 << 23) | (DMA_CFG_WID16<< 25))
#define DMA_CFG_DST_BST8_WIDTH32    ((DMA_CFG_BST8 << 23) | (DMA_CFG_WID32<< 25))
/* IO mode */
#define DMA_CFG_SRC_LINEAR			(DMA_LINEAR << 5)
#define DMA_CFG_SRC_IO				(DMA_IO << 5)
#define DMA_CFG_DST_LINEAR			(DMA_LINEAR << 21)
#define DMA_CFG_DST_IO				(DMA_IO << 21)
/* DMA security */
#define DMA_CFG_SRC_SECURITY        (DMA_SECURITY << 12)
#define DMA_CFG_SRC_NONSECURITY     (DMA_NONSECURITY << 12)
#define DMA_CFG_DST_SECURITY        (DMA_SECURITY << 28)
#define DMA_CFG_DST_NONSECURITY     (DMA_NONSECURITY << 28)

/* DMA irq flags */
#define DMA_HALF_PKG_IRQFLAG		(1 << 0)
#define DMA_FULL_PKG_IRQFLAG		(1 << 1)
#define DMA_QUEUE_END_IRQFLAG		(1 << 2)
#define DMA_IRQ_FLAG_MASK			(DMA_HALF_PKG_IRQFLAG|DMA_FULL_PKG_IRQFLAG|DMA_QUEUE_END_IRQFLAG)
#define DMA_USE_CONTINUE_MODE		(1 << 4)

#endif //#if 1 //AW1650 DMAC Configuration
#endif    // #ifndef _DMAC_H_
