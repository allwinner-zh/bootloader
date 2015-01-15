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
#include <asm/arch/drv_display.h>
#include <bat.h>
#include <sys_config.h>
#include <asm/arch/timer.h>
#include <pmu.h>
#include <power.h>
#include "bat_cartoon.h"
#include "power_probe.h"
#include "de.h"
#include <standby.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_NO_BOOT_STANDBY
int boot_standby_action = 0;

typedef int (* standby_func)(void);

static int board_try_boot_standby(void)
{
	uint func_addr = (uint)boot_standby_mode;
	standby_func   boot_standby_func;

	//cal the real function address of boot_standby_mode
	flush_dcache_all();
	boot_standby_func = (standby_func)(func_addr - gd->reloc_off);

	return boot_standby_func();
}
#endif
extern int efex_test;
static int board_probe_power_level(void)
{
	int power_status;
	int power_start;

	//清除power按键
	axp_probe_key();
	//获取电源状态
	power_status = axp_get_power_vol_level();
	debug("power status = %d\n", power_status);
	if(power_status == BATTERY_RATIO_TOO_LOW_WITHOUT_DCIN)
	{
		tick_printf("battery power is low without no dc or ac, should be set off\n");
		sunxi_bmp_display("bat\\low_pwr.bmp");
		__msdelay(3000);
        if(!efex_test)
		    return -1;
        else
            return 0;
	}
	power_start = 0;
	//power_start的含义
	//0: 不允许插火牛直接开机，必须通过判断：满足以下条件可以直接开机：长按power按键，前次是系统状态，如果电池电量过低，则不允许开机
	//1: 任意状态下，允许插火牛直接开机，同时要求电池电量足够高
	//2: 不允许插火牛直接开机，必须通过判断：满足以下条件可以直接开机：长按power按键，前次是系统状态，不要求电池电量
	//3: 任意状态下，允许插火牛直接开机，不要求电池电量
	script_parser_fetch(PMU_SCRIPT_NAME, "power_start", &power_start, 1);
	debug("power start cause = %d\n", power_start);
	if(power_start == 3)
	{
		return 0;
	}
	if(power_status == BATTERY_RATIO_TOO_LOW_WITH_DCIN_VOL_TOO_LOW)//低电量低电压，带外部电源状态
	{
		if(!(power_start & 0x02))	//根据配置，低电状态下不允许开机, power_start==0 | power_start==1
		{
			tick_printf("battery low power and vol with dc or ac, should charge longer\n");
			sunxi_bmp_display("bat\\bempty.bmp");
			__msdelay(3000);

			return -1;
		}
		//低电池低电量，此时配置为2，进入检测，按键则进入系统，插入火牛则待机
		return 1;
	}
	if(power_status == BATTERY_RATIO_TOO_LOW_WITH_DCIN)//低电量高电压，带外部电源状态
	{
		//如果配置为0，进入检测，按键则显示低电图标然后关机，插入火牛则待机
		//如果配置为1，进入检测，按键则显示低电图标然后关机，插入火牛则待机
		//如果配置为2，进入检测，按键则进入系统，插入火牛则待机
		if(!(power_start & 0x02))	//根据配置，低电状态下不允许开机, power_start==0 | power_start==1
		{
			tick_printf("battery low power with dc or ac\n");

			return 2;
		}
		//低电池低电量，此时配置为2，进入检测，按键则进入系统，插入火牛则待机
		return 1;
	}
	//电池电压电流都足够
	if(power_start == 0x01)		//如果第0bit的值为1，则进入系统
	{
		return 0;
	}

	return 1;
}

static int 	board_probe_battery_exist(void)		//获取电池状态
{
	int counter;
	int dc_exist, bat_exist;

	counter = 4;
	do
	{
		dc_exist = 0;
		bat_exist = 0;
		axp_power_get_dcin_battery_exist(&dc_exist, &bat_exist);
		printf("bat_exist=%d\n", bat_exist);
		if(bat_exist == -1)
		{
			printf("bat is unknown\n");
			__msdelay(500);
		}
		else
		{
			break;
		}
	}
	while(counter --);

	return bat_exist;
}


static int board_probe_poweron_cause(void)
{
	int status = -1;

	status = axp_probe_startup_cause();
	debug("startup status = %d\n", status);
#ifdef FORCE_BOOT_STANDBY
	status = 1;
#endif
	return status;
}

