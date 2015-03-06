/*
 * (C) Copyright 2012
 *     wangflord@allwinnertech.com
 *     wangyaliang@allwinnertech.com
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
 * 20130726
 */
#include "types.h"
#include "arch.h"
#include "arm_a8.h"
#define SW_PA_CPUCFG_IO_BASE              0x01c25c00

static __u32 cpu_freq = 0;
static __u32 overhead = 0;
#define CCU_REG_VA (0xf1c20000)
#define CCU_REG_PA (0x01c20000)

/*
 * CPUCFG
 */
#define AW_CPUCFG_P_REG0            0x01a4
#define CPUX_RESET_CTL(x) (0x40 + (x)*0x40)
#define CPUX_CONTROL(x)   (0x44 + (x)*0x40)
#define CPUX_STATUS(x)    (0x48 + (x)*0x40)
#define AW_CPUCFG_GENCTL            0x0184
#define AW_CPUCFG_DBGCTL0           0x01e0
#define AW_CPUCFG_DBGCTL1           0x01e4

#define AW_CPU1_PWR_CLAMP         0x01b0
#define AW_CPU1_PWROFF_REG        0x01b4
#define readl(addr)		(*((volatile unsigned long  *)(addr)))
#define writel(v, addr)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

#define IO_ADDRESS(IO_ADDR) (IO_ADDR)
#define IS_WFI_MODE(cpu)    (readl(IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + CPUX_STATUS(cpu)) & (1<<2))

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

	//切换到24M
	CCMU_REG_AHB_APB = 0x00010010;
	//设置PLL1到384M
	reg_val = (0x21005000) | (0x80000000);
	CCMU_REG_PLL1_CTRL = reg_val;
	//等待
	for(reg_val=0;reg_val<200;reg_val++);
	//切换到PLL1
	reg_val = CCMU_REG_AHB_APB;
	reg_val &= ~(3 << 16);
	reg_val |=  (2 << 16);
	CCMU_REG_AHB_APB = reg_val;
	//打开DMA
	CCMU_REG_AHB_MOD0 |= 1 << 6;
	//open PLL6,and setup to default 600M
	reg_val = CCMU_REG_PLL6_CTRL;
	reg_val |= 1 << 31;
	CCMU_REG_PLL6_CTRL = reg_val;

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
	*(volatile unsigned int *)(0x01c20000 + 0x144) |= (1U << 31);
	*(volatile unsigned int *)(0x01c20C00 + 0x80 )  = 1;
	*(volatile unsigned int *)(0x01c20C00 + 0x8C )  = 0x2EE0;
	*(volatile unsigned int *)(0x01c20C00 + 0x84 )  = 0;
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
	writel(0x2EE0, TMRC_AVS_DIVISOR);
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
static void  set_vldo_for_pll(void)
{
}


static void sram_area_init(void)
{
}


void cpu_init_s(void)
{
	sram_area_init();
	config_pll1_para();
	set_vldo_for_pll();
	disbale_cpus();
	set_pll();
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

	t1 = readl(TMRC_AVS_COUNT1);
	t2 = t1 + us;
	do
	{
		t1 = readl(TMRC_AVS_COUNT1);
	}
	while(t2 >= t1);

	return ;
}

__u32 get_cyclecount (void)
{
	__u32 value;
	// Read CCNT Register
	__asm{MRC p15, 0, value, c9, c13, 0}
	return value;
}

void init_perfcounters (__u32 do_reset, __u32 enable_divider)
{
	// in general enable all counters (including cycle counter)
	__u32 value = 1;

	// peform reset:
	if (do_reset)
	{
		value |= 2;     // reset all counters to zero.
		value |= 4;     // reset cycle counter to zero.
	}

	if (enable_divider)
		value |= 8;     // enable "by 64" divider for CCNT.

	value |= 16;

	// program the performance-counter control-register:
	__asm {MCR p15, 0, value, c9, c12, 0}

	// enable all counters:
	value = 0x8000000f;
	__asm {MCR p15, 0, value, c9, c12, 1}

	// clear overflows:
	__asm {MCR p15, 0, value, c9, c12, 3}

	return;
}

