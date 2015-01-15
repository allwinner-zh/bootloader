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
#include <asm/arch/cpu.h>
#include <asm/arch/timer.h>
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
void standby_timer_delay(unsigned int msec)
{
	u32 t1, t2;
	struct sunxi_timer_reg *timer_reg = (struct sunxi_timer_reg *)SUNXI_TIMER_BASE;

	t1 = timer_reg->avs.cnt0;
	t2 = t1 + msec;
	do
	{
		t1 = timer_reg->avs.cnt0;
	}
	while(t2 >= t1);

	return ;
}







