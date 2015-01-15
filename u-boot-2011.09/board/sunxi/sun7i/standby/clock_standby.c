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
//#include "include.h"
#include <asm/arch/ccmu.h>
#include <asm/io.h>
static  __u32  pll1_value = 0;
static  __u32  pll2_value = 0;
static  __u32  pll3_value = 0;
static  __u32  pll4_value = 0;
static  __u32  pll5_value = 0;
static  __u32  pll6_value = 0;
static  __u32  pll7_value = 0;
static  __u32  pll8_value = 0;
static  __u32  clock_div  = 0;

__s32 standby_clock_store(void)
{
    /*
    pll1_value = CCMU_REG_PLL1_CTRL;
    pll2_value = CCMU_REG_PLL2_CTRL;
    pll3_value = CCMU_REG_PLL3_CTRL;
    pll4_value = CCMU_REG_PLL4_CTRL;
    pll5_value = CCMU_REG_PLL5_CTRL;
    pll6_value = CCMU_REG_PLL6_CTRL;
    pll7_value = CCMU_REG_PLL7_CTRL;
    pll8_value = CCMU_REG_PLL8_CTRL;
    clock_div  = CCMU_REG_AHB_APB & 0x3ff;
    */
    pll1_value = readl(CCM_PLL1_CPUX_CTRL);
    pll2_value = readl(CCM_PLL2_AUDIO_CTRL);
    pll3_value = readl(CCM_PLL3_VIDEO_CTRL);
    pll4_value = readl(CCM_PLL4_VE_CTRL);
    pll5_value = readl(CCM_PLL5_DDR_CTRL);
    pll6_value = readl(CCM_PLL6_MOD_CTRL);
    pll7_value = readl(CCM_PLL7_VIDEO1_CTRL);
    pll8_value = readl(CCM_PLL8_GPU_CTRL);
    clock_div  = readl(CCM_AHB_APB0_CTRL) & 0x3ff;
    
    return 0;
}


__s32 standby_clock_restore(void)
{
    writel(pll1_value, CCM_PLL1_CPUX_CTRL);
    writel(pll2_value, CCM_PLL2_AUDIO_CTRL);
    writel(pll3_value, CCM_PLL3_VIDEO_CTRL);
    writel(pll4_value, CCM_PLL4_VE_CTRL);
    writel(pll5_value, CCM_PLL5_DDR_CTRL);
    writel(pll6_value, CCM_PLL6_MOD_CTRL);
    writel(pll7_value, CCM_PLL7_VIDEO1_CTRL);
    writel(pll8_value, CCM_PLL8_GPU_CTRL);

    return 0;
}

__s32 standby_clock_to_source(int clock_source)
{
    __u32 reg_val;

    reg_val = readl(CCM_AHB_APB0_CTRL);
    reg_val &= ~(0x03 << 16);
    if(clock_source == 32000)
    {
        ;
    }
    else if(clock_source == 24000000)
    {
        reg_val |= 1 << 16;
    }
    else
    {
        reg_val |= 2 << 16;
    }
    writel(reg_val, CCM_AHB_APB0_CTRL);

    return 0;
}

void standby_clock_plldisable(void)
{
    writel(readl(CCM_PLL1_CPUX_CTRL) & ~(1U << 31), CCM_PLL1_CPUX_CTRL);
    writel(readl(CCM_PLL2_AUDIO_CTRL) & ~(1U << 31), CCM_PLL2_AUDIO_CTRL);
    writel(readl(CCM_PLL3_VIDEO_CTRL) & ~(1U << 31), CCM_PLL3_VIDEO_CTRL);
    writel(readl(CCM_PLL4_VE_CTRL) & ~(1U << 31), CCM_PLL4_VE_CTRL);
    writel(readl(CCM_PLL5_DDR_CTRL) & ~(1U << 31), CCM_PLL5_DDR_CTRL);
    writel(readl(CCM_PLL6_MOD_CTRL) & ~(1U << 31), CCM_PLL6_MOD_CTRL);
    writel(readl(CCM_PLL7_VIDEO1_CTRL) & ~(1U << 31), CCM_PLL7_VIDEO1_CTRL);
    writel(readl(CCM_PLL8_GPU_CTRL) & ~(1U << 31), CCM_PLL8_GPU_CTRL);
}

