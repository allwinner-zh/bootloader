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
#include <asm/arch/rsb.h>

static struct rsb_info rsbc = {1, 1, 1};
static struct rsb_slave_set rsb_slave[]=
{
	{
		NULL,
		RSB_SADDR_AW1655,
		RSB_RTSADDR2,
	},
};
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
static s32 rsb_write(u32 rtsaddr,u32 daddr, u8 *data,u32 len)
{
	u32 cmd = 0;
	u32 dat = 0;
	s32 i	= 0;
	if (len > 4 || len==0||len==3) {
		return -1;
	}
	if(NULL==data){
		return -1;
	}
	rsbc.rsb_flag = 0;
	rsbc.rsb_busy = 1;
	rsbc.rsb_load_busy	= 0;

	rsb_reg_writel(rtsaddr<<RSB_RTSADDR_SHIFT,RSB_REG_SADDR);
	rsb_reg_writel(daddr, RSB_REG_DADDR0);

	for(i=0;i<len;i++){
		dat |= data[i]<<(i*8);
	}

	rsb_reg_writel(dat, RSB_REG_DATA0);

	switch(len)	{
	case 1:
		cmd = RSB_CMD_BYTE_WRITE;
		break;
	case 2:
		cmd = RSB_CMD_HWORD_WRITE;
		break;
	case 4:
		cmd = RSB_CMD_WORD_WRITE;
		break;
	default:
		break;
	}
	rsb_reg_writel(cmd,RSB_REG_CMD);

	rsb_reg_writel(rsb_reg_readl(RSB_REG_CTRL)|RSB_START_TRANS, RSB_REG_CTRL);
	while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
		//istat will be optimize?
		u32 istat = rsb_reg_readl(RSB_REG_STAT);

		if(istat & RSB_LBSY_INT){
			rsbc.rsb_load_busy = 1;
			break;
		}

		if (istat & RSB_TERR_INT) {
			rsbc.rsb_flag = (istat >> 8) & 0xffff;
			rsb_reg_writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TOVER_INT) {
			rsbc.rsb_busy = 0;
			rsb_reg_writel(istat, RSB_REG_STAT);
		}
#endif
	}

	if(rsbc.rsb_load_busy){
		return RET_FAIL;
	}

	if (rsbc.rsb_flag)
	{
		return -rsbc.rsb_flag;
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
static s32 rsb_read(u32 rtsaddr,u32 daddr, u8 *data, u32 len)
{
	u32 cmd = 0;
	u32 dat = 0;
	s32 i	= 0;
	if (len > 4 || len==0||len==3)
	{
		return -1;
	}
	if(NULL==data)
	{
		return -1;
	}
	rsbc.rsb_flag = 0;
	rsbc.rsb_busy = 1;
	rsbc.rsb_load_busy	= 0;

	rsb_reg_writel(rtsaddr<<RSB_RTSADDR_SHIFT,RSB_REG_SADDR);
	rsb_reg_writel(daddr, RSB_REG_DADDR0);

	switch(len){
	case 1:
		cmd = RSB_CMD_BYTE_READ;
		break;
	case 2:
		cmd = RSB_CMD_HWORD_READ;
		break;
	case 4:
		cmd = RSB_CMD_WORD_READ;
		break;
	default:
		break;
	}
	rsb_reg_writel(cmd,RSB_REG_CMD);

	rsb_reg_writel(rsb_reg_readl(RSB_REG_CTRL)|RSB_START_TRANS, RSB_REG_CTRL);
	while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
		//istat will be optimize?
		u32 istat = rsb_reg_readl(RSB_REG_STAT);

		if(istat & RSB_LBSY_INT){
			rsbc.rsb_load_busy = 1;
			break;
		}

		if (istat & RSB_TERR_INT) {
			rsbc.rsb_flag = (istat >> 8) & 0xffff;
			rsb_reg_writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TOVER_INT) {
			rsbc.rsb_busy = 0;
			rsb_reg_writel(istat, RSB_REG_STAT);
		}
#endif
	}

	if(rsbc.rsb_load_busy)
	{
		return RET_FAIL;
	}

	if (rsbc.rsb_flag)
	{
		return -rsbc.rsb_flag;
	}

	dat = rsb_reg_readl(RSB_REG_DATA0);
	for(i=0;i<len;i++){
		data[i]=(dat>>(i*8))&0xff;
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
static int standby_rsb_read(u32 slave_id, u32 daddr, u8 *data, u32 len)
{
	s32 ret 	= 0;
	u32 rtaddr 	= 0;

	if(slave_id >= (sizeof(rsb_slave)/sizeof(struct rsb_slave_set)))
	{
		return -1;
	}
	rtaddr = rsb_slave[slave_id].m_rtaddr;

	ret = rsb_read(rtaddr, daddr, data, len);
	if(ret)
		return ret;

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
static int standby_rsb_write(u32 slave_id, u32 daddr, u8 *data, u32 len)
{
	s32 ret 	= 0;
	u32 rtaddr 	= 0;

	if(slave_id >= (sizeof(rsb_slave)/sizeof(struct rsb_slave_set)))
	{
		return -1;
	}

	rtaddr = rsb_slave[slave_id].m_rtaddr;

	ret = rsb_write(rtaddr, daddr, data, len);
	if(ret)
		return ret;

	return 0;
}


int standby_axp_i2c_read(unsigned char chip, unsigned char addr, unsigned char *buffer)
{
	return standby_rsb_read(0, addr, buffer, 1);
}


int standby_axp_i2c_write(unsigned char chip, unsigned char addr, unsigned char data)
{
	return standby_rsb_write(0, addr, &data, 1);
}

