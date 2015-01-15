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
#include <common.h>
#include <linux/types.h>
#include <sys_config.h>
#define FUELGUAGE_LOW_VOL	3400	//<3.4v,2%
#define FUELGUAGE_VOL1		3500    //<3.5v,3%
#define FUELGUAGE_VOL2		3600
#define FUELGUAGE_VOL3		3700
#define FUELGUAGE_VOL4		3800
#define FUELGUAGE_VOL5		3900
#define FUELGUAGE_VOL6		4000
#define FUELGUAGE_VOL7		4100
#define FUELGUAGE_TOP_VOL	4160	//>4.16v,100%

#define FUELGUAGE_LOW_LEVEL	2		//<3.4v,2%
#define FUELGUAGE_LEVEL1	3		//<3.5v,3%
#define FUELGUAGE_LEVEL2	5
#define FUELGUAGE_LEVEL3	16
#define FUELGUAGE_LEVEL4	46
#define FUELGUAGE_LEVEL5	66
#define FUELGUAGE_LEVEL6	83
#define FUELGUAGE_LEVEL7	93
#define FUELGUAGE_TOP_LEVEL	100     //>4.16v,100%
/*
*********************************************************************************************************
*                                   SET DCDC2 VALUE FOR BOOT
*
* Description: set dcdc2 value to default for boot.
*
* Arguments  : none
*
* Returns    : result
*
* Note       :
*********************************************************************************************************
*/
__u32  trans_dcdc2_user_set;
__u32  PMU_type = 0;

