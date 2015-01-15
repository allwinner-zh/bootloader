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

#define CCM_PLL1_C0_CTRL  	    (CCMPLL_BASE+0x000)
#define CCM_PLL2_C1_CTRL 	    (CCMPLL_BASE+0x004)
#define CCM_PLL3_VIDEO_CTRL    	(CCMPLL_BASE+0x008)
#define CCM_PLL4_PERP0_CTRL    	(CCMPLL_BASE+0x00C)
#define CCM_PLL5_VE_CTRL  		(CCMPLL_BASE+0x010)
#define CCM_PLL6_DDR_CTRL  		(CCMPLL_BASE+0x014)
#define CCM_PLL7_VIDEO1_CTRL	(CCMPLL_BASE+0x018)
#define CCM_PLL8_VIDEO2_CTRL  	(CCMPLL_BASE+0x01C)
#define CCM_PLL9_GPU_CTRL		(CCMPLL_BASE+0x020)

#define CCM_PLL12_PERP1_CTRL    (CCMPLL_BASE+0x02C)


#define CCM_CPU_SOURCECTRL		(CCMPLL_BASE+0x050)
#define CCM_CLUSTER0_AXI_RATIO  (CCMPLL_BASE+0x054)
#define CCM_CLUSTER1_AXI_RATIO  (CCMPLL_BASE+0x058)
#define CCM_GTCLK_RATIO_CTRL	(CCMPLL_BASE+0x05C)
#define CCM_AHB0_RATIO_CTRL     (CCMPLL_BASE+0x060)
#define CCM_AHB1_RATIO_CTRL     (CCMPLL_BASE+0x064)
#define CCM_AHB2_RATIO_CTRL     (CCMPLL_BASE+0x068)
#define CCM_APB0_RATIO_CTRL     (CCMPLL_BASE+0x070)
#define CCM_APB1_RATIO_CTRL     (CCMPLL_BASE+0x074)
#define CCM_CCI400_CTRL         (CCMPLL_BASE+0x078)

#define CCM_PLL_C0CPUX_BIAS     (CCMPLL_BASE+0x0a0)
#define CCM_PLL_C1CPUX_BIAS     (CCMPLL_BASE+0x0a4)
#define CCM_PLL_AUDIO_BIAS      (CCMPLL_BASE+0x0a8)
#define CCM_PLL_PERIPH0_BIAS    (CCMPLL_BASE+0x0ac)
#define CCM_PLL_VE_BIAS         (CCMPLL_BASE+0x0b0)
#define CCM_PLL_DDR_BIAS        (CCMPLL_BASE+0x0b4)
#define CCM_PLL_VIDEO0_BIAS     (CCMPLL_BASE+0x0b8)
#define CCM_PLL_VIDEO1_BIAS     (CCMPLL_BASE+0x0bc)
#define CCM_PLL_GPU_BIAS        (CCMPLL_BASE+0x0c0)
#define CCM_PLL_DE_BIAS         (CCMPLL_BASE+0x0c4)
#define CCM_PLL_ISP_BIAS        (CCMPLL_BASE+0x0c8)
#define CCM_PLL_PERIPH1_BIAS    (CCMPLL_BASE+0x0cc)

#define CCM_NAND0_SCLK_CTRL0	(CCMMODULE_BASE+0x000)
#define CCM_NAND0_SCLK_CTRL1	(CCMMODULE_BASE+0x004)
#define CCM_NAND1_SCLK_CTRL0	(CCMMODULE_BASE+0x008)
#define CCM_NAND1_SCLK_CTRL1	(CCMMODULE_BASE+0x00C)

#define CCM_SDC0_SCLK_CTRL		(CCMMODULE_BASE+0x010)
#define CCM_SDC1_SCLK_CTRL		(CCMMODULE_BASE+0x014)
#define CCM_SDC2_SCLK_CTRL		(CCMMODULE_BASE+0x018)
#define CCM_SDC3_SCLK_CTRL		(CCMMODULE_BASE+0x01C)

#define CCM_SS_SCLK_CTRL		(CCMMODULE_BASE+0x02C)

#define CCM_SPI0_SCLK_CTRL		(CCMMODULE_BASE+0x030)
#define CCM_SPI1_SCLK_CTRL		(CCMMODULE_BASE+0x034)
#define CCM_SPI2_SCLK_CTRL		(CCMMODULE_BASE+0x038)
#define CCM_SPI3_SCLK_CTRL		(CCMMODULE_BASE+0x03C)

