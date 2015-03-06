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

void __msdelay(__u32 ms);
void __usdelay(__u32 us);

void set_pll_bias(void)
{
    __u32 reg_val = 0 ;
    __u32 i = 0 ;
    __u32 reg_addr = CCMU_PLL_C0CPUX_BIAS_REG;
    for(i = 0;i<12;i++)
    {
        reg_val = readl(reg_addr + (i<<2));
        reg_val &= ~(0x1f<<16);
        reg_val |= 0x04<<16;
        writel(reg_val,reg_addr + (i<<2));
    }
}


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

	//select C0_CPUX  clock src: OSC24M，AXI divide ratio is 2
	//cpu/axi clock ratio
	writel((1<<16 | 1<<0), CCMU_CPUX_AXI_CFG_REG);


    //set PLL_C0CPUX, the  default  clk is 408M  ,PLL_OUTPUT= 24M*N/P  
	reg_val = 0x82001100;
	writel(reg_val, CCMU_PLL_C0CPUX_CTRL_REG);
    //wait pll1 stable
#ifndef CONFIG_SUNXI_FPGA
	do
	{
		reg_val = readl(CCMU_PLL_STB_STATUS_REG);
	}
	while(!(reg_val & 0x1));
#endif

    //set PLL_C1CPUX, the  default  clk is 408M  ,PLL_OUTPUT= 24M*N/P  
    reg_val = 0x82001100;
    writel(reg_val, CCMU_PLL_C1CPUX_CTRL_REG);
    //wait pll1 stable
#ifndef CONFIG_SUNXI_FPGA
    do
    {
        reg_val = readl(CCMU_PLL_STB_STATUS_REG);
    }
    while(!(reg_val & 0x2));
#endif

    //enable pll_hsic, default is 480M
    writel(0x42800, CCMU_PLL_HSIC_CTRL_REG);  //set default value
    writel(readl(CCMU_PLL_HSIC_CTRL_REG) | (1U << 31), CCMU_PLL_HSIC_CTRL_REG);

    
    //cci400 clk src is pll_hsic: (2<<24) , div is 1:(0<<0)
    __asm{dmb};
	__asm{isb};
    reg_val = readl(CCMU_CCI400_CFG_REG);
    if(!(reg_val & (0x3<<24))) //src is osc24M
    {
        writel(0x0, CCMU_CCI400_CFG_REG);
        __usdelay(50);
    }
    writel((2<<24 | 0<<0), CCMU_CCI400_CFG_REG);
    __usdelay(100);
    __asm{dmb};
	__asm{isb};
    
    //change ahb src before set pll6
    writel((0x01 << 12) | (readl(CCMU_AHB1_APB1_CFG_REG)&(~(0x3<<12))), CCMU_AHB1_APB1_CFG_REG);
    //enable PLL6
    writel( (0x32<<8) | (1U << 31), CCMU_PLL_PERIPH_CTRL_REG);
    __usdelay(10);

    //set AHB1/APB1 clock  divide ratio
    //ahb1 clock src is PLL6,                           (0x02 << 12)
    //apb1 clk src is ahb1 clk src, divide  ratio is 2  (1<<8)
    //ahb1 pre divide  ratio is 2:    0:1  , 1:2,  2:3,   3:4 (2<<6) 
    //PLL6:AHB1:APB1 = 600M:200M:100M ,
    writel((0x02 << 12) | (1<<8) | (2<<6) | (1<<4), CCMU_AHB1_APB1_CFG_REG);
    

	//set and change cpu clk src to pll1,  PLL1:AXI0 = 408M:204M
	reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
	reg_val &= ~(3 << 0);
	reg_val |=  (1 << 0);       //axi0  clk src is C0_CPUX , divide  ratio is 2
	reg_val |=  (1 << 12);      //C0_CPUX clk src is PLL_C0CPUX
	writel(reg_val, CCMU_CPUX_AXI_CFG_REG);


    //set  C1_CPUX clk src is PLL_C1CPUX
    reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
    reg_val &= ~(3<<16);
    reg_val |= 1<<16;          //axi1  clk src is C1_CPUX , divide  ratio is 2
    reg_val |= 1<<28;          //C1_CPUX clk src is PLL_C1CPUX
    writel(reg_val, CCMU_CPUX_AXI_CFG_REG);
    __usdelay(1000);

	//----DMA function--------
	//dma reset
	writel(readl(CCMU_BUS_SOFT_RST_REG0)  | (1 << 6), CCMU_BUS_SOFT_RST_REG0);
	__usdelay(20);
	//gating clock for dma pass
	writel(readl(CCMU_BUS_CLK_GATING_REG0) | (1 << 6), CCMU_BUS_CLK_GATING_REG0);
    //auto gating disable
	writel(7, (DMAC_REGS_BASE+0x20));
	//reset mbus domain
	writel(0x80000000, CCMU_MBUS_RST_REG); 
    //open MBUS,clk src is pll6 , pll6/(m+1) = 300M
	writel(0x81000002, CCMU_MBUS_CLK_REG);  
    
    //open TIMESTAMP ,aw1673_cpux_cfg.pdf descript this register
    writel(1, 0x01720000);
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
    writel(0x00000000, CCMU_CPUX_AXI_CFG_REG);
    //还原PLL_C0CPUX为默认值
	writel(0x02001100, CCMU_PLL_C0CPUX_CTRL_REG);
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
	writel(readl(CCMU_AVS_CLK_REG) | (1U << 31), CCMU_AVS_CLK_REG);
	//avs countor0 & countor1 enable
	writel(3, TMRC_AVS_CTRL);
	//divisor 1 to 7ff,    24M /Divisor_no
	writel((0xc <<16) | 0x2EE0, TMRC_AVS_DIVISOR);
	//init value
	writel(0, TMRC_AVS_COUNT0);
    writel(0, TMRC_AVS_COUNT1);
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