static __s32  dcdc2_user_set;
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
__s32 boot_power_set_dcdc2(int set_vol)
{
    __u32 vol, tmp, i;
    __u8  reg_addr, value;

	if(set_vol == -1)
	{
		set_vol = dcdc2_user_set;
	}
    //PMU is AXP209
    reg_addr = BOOT_POWER20_DC2OUT_VOL;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    tmp     = value & 0x3f;
    vol     = tmp * 25 + 700;
    //如果电压过高，则调低
    while(vol > set_vol)
    {
        tmp -= 1;
        value &= ~0x3f;
        value |= tmp;
        reg_addr = BOOT_POWER20_DC2OUT_VOL;
        /*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &value))*/
        if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &value, 1))
        {
            return -1;
        }
        for(i=0;i<2000;i++);
        reg_addr = BOOT_POWER20_DC2OUT_VOL;
        /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
        if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
        {
            return -1;
        }
        tmp     = value & 0x3f;
        vol     = tmp * 25 + 700;
    }
    //如果电压过低，则调高，根据先调低再调高的过程，保证电压会大于等于用户设定电压
    while(vol < set_vol)
    {
        tmp += 1;
        value &= ~0x3f;
        value |= tmp;
        reg_addr = BOOT_POWER20_DC2OUT_VOL;
        /*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &value))*/
        if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &value, 1))
        {
            return -1;
        }
        for(i=0;i<2000;i++);
        reg_addr = BOOT_POWER20_DC2OUT_VOL;
        /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
        if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
        {
            return -1;
        }
        tmp     = value & 0x3f;
        vol     = tmp * 25 + 700;
    }
  //  __debug("after set dcdc2,the value =%dmv\n",vol);
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
#if 0
__s32 boot_power_set_dcdc3(int set_vol)
{
	__s32 vol_value;
	__s32 ret;
	__u8  reg_addr;
	__u8  reg_value;

	if(set_vol == -1)
	{
		if(!dcdc3_user_set)
		{
			ret = script_parser_fetch("target", "dcdc3_vol", &vol_value, 1);
			if(ret)
			{
				printf("boot power:unable to find dcdc3 set\n");

				return -1;
			}
			dcdc3_user_set = vol_value;
		}
		vol_value = dcdc3_user_set;
	}
	else
	{
		vol_value = set_vol;
	}

	if(!vol_value)
	{
		reg_addr = BOOT_POWER20_OUTPUT_CTL;
		/*if(!BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &reg_value))*/
		if(!standby_i2c_read(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
		{
			reg_value &= ~(1<<1);
			/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &reg_value))*/
			if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
			{
				printf("boot power:unable to close dcdc3\n");

				return -1;
			}
		}
	}
	else
	{
		reg_addr = BOOT_POWER20_DC3OUT_VOL;
		/*if(!BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &reg_value))*/
		if(!standby_i2c_read(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
		{
			if((vol_value >= 700) && (vol_value <= 3500))
			{
				reg_value &= 0x80;
				reg_value = ((vol_value - 700)/25);
				/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &reg_value))*/
				if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
				{
					printf("boot power:unable to set dcdc3\n");

					return -1;
				}
			}
		}
	}

	return 0;
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
#if 0
__s32 boot_power_set_ldo2(int set_vol)
{
	__s32 vol_value;
	__s32 ret;
	__u8  reg_addr;
	__u8  reg_value;

	if(set_vol == -1)
	{
		if(!ldo2_user_set)
		{
			ret = script_parser_fetch("target", "ldo2_vol", &vol_value, 1);
			if(ret)
			{
				printf("boot power:unable to find ldo2 set\n");

				return -1;
			}
			ldo2_user_set = vol_value;
		}
		vol_value = ldo2_user_set;
	}
	else
	{
		vol_value = set_vol;
	}
	if(!vol_value)
	{
		reg_addr = BOOT_POWER20_OUTPUT_CTL;
		/*if(!BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &reg_value))*/
		if(!standby_i2c_read(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
		{
			reg_value &= ~(1<<2);
			/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &reg_value))*/
			if(standby_i2c_write(AXP20_ADDR, reg_addr, 1,&reg_value, 1))
			{
				printf("boot power:unable to set ldo2\n");

				return -1;
			}
		}
	}
	else
	{
		reg_addr  = BOOT_POWER20_LDO24OUT_VOL;
		/*if(!BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &reg_value))*/
		if(!standby_i2c_read(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
		{
			if((vol_value >= 1800) && (vol_value <= 3300))
			{
				reg_value &= 0x0f;
				reg_value |= (((vol_value - 1800)/100) << 4);
				/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &reg_value))*/
				if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
				{
					printf("boot power:unable to set ldo2\n");

					return -1;
				}
			}
	    }
	}

	return 0;
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
#if 0
__s32 boot_power_set_ldo3(int set_vol)
{
	__s32 vol_value;
	__s32 ret;
	__u8  reg_addr;
	__u8  reg_value;

	if(set_vol == -1)
	{
		if(!ldo3_user_set)
		{
			ret = script_parser_fetch("target", "ldo3_vol", &vol_value, 1);
			if(ret)
			{
				printf("boot power:unable to find ldo3 set\n");

				return -1;
			}
			ldo3_user_set = vol_value;
		}
		vol_value = ldo3_user_set;
	}
	else
	{
		vol_value = set_vol;
	}
	if(!vol_value)
	{
		reg_addr = BOOT_POWER20_OUTPUT_CTL;
		/*if(!BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &reg_value))*/
		if(!standby_i2c_read(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
		{
			reg_value &= ~(1<<6);
			/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &reg_value))*/
			if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
			{
				printf("boot power:unable to set ldo2\n");

				return -1;
			}
		}
	}
	else
	{
		reg_addr  = BOOT_POWER20_LDO3OUT_VOL;
		/*if(!BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &reg_value))*/
		if(!standby_i2c_read(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
		{
			if((vol_value >= 700) && (vol_value <= 3500))
			{
				reg_value &= 0x80;
				reg_value |= ((vol_value - 700)/25);
				/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &reg_value))*/
				if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
				{
					printf("boot power:unable to set ldo3\n");

					return -1;
				}
			}
	    }
	}

	return 0;
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
#if 0
__s32 boot_power_set_ldo4(int set_vol)
{
	__s32 vol_value;
	__s32 ret;
	__u8  reg_addr;
	__u8  reg_value;

	if(-1 == set_vol)
	{
		if(!ldo4_user_set)
		{
			ret = script_parser_fetch("target", "ldo4_vol", &vol_value, 1);
			if(ret)
			{
				printf("boot power:unable to find ldo4 set\n");

				return -1;
			}
			ldo4_user_set = vol_value;
		}
		vol_value = ldo4_user_set;
	}
	else
	{
		vol_value = set_vol;
	}
	if(!vol_value)
	{
		reg_addr = BOOT_POWER20_OUTPUT_CTL;
		/*if(!BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &reg_value))*/
		if(!standby_i2c_read(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
		{
			reg_value &= ~(1<<3);
			/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &reg_value))*/
			if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
			{
				printf("boot power:unable to set ldo2\n");

				return -1;
			}
		}
	}
	else
	{
		reg_addr  = BOOT_POWER20_LDO24OUT_VOL;
		/*if(!BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &reg_value))*/
		if(!standby_i2c_read(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
	    {
	        reg_value &= 0xf0;
	/*************************************************************************************
	0    1300     2000    2500     2700   2800    3000   3300  max


	*************************************************************************************/
			if(vol_value < 1300)
	        {
	            reg_value |= 0x00;
	        }
	        else if(vol_value <= 2000)
	        {
	        	reg_value |= (vol_value - 1200)/100;
	        }
	        else if(vol_value < 2700)
	        {
	        	reg_value |= 0x09;
	        }
	        else if(vol_value <= 2800)
	        {
	        	reg_value |= ((vol_value - 2700)/100) + 0x0a;
	        }
	        else
	        {
	        	if(vol_value < 3000)
	        	{
	        		vol_value = 3000;
	        	}
	        	else if(vol_value > 3300)
	        	{
	        		vol_value = 3300;
	        	}
	        	reg_value |= ((vol_value - 3000)/100) + 0x0c;
	        }
			/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &reg_value))*/
	    	if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &reg_value, 1))
	    	{
				printf("boot power:unable to set ldo4\n");

				return -1;
			}
	    }
	}

	return 0;
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
__s32 boot_power_get_key(void)
{
    __u8  reg_addr, value;
    __s32 key_exist;

    key_exist = 0;
    reg_addr = BOOT_POWER20_INTSTS3;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    key_exist = (value >> 0) & 0x03;
    if(key_exist)
    {
        value |= 0x01 << 1;
        /*BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &value);*/
        standby_i2c_write(AXP20_ADDR, reg_addr, 1, &value, 1);
    }
    return key_exist;
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
#if 0
__s32 boot_power_get_short_key(void)
{
	__s32 ret;

	ret = boot_power_get_key();
	if(ret > 0)
	{
		return ((ret & 0x02)>>1);
	}

	return ret;
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
#if 0
int boot_power_get_long_key(void)
{
	__s32 ret;

	ret = boot_power_get_key();
	if(ret > 0)
	{
		return (ret & 0x01);
	}

	return ret;
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
__s32  boot_power_get_dcin_battery_exist(__u32 *dcin_exist, __u32 *battery_exist)
{
    __u8  reg_addr, value;

    //pmu is AXP209
    reg_addr = BOOT_POWER20_STATUS;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    *dcin_exist = ((value >> 4) & 0x01) |  ((value >> 6) & 0x01);
    reg_addr = BOOT_POWER20_MODE_CHGSTATUS;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    *battery_exist  = (value >> 5) & 0x01;

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
*    说明    ：获取电池充电是否充满
*
*
************************************************************************************************************
*/
__s32 boot_power_battery_charge_status(void)
{
    __u8  reg_addr, value;
    __s32 charge_status;

    //pmu is AXP209
    reg_addr = BOOT_POWER20_INTSTS2;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    charge_status = (value & 0x04);
    value = charge_status;
    if(charge_status)
    {
        /*BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value);*/
        standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1);
    }

    return charge_status;
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
#if 0
__s32 ADC_Freq_Get()
{
	__u8 reg_addr, value;

	reg_addr = BOOT_POWER20_ADC_SPEED;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
	value &= 0xc0;
	switch(value >> 6)
	{
		case 0:
			value = 25;
			break;
		case 1:
			value = 50;
			break;
		case 2:
			value = 100;
			break;
		case 3:
			value = 200;
			break;
		default:
			value = 25;
			break;
	}
	return 	value;
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
#if 0
__u8 axp_vol_rate(__s32 Bat_Ocv_Vol)
{
    if(Bat_Ocv_Vol >= FUELGUAGE_TOP_VOL)         //4160
    {
        return FUELGUAGE_TOP_LEVEL;
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_LOW_VOL)    //<3400
    {
        return FUELGUAGE_LOW_LEVEL;
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_VOL1)       //3500
    {
        return FUELGUAGE_LOW_LEVEL + (FUELGUAGE_LEVEL1 - FUELGUAGE_LOW_LEVEL) * ((int)Bat_Ocv_Vol - FUELGUAGE_LOW_VOL) / (FUELGUAGE_VOL1 - FUELGUAGE_LOW_VOL);
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_VOL2)       //3600
    {
        return FUELGUAGE_LEVEL1 + (FUELGUAGE_LEVEL2 - FUELGUAGE_LEVEL1) * ((int)Bat_Ocv_Vol - FUELGUAGE_VOL1) / (FUELGUAGE_VOL2 - FUELGUAGE_VOL1);
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_VOL3)       //3700
    {
        return FUELGUAGE_LEVEL2 + (FUELGUAGE_LEVEL3 - FUELGUAGE_LEVEL2) * ((int)Bat_Ocv_Vol - FUELGUAGE_VOL2) / (FUELGUAGE_VOL3 - FUELGUAGE_VOL2);
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_VOL4)       //3800
    {
        return FUELGUAGE_LEVEL3 + (FUELGUAGE_LEVEL4 - FUELGUAGE_LEVEL3) * ((int)Bat_Ocv_Vol - FUELGUAGE_VOL3) / (FUELGUAGE_VOL4 - FUELGUAGE_VOL3);
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_VOL5)       //3900
    {
        return FUELGUAGE_LEVEL4 + (FUELGUAGE_LEVEL5 - FUELGUAGE_LEVEL4) * ((int)Bat_Ocv_Vol - FUELGUAGE_VOL4) / (FUELGUAGE_VOL5 - FUELGUAGE_VOL4);
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_VOL6)       //4000
    {
        return FUELGUAGE_LEVEL5 + (FUELGUAGE_LEVEL6 - FUELGUAGE_LEVEL5) * ((int)Bat_Ocv_Vol - FUELGUAGE_VOL5) / (FUELGUAGE_VOL6 - FUELGUAGE_VOL5);
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_VOL7)       //4100
    {
        return FUELGUAGE_LEVEL6 + (FUELGUAGE_LEVEL7 - FUELGUAGE_LEVEL6) * ((int)Bat_Ocv_Vol - FUELGUAGE_VOL6) / (FUELGUAGE_VOL7 - FUELGUAGE_VOL6);
    }
    else if(Bat_Ocv_Vol < FUELGUAGE_TOP_VOL)    //4100
    {
        return FUELGUAGE_LEVEL7 + (FUELGUAGE_TOP_LEVEL - FUELGUAGE_LEVEL7) * ((int)Bat_Ocv_Vol - FUELGUAGE_VOL7) / (FUELGUAGE_TOP_VOL - FUELGUAGE_VOL7);
    }
    else
    {
        return 0;
    }
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
#if 0
__s32 Get_Bat_Coulomb_Count()
{
	__u8  reg_addr;
	__u8  temp[8];
	__s32  rValue1,rValue2;
	__s32 Cur_CoulombCounter_tmp;

	reg_addr = BOOT_POWER20_BAT_CHGCOULOMB3;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &temp[0]))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &temp[0], 1))
    {
        return -1;
    }
    reg_addr = BOOT_POWER20_BAT_CHGCOULOMB2;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &temp[1]))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &temp[1], 1))
    {
        return -1;
    }
	reg_addr = BOOT_POWER20_BAT_CHGCOULOMB1;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &temp[2]))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &temp[2], 1))
    {
        return -1;
    }
    reg_addr = BOOT_POWER20_BAT_CHGCOULOMB0;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &temp[3]))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &temp[3], 1))
    {
        return -1;
    }
    reg_addr = BOOT_POWER20_BAT_DISCHGCOULOMB3;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &temp[4]))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &temp[4], 1))
    {
        return -1;
    }
    reg_addr = BOOT_POWER20_BAT_DISCHGCOULOMB2;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &temp[5]))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &temp[5], 1))
    {
        return -1;
    }
	reg_addr = BOOT_POWER20_BAT_DISCHGCOULOMB1;
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &temp[6], 1))
    {
        return -1;
    }
    reg_addr = BOOT_POWER20_BAT_DISCHGCOULOMB0;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &temp[7]))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &temp[7], 1))
    {
        return -1;
    }

	rValue1 = ((temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3]);
	rValue2 = ((temp[4] << 24) + (temp[5] << 16) + (temp[6] << 8) + temp[7]);
	//printf("Get_Bat_Coulomb_Count -     CHARGINGOULB:[0]=0x%x,[1]=0x%x,[2]=0x%x,[3]=0x%x\n",temp[0],temp[1],temp[2],temp[3]);
	//printf("Get_Bat_Coulomb_Count - DISCHARGINGCLOUB:[4]=0x%x,[5]=0x%x,[6]=0x%x,[7]=0x%x\n",temp[4],temp[5],temp[6],temp[7]);

	Cur_CoulombCounter_tmp = (4369 * (rValue1 - rValue2) / ADC_Freq_Get() / 240 / 2);

	return Cur_CoulombCounter_tmp;				//unit mAh
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
#if 0
__s32 boot_power_axp_rest_cal(void)
{
	__u8  reg_addr,value1,value2;
    __s32 bat_rest,Cur_CoulombCounter;

    reg_addr = BOOT_POWER20_DATA_BUFFER1;
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value1, 1))
    {
        return -1;
    }

    bat_rest = (int) (value1 & 0x7F);

    reg_addr = 0xB9;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value2))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value2, 1))
    {
        return -1;
    }

    Cur_CoulombCounter = ABS(Get_Bat_Coulomb_Count());
    if(ABS(bat_rest-(value2 & 0x7F)) >= 10 || Cur_CoulombCounter > 50){
        bat_rest = (int) (value2 & 0x7F);
    }

    if(bat_rest == 127)
    {
        bat_rest = 100;
    }

    value1 = bat_rest | 0x80;
    /*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &value1))*/
    if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &value1, 1))
    {
        return -1;
    }

    return bat_rest;
}
#endif

