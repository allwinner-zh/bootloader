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
#include <malloc.h>
#include <asm/arch/intc.h>
#include <pmu.h>
#include "power_probe.h"
#include <asm/arch/usb.h>

DECLARE_GLOBAL_DATA_PTR;
#ifndef CONFIG_NO_BOOT_STANDBY
extern int boot_standby_action;
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
static void power_int_irq(void *p_arg)
{
#ifdef DEBUG
	int i;
#endif
	unsigned char power_int_status[8];
	int  dc_exist, bat_exist;

	axp_int_query(power_int_status);
#ifdef DEBUG
	for(i=0;i<5;i++)
	{
		tick_printf("int status %d %x\n", i, power_int_status[i]);
	}
#endif
	if(power_int_status[0] & 0x48)   //外部电源插入
	{
		axp_power_get_dcin_battery_exist(&dc_exist, &bat_exist);
		if(dc_exist)
		{
			tick_printf("power insert\n");
			boot_standby_action &= ~0x10;
			boot_standby_action |= 0x04;
		}
	}
	if(power_int_status[0] & 0x8)   //usb 插入中断，启动usb检测
	{
		tick_printf("usb in\n");
		boot_standby_action |= 8;
		usb_detect_enter();
	}
	if(power_int_status[0] & 0x4)
	{
		tick_printf("usb out\n");
		boot_standby_action &= ~0x04;
		boot_standby_action |= 0x10;
		usb_detect_exit();
	}
	if(power_int_status[2] & 0x2)	//短按键
	{
		tick_printf("short key\n");
		boot_standby_action |= 2;

	}
	if(power_int_status[2] & 0x1)	//长按键
	{
		tick_printf("long key\n");
		boot_standby_action |= 1;
	}

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
void power_limit_detect_enter(void)
{
	unsigned char power_int_enable[8];

	power_int_enable[0] = 0x4C;  //dc in/out, usb in/out
	power_int_enable[1] = 0;
	power_int_enable[2] = 3;
	power_int_enable[4] = 0;
	power_int_enable[5] = 0;

	tick_printf("power limit detect enter\n");

	axp_int_enable(power_int_enable);
	irq_install_handler(AW_IRQ_NMI, power_int_irq, 0);
	irq_enable(AW_IRQ_NMI);
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
void power_limit_detect_exit(void)
{
	usb_detect_exit();
	irq_disable(AW_IRQ_NMI);
	axp_int_disable();

	tick_printf("power limit detect exit\n");
}
#endif
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
int __usb_probe_vbus_type(void)		//如果没有声明，默认为pc类型电源
{
	return 0;
}

int usb_probe_vbus_type(void)
	__attribute__((weak, alias("__usb_probe_vbus_type")));



void power_limit_init(void)
{
	int battery_exist = 0;
	int dcin_exist = 0;
	int vbus_type = 0;
	int i = 0;

	do
	{
		#ifdef CONFIG_ARCH_HOMELET
			break;
		#endif
		axp_power_get_dcin_battery_exist(&dcin_exist, &battery_exist);//判断电池是否存在
		if(battery_exist >= 0)
		{
			break;
		}
		i ++;
		__msdelay(500);
	}
	while(i < 4);

	vbus_type = usb_probe_vbus_type();
	if(vbus_type == 1)								//属于dc类型电源
	{
		printf("vbus not exist\n");
		gd->vbus_status = SUNXI_VBUS_NOT_EXIST;
	}
	else
	{
		printf("vbus exist\n");
		gd->vbus_status = SUNXI_VBUS_EXIST;			//属于vbus类型
	}

	if(battery_exist != BATTERY_EXIST)			//电池不存在，则直接限流到DC模式
	{
		axp_set_vbus_limit_dc();
		puts("no battery, limit to dc\n");

		return ;
	}

	if(dcin_exist == AXP_DCIN_EXIST)		//如果普通外部电源存在，不是VBUS类型
	{
		axp_set_vbus_limit_dc();
		puts("normal dc exist, limit to dc\n");

		return ;
	}

	if(dcin_exist == AXP_VBUS_EXIST)		//如果VBUS电源存在
	{
		vbus_type = usb_probe_vbus_type();

		if(vbus_type == 1)					//属于dc类型电源
		{
			axp_set_vbus_limit_dc();    //dp_dm 拉高
			puts("vbus dc exist, limit to dc\n");
		}
		else
		{
			axp_set_vbus_limit_pc();    //dp_dm 拉低
			axp_set_charge_current(600);
			puts("vbus pc exist, limit to pc\n");
		}

		return ;
	}

	axp_set_vbus_limit_dc();				//只有电池存在
	puts("only battery exist, limit to dc\n");

	return ;
}
