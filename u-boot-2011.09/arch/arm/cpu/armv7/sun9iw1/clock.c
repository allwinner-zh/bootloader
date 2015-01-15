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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/cpu.h>
#include <pmu.h>
#include <smc.h>
#include <sys_config.h>
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
extern int get_cluster_id(void);
int sunxi_clock_get_corepll(void)
{
	unsigned int reg_val;
	int 	div_p;
	int 	factor_n;
	int 	clock;
    if(get_cluster_id() == 0)
	    reg_val  = smc_readl(CCM_PLL1_C0_CTRL);
    else
        reg_val = smc_readl(CCM_PLL2_C1_CTRL);
	factor_n = ((reg_val >>  8) & 0xff);

	div_p    = ((reg_val >> 16) & 0x1);
	if(!div_p)
	{
		div_p = 1;
	}
	else
	{
		div_p = 4;
	}

	clock = 24 * factor_n/div_p;

	return clock;
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
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int sunxi_clock_get_pll4_periph1(void)
{
	unsigned int reg_val;
	int 	div1, div2;
	int 	factor_n;

	reg_val = smc_readl(CCM_PLL4_PERP0_CTRL);

	factor_n = ((reg_val >>  8) & 0xff);
	div1     = ((reg_val >> 16) & 0x1) + 1;
	div2     = ((reg_val >> 18) & 0x1) + 1;

	return 24 * factor_n/div1/div2;
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
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int sunxi_clock_get_pll12_periph2(void)
{
	unsigned int reg_val;
	int 	div1, div2;
	int 	factor_n;

	reg_val = smc_readl(CCM_PLL12_PERP1_CTRL);

	factor_n = ((reg_val >>  8) & 0xff);
	div1     = ((reg_val >> 16) & 0x1) + 1;
	div2     = ((reg_val >> 18) & 0x1) + 1;

	return 24 * factor_n/div1/div2;
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
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int sunxi_clock_get_gtclock(void)
{
	unsigned int reg_val;
	int 	src_sel;
	int 	ratio;
	int 	clock;

	reg_val = smc_readl(CCM_GTCLK_RATIO_CTRL);

	ratio   = ((reg_val >>  0) & 0x3);
	src_sel = ((reg_val >> 24) & 0x3);

	if(src_sel == 0)
	{
		clock = 24;
	}
	else if(src_sel == 1)
	{
		clock = sunxi_clock_get_pll4_periph1();
	}
	else
	{
		clock = sunxi_clock_get_pll12_periph2();
	}

	return clock/(ratio + 1);
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
int sunxi_clock_get_pll5_ve(void)
{
	unsigned int reg_val;
	int 	div1, div2;
	int 	factor_n;

	reg_val = smc_readl(CCM_PLL5_VE_CTRL);

	factor_n = ((reg_val >>  8) & 0xff);
	div1     = ((reg_val >> 16) & 0x1) + 1;
	div2     = ((reg_val >> 18) & 0x1) + 1;

	return 24 * factor_n/(div1+1)/(div2+1);
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
int sunxi_clock_get_pll6_ddr(void)
{
	unsigned int reg_val;
	int 	div1, div2;
	int 	factor_n;

	reg_val = smc_readl(CCM_PLL6_DDR_CTRL);

	factor_n = ((reg_val >>  8) & 0xff);
	div1     = ((reg_val >> 16) & 0x1) + 1;
	div2     = ((reg_val >> 18) & 0x1) + 1;

	return 24 * factor_n/(div1+1)/(div2+1);
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
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int sunxi_clock_get_axi(void)
{
	int clock;
	unsigned int reg_val;
	int clock_src, factor;

	reg_val   = smc_readl(CCM_CPU_SOURCECTRL);
	clock_src = (reg_val >> 0) & 0x01;

	if(!clock_src)
	{
		clock = 24;
	}
	else
	{
		clock = sunxi_clock_get_corepll();
	}

	reg_val = smc_readl(CCM_CLUSTER0_AXI_RATIO);

	factor  = (reg_val >> 0) & 0x07;
	if(factor >= 3)
	{
		factor = 4;
	}
	else
	{
		factor ++;
	}

	return clock/factor;
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
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int sunxi_clock_get_ahb(int index)
{
	unsigned int reg_val;
	int 	ratio;
	int 	src_sel;
	int 	clock;

	if(index == 0)
	{
		reg_val = smc_readl(CCM_AHB0_RATIO_CTRL);
	}
	else if(index == 1)
	{
		reg_val = smc_readl(CCM_AHB1_RATIO_CTRL);
	}
	else if(index == 2)
	{
		reg_val = smc_readl(CCM_AHB2_RATIO_CTRL);
	}
	else
	{
		printf("sunxi ahb: invalid ahb index %d\n", index);

		return 0;
	}

	ratio   = ((reg_val >>  0) & 0x3);
	src_sel = ((reg_val >> 24) & 0x3);

	if(src_sel == 0)
	{
		clock = sunxi_clock_get_gtclock();
	}
	else if(src_sel == 1)
	{
		clock = sunxi_clock_get_pll4_periph1();
	}
	else
	{
		clock = sunxi_clock_get_pll12_periph2();
	}

	return clock/(1<<ratio);
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
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int sunxi_clock_get_apb0(void)
{
	unsigned int reg_val;
	int 	ratio;
	int 	src_sel;
	int 	clock;

	reg_val = smc_readl(CCM_APB0_RATIO_CTRL);

	ratio   = ((reg_val >>  0) & 0x3);
	src_sel = ((reg_val >> 24) & 0x1);

	if(src_sel == 0)
	{
		clock = sunxi_clock_get_gtclock();
	}
	else
	{
		clock = sunxi_clock_get_pll4_periph1();
	}

	return clock/(1<<ratio);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :   当小于288M，采用P=1除频(即除以4)
*                      当大于等于288M，采用P=0除频(即除以1)
*
*
************************************************************************************************************
*/
static int clk_set_pll1_para(int frequency, int core_vol)
{
	unsigned int reg_val;
	int 	div_p=0;
	int 	factor_n;

	reg_val  = smc_readl(CCM_PLL1_C0_CTRL);

	if(frequency <= 288)
	{
		div_p = 1;
		frequency <<= 2;
	}
	factor_n = frequency/24;

	reg_val &= ~(0x1ff << 8);
	reg_val |=  (div_p<<16) | (factor_n << 8);

	smc_writel(reg_val, CCM_PLL1_C0_CTRL);

	return 0;
}
static int clk_set_pll2_para(int frequency, int core_vol)
{
	unsigned int reg_val;
	int 	div_p=0;
	int 	factor_n;

	reg_val  = smc_readl(CCM_PLL2_C1_CTRL);

	if(frequency <= 288)
	{
		div_p = 1;
		frequency <<= 2;
	}
	factor_n = frequency/24;

	reg_val &= ~(0x1ff << 8);
	reg_val |=  (div_p<<16) | (factor_n << 8);

	smc_writel(reg_val, CCM_PLL2_C1_CTRL);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :    固定分频：固定来自于PERIPE2=1200
*
*                       CCI400=600
*                       GTBUS =400
*                       Other =200
*
*
************************************************************************************************************
*/
int sunxi_clk_set_divd(void)
{
	int div1=0, div2=0;
	int clock=1200;
	int factor_n;
	unsigned int reg_val;

	factor_n = clock*(div1+1)*(div2+1)/1200;
	reg_val  = smc_readl(CCM_PLL12_PERP1_CTRL);

	reg_val &= ~((1<<18) | (1<<16) | (0xff<<8));

	reg_val |=  (div2<<18) | (div1<<16) | (factor_n<<8);

	smc_writel(reg_val, CCM_PLL12_PERP1_CTRL);

	//set cci400 = 600
	reg_val  = smc_readl(CCM_CCI400_CTRL);
	reg_val &= ~((3<<24) | (3<<0));
	reg_val |=  (2<<24) | (1<<0);
	smc_writel(reg_val, CCM_CCI400_CTRL);
	//set gtbus  = 400
	reg_val  = smc_readl(CCM_GTCLK_RATIO_CTRL);
	reg_val &= ~((3<<24) | (3<<0));
	reg_val |=  (2<<24) | (2<<0);
	smc_writel(reg_val, CCM_GTCLK_RATIO_CTRL);
	//set AHB0  = 200
	reg_val  = smc_readl(CCM_AHB0_RATIO_CTRL);
	reg_val &= ~((3<<24) | (3<<0));
	reg_val |=  (0<<24) | (1<<0);
	smc_writel(reg_val, CCM_AHB0_RATIO_CTRL);
	//set AHB1  = 200
	reg_val  = smc_readl(CCM_AHB1_RATIO_CTRL);
	reg_val &= ~((3<<24) | (3<<0));
	reg_val |=  (0<<24) | (1<<0);
	smc_writel(reg_val, CCM_AHB1_RATIO_CTRL);
	//set AHB2  = 1200/8
	reg_val  = smc_readl(CCM_AHB2_RATIO_CTRL);
	reg_val &= ~((3<<24) | (3<<0));
	reg_val |=  (2<<24) | (3<<0);
	smc_writel(reg_val, CCM_AHB2_RATIO_CTRL);
	//set APB0  = 1200/8
	reg_val  = smc_readl(CCM_APB0_RATIO_CTRL);
	reg_val &= ~((1<<24) | (3<<0));
	reg_val |=  (1<<24) | (3<<0);
	smc_writel(reg_val, CCM_APB0_RATIO_CTRL);
    //set APB1  = 24
	reg_val  = smc_readl(CCM_APB0_RATIO_CTRL);
	reg_val &= ~(1<<24);
	smc_writel(reg_val, CCM_APB0_RATIO_CTRL);

	return 0;
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
*
*
*    返回值  ：
*
*    说明    ：只限于调整COREPLL
*
*
************************************************************************************************************
*/
int sunxi_clock_set_corepll(int frequency, int core_vol)
{
    unsigned int reg_val;
    unsigned int i;

    //检查时钟是否合法,为0或者超过2G
    if(!frequency)
    {
        //默认频率
        frequency = 408;
    }
    else if(frequency > 3000)
    {
    	frequency = 3000;
    }
    else if(frequency < 200)
    {
		frequency = 24;
    }
    //切换到24M
    reg_val = smc_readl(CCM_CPU_SOURCECTRL);
    reg_val &= ~0x01;
    smc_writel(reg_val, CCM_CPU_SOURCECTRL);
    //延时，等待时钟稳定
    for(i=0; i<0x400; i++);
    //调整时钟频率
    if(frequency != 24)
    {
		clk_set_pll1_para(frequency, core_vol);
		//切换AXI0的分频比，使得AXI0不超过400M，默认值是C0PLL:AXI0=2:1
		if(frequency>1600)
		{
		    smc_writel(0x303, CCM_CLUSTER0_AXI_RATIO);
		}
		else if(frequency>800)
		{
		    smc_writel(0x302, CCM_CLUSTER0_AXI_RATIO);
		}
		else if(frequency>400)
		{
		    smc_writel(0x101, CCM_CLUSTER0_AXI_RATIO);
		}
		//切换时钟到COREPLL上
	    reg_val = smc_readl(CCM_CPU_SOURCECTRL);
	    reg_val |= 0x01;
	    smc_writel(reg_val, CCM_CPU_SOURCECTRL);
	}

    return  0;
}
int sunxi_clock_set_C1corepll(int frequency, int core_vol)
{
    unsigned int reg_val;
    unsigned int i;

    //检查时钟是否合法,为0或者超过2G
    if(!frequency)
    {
        //默认频率
        frequency = 408;
    }
    else if(frequency > 3000)
    {
    	frequency = 3000;
    }
    else if(frequency < 200)
    {
		frequency = 24;
    }
    //切换到24M
    reg_val = smc_readl(CCM_CPU_SOURCECTRL);
    reg_val &= ~(0x01<<8);
    smc_writel(reg_val, CCM_CPU_SOURCECTRL);
    //延时，等待时钟稳定
    for(i=0; i<0x400; i++);
    //调整时钟频率
    if(frequency != 24)
    {
		clk_set_pll2_para(frequency, core_vol);
		//切换AXI0的分频比，使得AXI0不超过400M，默认值是C0PLL:AXI0=2:1
		if(frequency>1600)
		{
		    smc_writel(0x303, CCM_CLUSTER1_AXI_RATIO);
		}
		else if(frequency>800)
		{
		    smc_writel(0x302, CCM_CLUSTER1_AXI_RATIO);
		}
		else if(frequency>400)
		{
		    smc_writel(0x101, CCM_CLUSTER1_AXI_RATIO);
		}
		//切换时钟到COREPLL上
	    reg_val = smc_readl(CCM_CPU_SOURCECTRL);
	    reg_val |= 0x01<<8;
	    smc_writel(reg_val, CCM_CPU_SOURCECTRL);
	}

    return  0;
}

int sunxi_clock_get_C1corepll(void)
{
    unsigned int reg_val;
	unsigned int n, p;

	reg_val = smc_readl(CCM_PLL2_C1_CTRL);
	n = (reg_val >> 8 ) & 0xff;
	p = 1 << ( ((reg_val >> 16) & 0x01)*4 );

	return 24*n/p;
}

