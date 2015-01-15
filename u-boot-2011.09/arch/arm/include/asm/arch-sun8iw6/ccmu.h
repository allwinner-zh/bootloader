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

#ifndef __CCMU_H
#define __CCMU_H
#include <linux/types.h>
#include "platform.h"



#define CCMU_PLL_C0CPUX_CTRL_REG            (CCM_BASE + 0x00)
#define CCMU_PLL_C1CPUX_CTRL_REG            (CCM_BASE + 0x04)
#define CCMU_PLL_PERIPH_CTRL_REG            (CCM_BASE + 0x28)
#define CCMU_PLL_HSIC_CTRL_REG              (CCM_BASE + 0x44)


#define CCMU_CPUX_AXI_CFG_REG               (CCM_BASE + 0x50)
#define CCMU_AHB1_APB1_CFG_REG              (CCM_BASE + 0x54)
#define CCMU_APB2_CFG_GREG                  (CCM_BASE + 0x58)
#define CCMU_AHB2_CFG_GREG                  (CCM_BASE + 0x5C)

#define CCMU_BUS_CLK_GATING_REG0            (CCM_BASE + 0x60)
#define CCMU_BUS_CLK_GATING_REG1            (CCM_BASE + 0x64)
#define CCMU_BUS_CLK_GATING_REG2            (CCM_BASE + 0x68)
#define CCMU_BUS_CLK_GATING_REG3            (CCM_BASE + 0x6C)

#define CCMU_CCI400_CFG_REG                 (CCM_BASE + 0x78)
#define CCMU_MBUS_RST_REG                   (CCM_BASE + 0xFC)

#define CCMU_MBUS_CLK_REG                   (CCM_BASE + 0x15C)
#define CCMU_PLL_STB_STATUS_REG             (CCM_BASE + 0x20C)
#define CCMU_BUS_SOFT_RST_REG0              (CCM_BASE + 0x2C0)

#define CCMU_AVS_CLK_REG                    (CCM_BASE + 0x144)
#define CCMU_PLL_C0CPUX_BIAS_REG            (CCM_BASE + 0x220)

#define CCM_SS_SCLK_CTRL		            (CCM_BASE + 0x9C)

#define CCMU_BUS_SOFT_RST_REG0              (CCM_BASE + 0x2C0)
#define CCMU_BUS_SOFT_RST_REG1              (CCM_BASE + 0x2C4)
#define CCMU_BUS_SOFT_RST_REG2              (CCM_BASE + 0x2C8)
#define CCMU_BUS_SOFT_RST_REG3              (CCM_BASE + 0x2D0)
#define CCMU_BUS_SOFT_RST_REG4              (CCM_BASE + 0x2D8)
//#define CCMU_REG_SD_MMC0                    (CCM_BASE + 0x88)                   
//#define CCMU_REG_SD_MMC2                    (CCM_BASE + 0x90)  //clk reg

//sd/mmc clk reg  
#define CCM_SDC0_SCLK_CTRL                  (CCM_BASE+0x088)
#define CCM_SDC1_SCLK_CTRL                  (CCM_BASE+0x08c)
#define CCM_SDC2_SCLK_CTRL                  (CCM_BASE+0x090)
#define CCM_SDC3_SCLK_CTRL                  (CCM_BASE+0x094)

//--for other file ,use before define 
#define CCM_AVS_SCLK_CTRL                   (CCMU_AVS_CLK_REG)
#define CCM_AHB1_GATE0_CTRL			        (CCMU_BUS_CLK_GATING_REG0)
#define CCM_AHB1_RST_REG0                   (CCMU_BUS_SOFT_RST_REG0)


#endif
