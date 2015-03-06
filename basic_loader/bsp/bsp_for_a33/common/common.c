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

/*******************************************************************************
*函数名称: set_pll
*函数原型：void set_pll( void )
*函数功能: Boot0中用C语言编写的 调整CPU频率
*入口参数: void
*返 回 值: void
*备    注:
*******************************************************************************/
static void set_pll( void )
{
	__u32 reg_val;
	int delay;

	//选择cpu clock src: OSC24M，设置AXI分频为2
	//cpu/axi clock ratio
	writel(0x00010001, CCMU_REG_AXI_MOD);
	//设置PLL for CPUX CLK: 408M
	//PLL_OUTPUT= (24M*N*K)/M  --factor=[12:8]=16,N=17, K=N+1,M=N+1
	reg_val = (0x00001000) | (0x80000000);
	writel(reg_val, CCMU_REG_PLL1_CTRL);
	//等待lock
#ifndef CONFIG_SUNXI_FPGA
	do
	{
		reg_val = readl(CCMU_REG_PLL1_CTRL);
	}
	while(!(reg_val & (0x1 << 28)));
#endif
	//设置CPU:AXI:AHB:APB分频 4:2:2:1, set AHB1/APB1 clock ratio
	//ahb1 clock src is axi,  0x02 << 12,ahb1 为1分频(0<<4)
	//apb1 clk src is ahb1 clk src,为2分频 (1<<8)
	//
	writel((0x02 << 12) | (1<<8) | (0<<4), CCMU_REG_AHB1_APB1);
	//设置
	reg_val = readl(CCMU_REG_AXI_MOD);
	reg_val &= ~(3 << 8);
	reg_val |=  (1 << 8);		//system apb clk src is cpu clock src, 2分频
	reg_val |=  (1 << 0);       //axi   clk src is cpu clock src, 2分频 
	writel(reg_val, CCMU_REG_AXI_MOD);
	//切换到PLL1
	reg_val = readl(CCMU_REG_AXI_MOD);
	reg_val &= ~(3 << 16);
	reg_val |=  (2 << 16);//clk src is pll_cpu
	writel(reg_val, CCMU_REG_AXI_MOD);
	//打开DMA
	//dma reset
	writel(readl(CCMU_REG_AHB1_RESET0)  | (1 << 6), CCMU_REG_AHB1_RESET0);
	for(delay=0;delay<100;delay++);
	//gating clock for dma pass
	writel(readl(CCMU_REG_AHB1_GATING0) | (1 << 6), CCMU_REG_AHB1_GATING0);
	writel(7, (DMAC_REGS_BASE+0x20));
	//打开MBUS,clk src is pll6
	writel(0x80000000, CCMU_REG_MBUS_REST);       //Assert mbus domain
	writel(0x81000002, CCMU_REG_MBUS0);  //dram>600M, so mbus from 300M->400M
	//使能PLL6
	writel(readl(CCMU_REG_PLL6_CTRL) | (1U << 31), CCMU_REG_PLL6_CTRL);

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
	writel(0x00010000, CCMU_REG_AXI_MOD);
	writel(0x00001000, CCMU_REG_PLL1_CTRL);
	writel(0x00001010, CCMU_REG_AHB1_APB1);
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
	//clock on
	writel(readl(CCMU_REG_AVS) | (1U << 31), CCMU_REG_AVS);
	//avs countor0 enable
	writel(1, TMRC_AVS_CTRL);
	//divisor 1 to 7ff,    24M /Divisor_no
	writel(0x2EE0, TMRC_AVS_DIVISOR); // 0x5db is default by spec
	//init value
	writel(0, TMRC_AVS_COUNT0);
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

	t1 = readl(TMRC_AVS_COUNT0);
	t2 = t1 + ms;
	do
	{
		t1 = readl(TMRC_AVS_COUNT0);
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
	writel(0, TMRC_AVS_CTRL);
	writel(0x5db, TMRC_AVS_DIVISOR);//??
	writel(0, TMRC_AVS_COUNT0);

	writel(readl(CCMU_REG_AVS) & (~(1U << 31)), CCMU_REG_AVS);
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
#if 0
//	//open codec apb gate
//	*(volatile unsigned int *)(0x1c20000 + 0x68) |= 1;
//	//disable codec soft reset
//	*(volatile unsigned int *)(0x1c20000 + 0x2D0) |= 1;
//	//enable HBIASADCEN
//	*(volatile unsigned int *)(0x1c22C00 + 0x28) |= (1 << 29);
#endif
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
static void disbale_cpus(void)
{
#if 0
//	//disable watchdog
//	*(volatile unsigned int *)(0x01f01000 + 0x00) = 0;
//	*(volatile unsigned int *)(0x01f01000 + 0x04) = 1;
//	*(volatile unsigned int *)(0x01f01000 + 0x18) &= ~1;
//	//assert cups
//	*(volatile unsigned int *)(0x01f01C00 + 0x00) = 0;
//	//disable cpus module gating
//	*(volatile unsigned int *)(0x01f01400 + 0x28) = 0;
//	//disable cpus module assert
//	*(volatile unsigned int *)(0x01f01400 + 0xb0) = 0;
#endif
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
static void config_pll1_para(void)
{
#if 0
//	volatile unsigned int value;
//
//	//by sunny at 2013-1-20 17:53:21.
//	value = *(volatile unsigned int *)(0x1c20250);
//	value &= ~(1 << 26);
//	value |= (1 << 26);
//	value &= ~(0x7 << 23);
//	value |= (0x7 << 23);
//	*(volatile unsigned int *)(0x1c20250) = value;
//
//	value = *(volatile unsigned int *)(0x1c20220);
//	value &= ~(0xf << 24);
//	value |= (0xf << 24);
//	*(volatile unsigned int *)(0x1c20220) = value;
#endif
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
static void sram_area_init(void)
{
	volatile unsigned int reg_val;
	uint version;

	*(volatile unsigned int *)(0x01f01400 + 0x0) = 0x00010000;
	*(volatile unsigned int *)(0x01f01400 + 0xC) = 0x00000000;
	*(volatile unsigned int *)(0x01f01400 + 0x28) = 0x9;
	*(volatile unsigned int *)(0x01f01400 + 0xB0) = 0x8;

	/* config ema for cache sram */
	*(volatile unsigned int *)(0x01c00000 + 0x24) |=  (1 << 15);
	version = *(volatile unsigned int *)(0x01c00000 + 0x24);
	*(volatile unsigned int *)(0x01c00000 + 0x24) &= ~(1 << 15);

	version = (version >> 16) & 0xffff;

	if(version == 0x1667)
    	;//*(volatile unsigned int *)(0x01c00044 + 0x00) |= 0x1800;
    else
    	*(volatile unsigned int *)(0x01c00044 + 0x00) |= 0xC0;

}


void cpu_init_s(void)
{
	sram_area_init();
//	config_pll1_para();
	disbale_cpus();
	set_pll();
}

//---------for rtc --------

#define msg(fmt,args...)				UART_printf2(fmt ,##args)

unsigned int  get_fel_flag(void)
{
    unsigned int fel_flag;
    fel_flag = *(volatile unsigned int *)(0x01f00000 + 0x108);
    return fel_flag;
}

void show_rtc_reg(void)
{
    unsigned int reg_val;
    int index = 0;
	volatile unsigned int *reg_addr = 0;
	while(index < 0x18)
	{
		reg_addr = (volatile unsigned int *)(0x01f00000 + 0x100 + index);
		reg_val = *reg_addr;
		msg("reg_addr %x =%x\n", reg_addr, reg_val);
		index+=0x4;
	}
	
}

void  clear_fel_flag(void)
{
	int index = 0;
	volatile unsigned int *reg_addr = 0;
	while(index < 0x18)
	{
		reg_addr = (volatile unsigned int *)(0x01f00000 + 0x100 + index);
		*reg_addr = 0;
		index+=0x4;
	}
}