void standby_clock_divsetto0(void)
{
    __u32 reg_val;

    reg_val = readl(CCM_AHB_APB0_CTRL);
    reg_val &= ~0x3ff;
    writel(reg_val, CCM_AHB_APB0_CTRL);
}

void standby_clock_divsetback(void)
{
    __u32 reg_val;

    reg_val = readl(CCM_AHB_APB0_CTRL);
    reg_val &= ~0x3ff;
    reg_val |= clock_div;
    writel(reg_val, CCM_AHB_APB0_CTRL);
}

void standby_clock_drampll_ouput(int op)
{
    __u32 reg_val;

    reg_val = readl(CCM_PLL5_DDR_CTRL);
    if(op)
    {
        reg_val |= 1 << 29;
    }
    else
    {
        reg_val &= ~(1 << 29);
    }
    writel(reg_val, CCM_PLL5_DDR_CTRL);

    return;
}

/*
 *********************************************************************************************************
 *                           standby_tmr_enable_watchdog
 *
 *Description: enable watch-dog.
 *
 *Arguments  : none.
 *
 *Return     : none;
 *
 *Notes      :
 *
 *********************************************************************************************************
 */
#define  TMR_WATCHDOG_MODE   (*(volatile unsigned int *)(0x1c20C00 + 0x94))

void standby_tmr_enable_watchdog(void)
{
    /* set watch-dog reset, timeout is 2 seconds */
    TMR_WATCHDOG_MODE = (2<<3) | (1<<1);
    /* enable watch-dog */
    TMR_WATCHDOG_MODE |= (1<<0);
}


/*
 *********************************************************************************************************
 *                           standby_tmr_disable_watchdog
 *
 *Description: disable watch-dog.
 *
 *Arguments  : none.
 *
 *Return     : none;
 *
 *Notes      :
 *
 *********************************************************************************************************
 */
void standby_tmr_disable_watchdog(void)
{
    /* disable watch-dog reset */
    TMR_WATCHDOG_MODE &= ~(1<<1);
    /* disable watch-dog */
    TMR_WATCHDOG_MODE &= ~(1<<0);
}

void standby_clock_apb1_to_source(int clock)
{
    __u32 reg_val;

    reg_val = readl(CCM_APB1_CLK_CTRL);
    reg_val &= ~(0x03 << 24);
    if(clock == 32000)
    {
        reg_val |= (2 << 24);
    }
    writel(reg_val, CCM_APB1_CLK_CTRL);
}


void standby_clock_24m_op(int op)
{
    __u32 reg_val1, reg_val2;

    if(!op)
    {
        reg_val1 = readl(CCM_OSC24M_CTRL);
        reg_val1 &= ~1;
        writel(reg_val1, CCM_OSC24M_CTRL);

        reg_val2 = readl(CCM_OSC24M_CTRL);
        reg_val2 &= ~(0xffU << 24);
        reg_val2 |=  (0xA7U << 24);
        reg_val2 &= ~(0x01 << 16);
        writel(reg_val2, CCM_OSC24M_CTRL);

        reg_val1 = readl(CCM_PLL5_DDR_CTRL);
        //////////////////////cut here////////////////////////
        reg_val1 &= ~(1 << 7);
        writel(reg_val1, CCM_PLL5_DDR_CTRL);

        reg_val2 &= ~(0xffU << 24);
        writel(reg_val2, CCM_OSC24M_CTRL);
    }
    else
    {
        volatile int i;

        reg_val1 = readl(CCM_OSC24M_CTRL);
        reg_val1 &= ~(0xffU << 24);
        reg_val1 |=  (0xA7U << 24);
        reg_val1 |=  (0x01 << 16);
        writel(reg_val1, CCM_OSC24M_CTRL);

        reg_val2 = readl(CCM_PLL5_DDR_CTRL);
        reg_val2 |= 1 << 7;
        writel(reg_val2, CCM_PLL5_DDR_CTRL);

        reg_val1 &= ~(0xffU << 24);
        writel(reg_val1, CCM_OSC24M_CTRL);

        for(i=0;i<100;i++);

        reg_val1 = readl(CCM_OSC24M_CTRL);
        reg_val1 |= 1;
        writel(reg_val1, CCM_OSC24M_CTRL);
        for(i=0;i<100;i++);
    }

    return;
}