void change_runtime_env(__u32 mmu_flag)
{
	__u32 factor_n = 0;
	__u32 factor_k = 0;
	__u32 factor_m = 0;
	__u32 factor_p = 0;
	__u32 start = 0;
	__u32 cmu_reg = 0;
	volatile __u32 reg_val = 0;

	if(mmu_flag){
		cmu_reg = CCU_REG_VA;
	}else{
		cmu_reg = CCU_REG_PA;
	}
	//init counters:
	//init_perfcounters (1, 0);
	// measure the counting overhead:
	start = get_cyclecount();
	overhead = get_cyclecount() - start;
	//busy_waiting();
	//get runtime freq: clk src + divider ratio
	//src selection
	reg_val = *(volatile int *)(cmu_reg + 0x54);
	reg_val >>= 16;
	reg_val &= 0x3;
	if(0 == reg_val){
		//32khz osc
		cpu_freq = 32;

	}else if(1 == reg_val){
		//hosc, 24Mhz
		cpu_freq = 24000; 			//unit is khz
	}else if(2 == reg_val){
		//get pll_factor
		reg_val = *(volatile int *)(cmu_reg + 0x00);
		factor_p = 0x3 & (reg_val >> 16);
		factor_p = 1 << factor_p;		//1/2/4/8
		factor_n = 0x1f & (reg_val >> 8); 	//the range is 0-31
		factor_k = (0x3 & (reg_val >> 4)) + 1; 	//the range is 1-4
		factor_m = (0x3 & (reg_val >> 0)) + 1; 	//the range is 1-4

		cpu_freq = (24000*factor_n*factor_k)/(factor_p*factor_m);
		//cpu_freq = raw_lib_udiv(24000*factor_n*factor_k, factor_p*factor_m);
		//msg("cpu_freq = dec(%d). \n", cpu_freq);
		//busy_waiting();
	}

}

/*
 * input para range: 1-1000 us, so the max us_cnt equal = 1008*1000;
 */
void delay_us(__u32 us)
{
	__u32 us_cnt = 0;
	__u32 cur = 0;
	__u32 target = 0;

	//us_cnt = ((raw_lib_udiv(cpu_freq, 1000)) + 1)*us;
	us_cnt = ((cpu_freq/1000) + 1)*us;
	cur = get_cyclecount();
	target = cur - overhead + us_cnt;

	while(!(((__s32)cur - (__s32)target) >= 0))
	{
		cur = get_cyclecount();
	}



	return;
}


__asm void cpuX_startup_to_wfi(void)
{

    mrs r0, cpsr
    bic r0, r0, #ARMV7_MODE_MASK
    orr r0, r0, #ARMV7_SVC_MODE
    orr r0, r0, #( ARMV7_IRQ_MASK | ARMV7_FIQ_MASK )   // After reset, ARM automaticly disables IRQ and FIQ, and runs in SVC mode.
    bic r0, r0, #ARMV7_CC_E_BIT                         // set little-endian
    msr cpsr_c, r0

    // configure memory system : disable MMU,cache and write buffer; set little_endian;
    mrc p15, 0, r0, c1, c0
    bic r0, r0, #( ARMV7_C1_M_BIT | ARMV7_C1_C_BIT )// disable MMU, data cache
    bic r0, r0, #( ARMV7_C1_I_BIT | ARMV7_C1_Z_BIT )// disable instruction cache, disable flow prediction
    bic r0, r0, #( ARMV7_C1_A_BIT)                  // disable align
    mcr p15, 0, r0, c1, c0                          
    // set SP for SVC mode
    mrs r0, cpsr
    bic r0, r0, #ARMV7_MODE_MASK
    orr r0, r0, #ARMV7_SVC_MODE
    msr cpsr_c, r0
    ldr sp, =0xb400

    //let the cpu1+ enter wfi state;
    /* step3: execute a CLREX instruction */
    clrex
    /* step5: execute an ISB instruction */
    isb sy
    /* step6: execute a DSB instruction  */
    dsb sy
    /* step7: execute a WFI instruction */
    wfi
    /* step8:wait here */
    b .
}

