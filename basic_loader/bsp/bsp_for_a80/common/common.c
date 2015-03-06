/*
 * (C) Copyright 2012
 *     wangflord@allwinnertech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */
#include "types.h"
#include "arch.h"

void __usdelay(__u32 us);
void __msdelay(__u32 ms);

void set_pll_bias(void)
{
    __u32 reg_val = 0 ;
    __u32 i = 0 ;
    __u32 reg_addr = CCM_PLL_C0CPUX_BIAS;
    for(i = 0;i<12;i++)
    {
        reg_val = readl(reg_addr + (i<<2));
        reg_val &= ~(0x1f<<16);
        reg_val |= 0x04<<16;
        writel(reg_val,reg_addr + (i<<2));
    }
}

#define SUNXI_GTBUS_PBASE           (0x01c09000)
#define GT_MST_CFG_REG(n)   		((SUNXI_GTBUS_PBASE + 0x0000 + (0x4 * n))) /* n = 0 ~ 35 */
#define GT_BW_WDW_CFG_REG   		((SUNXI_GTBUS_PBASE + 0x0100))
#define GT_MST_READ_PROI_CFG_REG0	((SUNXI_GTBUS_PBASE + 0x0104))
#define GT_MST_READ_PROI_CFG_REG1	((SUNXI_GTBUS_PBASE + 0x0108))
#define GT_LVL2_MAST_CFG_REG    	((SUNXI_GTBUS_PBASE + 0x010c))
#define GT_SW_CLK_ON_REG    		((SUNXI_GTBUS_PBASE + 0x0110))
#define GT_SW_CLK_OFF_REG   		((SUNXI_GTBUS_PBASE + 0x0114))
#define GT_PMU_MST_EN_REG   		((SUNXI_GTBUS_PBASE + 0x0118))
#define GT_PMU_CFG_REG  			((SUNXI_GTBUS_PBASE + 0x011c))
#define GT_PMU_CNT_REG(n)   		((SUNXI_GTBUS_PBASE + 0x0120 + (0x4 * n))) /* n = 0 ~ 18 */
/* generaly, the below registers are not needed to modify */
#define GT_CCI400_CONFIG_REG0   	((SUNXI_GTBUS_PBASE + 0x0200))
#define GT_CCI400_CONFIG_REG1   	((SUNXI_GTBUS_PBASE + 0x0204))
#define GT_CCI400_CONFIG_REG2   	((SUNXI_GTBUS_PBASE + 0x0208))
#define GT_CCI400_STATUS_REG0   	((SUNXI_GTBUS_PBASE + 0x020c))
#define GT_CCI400_STATUS_REG1   	((SUNXI_GTBUS_PBASE + 0x0210))
/*******************************************************************************
*函数名称: gtbus
*函数原型：void gtbus( void )
*函数功能: Boot0中用C语言编写的 调试显示的优先级
*入口参数: void
*返 回 值: void
*备    注:
*******************************************************************************/
void gtbus_init(void)
{
    writel(0xff0     ,GT_MST_READ_PROI_CFG_REG0);
    writel(0x3fffffff,GT_MST_CFG_REG(4));
    writel(0x3fffffff,GT_MST_CFG_REG(5));
    writel(0x3fffffff,GT_MST_CFG_REG(6));
    writel(0x3fffffff,GT_MST_CFG_REG(7));
    writel(0x3fffffff,GT_MST_CFG_REG(8));
    writel(0x3fffffff,GT_MST_CFG_REG(9));
    writel(0x3fffffff,GT_MST_CFG_REG(10));
    writel(0x3fffffff,GT_MST_CFG_REG(11));
}

