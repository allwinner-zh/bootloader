/*
* (C) Copyright 2007-2013
* Allwinner Technology Co., Ltd. <www.allwinnertech.com>
* Martin zheng <zhengjiewen@allwinnertech.com>
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

#ifndef _CCMU_H_
#define _CCMU_H_

#include  "hd_scrpt.h"

/* Offset */
#define CCMU_REG_o_PLL1_CTRL            0x00
#define CCMU_REG_o_PLL1_TUNING          0x04
#define CCMU_REG_o_PLL2_CTRL            0x08
#define CCMU_REG_o_PLL2_TUNING          0x0C
#define CCMU_REG_o_PLL3_CTRL            0x10
#define CCMU_REG_o_PLL3_TUNING          0x14
#define CCMU_REG_o_PLL4_CTRL            0x18
#define CCMU_REG_o_PLL4_TUNING          0x1C
#define CCMU_REG_o_PLL5_CTRL            0x20
#define CCMU_REG_o_PLL5_TUNING          0x24
#define CCMU_REG_o_PLL6_CTRL            0x28
#define CCMU_REG_o_PLL6_TUNING          0x2C
#define CCMU_REG_o_PLL7_CTRL            0x30
#define CCMU_REG_o_PLL7_TUNING          0x34
#define CCMU_REG_o_PLL1_TUNING2         0x38
#define CCMU_REG_o_PLL5_TUNING2         0x3C
#define CCMU_REG_o_PLL8_CTRL            0x40

#define CCMU_REG_o_PLTD                 0x4C
#define CCMU_REG_o_OSC24M				0x50
#define CCMU_REG_o_AHB_APB              0x54
#define CCMU_REG_o_APB1					0x58
#define CCMU_REG_o_AXI_MOD				0x5C
#define CCMU_REG_o_AHB_MOD0				0x60
#define CCMU_REG_o_AHB_MOD1				0x64
#define CCMU_REG_o_APB_MOD0				0x68
#define CCMU_REG_o_APB_MOD1				0x6C

#define CCMU_REG_o_NAND                 0x80
#define CCMU_REG_o_SD_MMC0              0x88
#define CCMU_REG_o_SD_MMC2              0x90
#define CCMU_REG_o_SPI0                 0xA0
#define CCMU_REG_o_SPI1                 0xA4

#define CCMU_REG_o_AVS					0x144
#define CCMU_REG_AVS					( CCMU_REGS_BASE + CCMU_REG_o_AVS)
/* registers */
#define CCMU_REG_PLL1_CTRL              __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL1_CTRL   )
#define CCMU_REG_PLL1_TUNING            __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL1_TUNING )
#define CCMU_REG_PLL2_CTRL              __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL2_CTRL   )
#define CCMU_REG_PLL2_TUNING            __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL2_TUNING )
#define CCMU_REG_PLL3_CTRL              __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL3_CTRL   )
#define CCMU_REG_PLL3_TUNING            __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL3_TUNING )
#define CCMU_REG_PLL4_CTRL              __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL4_CTRL   )
#define CCMU_REG_PLL4_TUNING            __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL4_TUNING )
#define CCMU_REG_PLL5_CTRL              __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL5_CTRL   )
#define CCMU_REG_PLL5_TUNING            __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL5_TUNING )
#define CCMU_REG_PLL6_CTRL              __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL6_CTRL   )
#define CCMU_REG_PLL6_TUNING            __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL6_TUNING )
#define CCMU_REG_PLL7_CTRL              __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL7_CTRL   )
#define CCMU_REG_PLL7_TUNING            __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL7_TUNING )
#define CCMU_REG_PLL8_CTRL              __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL8_CTRL   )

#define CCMU_REG_PLL1_TUNING2           __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL1_TUNING2)
#define CCMU_REG_PLL5_TUNING2           __REG( CCMU_REGS_BASE + CCMU_REG_o_PLL5_TUNING2)

#define CCMU_REG_PLTD                   __REG( CCMU_REGS_BASE + CCMU_REG_o_PLTD        )
#define CCMU_REG_OSC24M				    __REG( CCMU_REGS_BASE + CCMU_REG_o_OSC24M	   )
#define CCMU_REG_AHB_APB                __REG( CCMU_REGS_BASE + CCMU_REG_o_AHB_APB     )
#define CCMU_REG_APB1				    __REG( CCMU_REGS_BASE + CCMU_REG_o_APB1		   )
#define CCMU_REG_AXI_MOD			    __REG( CCMU_REGS_BASE + CCMU_REG_o_AXI_MOD	   )
#define CCMU_REG_AHB_MOD0			    __REG( CCMU_REGS_BASE + CCMU_REG_o_AHB_MOD0	   )
#define CCMU_REG_AHB1_GATING0           ( CCMU_REGS_BASE + CCMU_REG_o_AHB_MOD0	   )
#define CCMU_REG_AHB_MOD1			    __REG( CCMU_REGS_BASE + CCMU_REG_o_AHB_MOD1	   )
#define CCMU_REG_APB_MOD0			    __REG( CCMU_REGS_BASE + CCMU_REG_o_APB_MOD0	   )
#define CCMU_REG_APB_MOD1			    __REG( CCMU_REGS_BASE + CCMU_REG_o_APB_MOD1	   )

#define CCMU_REG_NAND                   __REG( CCMU_REGS_BASE + CCMU_REG_o_NAND        )
#define CCMU_REG_SD_MMC0                ( CCMU_REGS_BASE + CCMU_REG_o_SD_MMC0     )
#define CCMU_REG_SD_MMC2                ( CCMU_REGS_BASE + CCMU_REG_o_SD_MMC2     )
#define CCMU_REG_SPI0                   __REG( CCMU_REGS_BASE + CCMU_REG_o_SPI0        )
#define CCMU_REG_SPI1                   __REG( CCMU_REGS_BASE + CCMU_REG_o_SPI1        )


