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
#include <asm/arch/key.h>
#include <linux/types.h>
#include <asm/arch/cpu.h>
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
#if 0
__s32 boot_key_get_status(void)
{
    __u32 reg_val;

    struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_LRADC_BASE;
    reg_val = sunxi_key_base->ints;
    if(reg_val & (1 << 1))     //判断是否是第一次按下
    {
        if(reg_val & (1 << 0))  //是，则判断按下是否足够
        {
            sunxi_key_base->ints |= (reg_val & 0x1f);//按下时间足够长，则认为按键合法
            return 1;
        }
                               //否，按下时间不够长，不处理这个动作
    }
    else if(reg_val & (1 << 0))//表示不是第一次按下，直接清除掉pengding，不处理这个按键
    {
        sunxi_key_base->ints |= (1 << 0);

        return 0;              //代表重复键
    }
    //没有任何按键出现
    return -1;
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
__s32 boot_key_get_value(void)
{
    __u32 reg_val;
    __u32 key_val;

    struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_LRADC_BASE;
    reg_val = sunxi_key_base->ints;
    if(reg_val & (1 << 1))     //判断是否是第一次按下
    {
        if(reg_val & (1 << 0))  //是，则判断按下是否足够
        {
            sunxi_key_base->ints |= (reg_val & 0x1f);//按下时间足够长，则认为按键合法
            key_val = sunxi_key_base->data0 & 0x3f;
            return key_val;
        }
                               //否，按下时间不够长，不处理这个动作
    }
    else if(reg_val & (1 << 0))//表示不是第一次按下，直接清除掉pengding，不处理这个按键
    {
        sunxi_key_base->ints |= (1 << 0);
        key_val = sunxi_key_base->data0 & 0x3f;

        return key_val;              //代表重复键
    }
    //没有任何按键出现
    return -1;
}



#endif
