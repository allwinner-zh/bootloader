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
#include "common.h"
#include "asm/io.h"
#include "asm/arch/efuse.h"

#define SID_OP_LOCK  (0xAC)
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
static void sid_program_key(uint key_index, uint key_value)
{
	uint reg_val;

	writel(key_value, SID_PRKEY);

	reg_val = readl(SID_PRCTL);
	reg_val &= ~((0x1ff<<16)|0x3);
	reg_val |= key_index<<16;
	writel(reg_val, SID_PRCTL);

	reg_val &= ~((0xff<<8)|0x3);
	reg_val |= (SID_OP_LOCK<<8) | 0x1;
	writel(reg_val, SID_PRCTL);

	while(readl(SID_PRCTL)&0x1){};

	reg_val &= ~((0x1ff<<16)|(0xff<<8)|0x3);
	writel(reg_val, SID_PRCTL);

	return;
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
static uint sid_read_key(uint key_index)
{
	uint reg_val;

	reg_val = readl(SID_PRCTL);
	reg_val &= ~((0x1ff<<16)|0x3);
	reg_val |= key_index<<16;
	writel(reg_val, SID_PRCTL);

	reg_val &= ~((0xff<<8)|0x3);
	reg_val |= (SID_OP_LOCK<<8) | 0x2;
	writel(reg_val, SID_PRCTL);

	while(readl(SID_PRCTL)&0x2){};

	reg_val &= ~((0x1ff<<16)|(0xff<<8)|0x3);
	writel(reg_val, SID_PRCTL);

	reg_val = readl(SID_RDKEY);

	return reg_val;
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
void sid_set_security_mode(void)
{
    uint reg_val;

    reg_val  = sid_read_key(EFUSE_LCJS);
    reg_val |= (0x01 << 11);		//Ê¹ÄÜsecurebit
    sid_program_key(EFUSE_LCJS, reg_val);
    reg_val = (sid_read_key(EFUSE_LCJS) >> 11) & 1;

    return;
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
int sid_probe_security_mode(void)
{
	return ((sid_read_key(EFUSE_LCJS)>>11) & 1);
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
void sid_read_chipid(void)
{
	uint chipid_index = 0;
	uint id_length = 4;
	uint i = 0;
	for(i = 0 ; i < id_length ;i++ )
	{
		sid_read_key(chipid_index + i *4);
	}
	return ;
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
void sid_read_thermal_sensor(void)
{
	uint thermal_sensor_index = 0x34;
	uint id_length = 2;
	uint i = 0;
	for(i = 0 ; i < id_length ;i++ )
	{
		sid_read_key(thermal_sensor_index + i *4);
	}
	return ;
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
void sid_read_oem(void)
{
	uint oem_index = 0x10;
	uint id_length = 1;
	uint i = 0;
	for(i = 0 ; i < id_length ;i++ )
	{
		sid_read_key(oem_index + i *4);
	}
	return ;
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
void sid_read(void)
{
	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
		return ;
	sid_read_chipid();
	sid_read_thermal_sensor();
        sid_read_oem();
        printf("sid read already \n");
	return ;
}