/* CCMU_REG_OSC24M bit position */
#define CCMU_BP_LDO_KEY_FIELD           24
#define CCMU_BP_LDO_EN                  16
#define CCMU_BP_OSC2M_GSM                1
#define CCMU_BP_OSC2M_EN                 0

    /* CCMU_REG_AHB_APB  bit position */
#define CCMU_BP_AC328_CLK_SRC           16
#define CCMU_BP_APB0_CLK_DIV             8
#define CCMU_BP_AHB_CLK_DIV              4
#define CCMU_BP_AXI_CLK_DIV              0

  	/* CCMU_REG_AHB_MODULE0 bit position */
#define CCMU_BP_SPI1_AHB_CLK_GATE       21
#define CCMU_BP_SPI0_AHB_CLK_GATE       20
#define CCMU_BP_NAND_AHB_CLK_GATE       13
#define CCMU_BP_SDMMC2_AHB_CLK_GATE     10
#define CCMU_BP_SDMMC0_AHB_CLK_GATE      8
#define CCMU_BP_DMA_AHB_CLK_GATE         6
#define CCMU_BP_CSI_AHB_CLK_GATE         5

	/* CCMU_REG_APB_MODULE1 bit position */
#define CCMU_BP_UART0_APB_CLK_GATE      16
#define CCMU_BP_TWI0_APB_CLK_GATE        0

  	/* CCMU_REG_APB_MODULE0 bit position */
#define CCMU_BP_PIO_APB_CLK_GATE         5

	/* CCMU_REG_NAND_MODULE bit position */
#define CCMU_BP_NAND_CLK_GATE           31
#define CCMU_BP_NAND_CLK_SRC            24
#define CCMU_BP_NAND_CLK_PRE_DIV        16
#define CCMU_BP_NAND_CLK_DIV             0

  /* bit field mask */
#define CCMU_BITS_0				        0x00
#define CCMU_BITS_1                     0x01
#define CCMU_BITS_2                     0x03
#define CCMU_BITS_3                     0x07
#define CCMU_BITS_4                     0x0F
#define CCMU_BITS_5                     0x1F
#define CCMU_BITS_6                     0x3F
#define CCMU_BITS_7                     0x7F
#define CCMU_BITS_8                     0xFF

   /* CCMU_REG_AC320_PLL bit field mask */
#define CCMU_MASK_LDO_KEY_FIELD        ( CCMU_BITS_15<< CCMU_BP_LDO_KEY_FIELD  )
#define CCMU_MASK_LDO_EN               ( CCMU_BITS_1 << CCMU_BP_LDO_EN         )
#define CCMU_MASK_OSC2M_GSM            ( CCMU_BITS_1 << CCMU_BP_OSC2M_GSM      )
#define CCMU_MASK_OSC2M_EN             ( CCMU_BITS_1 << CCMU_BP_OSC2M_EN       )

  	/* CCMU_REG_AHB_APB bit field mask */
#define CCMU_MASK_AC328_CLK_SRC        ( CCMU_BITS_2 << CCMU_BP_CORE_CLK_SRC   )
#define CCMU_MASK_APB0_CLK_DIV         ( CCMU_BITS_2 << CCMU_BP_APB0_CLK_DIV   )
#define CCMU_MASK_AHB_CLK_DIV          ( CCMU_BITS_2 << CCMU_BP_AHB_CLK_DIV    )
#define CCMU_MASK_AXI_CLK_DIV          ( CCMU_BITS_2 << CCMU_BP_AXI_CLK_DIV    )

  	/* CCMU_REG_AHB_MODULE bit field mask */
#define CCMU_MASK_SPI1_AHB_CLK_GATE    ( CCMU_BITS_1 << CCMU_BP_SPI1_AHB_CLK_GATE   )
#define CCMU_MASK_SPI0_AHB_CLK_GATE    ( CCMU_BITS_1 << CCMU_BP_SPI0_AHB_CLK_GATE   )
#define CCMU_MASK_NAND_AHB_CLK_GATE    ( CCMU_BITS_1 << CCMU_BP_NAND_AHB_CLK_GATE   )
#define CCMU_MASK_SDMMC2_AHB_CLK_GATE  ( CCMU_BITS_1 << CCMU_BP_SDMMC2_AHB_CLK_GATE )
#define CCMU_MASK_SDMMC0_AHB_CLK_GATE  ( CCMU_BITS_1 << CCMU_BP_SDMMC0_AHB_CLK_GATE )
#define CCMU_MASK_DMA_AHB_CLK_GATE     ( CCMU_BITS_1 << CCMU_BP_DMA_AHB_CLK_GATE    )

  	/* CCMU_REG_APB_MODULE bit field mask */
#define CCMU_MASK_PIO_APB_CLK_GATE     ( CCMU_BITS_1 << CCMU_BP_PIO_APB_CLK_GATE   )

	/* CCMU_REG_NAND_MODULE bit field mask */
#define CCMU_MASK_NAND_CLK_GATE        ( CCMU_BITS_1 << CCMU_BP_NAND_CLK_GATE      )
#define CCMU_MASK_NAND_CLK_SRC         ( CCMU_BITS_2 << CCMU_BP_NAND_CLK_SRC       )
#define CCMU_MASK_NAND_CLK_PRE_DIV     ( CCMU_BITS_2 << CCMU_BP_NAND_CLK_PRE_DIV   )
#define CCMU_MASK_NAND_CLK_DIV         ( CCMU_BITS_4 << CCMU_BP_NAND_CLK_DIV       )


#endif    // #ifndef _CCMU_H_