static  __u8  power_int_value[5];
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

#define NMI_CTL_REG            (0x01c00030)
#define NMI_IRG_PENDING_REG    (0x01c00034)
#define NMI_INT_ENABLE_REG     (0x01c00038)
    
#define writel(v, addr)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

__s32 boot_power_int_enable(void)
{
    __u8  reg_addr;
    __u8  int_enable[5];
    int	  i;

    reg_addr = BOOT_POWER20_INTEN1;
    /*if(BOOT_TWI_ReadEx(AXP20_ADDR, &reg_addr, power_int_value, 5))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, power_int_value, 5))
    {
        return -1;
    }

	int_enable[0] = 0x2C;	//开启：VBUS移除，ACIN移除
	int_enable[1] = 0;		//开启：充电完成
	int_enable[2] = 0x3;	//开启：电源按键短按，长按
	int_enable[3] = 0;
	int_enable[4] = 0;

	for(i=0;i<5;i++)
	{
		reg_addr = BOOT_POWER20_INTEN1 + i;
        /*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, int_enable + i))*/
    	if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &int_enable[i], 1))
    	{
        	return -1;
    	}
    }

    writel(0x0,NMI_CTL_REG);
    writel(0x1,NMI_IRG_PENDING_REG);
    writel(0x1,NMI_INT_ENABLE_REG);
    
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
__s32 boot_power_int_disable(void)
{
    __u8  reg_addr;
    int   i;

	for(i=0;i<5;i++)
	{
		reg_addr = BOOT_POWER20_INTEN1 + i;
        /*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, power_int_value + i))*/
    	if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, power_int_value + i, 1))
    	{
        	return -1;
    	}
    }
    writel(0x0,NMI_CTL_REG);
    writel(0x1,NMI_IRG_PENDING_REG);
    writel(0x0,NMI_INT_ENABLE_REG);
    
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