#define CCM_USHPHY0_SCLK_CTRL	(CCMMODULE_BASE+0x050)

#define CCM_AVS_SCLK_CTRL       (CCMMODULE_BASE+0x0D4)

#define CCM_AHB0_GATE0_CTRL     (CCMMODULE_BASE+0x180)
#define CCM_AHB1_GATE0_CTRL     (CCMMODULE_BASE+0x184)
#define CCM_AHB2_GATE0_CTRL     (CCMMODULE_BASE+0x188)
#define CCM_APB0_GATE0_CTRL     (CCMMODULE_BASE+0x190)
#define CCM_APB1_GATE0_CTRL     (CCMMODULE_BASE+0x194)

#define CCM_AHB0_RST_REG0       (CCMMODULE_BASE+0x1A0)
#define CCM_AHB1_RST_REG0       (CCMMODULE_BASE+0x1A4)
#define CCM_AHB2_RST_REG0       (CCMMODULE_BASE+0x1A8)
#define CCM_APB0_RST_REG0       (CCMMODULE_BASE+0x1B0)
#define CCM_APB1_RST_REG0       (CCMMODULE_BASE+0x1B4)

#define GT_MST_CFG_REG(n)   		((GTBUS_BASE + 0x0000 + (0x4 * n))) /* n = 0 ~ 35 */
#define GT_BW_WDW_CFG_REG   		(GTBUS_BASE + 0x0100)
#define GT_MST_READ_PROI_CFG_REG0	(GTBUS_BASE + 0x0104)
#define GT_MST_READ_PROI_CFG_REG1	(GTBUS_BASE + 0x0108)
#define GT_LVL2_MAST_CFG_REG    	(GTBUS_BASE + 0x010c)
#define GT_SW_CLK_ON_REG    		(GTBUS_BASE + 0x0110)
#define GT_SW_CLK_OFF_REG   		(GTBUS_BASE + 0x0114)
#define GT_PMU_MST_EN_REG   		(GTBUS_BASE + 0x0118)
#define GT_PMU_CFG_REG  			(GTBUS_BASE + 0x011c)
#define GT_PMU_CNT_REG(n)   		((GTBUS_BASE + 0x0120 + (0x4 * n))) /* n = 0 ~ 18 */


void ccm_clock_enable(u32 clk_id);
void ccm_clock_disable(u32 clk_id);
void ccm_module_enable(u32 clk_id);
void ccm_module_disable(u32 clk_id);
void ccm_module_reset(u32 clk_id);
void ccm_clock_disable_all(void);
void ccm_reset_all_module(void);

u32 ccm_setup_pll1_cpux_clk(u32 pll1_clk);
u32 ccm_get_pll1_cpux_clk(void);
u32 ccm_setup_pll3_video0_clk(u32 mode_sel, u32 pll_clk);
u32 ccm_get_pll3_video0_clk(void);
u32 ccm_setup_pll4_ve_clk(u32 mode_sel, u32 pll_clk);
u32 ccm_get_pll4_ve_clk(void);
u32 ccm_setup_pll5_ddr_clk(u32 pll5_clk);
u32 ccm_get_pll5_ddr_clk(void);
u32 ccm_setup_pll6_dev_clk(u32 pll6_clk);
u32 ccm_get_pll6_dev_clk(void);
u32 ccm_setup_pll7_video0_clk(u32 mode_sel, u32 pll_clk);
u32 ccm_get_pll7_video0_clk(void);
u32 ccm_setup_pll8_gpu_clk(u32 mode_sel, u32 pll_clk);
u32 ccm_get_pll8_gpu_clk(void);

void ccm_set_cpu_clk_src(u32 src);
u32  ccm_set_mbus0_clk(u32 src, u32 clk);
u32  ccm_set_mbus1_clk(u32 src, u32 clk);
void ccm_set_cpu_l2_axi_div(u32 periph_div, u32 l2_div, u32 axi_div);
void ccm_set_ahb1_clk_src(u32 src);
void ccm_set_ahb1_apb1_div(u32 prediv, u32 ahb1_div, u32 apb1_div);
s32  ccm_set_apb2_clk(u32 src);
void ccm_set_pll_stable_time(u32 time);
void ccm_set_mclk_stable_time(u32 time);
u32  ccm_get_axi_clk(void);
u32  ccm_get_ahb1_clk(void);
u32  ccm_get_apb1_clk(void);
u32  ccm_get_apb2_clk(void);

#endif
