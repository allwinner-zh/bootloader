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
#ifndef _CCMU_H_
#define _CCMU_H_

#include  "hd_scrpt.h"

/* Offset */
#define CCMU_REG_o_PLL1_CTRL            0x00
#define CCMU_REG_o_PLL2_CTRL            0x08
#define CCMU_REG_o_PLL3_CTRL            0x10
#define CCMU_REG_o_PLL4_CTRL            0x18
#define CCMU_REG_o_PLL5_CTRL            0x20
#define CCMU_REG_o_PLL6_CTRL            0x28
//#define CCMU_REG_o_PLL7_CTRL            0x30
#define CCMU_REG_o_PLL8_CTRL            0x38

#define CCMU_REG_o_AXI_MOD				0x50
#define CCMU_REG_o_AHB1_APB1			0x54
#define CCMU_REG_o_APB2_DIV				0x58
//#define CCMU_REG_o_AXI_GATING			0x5C
#define CCMU_REG_o_AHB1_GATING0			0x60
#define CCMU_REG_o_AHB1_GATING1			0x64
#define CCMU_REG_o_APB1_GATING			0x68
#define CCMU_REG_o_APB2_GATING			0x6C

#define CCMU_REG_o_NAND0                0x80
#define CCMU_REG_o_NAND1                0x84
#define CCMU_REG_o_SD_MMC0              0x88
#define CCMU_REG_o_SD_MMC2              0x90

#define CCMU_REG_o_AVS                  0x144

#define CCMU_REG_o_MBUS0                0x15c
#define CCMU_REG_o_MBUS1                0x160


#define CCMU_REG_o_AHB1_RESET0			0x2C0
#define CCMU_REG_o_AHB1_RESET1			0x2C4
#define CCMU_REG_o_APB1_RESET			0x2D0
#define CCMU_REG_o_APB2_RESET			0x2D4


#define CCMU_REG_PLL_DDR0_CTRL          (CCMU_REGS_BASE + 0x20)
#define CCMU_REG_PLL_DDR1_CTRL          (CCMU_REGS_BASE + 0x4C)

/* registers */
#define CCMU_REG_PLL1_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL1_CTRL   )
#define CCMU_REG_PLL2_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL2_CTRL   )
#define CCMU_REG_PLL3_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL3_CTRL   )
#define CCMU_REG_PLL4_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL4_CTRL   )
#define CCMU_REG_PLL5_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL5_CTRL   )
#define CCMU_REG_PLL6_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL6_CTRL   )
//#define CCMU_REG_PLL7_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL7_CTRL   )
#define CCMU_REG_PLL8_CTRL              ( CCMU_REGS_BASE + CCMU_REG_o_PLL8_CTRL   )


#define CCMU_REG_AXI_MOD                ( CCMU_REGS_BASE + CCMU_REG_o_AXI_MOD     )
#define CCMU_REG_AHB1_APB1				( CCMU_REGS_BASE + CCMU_REG_o_AHB1_APB1   )
#define CCMU_REG_APB2_DIV               ( CCMU_REGS_BASE + CCMU_REG_o_APB2_DIV    )
//#define CCMU_REG_AXI_GATING				( CCMU_REGS_BASE + CCMU_REG_o_AXI_GATING  )
#define CCMU_REG_AHB1_GATING0			( CCMU_REGS_BASE + CCMU_REG_o_AHB1_GATING0)
#define CCMU_REG_AHB1_GATING1			( CCMU_REGS_BASE + CCMU_REG_o_AHB1_GATING1)
#define CCMU_REG_APB1_GATING			( CCMU_REGS_BASE + CCMU_REG_o_APB1_GATING )
#define CCMU_REG_APB2_GATING			( CCMU_REGS_BASE + CCMU_REG_o_APB2_GATING )


#define CCMU_REG_SD_MMC0                ( CCMU_REGS_BASE + CCMU_REG_o_SD_MMC0     )
#define CCMU_REG_SD_MMC2                ( CCMU_REGS_BASE + CCMU_REG_o_SD_MMC2     )


#define CCMU_REG_MBUS0                  ( CCMU_REGS_BASE + CCMU_REG_o_MBUS0       )
#define CCMU_REG_MBUS1                  ( CCMU_REGS_BASE + CCMU_REG_o_MBUS1       )
#define CCMU_REG_MBUS_REST              ( CCMU_REGS_BASE + 0xFC                   )

#define CCMU_REG_AVS					( CCMU_REGS_BASE + CCMU_REG_o_AVS         )


#define CCMU_REG_AHB1_RESET0			( CCMU_REGS_BASE + CCMU_REG_o_AHB1_RESET0 )
#define CCMU_REG_AHB1_RESET1			( CCMU_REGS_BASE + CCMU_REG_o_AHB1_RESET1 )
#define CCMU_REG_APB1_RESET	    		( CCMU_REGS_BASE + CCMU_REG_o_APB1_RESET  )
#define CCMU_REG_APB2_RESET  			( CCMU_REGS_BASE + CCMU_REG_o_APB2_RESET  )



	/* CCMU_REG_AHB1_RST_REG0*/   //--for 1650
#define CCMU_BP_NAND_AHB_MOD_RST        13
#define CCMU_BP_DMA_AHB_MOD_RST         6


  	/* CCMU_REG_AHB_MODULE0 bit position */
#define CCMU_BP_NAND_AHB_CLK_GATE       13
#define CCMU_BP_DMA_AHB_CLK_GATE         6

#endif    // #ifndef _CCMU_H_