__s32 boot_power_int_query(__u8 *int_status)
{
    __u8  reg_addr;
    int	  i;

//	reg_addr = BOOT_POWER20_INTSTS1;
//    if(BOOT_TWI_ReadEx(AXP20_ADDR, &reg_addr, int_status, 5))
//    {
//        return -1;
//    }
//    if(BOOT_TWI_WriteEx(AXP20_ADDR, &reg_addr, int_status, 5))
//    {
//        return -1;
//    }
	for(i=0;i<5;i++)
	{
		reg_addr = BOOT_POWER20_INTSTS1 + i;
        /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, int_status + i))*/
    	if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, int_status + i, 1))
    	{
        	return -1;
    	}
        /*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, int_status + i))*/
    	if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, int_status + i, 1))
    	{
        	return -1;
    	}
    }

     writel(0x01,NMI_IRG_PENDING_REG);

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
#if 0
__s32 boot_power_vbus_cur_limit(int current)
{
	__u8 reg_addr, value;

	//set bus current limit off
    reg_addr = BOOT_POWER20_IPS_SET;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    value &= 0xfC;
	if(!current)
	{
	    value |= 0x03;
	}
	else if(current < 500)		//limit to 100
	{
		value |= 0x02;
	}
	else if(current < 900)		//limit to 500
	{
		value |= 0x01;
	}
	else						//limit to 900
	{
		;
	}
	/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &value))*/
	if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }

    return 0;
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
#if 0
__s32 boot_power_vbus_vol_limit(int vol)
{
	__u8 reg_addr, value;

	//set bus current limit off
    reg_addr = BOOT_POWER20_IPS_SET;
    /*if(BOOT_TWI_Read(AXP20_ADDR, &reg_addr, &value))*/
    if(standby_i2c_read(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }
    value &= ~(7 << 3);
	if(!vol)
	{
	    value &= ~(1 << 6);
	}
	else
	{
		if(vol < 4000)
		{
			vol = 4000;
		}
		else if(vol > 4700)
		{
			vol = 4700;
		}
		value |= ((vol-4000)/100) << 3;
	}
	/*if(BOOT_TWI_Write(AXP20_ADDR, &reg_addr, &value))*/
	if(standby_i2c_write(AXP20_ADDR, reg_addr, 1, &value, 1))
    {
        return -1;
    }

    return 0;
}
#endif