void __usdelay(__u32 us)
{
	__u32 t1, t2;

	t1 = readl(TMRC_AVS_COUNT1);
	t2 = t1 + us;
	do
	{
		t1 = readl(TMRC_AVS_COUNT1);
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

	writel(readl(CCMU_AVS_CLK_REG) & (~(1U << 31)), CCMU_AVS_CLK_REG);
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

	*(volatile unsigned int *)(0x01f01400 + 0x0) = 0x00010000;//cpus_clk_reg :set to default 
	*(volatile unsigned int *)(0x01f01400 + 0xC) = 0x00000000;//apb0_cfg_reg :set to default 
	*(volatile unsigned int *)(0x01f01400 + 0x28) = 0x9;// apb0_clk_gating: r_rsb && r_pio gating open 
	*(volatile unsigned int *)(0x01f01400 + 0xB0) = 0x8;// apb0_soft_rst :r-uart

	/* read ic version */
	*(volatile unsigned int *)(0x01c00000 + 0x24) |=  (1 << 15);//version reg
	version = *(volatile unsigned int *)(0x01c00000 + 0x24);
	*(volatile unsigned int *)(0x01c00000 + 0x24) &= ~(1 << 15);

	version = (version >> 16) & 0xffff;

	if(version == 0x1673)
	{
    }
    


}


void cpu_init_s(void)
{
    timer_init();
	sram_area_init();
//	config_pll1_para();
	disbale_cpus();
	set_pll();
}

//-----------for rtc --------------------
#define msg(fmt,args...)				UART_printf2(fmt ,##args)
#define BOOT_FEL_FLAG                   (0x5AA5A55A)

unsigned int  get_fel_flag(void)
{
    unsigned int fel_flag;
    volatile unsigned int * rtc_addr = (volatile unsigned int *)(0x01f01400 + 0x1f0);
    
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
    volatile unsigned int * rtc_addr = (volatile unsigned int *)(0x01f01400 + 0x1f0);
    
    for(i=1;i<=3;i++)
    {
        *rtc_addr = (i<<16);
        rtc_flag = *rtc_addr;
        msg("rtc %d value %x\n", i, rtc_flag);
    }
}

void  clear_fel_flag(void)
{
    volatile unsigned int *rtc_addr = (volatile unsigned int *)(0x01f01400 + 0x1f0);
	do
    {
		*rtc_addr = (1<<16);
    	*rtc_addr = (1U<<31) | (1<<16);
    	__usdelay(10);
    	*rtc_addr = (1<<16);
	}
	while((*rtc_addr & 0xff) != 0);
}