#define SW_PA_CPUCFG_IO_BASE              0x01c25c00
/*
 * CPUCFG
 */
#define AW_CPUCFG_P_REG0            0x01a4
#define CPUX_RESET_CTL(x) (0x40 + (x)*0x40)
#define CPUX_CONTROL(x)   (0x44 + (x)*0x40)
#define CPUX_STATUS(x)    (0x48 + (x)*0x40)
#define AW_CPUCFG_GENCTL            0x0184
#define AW_CPUCFG_DBGCTL0           0x01e0
#define AW_CPUCFG_DBGCTL1           0x01e4

#define AW_CPU1_PWR_CLAMP         0x01b0
#define AW_CPU1_PWROFF_REG        0x01b4
#define readl(addr)		(*((volatile unsigned long  *)(addr)))
#define writel(v, addr)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

#define IO_ADDRESS(IO_ADDR) (IO_ADDR)
#define IS_WFI_MODE(cpu)    (readl(IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + CPUX_STATUS(cpu)) & (1<<2))

void open_cpuX(__u32 cpu)
{
    long paddr;
    __u32 pwr_reg;

    paddr = (__u32)cpuX_startup_to_wfi;
    writel(paddr, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_P_REG0);

    /* step1: Assert nCOREPORESET LOW and hold L1RSTDISABLE LOW.
              Ensure DBGPWRDUP is held LOW to prevent any external
              debug access to the processor.
    */
    /* assert cpu core reset */
    writel(0, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + CPUX_RESET_CTL(cpu));
    /* L1RSTDISABLE hold low */
    pwr_reg = readl(IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_GENCTL);
    pwr_reg &= ~(1<<cpu);
    writel(pwr_reg, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_GENCTL);
    /* DBGPWRDUP hold low */
    pwr_reg = readl(IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_DBGCTL1);
    pwr_reg &= ~(1<<cpu);
    writel(pwr_reg, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_DBGCTL1);


    /* step3: clear power-off gating */
    pwr_reg = readl(IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPU1_PWROFF_REG);
    pwr_reg &= ~(1);
    writel(pwr_reg, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPU1_PWROFF_REG);
    delay_us(1000);

    /* step4: de-assert core reset */
    writel(3, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + CPUX_RESET_CTL(cpu));

    /* step5: assert DBGPWRDUP signal */
    pwr_reg = readl(IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_DBGCTL1);
    pwr_reg |= (1<<cpu);
    writel(pwr_reg, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_DBGCTL1);

}

void close_cpuX(__u32 cpu)
{
    __u32 pwr_reg;

    while(!IS_WFI_MODE(cpu));
    /* step1: deassert cpu core reset */
    writel(0, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + CPUX_RESET_CTL(cpu));

    /* step2: deassert DBGPWRDUP signal */
    pwr_reg = readl(IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_DBGCTL1);
    pwr_reg &= ~(1<<cpu);
    writel(pwr_reg, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPUCFG_DBGCTL1);

    /* step3: set up power-off signal */
    pwr_reg = readl(IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPU1_PWROFF_REG);
    pwr_reg |= 1;
    writel(pwr_reg, IO_ADDRESS(SW_PA_CPUCFG_IO_BASE) + AW_CPU1_PWROFF_REG);
   
}

void reset_cpux(unsigned int cpu)
{
	init_perfcounters(1, 0);
	change_runtime_env(0);
    open_cpuX(cpu);
    close_cpuX(cpu);
}

unsigned int  get_fel_flag(void)
{
      unsigned int  fel_flag;
      fel_flag = *(volatile unsigned int *)(0x1C20C00 + 0x124);
      return fel_flag;
}
void show_rtc_reg(void)
{
}

void  clear_fel_flag(void)
{
    *(volatile unsigned int *)(0x1C20C00 + 0x124) = 0;
}