/*******************************************************************************
*函数名称: set_pll
*函数原型：void set_pll( void )
*函数功能: Boot0中用C语言编写的 调整CPU频率
*入口参数: void
*返 回 值: void
*备    注:
*******************************************************************************/
void set_pll( void )
{
	__u32 reg_val;

	//切换到24M
	reg_val = readl(CCM_CPU_SOURCECTRL);
	reg_val &= ~1;
	writel(reg_val, CCM_CPU_SOURCECTRL);

	__usdelay(10);
	__asm{dmb};
	__asm{isb};
	//设置PLL_P1=1200
	reg_val = readl(CCM_PLL12_PERP1_CTRL);
	if(!(reg_val & 0x80000000))
	{
		writel(0x00003200, CCM_PLL12_PERP1_CTRL);
		do
		{
			__usdelay(10);
		}
		while(readl(CCM_PLL12_PERP1_CTRL) != 0x00003200);
		writel(0x80003200, CCM_PLL12_PERP1_CTRL);
	}

	//设置PLL_C0CPUX到408M
	writel(0x02001100, CCM_PLL1_C0_CTRL);
	do
	{
		__usdelay(10);
	}
	while(readl(CCM_PLL1_C0_CTRL) != 0x02001100);
	writel(0x82001100, CCM_PLL1_C0_CTRL);

	//设置PLL_C1CPUX到408M
	writel(0x02001100, CCM_PLL2_C1_CTRL);
	do
	{
		__usdelay(10);
	}
	while(readl(CCM_PLL2_C1_CTRL) != 0x02001100);
	writel(0x82001100, CCM_PLL2_C1_CTRL);

	//设置PLL_P0=960
	reg_val = readl(CCM_PLL4_PERP0_CTRL);
	if(!(reg_val & 0x80000000))
	{
		writel(0x00002800, CCM_PLL4_PERP0_CTRL);
		do
		{
			__usdelay(10);
		}
		while(readl(CCM_PLL4_PERP0_CTRL) != 0x00002800);
		writel(0x80002800, CCM_PLL4_PERP0_CTRL);
	}
	else
	{
		writel(0x80002800, CCM_PLL4_PERP0_CTRL);
	}

	__usdelay(5000);

	__asm{dmb};
	__asm{isb};
	//设置AXI0 & ATB0 分频 2:2
	writel(0x101, CCM_CLUSTER0_AXI_RATIO);
	__usdelay(50);
	__asm{dmb};
	__asm{isb};
	//设置AHB0，时钟源为PLL_P0，除频=2，即AHB0=960/8=120
	writel(0x00000003, CCM_AHB0_RATIO_CTRL);
	do
	{
		__usdelay(10);
	}
	while(readl(CCM_AHB0_RATIO_CTRL) != 0x00000003);
	writel(0x01000003, CCM_AHB0_RATIO_CTRL);
	//设置AHB1，时钟源为PLL_P0，除频=2，即AHB1=960/4=240
	writel(0x00000002, CCM_AHB1_RATIO_CTRL);
	do
	{
		__usdelay(10);
	}
	while(readl(CCM_AHB1_RATIO_CTRL) != 0x00000002);
	writel(0x01000002, CCM_AHB1_RATIO_CTRL);
//	设置AHB2，时钟源为PLL_P0，除频=2，即GTBUS=960/8=240
	writel(0x00000003, CCM_AHB2_RATIO_CTRL);
	do
	{
		__usdelay(10);
	}
	while(readl(CCM_AHB2_RATIO_CTRL) != 0x00000003);
	writel(0x01000003, CCM_AHB2_RATIO_CTRL);
//	设置APB0，时钟源为PLL_P0，除频=3，即GTBUS=960/8=120
	writel(0x00000003, CCM_APB0_RATIO_CTRL);
	do
	{
		__usdelay(10);
	}
	while(readl(CCM_APB0_RATIO_CTRL) != 0x00000003);
	writel(0x01000003, CCM_APB0_RATIO_CTRL);
	//设置GTBUS，时钟源为PLL_P1，除频=2，即GTBUS=1200/3=400
	writel(0x00000002, CCM_GTCLK_RATIO_CTRL);
	do
	{
		__usdelay(10);
	}
	while(readl(CCM_GTCLK_RATIO_CTRL) != 0x00000002);
	writel(0x02000002, CCM_GTCLK_RATIO_CTRL);
//	设置CCI400，时钟源为PLL_P0，除频=1，即CCI400=960/2=480
	__usdelay(100);
	__asm{dmb};
	__asm{isb};
	reg_val = readl(CCM_CCI400_RATIO_CTRL);
	if(reg_val & (0x3<<24))
	{
	    writel(0x1, CCM_CCI400_RATIO_CTRL);
	    __usdelay(50);
	}
	writel(0x01000001, CCM_CCI400_RATIO_CTRL);

	__usdelay(100);
	__asm{dmb};
	__asm{isb};
	//设置Cluster0时钟源为PLL_C0CPUX
	reg_val = readl(CCM_CPU_SOURCECTRL);
	reg_val |= 1;
	writel(reg_val, CCM_CPU_SOURCECTRL);

	//设置Cluster1时钟源为PLL_C1CPUX
	reg_val = readl(CCM_CPU_SOURCECTRL);
	reg_val |= 1 << 8;
	writel(reg_val, CCM_CPU_SOURCECTRL);

	__usdelay(1000);
	__asm{dmb};
	__asm{isb};
	//打开GPIO
	writel(readl(CCM_APB0_GATE0_CTRL)   | (1 << 5), CCM_APB0_GATE0_CTRL);
	writel(readl(0x08001400 + 0x28)       | 0x01,   0x08001400 + 0x28);
	//打开DMA
	writel(readl(CCM_AHB1_RST_REG0)   | (1 << 24), CCM_AHB1_RST_REG0);
	writel(readl(CCM_AHB1_GATE0_CTRL) | (1 << 24), CCM_AHB1_GATE0_CTRL);
	//关闭dma autoclock gating
	//writel(3, DMAC_REGS_BASE + 0x28);
	//打开TIMESTAMP
	writel(1, 0x01720000);
        set_pll_bias();
        gtbus_init();
	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void pll_reset( void )
{
    //切换CPU时钟源为24M
    writel(0x00000000, CCM_CPU_SOURCECTRL);
    //还原PLL_C0CPUX为默认值
	writel(0x02001100, CCM_PLL1_C0_CTRL);

	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void timer_init(void)
{
	CCMU_REG_AVS |= (1U << 31);

	TMRC_AVS_CTRL = 3;
	TMRC_AVS_DIVISOR = 0xC2EE0;
	TMRC_AVS_COUNT0 = 0;
	TMRC_AVS_COUNT1 = 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void __msdelay(__u32 ms)
{
	__u32 t1, t2;

	t1 = TMRC_AVS_COUNT0;
	t2 = t1 + ms;
	do
	{
		t1 = TMRC_AVS_COUNT0;
	}
	while(t2 >= t1);

	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void __usdelay(__u32 us)
{
	__u32 t1, t2;

	t1 = TMRC_AVS_COUNT1;
	t2 = t1 + us;
	do
	{
		t1 = TMRC_AVS_COUNT1;
	}
	while(t2 >= t1);

	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void timer_exit(void)
{
	TMRC_AVS_CTRL = 0;
	TMRC_AVS_DIVISOR = 0xC2EE0;
	TMRC_AVS_COUNT0 = 0;

	CCMU_REG_AVS &= ~(1U << 31);
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void bias_calibration(void)
{

	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void disbale_cpus(void)
{

	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void config_pll1_para(void)
{

	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
void  set_vldo_for_pll(void)
{

	return ;
}
void cpu_init_s(void)
{
	timer_init();

	config_pll1_para();
	set_vldo_for_pll();
	disbale_cpus();
	set_pll();
}

//-----------for rtc --------------------
#define msg(fmt,args...)				UART_printf2(fmt ,##args)
#define BOOT_FEL_FLAG                   (0x5AA5A55A)

unsigned int  get_fel_flag(void)
{
    unsigned int fel_flag;
    volatile unsigned int * rtc_addr = (volatile unsigned int *)(0x08001400 + 0x1f0);
    
    *rtc_addr = (1<<16); //set index 1
    fel_flag = *rtc_addr & 0xff;
    if(fel_flag == (BOOT_FEL_FLAG & 0xff))
        fel_flag = BOOT_FEL_FLAG;
    
    return fel_flag;
}
void show_rtc_reg(void)
{
    int i;
    __u32 rtc_flag;
    volatile unsigned int * rtc_addr = (volatile unsigned int *)(0x08001400 + 0x1f0);
    
    for(i=1;i<=3;i++)
    {
        *rtc_addr = (i<<16);
        rtc_flag = *rtc_addr;
        msg("rtc %d value %x\n", i, rtc_flag);
    }
}

void  clear_fel_flag(void)
{
    volatile unsigned int *rtc_addr = (volatile unsigned int *)(0x08001400 + 0x1f0);
	do
    {
		*rtc_addr = (1<<16);
    	*rtc_addr = (1U<<31) | (1<<16);
    	__usdelay(10);
    	*rtc_addr = (1<<16);
	}
	while((*rtc_addr & 0xff) != 0);
}