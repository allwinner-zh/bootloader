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
#include "axp_i.h"
#include "standby.h"
#include <common.h>
#include <linux/types.h>
#include <asm/arch/power.h>
#include <sys_config.h>

extern int standby_i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern int standby_i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);
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
__s32 boot_set_charge_current(int current)
{
    __u8  reg_addr, value;
    int   step;

    reg_addr = BOOT_POWER20_CHARGE1;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    value &= ~0x0f;
    if(current > 1800)
    {
        current = 1800;
    }
    else if(current < 300)
    {
        current = 300;
    }
    step   = (current-300)/100 ;//before is (current/100-3)
    value |= (step & 0x0f);
    /*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
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
__s32 boot_get_charge_current(void)
{
    __u8  reg_addr, value;
    int	  current;

    reg_addr = BOOT_POWER20_CHARGE1;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    value &= 0x0f;
    current = (value + 3) * 100;

    return current;
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
__s32 boot_config_charge_current(int start_time)
{
    __s32 ret;
    int   value;
    script_gpio_set_t  gpio;
    __u32  gpio_hd;

    //1 代表关机状态
    //0 代表开机状态
    if(start_time == 1)
    {
        ret = script_parser_fetch("pmu_para", "pmu_used2", &value, 1);
        if((ret >= 0) && (value == 1))
        {
            ret = script_parser_fetch("pmu_para", "pmu_adpdet", (void *)&gpio, sizeof(script_gpio_set_t)/sizeof(int));
            if(ret >= 0)
            {
                gpio_hd = gpio_request((void *)&gpio, 1);
                value = gpio_read_one_pin_value(gpio_hd, 0);
                if(value > 0)
                {
                    ret = script_parser_fetch("pmu_para", "pmu_shutdown_chgcur", &value, 1);
                }
                else
                {
                    ret = script_parser_fetch("pmu_para", "pmu_shutdown_chgcur2", &value, 1);
                }
                if(ret >= 0)
                {
                    boot_set_charge_current(value);

                    return 0;
                }
            }
        }
        else
        {
            ret = script_parser_fetch("pmu_para", "pmu_shutdown_chgcur", &value, 1);
            if(ret >= 0)
            {
                boot_set_charge_current(value);

                return 0;
            }
        }
    }
    else
    {
        boot_set_charge_current(300);
    }

    return -1;
}