static int board_probe_bat_status(int standby_mode)
{
	int   bat_cal = 1;
	int   ret, chargemode = 0;
	//当前可以确定是火牛开机，但是是否开机还不确定，需要确认电池是否存在
	//当电池不存在即开机，电池存在则关机
	//新添加，根据环境变量，是启动当前的待机，或者android待机功能
	ret = script_parser_fetch("charging_type", "charging_type", &chargemode, 1);
	if((!ret) && chargemode)
	{
		gd->chargemode = 1;

		return 0;
	}
	if(battery_charge_cartoon_init(0) < 0)
	{
		tick_printf("init charge cartoon fail\n");

		return -1;
	}
	bat_cal = axp_probe_rest_battery_capacity();
	printf("bat not inited\n");
	if(battery_charge_cartoon_init(bat_cal/(100/(SUNXI_BAT_BMP_MAX-1))) < 0)
	{
		tick_printf("init charge cartoon fail\n");

		return -1;
	}
	if((!bat_cal) && (standby_mode))
	{
		bat_cal = 100;
	}

	return bat_cal;
}

#ifndef CONFIG_NO_BOOT_STANDBY
static int board_standby_status(int source_bat_cal)
{
	int   bat_cal, this_bat_cal;
	int   i, j, status;
	int   one_delay;
	int   ret;

	boot_standby_action = 0;
	this_bat_cal = source_bat_cal;
	tick_printf("base bat_cal = %d\n", this_bat_cal);
	if(this_bat_cal > 95)
	{
		this_bat_cal = 100;
	}
	//启动中断检测
	usb_detect_for_charge(BOOT_USB_DETECT_DELAY_TIME + 200);
	//启动axp检测
	power_limit_detect_enter();
	status = 1;
	goto __start_case_status__;
/******************************************************************
*
*	standby 返回值说明
*
*	   -1: 进入standby失败
*		1: 普通按键唤醒
*		2: 电源按键短按唤醒
*		3: 电源按键长按唤醒
*		4: 外部电源移除唤醒
*		5: 电池充电完成
*		6: 在唤醒状态下外部电源被移除
*		7: 在唤醒状态下充电完成
*
******************************************************************/
	do
	{
		tick_printf("enter standby\n");
		board_display_layer_close();
		power_limit_detect_exit();
		status = board_try_boot_standby();
		tick_printf("exit standby by %d\n", status);

		bat_cal = axp_probe_rest_battery_capacity();
		tick_printf("current bat_cal = %d\n", bat_cal);
		if(bat_cal > this_bat_cal)
		{
			this_bat_cal = bat_cal;
		}
__start_case_status__:
		tick_printf("status = %d\n", status);
		switch(status)
		{
			case 2:		//短按power按键导致唤醒
				//启动中断检测
				boot_standby_action = 0;
				power_limit_detect_enter();
				board_display_layer_open();
			case 1:
				//重新计算动画延时时间
				if(this_bat_cal == 100)
				{
					one_delay = 1000;
				}
				else
				{
					one_delay = 1000/(10 - (this_bat_cal/10));
				}
				//绘制动画
				for(j=0;j<3;j++)
				{
					for(i=this_bat_cal/(100/(SUNXI_BAT_BMP_MAX-1));i<SUNXI_BAT_BMP_MAX;i++)
					{
						battery_charge_cartoon_rate(i);
						if(boot_standby_action & 0x08)		//存在外部电源
						{
							boot_standby_action &= ~0x08;
							j = 0;
						}
						else if(boot_standby_action & 0x02)	//短按
						{
							boot_standby_action &= ~2;
							j = 0;
						}
						else if(boot_standby_action & 0x01) //长按
						{
							battery_charge_cartoon_exit();
							power_limit_detect_exit();

							return 0;
						}
						else if(boot_standby_action & 0x10) //拔掉外部电源，没有外部电源
						{
							status = 10;
							boot_standby_action &= ~0x10;

							goto __start_case_status__;
						}
						__msdelay(one_delay);
					}
				}
				//停止动画，固定显示当前电量
				battery_charge_cartoon_rate(this_bat_cal/(100/(SUNXI_BAT_BMP_MAX-1)));
				for(j=0;j<4;j++)
				{
					if(boot_standby_action & 0x08)		//存在外部电源
					{
						boot_standby_action &= ~0x08;
						j = 0;
					}
					else if(boot_standby_action & 0x10) //拔掉外部电源，没有外部电源
					{
						status = 10;
						boot_standby_action &= ~0x10;

						goto __start_case_status__;
					}
					else if(boot_standby_action & 0x01) //长按
					{
						battery_charge_cartoon_exit();
						power_limit_detect_exit();

						return 0;
					}
					__msdelay(250);
				}
				break;

			case 3:		//长按电源按键之后，关闭电池图标，进入系统
				battery_charge_cartoon_exit();

				return 0;

			case 4:		//当移除外部电源时候，重新显示当前电池图标后，3秒后关机
			case 5:		//当电池充电完成的时候，需要关机
				//启动中断检测
				boot_standby_action = 0;
				power_limit_detect_enter();

				board_display_layer_open();
				battery_charge_cartoon_rate(this_bat_cal/(100/(SUNXI_BAT_BMP_MAX-1)));
			case 6:
			case 7:
				if((status != 4) && (status != 5))
				{
					board_display_layer_open();
					battery_charge_cartoon_rate(this_bat_cal/(100/(SUNXI_BAT_BMP_MAX-1)));
				}
			case 10:
				battery_charge_cartoon_rate(this_bat_cal/(100/(SUNXI_BAT_BMP_MAX-1)));
				__msdelay(500);
				do
				{
					if(!(boot_standby_action & 0x04))
					{
						ret = battery_charge_cartoon_degrade(5);
					}
					else
					{
						status = 1;
						battery_charge_cartoon_reset();

						goto __start_case_status__;
					}
				}
				while(!ret);

				battery_charge_cartoon_exit();

				power_limit_detect_exit();

				return -1;

			case 8:		//standby过程中检测到vbus存在变化
			{
				usb_detect_for_charge(BOOT_USB_DETECT_DELAY_TIME + 200);
			}
			break;

			case 9:		//standby过程中检测到vbus移除，同时存在普通dc
			{
//					power_set_usbpc();
			}
			break;

			default:
				break;
		}
	}
	while(1);
}
#endif
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :  standby_mode: 0, 普通模式，需要检测电源状态
*
*					                1, 测试模式，强制进入standby模式，不论电源状态
*
*    return        :
*
*    note          :  probe power and other condition
*
*
*
************************************************************************************************************
*/
void board_status_probe(int standby_mode)
{
	int ret;
	int start_condition = 0;
	int bat_exist;

	//清除power按键
	axp_probe_key();
	//启动条件判断，第一阶段，检测电源电压状态
	if(!standby_mode)
	{
		ret = board_probe_power_level();	//负数：关机；0：进入系统；正数：检测
		debug("stage1 resule %d\n", ret);
		if(ret < 0)
		{
			do_shutdown(NULL, 0, 1, NULL);
		}
		else if(!ret)
		{
			return ;
		}
		else if(ret == 2)           //按键则显示低电图标然后关机，插入火牛则待机
		{
			start_condition = 1;
		}
		//启动条件判断，第二阶段，检测开机原因
		ret = board_probe_poweron_cause();		//负数，0：进入系统；正数：待机或者直接关机
		debug("stage2 resule %d\n", ret);
		if(ret <= 0)
		{
			if(!start_condition)
			{
				return ;
			}
			else
			{
				tick_printf("battery low power with dc or ac, should charge longer\n");
				sunxi_bmp_display("bat\\bempty.bmp");
				__msdelay(3000);

				do_shutdown(NULL, 0, 1, NULL);
			}
		}
		else if(ret == AXP_VBUS_DCIN_NOT_EXIST) //当前一次为boot standby状态，但是启动时检查无外部电源，直接关机
		{
			do_shutdown(NULL, 0, 1, NULL);
		}
	}
#ifdef FORCE_BOOT_STANDBY
	bat_exist = 1;
#else
	if(standby_mode)
	{
		bat_exist = 1;
	}
	else
	{
		bat_exist = board_probe_battery_exist();
		if(bat_exist <= 0)
		{
			tick_printf("no battery exist\n");

			return;
		}
	}
#endif
	//启动条件判断，第三阶段，检测电池存在
	//负数：关机；0：进入系统；正数：待机
	ret = board_probe_bat_status(standby_mode);
	debug("stage3 resule %d\n", ret);
	if(ret < 0)
	{
		do_shutdown(NULL, 0, 1, NULL);
	}
	else if(!ret)
	{
		return ;
	}
#ifndef CONFIG_NO_BOOT_STANDBY
        //启动条件判断，第四阶段，进入boot待机
	//负数：关机，其它：进入系统
	ret = board_standby_status(ret);
	debug("stage4 resule %d\n", ret);
	if(ret < 0)
	{
		do_shutdown(NULL, 0, 1, NULL);
	}
#endif
	return ;
}


