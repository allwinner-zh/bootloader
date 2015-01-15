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
#include <asm/arch/cpu.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/gic.h>
#include <asm/arch/intc.h>

struct _irq_handler
{
	void                *m_data;
	void (*m_func)( void * data);
};

struct _irq_handler sunxi_int_handlers[INT_IRQ_NUM];
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
*    note          :
*
*
************************************************************************************************************
*/
static void default_isr(void *data)
{
	printf("default_isr():  called from IRQ %d\n", (uint)data);
	while(1);
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
*    note          :
*
*
************************************************************************************************************
*/
int irq_enable(int irq_no)
{
	uint reg_val;

    if(irq_no < 32)
    {
    	writel((readl(INTC_REG_ENABLE0) | (  1<<irq_no)), INTC_REG_ENABLE0);
    	reg_val = readl(INTC_REG_MASK0);
    	reg_val &= ~(1<<irq_no);
    	writel(reg_val, INTC_REG_MASK0);

        if(irq_no == readl(AW_IRQ_NMI)) /* must clear pending bit when enabled */
        {
        	reg_val = readl(INTC_REG_FIQ_PENDCLR0);
    		reg_val &= ~(1<<AW_IRQ_NMI);
    		writel(reg_val, INTC_REG_FIQ_PENDCLR0);
    	}
    }
    else if(irq_no < 64)
    {
    	irq_no              -= 32;
    	writel((readl(INTC_REG_ENABLE1) | (  1<<irq_no)), INTC_REG_ENABLE1);
    	reg_val = readl(INTC_REG_MASK1);
    	reg_val &= ~(1<<irq_no);
    	writel(reg_val, INTC_REG_MASK1);
    }
    else
    {
        irq_no               -= 64;
        writel((readl(INTC_REG_ENABLE2) | (  1<<irq_no)), INTC_REG_ENABLE2);
    	reg_val = readl(INTC_REG_MASK2);
    	reg_val &= ~(1<<irq_no);
    	writel(reg_val, INTC_REG_MASK2);
    }

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
*    note          :
*
*
************************************************************************************************************
*/
int irq_disable(int irq_no)
{
	uint reg_val;

    if(irq_no < 32)
    {
    	reg_val = readl(INTC_REG_ENABLE0);
    	reg_val &= ~(1<<irq_no);
    	writel(reg_val, INTC_REG_ENABLE0);
    	writel((readl(INTC_REG_MASK0)   | (  1<<irq_no)), INTC_REG_MASK0);
    }
    else if(irq_no < 64)
    {
    	irq_no -= 32;
    	reg_val = readl(INTC_REG_ENABLE1);
    	reg_val &= ~(1<<irq_no);
    	writel(reg_val, INTC_REG_ENABLE1);
    	writel((readl(INTC_REG_MASK1)   | (  1<<irq_no)), INTC_REG_MASK1);
    }
    else
    {
        irq_no -= 64;
        reg_val = readl(INTC_REG_ENABLE2);
    	reg_val &= ~(1<<irq_no);
    	writel(reg_val, INTC_REG_ENABLE2);
    	writel((readl(INTC_REG_MASK2)   | (  1<<irq_no)), INTC_REG_MASK2);
    }

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
*    note          :
*
*
************************************************************************************************************
*/
void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data)
{
	disable_interrupts();
	if ((irq >= INT_IRQ_NUM) || (!handle_irq))
	{
		enable_interrupts();
		return;
	}

	sunxi_int_handlers[irq].m_data = data;
	sunxi_int_handlers[irq].m_func = handle_irq;

    enable_interrupts();
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
*    note          :
*
*
************************************************************************************************************
*/
void irq_free_handler(int irq)
{
	disable_interrupts();
	if (irq >= INT_IRQ_NUM)
	{
		enable_interrupts();
		return;
	}

	sunxi_int_handlers[irq].m_data = (void *)0;
	sunxi_int_handlers[irq].m_func = default_isr;

	enable_interrupts();
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
*    note          :
*
*
************************************************************************************************************
*/
#ifdef CONFIG_USE_IRQ
void do_irq (struct pt_regs *pt_regs)
{
	u32 base;

	disable_interrupts();
	base = readl(INTC_REG_VCTR)>>2;
	sunxi_int_handlers[base].m_func(sunxi_int_handlers[base].m_data);
	enable_interrupts();

	return;
}
#endif
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
*    note          :
*
*
************************************************************************************************************
*/
int arch_interrupt_init (void)
{
	//关闭所有中断使能
	writel(0, INTC_REG_ENABLE0);
	writel(0, INTC_REG_ENABLE1);
	writel(0, INTC_REG_ENABLE2);
	//打开所有中断mask
	writel(0, INTC_REG_MASK0);
	writel(0, INTC_REG_MASK1);
	writel(0, INTC_REG_MASK2);
	//清除所有中断pengding
	writel(0xffffffff, INTC_REG_FIQ_PENDCLR0);
	writel(0xffffffff, INTC_REG_FIQ_PENDCLR1);
	writel(0xffffffff, INTC_REG_FIQ_PENDCLR2);

	writel(0xffffffff, INTC_REG_IRQ_PENDCLR0);
	writel(0xffffffff, INTC_REG_IRQ_PENDCLR1);
	writel(0xffffffff, INTC_REG_IRQ_PENDCLR2);

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
*    note          :
*
*
************************************************************************************************************
*/
int arch_interrupt_exit(void)
{
    //关闭TIMER中断
    *(volatile unsigned int *)(0x01c20c00 + 0x00) = 0;
    *(volatile unsigned int *)(0x01c20c00 + 0x04) |= 0x043f;
    *(volatile unsigned int *)(0x01c20c00 + 0x10) = 0;
    *(volatile unsigned int *)(0x01c20c00 + 0x20) = 0;
    //关闭DMA中断
    *(volatile unsigned int *)(0x01c02000 + 0x00) = 0;
    *(volatile unsigned int *)(0x01c02000 + 0x04) = 0xffffffff;

	//关闭所有中断使能
	writel(0, INTC_REG_ENABLE0);
	writel(0, INTC_REG_ENABLE1);
	writel(0, INTC_REG_ENABLE2);
	//打开所有中断mask
	writel(0, INTC_REG_MASK0);
	writel(0, INTC_REG_MASK1);
	writel(0, INTC_REG_MASK2);
	//清除所有中断pengding
	writel(0xffffffff, INTC_REG_FIQ_PENDCLR0);
	writel(0xffffffff, INTC_REG_FIQ_PENDCLR1);
	writel(0xffffffff, INTC_REG_FIQ_PENDCLR2);

	writel(0xffffffff, INTC_REG_IRQ_PENDCLR0);
	writel(0xffffffff, INTC_REG_IRQ_PENDCLR1);
	writel(0xffffffff, INTC_REG_IRQ_PENDCLR2);

	return 0;
}

