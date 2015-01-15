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
#include "standby_i.h"
#include <asm/arch/gic.h>
/*
************************************************************************************************************
*
*                                             eGon2_int_enter_standby
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：	进入standby
*				1) 清除DMA PENDING，保存DMA enable
*				2) 关闭DMA AHB
*
************************************************************************************************************
*/
int standby_int_disable(void)
{
	asm("mrs r0, cpsr");
	asm("orr r0, r0, #(0x40|0x80)");
	asm("msr cpsr_c, r0");

    return 0;
}
/*
************************************************************************************************************
*
*                                             eGon2_int_exit_standby
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：	退出standby
*				1) 打开DMA AHB
*				2) 清除DMA PENDING，恢复DMA enable
*
*
************************************************************************************************************
*/
int standby_int_enable(void)
{
	asm("mrs r0, cpsr");
	asm("bic r0, r0, #(0x40|0x80)");
	asm("msr cpsr_c, r0");

    return 0;
}
/*
*********************************************************************************************************
*										   EnableInt
*
* Description:  使能中断
*
* Arguments	 : irq_no     中断号
*
* Returns	 :
*
*********************************************************************************************************
*/
static int standby_enable_irq(__u32 irq_no)
{
	__u32 reg_val;
	__u32 offset;

	if (irq_no >= GIC_IRQ_NUM)
	{
		return -1;
	}

	offset   = irq_no >> 5; // 除32
	reg_val  = readl(GIC_SET_EN(offset));
	reg_val |= 1 << (irq_no & 0x1f);
	writel(reg_val, GIC_SET_EN(offset));

    return 0;
}
/*
*********************************************************************************************************
*										   DisableInt
*
* Description:  禁止中断
*
* Arguments	 : irq_no     中断号
*
* Returns	 :
*
*********************************************************************************************************
*/
static __s32 standby_disable_irq(__u32 irq_no)
{
	__u32 reg_val;
	__u32 offset;

	if (irq_no >= GIC_IRQ_NUM)
	{
		return -1;
	}

	offset   = irq_no >> 5; // 除32
	reg_val  = readl(GIC_SET_EN(offset));
	reg_val &= ~(1 << (irq_no & 0x1f));
	writel(reg_val, GIC_SET_EN(offset));

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
void standby_gic_store(void)
{
	standby_enable_irq(AW_IRQ_NMI);
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
void standby_gic_restore(void)
{
	standby_disable_irq(AW_IRQ_NMI);
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
void standby_gic_clear_pengding(void)
{
	__u32 reg_val;
	__u32 offset;
	__u32 idnum;

	idnum = readl(GIC_INT_ACK_REG);

	writel(idnum, GIC_END_INT_REG);
	writel(idnum, GIC_DEACT_INT_REG);

	offset = idnum >> 5; // 除32
	reg_val = readl(GIC_PEND_CLR(offset));
	reg_val |= (1 << (idnum & 0x1f));
	writel(reg_val, GIC_PEND_CLR(offset));

	return ;
}
