/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <common.h>
#include <asm/io.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/cpu.h>
#include <asm/arch/timer.h>
#include <asm/armv7.h>

static __inline void __delay_null(unsigned long time);
static __inline void __msdelay_nssecure(unsigned long ms);
static __inline void __usdelay_nssecure(unsigned long us);
static __inline void __msdelay_secure(unsigned long ms);
static __inline void __usdelay_secure(unsigned long us);

void (* __msdelay_pt )(unsigned long ms) = __delay_null;
void (* __usdelay_pt )(unsigned long us) = __delay_null;

int timer_init(void)
{
	u32 reg_value;

	writel(readl(CCM_AVS_SCLK_CTRL) | (1U << 31), CCM_AVS_SCLK_CTRL);

	reg_value = readl(CCM_AVS_SCLK_CTRL);
	if(reg_value & 0x80000000)
	{
		struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;

		timer_reg->tirqen  = 0;
		timer_reg->tirqsta |= 0x03f;
		/* start avs as counter */
		//ccm_reg->avs_clk_cfg |= (1 << 31);
		timer_reg->avs.ctl  = 3; //enable avs cnt0 and cnt1,source is 24M
		/* div cnt0 12000 to 2000hz, high 32 bit means 1000hz.*/
		/* div cnt 1 12 to 2000000hz ,high 32 bit means 1000000hz */
		timer_reg->avs.div   = 0xc2ee0;
		timer_reg->avs.cnt0  = 0;
		timer_reg->avs.cnt1  = 0;
		//writel(0, TMRC_AVS_COUNT0);
		//writel(0, TMRC_AVS_COUNT1);
		__usdelay_pt = __usdelay_secure;
		__msdelay_pt = __msdelay_secure;
	}
	else
	{
		u32 reg_val;

		reg_val =   (0 << 0)  |            // 不启动TIMER
					(1 << 1)  |            // 使用单次模式
					(1 << 2)  |            // 使用高频晶振24M
					(7 << 4)  |            // 除频系统 128
					(1 << 7);			   // 使用单次模式

		writel(reg_val, TMRC_CTRL(4));
		writel(0xffffffff, TMRC_INTV(4));
		reg_val = 0;
		do
		{
			reg_val = readl(TMRC_CTRL(4));
		}
		while(reg_val & 0x02);
		writel(readl(TMRC_CTRL(4)) | 1, TMRC_CTRL(4));

		__usdelay_pt = __usdelay_nssecure;
		__msdelay_pt = __msdelay_nssecure;
	}

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
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static __inline void __msdelay_nssecure(unsigned long ms)
{
	u32 reg_val;

	reg_val =   (0 << 0)  |            // 不启动TIMER
				(1 << 1)  |            // 使用单次模式
				(1 << 2)  |            // 使用高频晶振24M
				(3 << 4)  |            // 除频系统 8
				(1 << 7);			   // 使用单次模式

	writel(reg_val, TMRC_CTRL(5));
	writel(ms * 3000, TMRC_INTV(5));
	reg_val = 0;
	do
	{
		reg_val = readl(TMRC_CTRL(5));
	}
	while(reg_val & 0x02);
	writel(readl(TMRC_CTRL(5)) | 1, TMRC_CTRL(5));

	while(!(readl(TMRC_INT_ST) & (1<<5)));
	writel(1<<5, TMRC_INT_ST);

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
static __inline void __usdelay_nssecure(unsigned long us)
{
	u32 reg_val;

	reg_val =   (0 << 0)  |            // 不启动TIMER
				(1 << 1)  |            // 使用单次模式
				(1 << 2)  |            // 使用高频晶振24M
				(3 << 4)  |            // 除频系统 8
				(1 << 7);			   // 使用单次模式

	writel(reg_val, TMRC_CTRL(5));
	writel(us * 3, TMRC_INTV(5));
	reg_val = 0;
	do
	{
		reg_val = readl(TMRC_CTRL(5));
	}
	while(reg_val & 0x02);
	writel(readl(TMRC_CTRL(5)) | 1, TMRC_CTRL(5));

	while(!(readl(TMRC_INT_ST) & (1<<5)));
	writel(1<<5, TMRC_INT_ST);

	return;
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
static __inline void __msdelay_secure(unsigned long ms)
{
	u32 t1, t2;
	struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;

	t1 = timer_reg->avs.cnt0;
	t2 = t1 + ms;
	do
	{
		t1 = timer_reg->avs.cnt0;
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
static __inline void __usdelay_secure(unsigned long us)
{
	u32 t1, t2;
	struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;

	t1 = timer_reg->avs.cnt1;
	t2 = t1 + us;
	do
	{
		t1 = timer_reg->avs.cnt1;
	}
	while(t2 >= t1);

	return ;
}

static __inline void __delay_null(unsigned long time)
{
	;
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
void __msdelay(unsigned long ms)
{
	__msdelay_pt(ms);

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
void __usdelay(unsigned long us)
{
	__usdelay_pt(us);

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
	writel(0, TMRC_CTRL(5));
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
void watchdog_enable(void)
{
	struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;
	struct sunxi_wdog *wdog = &timer_reg->wdog[0];
	/* enable watchdog */
	debug("write to %x value 1\n", (uint)&(wdog->mode));
	wdog->cfg = 1;
	wdog->mode = 1;
	for(;;);

	return ;

}
