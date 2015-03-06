/*
 * (C) Copyright 2007-2015
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
//*****************************************************************************
//	Allwinner Technology, All Right Reserved. 2006-2010 Copyright (c)
//
//	File: 				mctl_hal.c
//
//	Description:  This file implements basic functions for AW1633 DRAM controller
//
//	History:
//              2014/02/15      ysz       0.10    Initial version
//              2014/02/22      ysz       0.20    VTC disable & W/R data training enable
//              2014/02/28      ysz       0.30    disable eye training when dram initial,
//                                                write leveling adjust option enable
//              2014/03/05      ysz       0.40    Controller Priority re-configuration
//              2014/03/10      ysz       0.50    LPDDR3 funciton teseted, also Voltage changed to 1.2V
//              2014/03/11      ysz       0.60    LPDDR2 funciton teseted, android system OK;priority changed for 4K media
//				2014/04/03      ysz       0.70    increase PHY_UPD gap, disable ZCAL after DRAM initial when tpr13[31] asserted
//              2014/04/03      zhuwei    0.80    add frequence extend
//              2014/04/03      zhuwei    0.90    updata pll6 set
//              2014/04/11      zhuwei    1.00    PHY PLL configurable, increase delay after remapping in space scan
//              2014/04/11      zhuwei    1.01    add pll bypass mode
//              2014/05/21      ysz       1.02    disable data compare and auto refresh when dqs gate training
//              2014/05/21      ysz       1.03    set DRAMTMG8 to 0x8 to decrease the delay when exit self refresh
//              2014/05/21      ysz       1.04    equilibration of dqs gate delay between 2 rank DDR3
//              2014/05/22      ysz       1.90    LPDDR3 DQS Gate extend and change version to 1.9 to meet ar200 code version
//              2014/06/15      ysz       2.00    LPDDR3 tdinit0/tdinit1 value increased and timing increased after PIR for data scan
//              2014/06/15      ysz       2.10    tccd change from 4 to 2 for efficency
//              2014/06/30      ysz       2.20    PTR3 tdinit0/tdinit1 exchanged in LPDDR2/3, since the SPEC description error
//*****************************************************************************
//#include "include.h"
#include "mctl_reg.h"
#include "mctl_hal.h"
#include "mctl_par.h"


//=====================rsb adjust pmu dc5 voltage===========================
#define RSB_SCK		3000000
#define RSB_CDODLY	1
#define R_RSB_BASE       0x08003400
/* register define */
#define RSB_REG_CTRL	(R_RSB_BASE + 0x00)
#define RSB_REG_CCR		(R_RSB_BASE + 0x04)
#define RSB_REG_INTE	(R_RSB_BASE + 0x08)
#define RSB_REG_STAT	(R_RSB_BASE + 0x0c)
#define RSB_REG_DADDR0	(R_RSB_BASE + 0x10)
#define RSB_REG_DADDR1	(R_RSB_BASE + 0x14)
//#define RSB_REG_DLEN	(R_RSB_BASE + 0x18)
#define RSB_REG_DATA0	(R_RSB_BASE + 0x1c)
#define RSB_REG_DATA1	(R_RSB_BASE + 0x20)
#define RSB_REG_LCR		(R_RSB_BASE + 0x24)
#define RSB_REG_PMCR	(R_RSB_BASE + 0x28)
#define RSB_REG_CMD		(R_RSB_BASE + 0x2c)//RSB Command Register
#define RSB_REG_SADDR	(R_RSB_BASE + 0x30)//RSB Slave address Register

/* bit field */
#define RSB_SOFT_RST		(1U << 0)
#define RSB_GLB_INTEN		(1U << 1)
#define RSB_ABT_TRANS		(1U << 6)
#define RSB_START_TRANS		(1U << 7)
#define	RSB_USE_RSB			(1U<<8)//Use RSB

#define RSB_TOVER_INT		(1U << 0)
#define RSB_TERR_INT		(1U << 1)
#define RSB_LBSY_INT		(1U << 2)
#define RSB_TRANS_ERR_ID	(0xFF<<8)
#define RSB_TRANS_ERR_ID1	(0xFF<<16)//8’b00000001 쭯 no ack

#define RSB_PMU_INIT		(1U << 31)

#define PMU_INIT_DAT_SHIFT		(16)//Value of PMU’s initial data
#define PMU_MOD_REG_ADDR_SHIFT	(8)//PMU MODE Control Register Address

#define ERR_TRANS_1ST_BYTE		(0x1)//error happened with the transmission of the 1st byte of data
#define ERR_TRANS_2ND_BYTE		(0x2)//error happened with the transmission of the 2nd byte of data
#define ERR_TRANS_3RD_BYTE		(0x4)//error happened with the transmission of the 3rd byte of data
#define ERR_TRANS_4TH_BYTE		(0x8)// error happened with the transmission of the 4th byte of data
#define ERR_TRANS_RT_NO_ACK		(0x1<<8)//no ack when set runtime slave address.

#define RSB_CMD_BYTE_WRITE		(0x4E)//(0x27)//Byte write
#define RSB_CMD_HWORD_WRITE		(0x59)//(0x2c)//Half word write
#define RSB_CMD_WORD_WRITE		(0x63)//(0x31)//Word write
#define RSB_CMD_BYTE_READ		(0x8B)//(0x45)//Byte read
#define RSB_CMD_HWORD_READ		(0x9C)//(0x4e)//Half word read
#define RSB_CMD_WORD_READ		(0xA6)//(0x53)//Word read
#define RSB_CMD_SET_RTSADDR		(0xE8)//(0x74)//Set Run-time Address
/* RSB SHIFT */
#define RSB_RTSADDR_SHIFT	(16)//runtime slave address shift
#define RSB_SADDR_SHIFT		(0)//Slave Address shift

struct rsb_info {
	volatile u32 rsb_flag;
	volatile u32 rsb_busy;
	volatile u32 rsb_load_busy;
};

struct sunxi_rsb_slave_set
{
	u8 *m_slave_name;
	u32 m_slave_addr;
	u32 m_rtaddr;
	u32 chip_id;
};
#define  SUNXI_RSB_SLAVE_MAX      13
static struct sunxi_rsb_slave_set rsb_slave[SUNXI_RSB_SLAVE_MAX]={{NULL, 1, 1, 1}};
static int    sunxi_rsb_rtsaddr[16] = {0x2d, 0x3a, 0x3e, 0x59, 0x63, 0x74, 0x8b, 0x9c, 0xa6, 0xb1, 0xc5, 0xd2, 0xe8, 0xff};

static struct rsb_info rsbc = {1, 1, 1};
#define rsb_printk(format,arg...)	dram_dbg(format,##arg)
//#define rsb_printk(format,arg...)

static void rsb_cfg_io(void)
{
    //댔역CPUS PIO
    volatile unsigned int reg_val;

    reg_val = readl(0x08001400+0x28);
    rsb_printk("0x%x\n", reg_val);
    reg_val |= 1;
	writel(reg_val, 0x08001400+0x28);
	reg_val = readl(0x08001400+0x28);
	rsb_printk("0x%x\n", reg_val);
	//PN0,PN1 cfg 3
	writel(readl(0x08002c00+0x48)& ~0xff,0x08002c00+0x48);
	writel(readl(0x08002c00+0x48)|0x33,0x08002c00+0x48);
	//PN0,PN1 pull up 1
	writel(readl(0x08002c00+0x64)& ~0xf,0x08002c00+0x64);
	writel(readl(0x08002c00+0x64)|0x5,0x08002c00+0x64);
	//PN0,PN1 drv 2
	writel(readl(0x08002c00+0x5c)& ~0xf,0x08002c00+0x5c);
	writel(readl(0x08002c00+0x5c)|0xa,0x08002c00+0x5c);
}
static void rsb_module_reset(void)
{
//	r_prcm_module_reset(R_RSB_CKID);
	writel(readl(SUNXI_RPRCM_BASE + 0xb0)& ~(0x1U << 3),SUNXI_RPRCM_BASE + 0xb0);
	writel(readl(SUNXI_RPRCM_BASE + 0xb0)|(0x1U << 3),SUNXI_RPRCM_BASE + 0xb0);
}


static void rsb_clock_enable(void)
{
//	r_prcm_clock_enable(R_RSB_CKID);
	writel(readl(SUNXI_RPRCM_BASE + 0x28)|(0x1U << 3),SUNXI_RPRCM_BASE + 0x28);
}

static void rsb_set_clk(u32 sck)
{
	u32 src_clk = 0;
	u32 div = 0;
	u32 cd_odly = 0;
	u32 rval = 0;

	src_clk = 24000000;

	div = src_clk/sck/2;
	if(0==div){
		div = 1;
		rsb_printk("Source clock is too low\n");
	}else if(div>256){
		div = 256;
		rsb_printk("Source clock is too high\n");
	}
	div--;
	cd_odly = div >> 1;
	//cd_odly = 1;
	if(!cd_odly)
		cd_odly = 1;
	rval = div | (cd_odly << 8);
	writel(rval, RSB_REG_CCR);
}

/*	RSB function	*/
#ifdef RSB_USE_INT
static void rsb_irq_handler(void)
{
	u32 istat = readl(RSB_REG_STAT);

	if(istat & RSB_LBSY_INT){
		rsbc.rsb_load_busy = 1;
	}

	if (istat & RSB_TERR_INT) {
		rsbc.rsb_flag = (istat >> 8) & 0xffff;
	}

	if (istat & RSB_TOVER_INT) {
		rsbc.rsb_busy = 0;
	}

	writel(istat, RSB_REG_STAT);
}
#endif

static void rsb_init(void)
{
	rsbc.rsb_flag = 0;
	rsbc.rsb_busy = 0;
	rsbc.rsb_load_busy	= 0;

	rsb_cfg_io();
	rsb_module_reset();
	rsb_clock_enable();

	writel(RSB_SOFT_RST, RSB_REG_CTRL);
	rsb_set_clk(RSB_SCK);
#ifdef RSB_USE_INT
	writel(RSB_GLB_INTEN, RSB_REG_CTRL);
	writel(RSB_TOVER_INT|RSB_TERR_INT|RSB_LBSY_INT, RSB_REG_INTE);
	irq_request(GIC_SRC_RRSB, rsb_irq_handler);
	irq_enable(GIC_SRC_RRSB);
#endif
}

//
static s32 rsb_send_initseq(u32 slave_addr, u32 reg, u32 data)
{

	while(readl(RSB_REG_STAT)&(RSB_LBSY_INT|RSB_TERR_INT|RSB_TOVER_INT))
	{
		rsb_printk("status err\n");
	}

	rsbc.rsb_busy = 1;
	rsbc.rsb_flag = 0;
	rsbc.rsb_load_busy = 0;
	writel(RSB_PMU_INIT|(slave_addr << 1)				\
					|(reg << PMU_MOD_REG_ADDR_SHIFT)			\
					|(data << PMU_INIT_DAT_SHIFT), 				\
					RSB_REG_PMCR);
	while(readl(RSB_REG_PMCR) & RSB_PMU_INIT){
	}


	while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
		//istat will be optimize?
		u32 istat = readl(RSB_REG_STAT);

		if(istat & RSB_LBSY_INT){
			rsbc.rsb_load_busy = 1;
			writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TERR_INT) {
			rsbc.rsb_flag = (istat >> 8) & 0xffff;
			writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TOVER_INT) {
			rsbc.rsb_busy = 0;
			writel(istat, RSB_REG_STAT);
		}
#endif
	}

	if(rsbc.rsb_load_busy){
		rsb_printk("Load busy\n");
		return -1;
	}

	if (rsbc.rsb_flag) {
		rsb_printk(	"rsb write failed, flag 0x%x:%s%s%s%s%s !!\n",
					rsbc.rsb_flag,
					rsbc.rsb_flag & ERR_TRANS_1ST_BYTE	? " 1STE "  : "",
					rsbc.rsb_flag & ERR_TRANS_2ND_BYTE	? " 2NDE "  : "",
					rsbc.rsb_flag & ERR_TRANS_3RD_BYTE	? " 3RDE "  : "",
					rsbc.rsb_flag & ERR_TRANS_4TH_BYTE	? " 4THE "  : "",
					rsbc.rsb_flag & ERR_TRANS_RT_NO_ACK	? " NOACK "	: ""
					);
		return -rsbc.rsb_flag;
	}

	return 0;
}


static s32 set_run_time_addr(u32 saddr,u32 rtsaddr)
{

	while(readl(RSB_REG_STAT)&(RSB_LBSY_INT|RSB_TERR_INT|RSB_TOVER_INT))
	{
		rsb_printk("status err\n");
	}

	rsbc.rsb_busy = 1;
	rsbc.rsb_flag = 0;
	rsbc.rsb_load_busy = 0;
	writel((saddr<<RSB_SADDR_SHIFT)						\
					|(rtsaddr<<RSB_RTSADDR_SHIFT),				\
					RSB_REG_SADDR);
	writel(RSB_CMD_SET_RTSADDR,RSB_REG_CMD);
	writel(readl(RSB_REG_CTRL)|RSB_START_TRANS, RSB_REG_CTRL);

	while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
		//istat will be optimize?
		u32 istat = readl(RSB_REG_STAT);

		if(istat & RSB_LBSY_INT){
			rsbc.rsb_load_busy = 1;
			writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TERR_INT) {
			rsbc.rsb_flag = (istat >> 8) & 0xffff;
			writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TOVER_INT) {
			rsbc.rsb_busy = 0;
			writel(istat, RSB_REG_STAT);
		}
#endif
	}

	if(rsbc.rsb_load_busy){
		rsb_printk("Load busy\n");
		return -1;
	}

	if (rsbc.rsb_flag) {
		rsb_printk(	"rsb set run time address failed, flag 0x%x:%s%s%s%s%s !!\n",
					rsbc.rsb_flag,
					rsbc.rsb_flag & ERR_TRANS_1ST_BYTE	? " 1STE "  : "",
					rsbc.rsb_flag & ERR_TRANS_2ND_BYTE	? " 2NDE "  : "",
					rsbc.rsb_flag & ERR_TRANS_3RD_BYTE	? " 3RDE "  : "",
					rsbc.rsb_flag & ERR_TRANS_4TH_BYTE	? " 4THE "  : "",
					rsbc.rsb_flag & ERR_TRANS_RT_NO_ACK	? " NOACK "	: ""
					);
		return -rsbc.rsb_flag;
	}
	return 0;
}


//s32 rsb_write(u32 rtsaddr,struct rsb_ad *ad, u32 len)
static s32 rsb_write(u32 rtsaddr,u32 daddr, u8 *data,u32 len)
{
	u32 cmd = 0;
	u32 dat = 0;
	s32 i	= 0;
	if (len > 4 || len==0||len==3) {
		rsb_printk("error length %d\n", len);
		return -1;
	}
	if(NULL==data){
		rsb_printk("data should not be NULL\n");
		return -1;
	}

	while(readl(RSB_REG_STAT)&(RSB_LBSY_INT|RSB_TERR_INT|RSB_TOVER_INT))
	{
		rsb_printk("status err\n");
	}

	rsbc.rsb_flag = 0;
	rsbc.rsb_busy = 1;
	rsbc.rsb_load_busy	= 0;

	writel(rtsaddr<<RSB_RTSADDR_SHIFT,RSB_REG_SADDR);
	writel(daddr, RSB_REG_DADDR0);

	for(i=0;i<len;i++){
		dat |= data[i]<<(i*8);
	}

	writel(dat, RSB_REG_DATA0);
	//writel(*((u32*)data), RSB_REG_DATA0);
	//rsb_reg_writew(*((u16*)data), RSB_REG_DATA0);
//	writel((len-1)|RSB_WRITE_FLAG, RSB_REG_DLEN);

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
	writel(cmd,RSB_REG_CMD);

	writel(readl(RSB_REG_CTRL)|RSB_START_TRANS, RSB_REG_CTRL);
	while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
		//istat will be optimize?
		u32 istat = readl(RSB_REG_STAT);

		if(istat & RSB_LBSY_INT){
			rsbc.rsb_load_busy = 1;
			writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TERR_INT) {
			rsbc.rsb_flag = (istat >> 8) & 0xffff;
			writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TOVER_INT) {
			rsbc.rsb_busy = 0;
			writel(istat, RSB_REG_STAT);
		}
#endif
	}

	if(rsbc.rsb_load_busy){
		rsb_printk("Load busy\n");
		return -1;
	}

	if (rsbc.rsb_flag) {
		rsb_printk(	"rsb write failed, flag 0x%x:%s%s%s%s%s !!\n",
					rsbc.rsb_flag,
					rsbc.rsb_flag & ERR_TRANS_1ST_BYTE	? " 1STE "  : "",
					rsbc.rsb_flag & ERR_TRANS_2ND_BYTE	? " 2NDE "  : "",
					rsbc.rsb_flag & ERR_TRANS_3RD_BYTE	? " 3RDE "  : "",
					rsbc.rsb_flag & ERR_TRANS_4TH_BYTE	? " 4THE "  : "",
					rsbc.rsb_flag & ERR_TRANS_RT_NO_ACK	? " NOACK "	: ""
					);
		return -rsbc.rsb_flag;
	}

	return 0;
}


//s32 rsb_read(u32 rtsaddr,struct rsb_ad *ad, u32 len)
static s32 rsb_read(u32 rtsaddr,u32 daddr, u8 *data, u32 len)
{
	u32 cmd = 0;
	u32 dat = 0;
	s32 i	= 0;
	if (len > 4 || len==0||len==3) {
		rsb_printk("error length %d\n", len);
		return -1;
	}
	if(NULL==data){
		rsb_printk("data should not be NULL\n");
		return -1;
	}

	while(readl(RSB_REG_STAT)&(RSB_LBSY_INT|RSB_TERR_INT|RSB_TOVER_INT))
	{
		rsb_printk("status err\n");
	}

	rsbc.rsb_flag = 0;
	rsbc.rsb_busy = 1;
	rsbc.rsb_load_busy	= 0;

	writel(rtsaddr<<RSB_RTSADDR_SHIFT,RSB_REG_SADDR);
	writel(daddr, RSB_REG_DADDR0);
//	writel((len-1)|RSB_READ_FLAG, RSB_REG_DLEN);

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
	writel(cmd,RSB_REG_CMD);

	writel(readl(RSB_REG_CTRL)|RSB_START_TRANS, RSB_REG_CTRL);
	while(rsbc.rsb_busy){
#ifndef RSB_USE_INT
		//istat will be optimize?
		u32 istat = readl(RSB_REG_STAT);

		if(istat & RSB_LBSY_INT){
			rsbc.rsb_load_busy = 1;
			writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TERR_INT) {
			rsbc.rsb_flag = (istat >> 8) & 0xffff;
			writel(istat, RSB_REG_STAT);
			break;
		}

		if (istat & RSB_TOVER_INT) {
			rsbc.rsb_busy = 0;
			writel(istat, RSB_REG_STAT);
		}
#endif
	}

	if(rsbc.rsb_load_busy){
		rsb_printk("Load busy\n");
		return -1;
	}

	if (rsbc.rsb_flag) {
		rsb_printk(	"rsb read failed, flag 0x%x:%s%s%s%s%s !!\n",
					rsbc.rsb_flag,
					rsbc.rsb_flag & ERR_TRANS_1ST_BYTE	? " 1STE "  : "",
					rsbc.rsb_flag & ERR_TRANS_2ND_BYTE	? " 2NDE "  : "",
					rsbc.rsb_flag & ERR_TRANS_3RD_BYTE	? " 3RDE "  : "",
					rsbc.rsb_flag & ERR_TRANS_4TH_BYTE	? " 4THE "  : "",
					rsbc.rsb_flag & ERR_TRANS_RT_NO_ACK	? " NOACK "	: ""
					);
		return -rsbc.rsb_flag;
	}

	//*((u32*)data) = readl(RSB_REG_DATA0);
	//*((u16*)data) = rsb_reg_readw(RSB_REG_DATA0);

	dat = readl(RSB_REG_DATA0);
	for(i=0;i<len;i++){
		data[i]=(dat>>(i*8))&0xff;
	}


	return 0;
}

s32 sunxi_rsb_init(u32 slave_id)
{
    memset(rsb_slave, 0, SUNXI_RSB_SLAVE_MAX * sizeof(struct sunxi_rsb_slave_set));

	rsb_init();
	return rsb_send_initseq(0x00, 0x3e, 0x7c);
}

s32 sunxi_rsb_config(u32 slave_id, u32 rsb_addr)
{
	u32 rtaddr 		= 0;
    int i;

    for(i=0;i<SUNXI_RSB_SLAVE_MAX;i++)
    {
        if(!rsb_slave[i].m_slave_addr)
        {
            rsb_slave[i].m_slave_addr = rsb_addr;
            rsb_slave[i].m_rtaddr     = sunxi_rsb_rtsaddr[i];
            rsb_slave[i].chip_id      = slave_id;

            rtaddr     = sunxi_rsb_rtsaddr[i];

            return set_run_time_addr(rsb_addr, rtaddr);
        }
    }
    return -1;
}

s32 sunxi_rsb_read(u32 slave_id,u32 daddr, u8 *data, u32 len)
{
	u32 rtaddr 	= 0;
	u32 tmp_slave_id;
    int i;

	for(i=0;;)
    {
        tmp_slave_id = rsb_slave[i].chip_id;
        if(tmp_slave_id == slave_id)
        {
            break;
        }
        else if(!tmp_slave_id)
        {
            rsb_printk("sunxi_rsb_read err: bad id\n");
		    return -1;
        }
        i++;
    }

	rtaddr = rsb_slave[i].m_rtaddr;

	return rsb_read(rtaddr,daddr, data,len);
}

s32 sunxi_rsb_write(u32 slave_id,u32 daddr, u8 *data, u32 len)
{
	u32 rtaddr 	= 0;
	u32 tmp_slave_id;
    int i;

	for(i=0;;)
    {
        tmp_slave_id = rsb_slave[i].chip_id;
        if(tmp_slave_id == slave_id)
        {
            break;
        }
        else if(!tmp_slave_id)
        {
            rsb_printk("sunxi_rsb_write err: bad id\n");
		    return -1;
        }
        i++;
    }

	rtaddr = rsb_slave[i].m_rtaddr;

	return rsb_write(rtaddr,daddr, data,len);
}

//#define AW1639_DCDC2_VOL_CTRL	(0x22)
//#define AW1639_DCDC3_VOL_CTRL	(0x23)
//#define AW1639_DCDC4_VOL_CTRL	(0x24)
#define BOOT_POWER809_VERSION			(0x3)
#define BOOT_POWER809_DC5OUT_VOL		(0x25)

#define BOOT_POWER808_VERSION			(0x3)
#define BOOT_POWER808_DCBOUT_VOL		(0x13)
#define BOOT_POWER808_PRESET_CTL		(0x32)

#define SUNXI_AXP_809			(809)
#define SUNXI_AXP_808			(808)

#define AXP808_ADDR				(0x10)
#define AXP809_ADDR				(0x12)

#define  RSB_SADDR_AXP809		        (0x3A3)
#define  RSB_SADDR_AXP808		        (0x745)

int axp809_probe(void)
{
	u8    pmu_type;

	if(sunxi_rsb_config(AXP809_ADDR, RSB_SADDR_AXP809))
	{
		return -1;
	}
	if(sunxi_rsb_read(AXP809_ADDR, BOOT_POWER809_VERSION, &pmu_type, 1))
	{
		dram_dbg("Get pmu ID failed\n");
		return -1;
	}

	pmu_type &= 0xCF;
	if(pmu_type == 0x42)
	{
		dram_dbg("axp809 found\n");
		return 0;
	}

	return -1;
}

int axp808_probe(void)
{
	u8    pmu_type;
	sunxi_rsb_config(AXP808_ADDR, RSB_SADDR_AXP808);

	if(sunxi_rsb_read(AXP808_ADDR, BOOT_POWER808_VERSION, &pmu_type, 1))
	{
		dram_dbg("Get pmu ID failed\n");
		return -1;
	}

	pmu_type &= 0xCF;
	if(pmu_type == 0x40)
	{
		dram_dbg("axp808 found\n");
		return 0;
	}

	return -1;
}

int set_cpu_voltage(int dram_type)
{
	int ret = -1;
	u8 data = 0;
	unsigned int cpu_reg = 0;
	u8  pmu_id = 0;
	u8  vol;

    sunxi_rsb_init(0);
    if((dram_type == 3) && (axp809_probe() == 0)) //axp809
    {
		vol = (1500-1000)/50;
		ret = sunxi_rsb_write(AXP809_ADDR, BOOT_POWER809_DC5OUT_VOL, &vol, 1);
		if (ret)
		{
			dram_dbg("dram vol to %d failed\n", vol);
			return -1;
		}
    }
    else if(!axp808_probe()) //axp808
    {
		if(sunxi_rsb_read(AXP808_ADDR, BOOT_POWER808_PRESET_CTL, &data, 1))
		{
			dram_dbg("sel-mode reset bit read fail\n");
			return -1;
		}

		data |= (0x1 << 4);;
		ret = sunxi_rsb_write(AXP808_ADDR, BOOT_POWER808_PRESET_CTL, &data, 1);
		if (ret)
		{
			dram_dbg("sel-mode reset bit write fail\n", vol);
			return -1;
		}

    	if(dram_type != 3)
    	{
    		vol = (1200-1000)/50;
    		ret = sunxi_rsb_write(AXP808_ADDR, BOOT_POWER808_DCBOUT_VOL, &vol, 1);
			if (ret)
			{
				dram_dbg("dram vol to %d failed\n", vol);
				return -1;
			}
    	}
    }

	return 0;
}
//*****************************************************************************
//	void aw_delay (unsigned int n)
//  Description:	delay 1us, should be replaced by common delay function from SW
//
//	Arguments:
//
//	Return Value:	NONE
//*****************************************************************************
__inline void aw_delay (unsigned int n)
{
	//while(n--);
	__usdelay(n);
}

//*****************************************************************************
//	void paraconfig(unsigned int *para, unsigned int mask, unsigned int value)
//  Description:	DRAM structure value change
//
//	Arguments:
//
//	Return Value:	NONE
//*****************************************************************************
void paraconfig(unsigned int *para, unsigned int mask, unsigned int value)
{
	*para &= ~(mask);
	*para |= value;
}

//***********************************************************************************************
//	void auto_set_timing_para(__dram_para_t *para)
//
//  Description:	auto set the timing para base on the DRAM Frequency in structure
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************

void auto_set_timing_para(__dram_para_t *para)
{
	unsigned int dram_freq;
	unsigned int ctrl_freq;

	//default timing is for 1600bps of DDR3 (2GbX16)
	unsigned char tcl 		= 11;
	unsigned char tcwl		= 8;
	unsigned char trcd		= 11;
	unsigned char trc		= 39;
	unsigned char trrd		= 6;
	unsigned char tfaw		= 32;
	unsigned char tccd		= 4;
	unsigned char tras		= 28;
	unsigned char trp		= 11;
	unsigned char twtr		= 6;
	unsigned char twr		= 12;	//15ns min
	unsigned char trtp		= 6;
	unsigned char taond		= 0;
	unsigned char twlo		= 6;
	unsigned char twlmrd	= 40;
	unsigned short trfc		= 128;
	unsigned char tmod		= 12;
	unsigned char tmrd		= 4;
	unsigned char tmrw		= 10;
	unsigned char tmrr		= 4;
	unsigned char trtw		= 1;
	unsigned char trtodt	= 0;
	unsigned short tdllk	= 512;
	unsigned char tcke		= 4;
	unsigned char txp		= 20;
	unsigned short txs		= 512;
	unsigned char tdqsckmax	= 0;
	unsigned char tdqsck	= 0;
	unsigned short trefi	= 195;	//
	unsigned char twtp		= 24;	//write to pre_charge
	unsigned char trasmax	= 54;	//54*1024ck
	unsigned char twr2rd	= 16;
	unsigned char trd2wr	= 7;
	unsigned char tckesr	= 5;
	unsigned char tcksrx	= 8;
	unsigned char tcksre	= 8;
	unsigned char tcl_ctrl	= 6;	//CL to configure Controller, half rate
	unsigned char tcwl_ctrl	= 4;	//CWL to configre Controller

	if(para->dram_tpr13 & (0x1<<1) )	//User define timing
		return;

	dram_freq = (para->dram_clk);
	ctrl_freq = (para->dram_clk)/2;

	if(para->dram_type == 3)	//DDR3
	{
		//8Gb 350ns
		trfc		= (350*ctrl_freq)/1000 + ( ( ((350*ctrl_freq)%1000) != 0) ? 1 :0);

		trefi	= ( (7800*ctrl_freq)/1000 + ( ( ((7800*ctrl_freq)%1000) != 0) ? 1 :0) )/32;
		trcd	= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns
		trc		= (53*ctrl_freq)/1000 + ( ( ((53*ctrl_freq)%1000) != 0) ? 1 :0);	//52.5ns
		trp		= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
		tccd	= 2;
		tras	= (38*ctrl_freq)/1000 + ( ( ((38*ctrl_freq)%1000) != 0) ? 1 :0);	//37.5ns;
		twtr	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(twtr<4) twtr=4;	//4ck min
		twr		= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
		trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(trtp<4) trtp=4;	//4ck min

		if(para->dram_para2 & 0x4)	//X16-2KB pagesize
		{
			trrd	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
			if(trrd<4) trrd=4;	//4ck min
			tfaw	= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
		}
		else
		{
			trrd	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
			if(trrd<4) trrd=4;	//4ck min
			tfaw	= (40*ctrl_freq)/1000 + ( ( ((40*ctrl_freq)%1000) != 0) ? 1 :0);	//40ns;
		}

		if(dram_freq <= 400)
		{
			tcl		= 6;	//CAS latency
			tcwl	= 5;	//
			tcl_ctrl	= 3;	//round up of tcl/2
			tcwl_ctrl	= 3;	//round up of tcwl/2
		}
		else if(dram_freq <= 533)
		{
			tcl		= 8;	//CAS latency
			tcwl	= 6;	//
			tcl_ctrl	= 4;	//round up of tcl/2
			tcwl_ctrl	= 3;	//round up of tcwl/2
		}
		else if(dram_freq <= 667)
		{
			tcl		= 10;	//CAS latency
			tcwl	= 7;	//
			tcl_ctrl	= 5;	//round up of tcl/2
			tcwl_ctrl	= 4;	//round up of tcwl/2
		}
		else if(dram_freq <= 800)
		{
			tcl		= 11;	//CAS latency
			tcwl	= 8;	//
			tcl_ctrl	= 6;	//round up of tcl/2
			tcwl_ctrl	= 4;	//round up of tcwl/2
		}

	}
	else if(para->dram_type == 6)	//LPDDR2
	{
	//	tcl		= 8;	//depended on the hardware logic
	//	tcwl	= 5;
  //  tcl_ctrl	= 4;	//round up of tcl/2
	//	tcwl_ctrl	= 3;	//round up of tcwl/2

		//  lihongkai
		tcl		= 8;
		tcwl	= 4;
		tcl_ctrl	= 4;	//round up of tcl/2
		tcwl_ctrl	= 2;	//round up of tcwl/2

		tccd =4 ;
		// lihongkai end

		trfc	= (210*ctrl_freq)/1000 + ( ( ((210*ctrl_freq)%1000) != 0) ? 1 :0);	//trfcab=210ns;
		tdqsck	= (25*ctrl_freq)/10000 + ( ( ((25*ctrl_freq)%10000) != 0) ? 1 :0);	//2.5ns;
		tdqsckmax = tdqsck;	//5.5ns;--tdqsck+dqsgx-1
		trefi	= ( (3900*ctrl_freq)/1000 + ( ( ((3900*ctrl_freq)%1000) != 0) ? 1 :0) )/32;	//3.9us when chip density more than 2Gb
		tras	= (42*ctrl_freq)/1000 + ( ( ((42*ctrl_freq)%1000) != 0) ? 1 :0);	//42ns min;
		if(tras<3) tras	= 3;
		trasmax	= (70*ctrl_freq);	//70us max
//		tckesr	= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
//		if(tckesr<3) tckesr = 3;
		txs		= (140*ctrl_freq)/1000 + ( ( ((140*ctrl_freq)%1000) != 0) ? 1 :0);	//trfcab+10    130ns+10ns  210ns trfcab for 8Gb chip
		if(txs<2) txs = 2;
		txp		= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(txp<2) txp = 2;
		tccd	= 2;	//for LPDDR2-S4
		trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(trtp<2) trtp	= 2;
		trtp	= trtp + 1;	// BL/2 - 1 + max(7.5ns,2CK)	//added at 20130716
		trcd	= (24*ctrl_freq)/1000 + ( ( ((24*ctrl_freq)%1000) != 0) ? 1 :0);	//24ns;
		if(trcd<3) trcd	= 3;
		trp		= (27*ctrl_freq)/1000 + ( ( ((27*ctrl_freq)%1000) != 0) ? 1 :0);	//trpab 27ns;
		if(trp<3) trp = 3;
		twr		= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
		if(twr<3) twr = 3;
		trrd	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
		if(trrd<2) trrd	= 2;

		if(ctrl_freq<=200)
			twtr	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
		else
			twtr	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(twtr<2) twtr	= 2;

		if(ctrl_freq<=166)
			tfaw	= (60*ctrl_freq)/1000 + ( ( ((60*ctrl_freq)%1000) != 0) ? 1 :0);	//60ns;
		else
			tfaw	= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
		if(tfaw<8) tfaw	= 8;

	}
	else if(para->dram_type == 7)	//LPDDR3
	{
//		if(dram_freq<=400)	//RL and WL
//		{
//			tcl		= 6;
//			tcwl	= 3;
//		}
//		else if(dram_freq<=533)

/*		if(dram_freq<=533)
		{
			tcl		= 8;
			tcwl	= 5;		//WL = 4 in LPDDR3, set the controller with 5 depend on ZYF advice
			tcl_ctrl	= 4;	//round up of tcl/2
			tcwl_ctrl	= 3;	//round up of tcwl/2
		}
		else
		{
			tcl		= 9 + (dram_freq-533)/66;
			tcwl	= 5 + (dram_freq-533)/132;
			if(tcwl>6) tcwl = 6;
			tcl_ctrl	= 5;	//round up of tcl/2
			tcwl_ctrl	= 3;	//round up of tcwl/2
		}
		*/

		//  lihongkai
		tcl		= 12;
		tcwl	= 6;
		tcl_ctrl	= 6;	//round up of tcl/2
		tcwl_ctrl	= 3;	//round up of tcwl/2

		tccd =2 ;
		// lihongkai end

		tdqsck	= (25*ctrl_freq)/10000 + ( ( ((25*ctrl_freq)%10000) != 0) ? 1 :0);	//2.5ns;
		tdqsckmax = tdqsck;	//5.5ns;--tdqsck+dqsgx-1
		trfc	= (210*ctrl_freq)/1000 + ( ( ((210*ctrl_freq)%1000) != 0) ? 1 :0);	//trfcab=210ns;
		tcke	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(tcke<3)	tcke = 3;
		tckesr	= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
		if(tckesr<3) tckesr = 3;
		txs		= (140*ctrl_freq)/1000 + ( ( ((140*ctrl_freq)%1000) != 0) ? 1 :0);	//trfcab+10    130ns+10ns  210ns trfcab for 8Gb chip
		if(txs<2) txs = 2;
		txp		= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(txp<3) txp = 3;
		trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(trtp<4) trtp=4;	//4ck min
		trcd	= (24*ctrl_freq)/1000 + ( ( ((24*ctrl_freq)%1000) != 0) ? 1 :0);	//24ns;
		if(trcd<3) trcd	= 3;
		trp		= (27*ctrl_freq)/1000 + ( ( ((27*ctrl_freq)%1000) != 0) ? 1 :0);	//trpab 27ns;
		if(trp<3) trp = 3;
		tras	= (42*ctrl_freq)/1000 + ( ( ((42*ctrl_freq)%1000) != 0) ? 1 :0);	//42ns min;
		if(tras<3) tras	= 3;
		trasmax	= (70*ctrl_freq);	//70us max
		twr		= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
		if(twr<4) twr = 4;
		twtr	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
		if(twtr<4) twtr=4;	//4ck min
		trrd	= (10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);	//10ns;
		if(trrd<4) trrd	= 4;	//for burst length = 8
		tfaw	= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
		if(tfaw<8) tfaw	= 8;
		trefi	= ( (3900*ctrl_freq)/1000 + ( ( ((3900*ctrl_freq)%1000) != 0) ? 1 :0) )/32;	//3.9us

	}

	//reset the parameter in structure
	para->dram_tpr0 = (tcl<<0) | (tcwl<<5) | (trcd<<10) | (trrd<<15) | (tfaw<<20) | (tccd<<25);
	para->dram_tpr1 = (tras<<0) | (trp<<5) | (twr<<10) | (trtp<<15) | (twtr<<20) | (tdqsckmax<<25) | (tdqsck<<28);
	para->dram_tpr2 = (trefi<<0) | (trfc<<12) | (tcl_ctrl<<21) | (tcwl_ctrl<<26);

	if(para->dram_type == 3)	//DDR3
	{
		paraconfig(&(para->dram_mr2), 0x7U<<3, ((tcwl-5)<<3) );
		paraconfig(&(para->dram_mr0), 0x7U<<9, ((twr-4)<<9) );
		paraconfig(&(para->dram_mr0), 0x7U<<4, (((tcl-4)&0x7)<<4) );	//MR0[6:4]
		paraconfig(&(para->dram_mr0), 0x1U<<2, (((tcl-4)>>3)<<2) );	//MR0[2]
	}
	else	//LPDDR2/3
	{
		paraconfig(&(para->dram_mr1), 0x7U<<5, ((twr-2)<<5) );
		if(para->dram_type == 6)	//BL=4 for LPDDR2
			paraconfig(&(para->dram_mr1), 0x7U<<0, (2<<0) );
		else	//BL=8 for LPDDR#
			paraconfig(&(para->dram_mr1), 0x7U<<0, (3<<0) );
		paraconfig(&(para->dram_mr2), 0xfU<<0, ((tcl-2)<<0) );
	}

}

//*****************************************************************************
//	void ccm_wait_pll_stable()
//  Description:	wait PLL stable
//
//	Arguments:
//
//	Return Value:	NONE
//*****************************************************************************
static void ccm_wait_pll_stable(u32 pll_base)
{

	u32 rval = 0;
	u32 pll_bit = (pll_base - CCM_PLL_BASE) / 4;

	do {
		rval = (readl(CCM_PLL_LOCK_STA_REG) >> pll_bit) & 1;
	} while (rval == 0);

}

//*****************************************************************************
//	void ccm_setup_pll6_ddr_clk()
//  Description:	PLL_DDR congigure
//
//	Arguments:
//
//	Return Value:	NONE
//*****************************************************************************
 /* pll6 = 24*n/(div1+1)/(div2+1) */
unsigned int ccm_setup_pll6_ddr_clk(unsigned int pll_clk,unsigned int mode)
{

	unsigned int n,div1=0,div2=0;
	unsigned int rval = 0;

	if(mode==1)//pll nomal mode
	{
		div2=1;
		n=pll_clk/12000000;
	}
	else// pll bypass mode
	{
		if(pll_clk<=480)
		{
			div2=1;
			n=pll_clk/12000000;
		}
		else
		{
			n=pll_clk/24000000;
		}
	}

	rval = (1U << 31)| (div2 << 18) | (div1 << 16)  | (n << 8) ;;
	mctl_write_w(CCM_PLL6_DDR_REG, rval);
	rval |= (1U << 30);
	mctl_write_w(CCM_PLL6_DDR_REG, rval);  //Update PLL6 Setting
	ccm_wait_pll_stable(CCM_PLL6_DDR_REG);

	return 24000000 * n / (div1+1) / (div2+1);
}

//*****************************************************************************
//	void mctl_sys_init()
//  Description:	DRAMC system configuration
//
//	Arguments:
//
//	Return Value:	NONE
//*****************************************************************************
unsigned int mctl_sys_init(__dram_para_t *para)
{
	unsigned int reg_val = 0;

//	//disable DRAM CK, the ck output will be tied '0'
//	mctl_write_w(P0_PGCR3 + 0x0000, 0x00000060);
//	mctl_write_w(P0_PGCR3 + 0x1000, 0x00000060);

//	//disable 2 channel global clock
//	reg_val = mctl_read_w(MC_CCR);
//	reg_val &= ~((0x1<<15)|(0x1U<<31));
//	mctl_write_w(MC_CCR, reg_val);

	//controller reset
	reg_val = mctl_read_w(CCM_DRAMCLK_CFG_REG);
	reg_val &= ~(0x1U<<31);
	mctl_write_w(CCM_DRAMCLK_CFG_REG, reg_val);

	//ccm_module_disable(DRAM_CKID);
	reg_val = mctl_read_w(CCM_AHB0_SOFT_RST_REG);
	reg_val &= ~(1U<<14);
	writel(reg_val, CCM_AHB0_SOFT_RST_REG);

	//Setup PLL6 for DRAMC
	if((para->dram_clk <= 400)|((para->dram_tpr8 & 0x1)==0))	//PLL6 should be 2*CK
	{
		ccm_setup_pll6_ddr_clk( (1000000 * (para->dram_clk)*2),0 );//ccm_setup_pll6_ddr_clk(PLL6_DDR_CLK);
	}
	else						//PLL6 should be CK/2
	{
		ccm_setup_pll6_ddr_clk( (1000000 * (para->dram_clk)/2),1 );
	}
	if( para->dram_tpr13 & (0xf<<18) )//-------------------------------------------------------------------------
	{
		//********************************************
		//bit21:bit18=0001:pll 겠류0.4
		//bit21:bit18=0010:pll 겠류0.3
		//bit21:bit18=0100:pll 겠류0.2
		//bit21:bit18=1000:pll 겠류0.1
		//**********************************************
		dram_dbg("DRAM fre extend open !\n");
		reg_val=mctl_read_w(CCM_PLL6_DDR_REG);
		reg_val&=(0x1<<16);
		reg_val=reg_val>>16;

		if(para->dram_tpr13 & (0x1<<18))
		{
			mctl_write_w(CCM_PLL_BASE + 0x114, (0x3333U|(0x3<<17)|(reg_val<<19)|(0x120U<<20)|(0x2U<<29)|(0x1U<<31)));
		}
		else if(para->dram_tpr13 & (0x1<<19))//------------------------------------------------------------------------------------------------------------------------------------------------------------
		{
			mctl_write_w(CCM_PLL_BASE + 0x114, (0x6666U|(0x3U<<17)|(reg_val<<19)|(0xD8U<<20)|(0x2U<<29)|(0x1U<<31)));
		}
		else if(para->dram_tpr13 & (0x1<<20))//------------------------------------------------------------------------------------------------------------------------------------------------------------
		{
			mctl_write_w(CCM_PLL_BASE + 0x114, (0x9999U|(0x3U<<17)|(reg_val<<19)|(0x90U<<20)|(0x2U<<29)|(0x1U<<31)));
		}
		else if(para->dram_tpr13 & (0x1<<21))//------------------------------------------------------------------------------------------------------------------------------------------------------------
		{
			mctl_write_w(CCM_PLL_BASE + 0x114, (0xccccU|(0x3U<<17)|(reg_val<<19)|(0x48U<<20)|(0x2U<<29)|(0x1U<<31)));
		}

		reg_val = mctl_read_w(CCM_PLL6_DDR_REG);//frequency extend open
		reg_val |= ((0x1<<24)|(0x1<<30));
		mctl_write_w(CCM_PLL6_DDR_REG, reg_val);


		while(mctl_read_w(CCM_PLL6_DDR_REG) & (0x1<<30));
	}

	aw_delay(0x20000);	//make some delay

	//Setup DRAM Clock
//	reg_val = 0x0; //mctl_read_w(CCM_DRAMCLK_CFG_REG);
//	reg_val |= 0x3<<12;	//PLL_DDR
//	reg_val |= 0x1<<16;
//	mctl_write_w(CCM_DRAMCLK_CFG_REG, reg_val);
//	while(mctl_read_w(CCM_DRAMCLK_CFG_REG) & (0x1<<16));
//	reg_val = mctl_read_w(CCM_DRAMCLK_CFG_REG);
//	reg_val |= 0x1U<<31;
//	mctl_write_w(CCM_DRAMCLK_CFG_REG, reg_val);

	//Release DRAMC BUS Clock and Reset
	//ccm_clock_disable(DRAM_CKID);
	reg_val = mctl_read_w(CCM_AHB0_CLK_GAT_REG);
	reg_val &= ~(1U<<14);
	writel(reg_val, CCM_AHB0_CLK_GAT_REG);
	//ccm_module_disable(DRAM_CKID);
	reg_val = mctl_read_w(CCM_AHB0_SOFT_RST_REG);
	reg_val &= ~(1U<<14);
	writel(reg_val, CCM_AHB0_SOFT_RST_REG);

	aw_delay(20);
	//aw_delay(0x10);

	//ccm_module_enable(DRAM_CKID);
	reg_val = mctl_read_w(CCM_AHB0_SOFT_RST_REG);
	reg_val |= (1U<<14);
	writel(reg_val, CCM_AHB0_SOFT_RST_REG);
	//ccm_clock_enable(DRAM_CKID);
	reg_val = mctl_read_w(CCM_AHB0_CLK_GAT_REG);
	reg_val |= (1U<<14);
	writel(reg_val, CCM_AHB0_CLK_GAT_REG);

	aw_delay(20);

	//*****************************************************************priority, should be configured before global clk enable
//	mctl_write_w(0x01c09104, 0xff0);
	mctl_write_w(M0_SCHED, 0x0f802001);//mctl_write_w(M0_SCHED, 0x0f803700);
	mctl_write_w(M0_PERFHPR0, 0x0000001f);
	mctl_write_w(M0_PERFHPR1, 0x1f00001f);
	//mctl_write_w(M0_PERFLPR0, 0x00000010);//mctl_write_w(M0_PERFLPR0, 0x0000001f);
	//mctl_write_w(M0_PERFLPR1, 0x3f000010);//mctl_write_w(M0_PERFLPR1, 0x370000ff);
	mctl_write_w(M0_PERFLPR0, 0x000000ff);
	mctl_write_w(M0_PERFLPR1, 0x0f0000ff);
	mctl_write_w(M0_PERFWR0, 0x000000ff);
	mctl_write_w(M0_PERFWR1, 0x0f0001ff);


	mctl_write_w(M0_SCHED + 0x1000, 0x0f802001);//mctl_write_w(M0_SCHED + 0x1000, 0x0f803700);
	mctl_write_w(M0_PERFHPR0 + 0x1000, 0x0000001f);
	mctl_write_w(M0_PERFHPR1 + 0x1000, 0x1f00001f);
	//mctl_write_w(M0_PERFLPR0 + 0x1000, 0x00000010);//mctl_write_w(M0_PERFLPR0 + 0x1000, 0x0000001f);
	//mctl_write_w(M0_PERFLPR1 + 0x1000, 0x3f000010);//mctl_write_w(M0_PERFLPR1 + 0x1000, 0x370000ff);
	mctl_write_w(M0_PERFLPR0 + 0x1000, 0x000000ff);
	mctl_write_w(M0_PERFLPR1 + 0x1000, 0x0f0000ff);
	mctl_write_w(M0_PERFWR0 + 0x1000, 0x000000ff);
	mctl_write_w(M0_PERFWR1 + 0x1000, 0x0f0001ff);
	aw_delay(20);
	//*****************************************************************priority end

		//Setup DRAM Clock
	reg_val = 0x0; //mctl_read_w(CCM_DRAMCLK_CFG_REG);
	reg_val |= 0x3<<12;	//PLL_DDR
	reg_val |= 0x1<<16;
	mctl_write_w(CCM_DRAMCLK_CFG_REG, reg_val);
	while(mctl_read_w(CCM_DRAMCLK_CFG_REG) & (0x1<<16));
	reg_val = mctl_read_w(CCM_DRAMCLK_CFG_REG);
	reg_val |= 0x1U<<31;
	mctl_write_w(CCM_DRAMCLK_CFG_REG, reg_val);

	if((para->dram_clk <= 400)|((para->dram_tpr8 & 0x1)==0))	//PLL6 should be 2*CK
	{
		//gating 2 channel pll
		reg_val = mctl_read_w(MC_CCR);
		reg_val |= ((0x1<<14)|(0x1U<<30));
		mctl_write_w(MC_CCR, reg_val);
		mctl_write_w(MC_RMCR, 0x2);		//controller clock use pll6/4
	}
	else
	{
		//enalbe 2 channel pll
		reg_val = mctl_read_w(MC_CCR);
		reg_val &= ~((0x1<<14)|(0x1U<<30));
		mctl_write_w(MC_CCR, reg_val);
		mctl_write_w(MC_RMCR, 0x0);		//controller clock use pll6
	}

	reg_val = mctl_read_w(MC_CCR);
	reg_val &= ~((0x1<<15)|(0x1U<<31));
	mctl_write_w(MC_CCR, reg_val);
	aw_delay(20);
	//aw_delay(0x10);

	if(( ((para->dram_para2)>>8) & 0x0f) == 2)	//2 channel enable
		reg_val |= ( (0x1U<<31) | (0x1<<15) );
	else																				//1 channel enable
		reg_val |= (0x1<<15);

	mctl_write_w(MC_CCR, reg_val);

	return (1);
}

//*****************************************************************************
//	void mctl_com_init()
//  Description:	DRAM address configuration
//
//	Arguments:
//
//	Return Value:	NONE
//*****************************************************************************
unsigned int mctl_com_init(__dram_para_t *para)
{
	unsigned int reg_val = 0;

	reg_val = (1U<<22);
	reg_val |= ((para->dram_type & 0x07)<<16);//DRAM type
	reg_val |= ((((para->dram_para1)>>28) & 0x01) << 2);	//BANK
	reg_val |= ((( ( ((para->dram_para1)>>20) & 0xff) - 1) & 0xf) << 4);	//Row number
	reg_val |= (( ( (para->dram_para2) & 0x01 )? 0x1:0x3) << 12);	//DQ width-----------------no use now
	//reg_val |= ( (para->dram_para1)>>4 & 0x03 );	//rank
	reg_val |= ( (((para->dram_para2)>>12) & 0x03) - 1);	//rank//

	switch( ((para->dram_para1)>>16) & 0xf )	//MCTL_PAGE_SIZE
	{
	case 8:
		reg_val |= 0xA << 8;
		break;
	case 4:
		reg_val |= 0x9 << 8;
		break;
	case 2:
		reg_val |= 0x8 << 8;
		break;
	case 1:
		reg_val |= 0x7 << 8;
		break;
	default:
		reg_val |= 0x6 <<8;
	}
	if( ( ((para->dram_para2)>>8) & 0xf )==2)	//channel number
		reg_val |= 0x58<<16;		//Address Remap for Dual Channel
	else
		reg_val &= ~(0x18<<16);

	mctl_write_w(MC_CR, reg_val);  // for Single Channel //0x5a39c4 for Dual Channel

	return (1);
}

//*****************************************************************************
//	void DRAMC_get_dram_size()
//  Description:	read the DRAM Size of the DRAMC system
//
//	Arguments:
//
//	Return Value:	DRAM size in MBytes
//*****************************************************************************
signed int DRAMC_get_dram_size(void)
{
	unsigned int reg_val;
	unsigned int dram_size;
	unsigned int temp;

    reg_val = mctl_read_w(MC_CR);

    temp = (reg_val>>8) & 0xf;	//page size code
    dram_size = (temp - 6);	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>4) & 0xf;	//row width code
    dram_size += (temp + 1);	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>2) & 0x3;	//bank number code
    dram_size += (temp + 2);	//(1<<dram_size) * 512Bytes

    temp = reg_val & 0x3;	//rank number code
    dram_size += temp;	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>19) & 0x1;	//channel number code
    dram_size += temp;	//(1<<dram_size) * 512Bytes

    dram_size = dram_size - 11;	//(1<<dram_size)MBytes

    return (1<<dram_size);
}

//*****************************************************************************
//	unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
//  Description:	DRAMC channel intial
//
//	Arguments:
//
//	Return Value:	0: initial fail   1: initial OK
//*****************************************************************************
unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
{
	unsigned int reg_val = 0;
	unsigned int ch_offset = ch_index*0x1000;
	unsigned int i = 0,j = 0;
	unsigned int tdinit0,tdinit1,tdinit2,tdinit3;
	unsigned int dram_freq;
	unsigned int pad_hold_flag = 0;
	//added by yeshaozhen at 20131230
	unsigned int tcl;
	unsigned int tcwl;
	unsigned int trcd;
	unsigned int trrd;
	unsigned int tfaw;
	unsigned int tccd;
	unsigned int tras;
	unsigned int trp;
	unsigned int twr;
	unsigned int trtp;
	unsigned int twtr;
	unsigned int tdqsckmax;
	unsigned int tdqsck;
	unsigned int trefi;	//after 32 divider
	unsigned int trfc;
	unsigned int tcl_ctrl;
	unsigned int tcwl_ctrl;
	unsigned int trasmax;
	unsigned int trc;

	//2rank debug
	unsigned int rank0_lcdlr2 = 0;
	unsigned int rank1_lcdlr2 = 0;
	unsigned int rank0_gtr = 0;
	unsigned int rank1_gtr = 0;
	unsigned int tprd = 0;
	//2 rand debug end


	tcl = ( (para->dram_tpr0 >> 0) & 0x1f );
	tcwl = ( (para->dram_tpr0 >> 5) & 0x1f );
	trcd = ( (para->dram_tpr0 >> 10) & 0x1f );
	trrd = ( (para->dram_tpr0 >> 15) & 0x1f );
	tfaw = ( (para->dram_tpr0 >> 20) & 0x1f );
	tccd = ( (para->dram_tpr0 >> 25) & 0x1f );
	tras = ( (para->dram_tpr1 >> 0) & 0x1f );
	trp = ( (para->dram_tpr1 >> 5) & 0x1f );
	twr = ( (para->dram_tpr1 >> 10) & 0x1f );
	trtp = ( (para->dram_tpr1 >> 15) & 0x1f );
	twtr = ( (para->dram_tpr1 >> 20) & 0x1f );
	tdqsckmax = ( (para->dram_tpr1 >> 25) & 0x7 );
	tdqsck = ( (para->dram_tpr1 >> 28) & 0x7 );
	trefi = ( (para->dram_tpr2 >> 0) & 0xfff );	//after 32 divider
	trfc = ( (para->dram_tpr2 >> 12) & 0x1ff );
	tcl_ctrl = ( (para->dram_tpr2 >> 21) & 0x1f );
	tcwl_ctrl = ( (para->dram_tpr2 >> 26) & 0x1f );
	trasmax = 9*trefi/32;	//unit in 1024
	trc = tras + trp;
	//added finished

	dram_freq = (para->dram_clk);

#ifdef SYSTEM_SIMULATION
	//CHx Initialization
	mctl_write_w(M0_INT0 + ch_offset, 0x10001);  	//[25:16]: post cke; [9:0] pre_cke
	mctl_write_w(M0_INT1 + ch_offset, 0x10000);  	//[23:16] :rst; [14:8] wait; [3:0] ocd
	mctl_write_w(M0_INT2 + ch_offset, 0x00000);
	if((para->dram_type) == 6 || (para->dram_type) == 7)	//LPDDR2 and LPDDR3
	{
		mctl_write_w(M0_INT3 + ch_offset, ( (para->dram_mr1)<<16) | (para->dram_mr2) );//mctl_write_w(M0_INT3 + ch_offset, 0x6200000);
		mctl_write_w(M0_INT4 + ch_offset, (para->dram_mr3)<<16 );//mctl_write_w(M0_INT4 + ch_offset, 0x80000);
	}
	else	//DDR3
	{
		mctl_write_w(M0_INT3 + ch_offset, ( (para->dram_mr0)<<16) | (para->dram_mr1) );//mctl_write_w(M0_INT3 + ch_offset, 0x6200000);
		mctl_write_w(M0_INT4 + ch_offset, ( (para->dram_mr2)<<16) | (para->dram_mr3) );//mctl_write_w(M0_INT4 + ch_offset, 0x80000);
	}

	mctl_write_w(M0_INT5 + ch_offset, 0x80000);		//[23:16]  zqcl init
#else
	//CHx Initialization
	if((para->dram_type) == 3)	//DDR3
		mctl_write_w(M0_INT0 + ch_offset, (0x1U<<16)|(500*para->dram_clk/1024));	//100CK | 500us
	else	//LPDDR2/3
		mctl_write_w(M0_INT0 + ch_offset, (500*para->dram_clk/1024)<<16);	//200us | 0

	mctl_write_w(M0_INT1 + ch_offset, 0x10000);  	//[23:16] :rst; [14:8] wait; [3:0] ocd
	if((para->dram_type) == 6 || (para->dram_type) == 7)	//LPDDR2 and LPDDR3
	{
		mctl_write_w(M0_INT3 + ch_offset, ( (para->dram_mr1)<<16) | (para->dram_mr2) );//mctl_write_w(M0_INT3 + ch_offset, 0x6200000);
		mctl_write_w(M0_INT4 + ch_offset, (para->dram_mr3)<<16 );//mctl_write_w(M0_INT4 + ch_offset, 0x80000);
	}
	else	//DDR3
	{
		mctl_write_w(M0_INT3 + ch_offset, ( (para->dram_mr0)<<16) | (para->dram_mr1) );//mctl_write_w(M0_INT3 + ch_offset, 0x6200000);
		mctl_write_w(M0_INT4 + ch_offset, ( (para->dram_mr2)<<16) | (para->dram_mr3) );//mctl_write_w(M0_INT4 + ch_offset, 0x80000);
	}

	if((para->dram_type) == 6 || (para->dram_type) == 7)	//LPDDR2 and LPDDR3
	{
		reg_val	= (para->dram_clk)/32+( ( ((para->dram_clk)%32) != 0) ? 1 :0);	//1us,unit-32CK
		reg_val = reg_val <<8;
		reg_val |= 5;
		mctl_write_w(M0_INT2 + ch_offset, reg_val);		////DDR3 don't care

		reg_val	= (para->dram_clk)/32+( ( ((para->dram_clk)%32) != 0) ? 1 :0);	//1us,unit-32CK
		reg_val = reg_val <<16;
		reg_val |= (10*para->dram_clk)/1024 + ( ( ((10*para->dram_clk)%1024) != 0) ? 1 :0);	//10us,unit-1024CK
		mctl_write_w(M0_INT5 + ch_offset, reg_val);		////DDR3 don't care
	}
#endif
	//mctl_write_w(M0_DRAMTMG4 + ch_offset, 0x05020405);
	reg_val = ( (tras<<0)	| (trasmax<<8) | (tfaw<<16) | ((tcwl_ctrl+2+twr)<<24) );//ras|rasmax|faw|wr2pre
	mctl_write_w(M0_DRAMTMG0 + ch_offset, reg_val);//mctl_write_w(M0_DRAMTMG0 + ch_offset, (para->mx_dramtmg0));

	reg_val = ( (trc<<0)	| (trtp<<8) | (8<<16) );	//trc|trtp|txp
	mctl_write_w(M0_DRAMTMG1 + ch_offset, reg_val);//mctl_write_w(M0_DRAMTMG1 + ch_offset, (para->mx_dramtmg1));

	reg_val = ( ((tcwl_ctrl+2+twtr)<<0)	| ((tcl_ctrl+2+2+1)<<8) | (tcl_ctrl<<16) | (tcwl_ctrl<<24) );	//wr2rd|rd2wr|txp
	mctl_write_w(M0_DRAMTMG2 + ch_offset, reg_val);//mctl_write_w(M0_DRAMTMG2 + ch_offset, (para->mx_dramtmg2));

	reg_val = ( (0xc<<0) | (0x4<<12) | (0x5<<16) );
	mctl_write_w(M0_DRAMTMG3 + ch_offset, reg_val);//mctl_write_w(M0_DRAMTMG3 + ch_offset, (para->mx_dramtmg3));

	reg_val = ( (trp<<0) | (trrd<<8) | (tccd<<16) | (trcd<<24) );
	mctl_write_w(M0_DRAMTMG4 + ch_offset, reg_val);//mctl_write_w(M0_DRAMTMG4 + ch_offset, (para->mx_dramtmg4));

	reg_val = ( (0x3<<0) | (0x4<<8) | (0x5<<16) | (0x5<<24) );//cksrx |cksre |ckesr |cke
	mctl_write_w(M0_DRAMTMG5 + ch_offset, reg_val);//mctl_write_w(M0_DRAMTMG5 + ch_offset, (para->mx_dramtmg5));

	reg_val = ( (0x5<<0) | (0x2<<16) | (0x2<<24) );// ckcsx | ckdpx | ckdpde
	mctl_write_w(M0_DRAMTMG6 + ch_offset, reg_val);//mctl_write_w(M0_DRAMTMG6 + ch_offset, (para->mx_dramtmg6));

	reg_val = (0x8<<0);// Minimum time to wait after coming out of self refresh before doing anything
	mctl_write_w(M0_DRAMTMG8 + ch_offset, reg_val);//mctl_write_w(M0_DRAMTMG6 + ch_offset, (para->mx_dramtmg6));

	reg_val = ( (trfc<<0) | (trefi<<16) );//
	mctl_write_w(M0_RFSHTMG + ch_offset, reg_val);	//mctl_write_w(M0_RFSHTMG + ch_offset, (para->mx_rfshtmg));

	if(para->dram_type == 3)	//DDR3
		reg_val = (( tcwl_ctrl - 2 ) | (0x1<<8) | (( tcl_ctrl - 2 ) << 16) | (0x2<<24));
	else											//LPDDR3 ---by lihongkai
		reg_val = (( tcwl_ctrl - 1 ) | (0x1<<8) | (( tcl_ctrl - 1 ) << 16) | (0x2<<24));

	if( (reg_val & 0x1f) ==0)
		reg_val |= 0x1;	//the write latency domain must not be zero
	mctl_write_w(M0_PITMG0 + ch_offset, reg_val);

	mctl_write_w(M0_PIMISC + ch_offset, 0x0);
	mctl_write_w(M0_UPD0 + ch_offset, 0x80000000);	//disable dll calibration								????????

//**********************************************************************
//increas DFI_PHY_UPD clock for nand dma debug
//**********************************************************************
//open protect registers---------------for test only
	reg_val = mctl_read_w(MC_RMCR);
	reg_val |= (0x1<<3);
	mctl_write_w(MC_RMCR, reg_val);

	aw_delay(100);

	reg_val = mctl_read_w(M0_UPD2 + ch_offset);
	reg_val &= ~(0xfff<<16);
	reg_val |= (0x50<<16);
	mctl_write_w(M0_UPD2 + ch_offset, reg_val);

	reg_val = mctl_read_w(MC_RMCR);
	reg_val &= ~(0x1<<3);
	mctl_write_w(MC_RMCR, reg_val);
//debug end

	reg_val = 0;	//
	if( ((para->dram_para2)>>12)==2 )	//two rank
		reg_val |= (0x3<<24);
	else															// one rank
		reg_val |= (0x1<<24);

	if((para->dram_type) == 2)	//DDR2
		reg_val |= (0x2<<16);
	else if((para->dram_type) == 3)	//
		reg_val |= (0x4<<16);
	else if((para->dram_type) == 6)	//LPDDR2
		reg_val |= (0x2<<16);
	else if((para->dram_type) == 7)	//LPDDR3
		reg_val |= (0x4<<16);														//check the burst length of LPDDR3
	else	//un-available
	{
		//printk("ERROR:DRAM type not available!!!!\n");
		return (0);
	}

	if((para->dram_para2)&0x01)	//if((para->dram_tpr4)&0x10)	//half dq
		reg_val |= (0x1<<12);
	else												// full dq
		reg_val |= (0x0<<12);

	if((para->dram_tpr13)&(0x01<<5))	//2T mode
		reg_val |= (0x1<<10);

	if((para->dram_type) == 2)	//DDR2
		reg_val |= (0x0<<0);
	else if((para->dram_type) == 3)	//
		reg_val |= (0x1<<0);
	else if((para->dram_type) == 6)	//LPDDR2
		reg_val |= (0x1<<2);
	else if((para->dram_type) == 7)	//LPDDR3
		reg_val |= (0x1<<3);
	else	//un-available
	{
		//printk("ERROR:DRAM type not available!!!!\n");
		return (0);
	}

	mctl_write_w(M0_MSTR + ch_offset, reg_val);

	if((para->dram_type) == 6 || (para->dram_type) == 7)	//LPDDR2 and LPDDR3
	{
		reg_val	= (0<<31)|(0<<31)|(0x120U<<16)|(0x48U<<0);	//ZQCS ZQCL tZQCL tZQCS
		mctl_write_w(M0_ZQCTRL0 + ch_offset, reg_val);
		reg_val	= (0x28U<<20)|(0x100);
		mctl_write_w(M0_ZQCTRL1 + ch_offset, reg_val);
	}
	else if((para->dram_type) == 3)	//DDR3
	{
		reg_val	= (0<<31)|(0<<31)|(0x200U<<16)|(0x40U<<0);	//ZQCS ZQCL tZQCL tZQCS
		mctl_write_w(M0_ZQCTRL0 + ch_offset, reg_val);
		reg_val	= 0x00000100;
		mctl_write_w(M0_ZQCTRL1 + ch_offset, reg_val);
	}

	mctl_write_w(M0_PIMISC + ch_offset, 0x01);      //start Controller init     ???
	mctl_write_w(M0_RFSHCTL3 + ch_offset, 0x01);	//disable auto-refresh generated by controller	???

	reg_val = 0x400;	//BYTEMASK
	if((para->dram_tpr13)&(0x01<<5))	//2T mode
		reg_val |= (0x1<<28);
	if((para->dram_para1) & (0x01<<28))	//if((para->dram_para1) & (0xf<<24) == (0x8<<24))	//8 banks?
		reg_val |= (0x1<<3);

	if((para->dram_type) == 2)	//DDR2
		reg_val |= (0x2<<0);
	else if((para->dram_type) == 3)	//DDR3
		reg_val |= (0x3<<0);
	else if((para->dram_type) == 6)	//LPDDR2
	{
		reg_val |= (0x0<<0);
		reg_val |= (0<<8);//LPDDR2-S4    reg_val |= ((para->lpddr2_type)<<8);
	}
	else if((para->dram_type) == 7)	//LPDDR3
	{
		reg_val |= (0x1<<0);
	}
	else	//un-available
	{
		//printk("ERROR:DRAM type not available!!!!\n");
		return (0);
	}

	mctl_write_w(P0_DCR + ch_offset, reg_val);//

	if((para->dram_type) == 6 || (para->dram_type) == 7)	//LPDDR2 and LPDDR3
	{
		reg_val = mctl_read_w(P0_DSGCR + ch_offset);
		reg_val &= (~(0x3<<6)); 	//set DQSGX to 0 before training
		//reg_val |= (0x1<<6);
		mctl_write_w(P0_DSGCR + ch_offset, reg_val);
	}

	mctl_write_w(P0_MR0 + ch_offset, ((para->dram_mr0)&0x1fff) );	//mctl_write_w(P0_MR0 + ch_offset, 0x620);
	mctl_write_w(P0_MR1 + ch_offset, ((para->dram_mr1)&0x1fff) );	//mctl_write_w(P0_MR1 + ch_offset, 0x0);
	mctl_write_w(P0_MR2 + ch_offset, ((para->dram_mr2)&0x7ff) );	//mctl_write_w(P0_MR2 + ch_offset, 0x8);
	mctl_write_w(P0_MR3 + ch_offset, ((para->dram_mr3)&0xf) );	//mctl_write_w(P0_MR3 + ch_offset, 0x0);

	reg_val = ( (trtp<<0) | (twtr<<4) | (trp<<8) | (trcd<<12)| (tras<<16)  | (trrd<<22) | (trc<<26) );
	mctl_write_w(P0_DTPR0 + ch_offset, reg_val );//mctl_write_w(P0_DTPR0 + ch_offset, (para->dram_tpr0) );

	reg_val = ( (0x2<<0) | (0x4<<2) | (tfaw<<5) | (trfc<<11) | (0x28<<20) | (0x8<<26) | (0x0<<30) );
	mctl_write_w(P0_DTPR1 + ch_offset, reg_val );//mctl_write_w(P0_DTPR1 + ch_offset, (para->dram_tpr1) );

	reg_val = ( (0x200<<0) | (0x1a<<10) | (0x6<<15) | (0x200<<19) | (0x0<<29) | (0x0<<30) | (0x0<<31) );
	mctl_write_w(P0_DTPR2 + ch_offset, reg_val );//mctl_write_w(P0_DTPR2 + ch_offset, (para->dram_tpr2) );

	reg_val = ( (tdqsck<<0) | (tdqsckmax<<3) );
	mctl_write_w(P0_DTPR3 + ch_offset, reg_val );//mctl_write_w(P0_DTPR3 + ch_offset, (para->dram_tpr3) );

#ifdef SYSTEM_SIMULATION
	mctl_write_w(P0_PTR0 + ch_offset, 0x01e007c3);
	mctl_write_w(P0_PTR1 + ch_offset, 0x00170023);
	mctl_write_w(P0_PTR3 + ch_offset, 0x0050001a);
	mctl_write_w(P0_PTR4 + ch_offset, 0x0800003a);		//should be different with different type and frequency
	mctl_write_w(P0_DTCR + ch_offset, 0x01000081);
#else
	mctl_write_w(P0_PTR0 + ch_offset, 0x42C21590);	//use default
	mctl_write_w(P0_PTR1 + ch_offset, 0xD05612C0);	//use default
//	mctl_write_w(P0_PTR3 + ch_offset, 0x18082356);	//use default
//	mctl_write_w(P0_PTR4 + ch_offset, 0x0C834156);	//use default
	if((para->dram_type) == 6 || (para->dram_type) == 7)	//LPDDR2	//LPDDR3
	{
		tdinit0	= (100*dram_freq)/1000 + 1;	//100ns
		tdinit1	= (200*dram_freq) + 1;	//200us
		tdinit2	= (22*dram_freq) + 1;	//11us
		tdinit3	= (2*dram_freq) + 1;	//1us
	}
	else if((para->dram_type) == 3)	//	DDR3
	{
		tdinit0	= (500*dram_freq) + 1;	//500us
		tdinit1	= (360*dram_freq)/1000 + 1;	//360ns
		tdinit2	= (200*dram_freq) + 1;	//200us
		tdinit3	= (1*dram_freq) + 1;	//1us
	}
	else if((para->dram_type) == 2)	//DDR2
	{
		tdinit0	= (200*dram_freq) + 1;	//200us
		tdinit1	= (400*dram_freq)/1000 + 1;	//400ns
		tdinit2	= (200*dram_freq) + 1;	//200us		use the value to DDR3
		tdinit3	= (1*dram_freq) + 1;	//1us		use the value to DDR3
	}
	mctl_write_w(P0_PTR3 + ch_offset, (tdinit1<<20) | tdinit0 );	//use default
	mctl_write_w(P0_PTR4 + ch_offset, (tdinit3<<18) | tdinit2 );	//use default

	reg_val = 0x00003007;	//reg_val = 0x80003087;
	for(i=0;i<((para->dram_para2)>>12 & 0xf);i++)	//rank information
		reg_val |= (1U<<(i+24));
	mctl_write_w(P0_DTCR + ch_offset, reg_val);	//use default
#endif

	if((para->dram_para2)&0x01)//if((para->dram_tpr4)&0x10)	//half DQ, disable high 2 group DQ
	{
		mctl_write_w(P0_DX2GCR0 + ch_offset, 0x0);	//disable high 2 group DQ
		mctl_write_w(P0_DX3GCR0 + ch_offset, 0x0);
	}
	else
	{
		mctl_write_w(P0_DX2GCR0 + ch_offset, 0x7C000285);	//default value
		mctl_write_w(P0_DX3GCR0 + ch_offset, 0x7C000285);
	}

	reg_val = mctl_read_w(P0_ZQ0PR + ch_offset);	//CK/CA
	reg_val &= ~(0xff);
	reg_val |= (para->dram_zq & 0xff);
	mctl_write_w(P0_ZQ0PR + ch_offset, reg_val);

	reg_val = mctl_read_w(P0_ZQ1PR + ch_offset);	//DX0/DX1
	reg_val &= ~(0xff);
	reg_val |= ( (para->dram_zq>>8) & 0xff);
	mctl_write_w(P0_ZQ1PR + ch_offset, reg_val);

	reg_val = mctl_read_w(P0_ZQ2PR + ch_offset);	//DX2/DX3
	reg_val &= ~(0xff);
	reg_val |= ( (para->dram_zq>>16) & 0xff);
	mctl_write_w(P0_ZQ2PR + ch_offset, reg_val);

	if(para->dram_odt_en)
	{
		reg_val = mctl_read_w(P0_DX0GCR2 + ch_offset);
		reg_val &= ~(0xffff);
		mctl_write_w(P0_DX0GCR2 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX0GCR3 + ch_offset);
		reg_val &= ~(0x3<<12);
		reg_val &= ~(0x3<<4);
		mctl_write_w(P0_DX0GCR3 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX1GCR2 + ch_offset);
		reg_val &= ~(0xffff);
		mctl_write_w(P0_DX1GCR2 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX1GCR3 + ch_offset);
		reg_val &= ~(0x3<<12);
		reg_val &= ~(0x3<<4);
		mctl_write_w(P0_DX1GCR3 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX2GCR2 + ch_offset);
		reg_val &= ~(0xffff);
		mctl_write_w(P0_DX2GCR2 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX2GCR3 + ch_offset);
		reg_val &= ~(0x3<<12);
		reg_val &= ~(0x3<<4);
		mctl_write_w(P0_DX2GCR3 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX3GCR2 + ch_offset);
		reg_val &= ~(0xffff);
		mctl_write_w(P0_DX3GCR2 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX3GCR3 + ch_offset);
		reg_val &= ~(0x3<<12);
		reg_val &= ~(0x3<<4);
		mctl_write_w(P0_DX3GCR3 + ch_offset, reg_val);

	}
	else
	{
		reg_val = mctl_read_w(P0_DX0GCR2 + ch_offset);
		reg_val &= ~(0xffff);
		reg_val |= 0xaaaa;
		mctl_write_w(P0_DX0GCR2 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX1GCR2 + ch_offset);
		reg_val &= ~(0xffff);
		reg_val |= 0xaaaa;
		mctl_write_w(P0_DX1GCR2 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX2GCR2 + ch_offset);
		reg_val &= ~(0xffff);
		reg_val |= 0xaaaa;
		mctl_write_w(P0_DX2GCR2 + ch_offset, reg_val);

		reg_val = mctl_read_w(P0_DX3GCR2 + ch_offset);
		reg_val &= ~(0xffff);
		reg_val |= 0xaaaa;
		mctl_write_w(P0_DX3GCR2 + ch_offset, reg_val);


		if(para->dram_type == 3)	//DDR3				Note:LPDDR2/3 can not Disable DQS ODT
		{
			reg_val = mctl_read_w(P0_DX0GCR3 + ch_offset);
			reg_val &= ~(0x3<<12);
			reg_val &= ~(0x3<<4);
			reg_val |= (0x2<<12);
			reg_val |= (0x2<<4);
			mctl_write_w(P0_DX0GCR3 + ch_offset, reg_val);

			reg_val = mctl_read_w(P0_DX1GCR3 + ch_offset);
			reg_val &= ~(0x3<<12);
			reg_val &= ~(0x3<<4);
			reg_val |= (0x2<<12);
			reg_val |= (0x2<<4);
			mctl_write_w(P0_DX1GCR3 + ch_offset, reg_val);

			reg_val = mctl_read_w(P0_DX2GCR3 + ch_offset);
			reg_val &= ~(0x3<<12);
			reg_val &= ~(0x3<<4);
			reg_val |= (0x2<<12);
			reg_val |= (0x2<<4);
			mctl_write_w(P0_DX2GCR3 + ch_offset, reg_val);

			reg_val = mctl_read_w(P0_DX3GCR3 + ch_offset);
			reg_val &= ~(0x3<<12);
			reg_val &= ~(0x3<<4);
			reg_val |= (0x2<<12);
			reg_val |= (0x2<<4);
			mctl_write_w(P0_DX3GCR3 + ch_offset, reg_val);
		}
		else//LPDDR3
		{
			//  TE ON AND PDD OFF
			mctl_write_w(P0_DX0GCR3 + ch_offset, 0x00000012);	//for test only
			mctl_write_w(P0_DX1GCR3 + ch_offset, 0x00000012);
			mctl_write_w(P0_DX2GCR3 + ch_offset, 0x00000012);
			mctl_write_w(P0_DX3GCR3 + ch_offset, 0x00000012);
		}
	}

#ifdef SYSTEM_SIMULATION
	mctl_write_w(P0_ZQ0CR + ch_offset, 0x04010d02);
	mctl_write_w(P0_ZQ1CR + ch_offset, 0x04010d02);
	mctl_write_w(P0_ZQ2CR + ch_offset, 0x04010d02);
#else
	mctl_write_w(P0_ZQ0CR + ch_offset, 0x04058D02);	//use default 0x04010d02
	mctl_write_w(P0_ZQ1CR + ch_offset, 0x04058D02);	//use default 0x04010d02
	mctl_write_w(P0_ZQ2CR + ch_offset, 0x04058D02);	//use default 0x04010d02
#endif

	//disable auto refresh before training
	reg_val = mctl_read_w(Mx_RFSHCTL3 + ch_offset);
	reg_val |= (0x1<<0);
	mctl_write_w(Mx_RFSHCTL3 + ch_offset, reg_val);

	//pad_hold_flag = (mctl_read_w(VDD_SYS_PWROFF_GATING) & (0x1<<ch_index));

//		if((para->dram_clk) < 333)	//PLL disable, PLL6 should be 2*dram_clk
//		{
//			reg_val = 0x02fff2;
//		}
//		else
//		{
//			reg_val = 0x00fff2;		//PLL enable, PLL6 should be dram_clk/2
//		}
//
//		if((para->dram_type) == 7)	//LPDDR3
//			reg_val	|= (1<<2);		//CA calibration
//
//		mctl_write_w(P0_PIR + ch_offset, reg_val);	//for DRAM training simulation, turn on all calibration item
//		mctl_write_w(P0_PIR + ch_offset, (reg_val | 0x1) );	//BIT 0 to trigger the initial
//
//		//check the error flag of training
//		while((mctl_read_w(P0_PGSR0 + ch_offset)& 0x1 ) != 0x1);	//wait for training done
//		reg_val = mctl_read_w(P0_PGSR0 + ch_offset);
//		if( (reg_val & 0x1ff00000)!= 0 )	//error find
//		{
//			pattern_goto(reg_val>>20);
//			pattern_end(0);
//			printk("ERROR:DRAM initial training failed!!!!\n");
//			return (0);
//		}

	reg_val = mctl_read_w(P0_DSGCR + ch_offset);
	reg_val |= (0xf<<24);
	mctl_write_w(P0_DSGCR + ch_offset, reg_val);

	reg_val = mctl_read_w(P0_PGCR1 + ch_offset);	//CA VT disable
	reg_val &= ~(0x3<<23);
	reg_val |= (0x1<<7);								//I/O DDR MODE
	reg_val |= (0x1<<26);   //vt close
	mctl_write_w(P0_PGCR1 + ch_offset, reg_val);

	reg_val = mctl_read_w(P0_PLLGCR + ch_offset);		//PLL clock FREQ select
	reg_val |= (0x3<<19);
	mctl_write_w(P0_PLLGCR + ch_offset, reg_val);
		if((para->dram_clk <= 400)|((para->dram_tpr8 & 0x1)==0))//PLL power down and bypass mode-----------------------------------------------------------------
	{
		reg_val = mctl_read_w(P0_PLLGCR + ch_offset);
		reg_val |= ( (0x1U<<29) | (0x1U<<31) ); //PLL Bypass and PLL Power Down
		mctl_write_w(P0_PLLGCR + ch_offset, reg_val);
	}

//	reg_val = mctl_read_w(P0_DXCCR + ch_offset);		//DX IOM
//	reg_val |= (0x1<<1);
//	mctl_write_w(P0_DXCCR + ch_offset, reg_val);

	//Disable VTC
	reg_val = mctl_read_w(P0_PGCR0 + ch_offset);	//
	reg_val &= ~(0x3f<<0);							//
	mctl_write_w(P0_PGCR0 + ch_offset, reg_val);
	dram_dbg("VTC Disabled!!\n",ch_index);


	if(((para->dram_clk) <= 400)|((para->dram_tpr8 & 0x1)==0))	//PLL disable, PLL6 should be 2*dram_clk
	{
		reg_val = 0x020000;//reg_val = 0x02cff3;
	}
	else
	{
		reg_val = 0x00000;		//reg_val = 0x0cff3;		//PLL enable, PLL6 should be dram_clk/2
	}

	if(para->dram_type == 3)	//DDR3
		reg_val |= 0x0df3;
	else	//LPDDR3
		reg_val |= 0x0c573;

	mctl_write_w(P0_PIR + ch_offset, reg_val);	//for fast simulation

	aw_delay(1000);	//make some delay

	while((mctl_read_w(P0_PGSR0 + ch_offset)&0x1) != 0x1);	//for fast simulation

	//****************************************************
	//for 2rank debug
	if(((para->dram_para2)>>12 & 0xf) == 2 )	//2rank used
	{
		//set the dqs gate delay to average between 2rank
		for(i=0;i<4;i++)
		{
			reg_val = mctl_read_w(P0_DX0LCDLR2 + ch_offset + i* 0x80);
			rank0_lcdlr2 = (reg_val & 0xff);
			rank1_lcdlr2 = ( (reg_val>>8) & 0xff );

			reg_val = mctl_read_w(P0_DX0GTR + ch_offset + i* 0x80);
			rank0_gtr = (reg_val & 0x7);
			rank1_gtr = ( (reg_val>>3) & 0x7 );

			reg_val = mctl_read_w(P0_DX0MDLR + ch_offset + i* 0x80);
			tprd = ( (reg_val >> 8) & 0xff );

			if(rank0_gtr != rank1_gtr)
				dram_dbg("Warn:CH%d BYTE%d GTR different\n",ch_index,i);
			if( (rank0_lcdlr2 & 0xf8) != (rank1_lcdlr2 & 0xf8) )
				dram_dbg("Warn:CH%d BYTE%d LCDLR2 different\n",ch_index,i);

			//calculate the average delay
			reg_val = ( rank0_lcdlr2 + rank1_lcdlr2 + ((rank0_gtr + rank1_gtr)*tprd) );
			reg_val /= 2;

			for(j=0;tprd <= reg_val;j++)
			{
				reg_val -= tprd;
			}
			rank0_lcdlr2 = reg_val;
			rank1_lcdlr2 = reg_val;
			rank0_gtr = j;
			rank1_gtr = j;

			mctl_write_w( (P0_DX0LCDLR2 + ch_offset + i* 0x80) , ( rank0_lcdlr2 | (rank1_lcdlr2<<8) ) );

			reg_val = mctl_read_w(P0_DX0GTR + ch_offset + i* 0x80);
			reg_val &= ~(0x3f);
			reg_val |= ( (rank1_gtr<<3) | rank0_gtr );
			mctl_write_w( (P0_DX0GTR + ch_offset + i* 0x80) , reg_val );
		}
	}
//****************************************************************************// 2rank debug end


//****************************************************//debug end


	if((para->dram_type) == 6 || (para->dram_type) == 7)	//LPDDR2 and LPDDR3
	{
		reg_val = mctl_read_w(P0_DSGCR + ch_offset);
		reg_val &= (~(0x3<<6)); 	//set DQSGX to 1
		reg_val |= (0x1<<6);  //dqs gate extend
		mctl_write_w(P0_DSGCR + ch_offset, reg_val);
		dram_dbg("DQS Gate Extend Enable!\n",ch_index);
	}

	//*****************************************************************************
	//Disable ZCAL after initial--for nand dma debug--20140330 by YSZ
	//*****************************************************************************
	if( para->dram_tpr13 & (0x1<<31) )
	{
		reg_val = mctl_read_w(P0_ZQ0CR + ch_offset);
		reg_val |= (0x7<<11);
		mctl_write_w(P0_ZQ0CR + ch_offset, reg_val);
	}
	//debug end


	if( (para->dram_tpr13 & 0x1) | ch_index )	//no space try-DQ and rank have been confirm before channel 1 intial
	{
		if( (mctl_read_w(P0_PGSR0 + ch_offset) & (0x1ff<<20)) != 0)	//initial ERROR
		{
			dram_dbg("Channel %d unavailable!\n",ch_index);
			return 0;
		}
		else	//initial OK
		{
			dram_dbg("Channel %d OK!\n",ch_index);
			//return 1;
		}
	}
	else	//try dram space
	{
		dram_dbg("DRAM Size Scan!\n",ch_index);

		if( (mctl_read_w(P0_DX0GSR0 + ch_offset) & (0x1<<24)) != 0)	//RANK0 DATX0 Gating ERROR
		{
			dram_dbg("Channel %d Rank0 Byte0 Gate Error!\n",ch_index);
			return 0;
		}
		else if( (mctl_read_w(P0_DX0GSR0 + ch_offset) & (0x1<<25)) != 0)	//RANK1 DATX0 Gating ERROR
		{
			dram_dbg("Channel %d Rank1 Byte0 Gate Error!\n",ch_index);
			dram_dbg("Only one rank available!\n");

			paraconfig(&(para->dram_para2), 0xfU<<12, 0x1U<<12 );//change parameter to 1 RANK

			//one rank set
			reg_val = mctl_read_w(M0_MSTR + ch_offset);
			reg_val &= ~(0x3<<24);
			reg_val |= (0x1<<24);
			mctl_write_w(M0_MSTR + ch_offset,reg_val);

			reg_val = mctl_read_w(P0_DTCR + ch_offset);
			reg_val &= ~(0xf<<24);
			reg_val |= (0x1<<24);
			mctl_write_w(P0_DTCR + ch_offset,reg_val);
			//one rank set finished

			//check MSB half DQ
			if( (mctl_read_w(P0_DX2GSR0 + ch_offset) & (0x1<<24)) != 0)	//high half DQ not available
			{
				dram_dbg("Half DQ Used!\n");
				paraconfig(&(para->dram_para2), 0xfU<<0, 0x1U<<0 );//change parameter to half DQ

				//half DQ set
				reg_val = mctl_read_w(M0_MSTR + ch_offset);
				reg_val &= ~(0x3<<12);
				reg_val |= (0x1<<12);
				mctl_write_w(M0_MSTR + ch_offset,reg_val);

				mctl_write_w(P0_DX2GCR0 + ch_offset, 0x0);	//disable high 2 group DQ
				mctl_write_w(P0_DX3GCR0 + ch_offset, 0x0);
				//half DQ set finished
			}
			else																											//full DQ available
			{
				dram_dbg("Full DQ Used!\n");
				//do not need,default is full DQ//paraconfig(&(para->dram_para2), 0xfU<<0, 0x0U<<0) );//change parameter to full DQ
				//do not need,default is full DQ//
				//do not need,default is full DQ////full DQ set
				//do not need,default is full DQ//reg_val = mctl_read_w(M0_MSTR + ch_offset);
				//do not need,default is full DQ//reg_val &= ~(0x3<<12);
				//do not need,default is full DQ//reg_val |= (0x0<<12);
				//do not need,default is full DQ//mctl_write_w(M0_MSTR + ch_offset,reg_val);
				//do not need,default is full DQ//
				//do not need,default is full DQ//mctl_write_w(P0_DX2GCR0 + ch_offset, 0x7C000285);	//default value
				//do not need,default is full DQ//mctl_write_w(P0_DX3GCR0 + ch_offset, 0x7C000285);
				//do not need,default is full DQ////full DQ set finished
			}

			//return 1;
		}
		else		//tow RANK available
		{
			dram_dbg("Channel %d Two RANK available!\n",ch_index);

			//do not need,default is 2 rank//paraconfig(&(para->dram_para2), 0xfU<<15, 0x2U<<15) );//change parameter to 2 RANK
			//do not need,default is 2 rank//
			//do not need,default is 2 rank////two rank set
			//do not need,default is 2 rank//reg_val = mctl_read_w(M0_MSTR + ch_offset);
			//do not need,default is 2 rank//reg_val |= (0x3<<24);
			//do not need,default is 2 rank//mctl_write_w(M0_MSTR + ch_offset,reg_val);
			//do not need,default is 2 rank//
			//do not need,default is 2 rank//reg_val = mctl_read_w(P0_DTCR + ch_offset);
			//do not need,default is 2 rank//reg_val |= (0x3<<24);
			//do not need,default is 2 rank//mctl_write_w(P0_DTCR + ch_offset,reg_val);
			//do not need,default is 2 rank////two rank set finished

			//check MSB half DQ
			if( (mctl_read_w(P0_DX2GSR0 + ch_offset) & (0x1<<24)) != 0)	//high half DQ not available
			{
				dram_dbg("Half DQ Used!\n");
				paraconfig(&(para->dram_para2), 0xfU<<0, 0x1U<<0);//change parameter to half DQ
			}
			else																											//full DQ available
			{
				dram_dbg("Full DQ Used!\n");
				//do not need,default is full DQ//paraconfig(&(para->dram_para2), 0xfU<<0, 0x0U<<0) );//change parameter to full DQ
				//return 1;
			}

			//return 1;
		}

		//after rank,DQ width confirm, trigger another initial process
		reg_val = mctl_read_w(P0_PIR + ch_offset);
		reg_val &= ~(0xffff);

		if(para->dram_type == 3)	//DDR3
			reg_val |= 0x000df3;//reg_val |= 0x00cff3;
		else	//LPDDR3
			reg_val |= 0x00c573;//
		mctl_write_w(P0_PIR + ch_offset, reg_val);	//do not change the configuration, just tigger bit0

		aw_delay(1000);	//make some delay

		//check the error flag of training
		while((mctl_read_w(P0_PGSR0 + ch_offset)& 0x1 ) != 0x1);	//wait for training done
		reg_val = mctl_read_w(P0_PGSR0 + ch_offset);
		if( (mctl_read_w(P0_PGSR0 + ch_offset) & (0x1ff<<20)) != 0)	//initial ERROR
		{
			dram_dbg("Channel %d unavailable!\n",ch_index);
			return 0;
		}
		else	//initial OK
		{
			dram_dbg("Channel %d OK!\n",ch_index);
			//return 1;
		}

	//****************************************************
	//for 2rank debug
	if(((para->dram_para2)>>12 & 0xf) == 2 )	//2rank used
	{
		//set the dqs gate delay to average between 2rank
		for(i=0;i<4;i++)
		{
			reg_val = mctl_read_w(P0_DX0LCDLR2 + ch_offset + i* 0x80);
			rank0_lcdlr2 = (reg_val & 0xff);
			rank1_lcdlr2 = ( (reg_val>>8) & 0xff );

			reg_val = mctl_read_w(P0_DX0GTR + ch_offset + i* 0x80);
			rank0_gtr = (reg_val & 0x7);
			rank1_gtr = ( (reg_val>>3) & 0x7 );

			reg_val = mctl_read_w(P0_DX0MDLR + ch_offset + i* 0x80);
			tprd = ( (reg_val >> 8) & 0xff );

			if(rank0_gtr != rank1_gtr)
				dram_dbg("Warn:CH%d BYTE%d GTR different\n",ch_index,i);
			if( (rank0_lcdlr2 & 0xf8) != (rank1_lcdlr2 & 0xf8) )
				dram_dbg("Warn:CH%d BYTE%d LCDLR2 different\n",ch_index,i);

			//calculate the average delay
			reg_val = ( rank0_lcdlr2 + rank1_lcdlr2 + ((rank0_gtr + rank1_gtr)*tprd) );
			reg_val /= 2;

			for(j=0;tprd <= reg_val;j++)
			{
				reg_val -= tprd;
			}
			rank0_lcdlr2 = reg_val;
			rank1_lcdlr2 = reg_val;
			rank0_gtr = j;
			rank1_gtr = j;

			mctl_write_w( (P0_DX0LCDLR2 + ch_offset + i* 0x80) , ( rank0_lcdlr2 | (rank1_lcdlr2<<8) ) );

			reg_val = mctl_read_w(P0_DX0GTR + ch_offset + i* 0x80);
			reg_val &= ~(0x3f);
			reg_val |= ( (rank1_gtr<<3) | rank0_gtr );
			mctl_write_w( (P0_DX0GTR + ch_offset + i* 0x80) , reg_val );
		}
	}
//****************************************************************************// 2rank debug end

	}	//try dram space-channel,rank,DQ width confirm


	while((mctl_read_w(M0_STATR + ch_offset)&0x1) != 0x1);	//init done

	reg_val = mctl_read_w(P0_PGCR3 + ch_offset);	//DDR is controlled by controller
	reg_val &= ~(0x1<<25);
	mctl_write_w(P0_PGCR3 + ch_offset, reg_val);

	mctl_write_w(M0_PIMISC + ch_offset, 0x00);		//?

	//mctl_write_w(M0_RFSHCTL3 + ch_offset, 0x00);	//enable auto refresh
	//enable auto-refresh
	reg_val = mctl_read_w(Mx_RFSHCTL3 + ch_offset);
	reg_val &= ~(0x1<<0);
	mctl_write_w(Mx_RFSHCTL3 + ch_offset, reg_val);

	return (1);
}







#ifdef A39_FPGA_PLATFORM

int mctl_soft_training(void)
{
	int i, j;
	unsigned int k;
	const unsigned int words[64] = {	0x12345678, 0xaaaaaaaa, 0x55555555, 0x00000000, 0x11223344, 0xffffffff, 0x55aaaa55, 0xaa5555aa,
								0x23456789, 0x18481113, 0x01561212, 0x12156156, 0x32564661, 0x61532544, 0x62658451, 0x15564795,
								0x10234567, 0x54515152, 0x33333333, 0xcccccccc, 0x33cccc33, 0x3c3c3c3c, 0x69696969, 0x15246412,
								0x56324789, 0x55668899, 0x99887744, 0x00000000, 0x33669988, 0x66554477, 0x5555aaaa, 0x54546212,
								0x21465854, 0x66998877, 0xf0f0f0f0, 0x0f0f0f0f, 0x77777777, 0xeeeeeeee, 0x3333cccc, 0x52465621,
								0x24985463, 0x22335599, 0x78945623, 0xff00ff00, 0x00ff00ff, 0x55aa55aa, 0x66996699, 0x66544215,
								0x54484653, 0x66558877, 0x36925814, 0x58694712, 0x11223344, 0xffffffff, 0x96969696, 0x65448861,
								0x48898111, 0x22558833, 0x69584701, 0x56874123, 0x11223344, 0xffffffff, 0x99669966, 0x36544551};

	for(i=0; i<0x10; i++)
	{
		for(j=0; j<0x4; j++)
		{

			mctl_write_w(MCTL_COM_BASE+0xc, ((3-j)<<4)|(0xf-i));
			for(k=0; k<0x10; k++);

			for(k=0; k<(1<<10); k++)
			{
				mctl_write_w(DRAM_MEM_BASE+(k<<2), words[k%64]);

			}

			for(k=0; k<(1<<10); k++)
			{
				if(words[k%64] != mctl_read_w(DRAM_MEM_BASE+(k<<2)))	break;
			}

			if(k==(1<<10))
				return 1;
		}
	}

	return 0;
}

unsigned int mctl_channel_init_fpga(unsigned int ch_index)
{
	unsigned int reg_val = 0;
	unsigned int ch_offset = ch_index*0x1000;

	mctl_write_w(M0_INT0 + ch_offset, 0x10001);
	mctl_write_w(M0_INT1 + ch_offset, 0x10000);
	mctl_write_w(M0_INT2 + ch_offset, 0x00000);
	mctl_write_w(M0_INT3 + ch_offset, 0xa620000);
	mctl_write_w(M0_INT4 + ch_offset, 0x0000);
	mctl_write_w(M0_INT5 + ch_offset, 0x80000);
	mctl_write_w(M0_DRAMTMG4 + ch_offset, 0x05020405);
	mctl_write_w(M0_PITMG0 + ch_offset, 0x02010101);
	mctl_write_w(M0_PIMISC + ch_offset, 0x0);
	mctl_write_w(M0_UPD0 + ch_offset, 0x80000000);

	//Setup Refresh Timing
	reg_val = 0;
	reg_val |= ((MCTL_TRFC*(MCTL_CLK))/1000);
	reg_val |= (((MCTL_TREFI*(MCTL_CLK))/1000)>>5)<<16;
	mctl_write_w(M0_RFSHTMG + ch_offset, reg_val);

	mctl_write_w(M0_MSTR + ch_offset, 0x1020400);
	mctl_write_w(M0_PIMISC + ch_offset, 0x01);      //start Controller init
	mctl_write_w(M0_RFSHCTL3 + ch_offset, 0x01);

	mctl_write_w(P0_DCR + ch_offset, 0x1000040a);
	mctl_write_w(P0_MR0 + ch_offset, MCTL_MR0);
	mctl_write_w(P0_MR1 + ch_offset, MCTL_MR1);
	mctl_write_w(P0_MR2 + ch_offset, MCTL_MR2);
	mctl_write_w(P0_MR3 + ch_offset, MCTL_MR3);
	mctl_write_w(P0_PTR0 + ch_offset, 0x01e007c3);
	mctl_write_w(P0_PTR1 + ch_offset, 0x00170023);
	mctl_write_w(P0_PTR3 + ch_offset, 0x0050001a);
	mctl_write_w(P0_PTR4 + ch_offset, 0x0050003a);
	mctl_write_w(P0_DTCR + ch_offset, 0x01000081);

	mctl_write_w(P0_PIR + ch_offset, 0x10000183);	//CTLDINIT
	while((mctl_read_w(P0_PGSR0 + ch_offset)&0x1) != 0x1);	//wait for DLL Lock

	while((mctl_read_w(M0_STATR + ch_offset)&0x1) != 0x1);	//init done

	mctl_write_w(P0_PGCR3 + ch_offset, 0x00aa0060);
	mctl_write_w(M0_PIMISC + ch_offset, 0x00);
	mctl_write_w(M0_RFSHCTL3 + ch_offset, 0x00);

	return mctl_soft_training();

}

unsigned int mctl_com_init_fpga(void)
{
	unsigned int reg_val = 0;

	reg_val = (1U<<22);
	reg_val |= (MCTL_DDR_TYPE & 0x07)<<16;
	reg_val |= ((MCTL_BANK_SIZE==8)? 0x1:0x0) << 2;
	reg_val |= ((MCTL_ROW_WIDTH - 1) & 0xf) << 4;
	reg_val |= ((MCTL_BUS_WIDTH==32)? 0x3:0x1) << 12;
	switch(MCTL_PAGE_SIZE)
	{
	case 8:
		reg_val |= 0xA << 8;
		break;
	case 4:
		reg_val |= 0x9 << 8;
		break;
	case 2:
		reg_val |= 0x8 << 8;
		break;
	case 1:
		reg_val |= 0x7 << 8;
		break;
	default:
		reg_val |= 0x6 <<8;
	}
	if(MCTL_CHANNEL_NUM==2)
		reg_val |= 0x58<<16;		//Address Remap for Dual Channel
	else
		reg_val &= ~(0x18<<16);

	mctl_write_w(MC_CR, reg_val);  // for Single Channel //0x5a39c4 for Dual Channel

	mctl_write_w(MCTL_COM_BASE+0xc, 0x0505);

	return (1);
}

unsigned int mctl_init_fpga(void)
{
	//mctl_sys_init();	//do not need
	mctl_com_init_fpga();
	mctl_channel_init_fpga(0);
	return (1);
}
#endif




//*****************************************************************************
//	signed int init_DRAM(int type, __dram_para_t *para)
//  Description:	DRAMC system initial
//
//	Arguments:
//
//	Return Value:	0:DRAM initial fail  others: DRAM size in MBytes
//*****************************************************************************
signed int init_DRAM(int type, __dram_para_t *para)
{
	signed int ret_val;
	unsigned int reg_val;
	unsigned int i=0,j=0,ret=0,tmp=0;

	dram_dbg("INFO: dram code V2.2\n");

	dram_dbg("dram clk=%d\n", para->dram_clk);
	dram_dbg("dram_zq = 0x%x!!\n", para->dram_zq);
	if(para->dram_type == 7)
		dram_dbg("LPDDR3 used!!\n");
	else if(para->dram_type == 3)
		dram_dbg("DDR3 used!!\n");
	else if(para->dram_type == 6)
		dram_dbg("LPDDR2 used!!\n");

	if(para->dram_para2 & 0x200)
	{
		dram_dbg("dram channle 2\n");
	}
	else
	{
		dram_dbg("dram channle 1\n");
	}

    set_cpu_voltage(para->dram_type);

    aw_delay(100000);	//make some delay

#ifdef A39_FPGA_PLATFORM
	mctl_init_fpga();
	ret_val = DRAMC_get_dram_size();
	return ret_val;
#else
	//disable A80 internal 240Ohm Register
	reg_val = mctl_read_w(R_PRCM_BASE + 0x1e0);	//select the configuration of 0x1e8
	reg_val |= (0x3<<8);
	mctl_write_w((R_PRCM_BASE + 0x1e0),reg_val);

	mctl_write_w((R_PRCM_BASE + 0x1e8),0);	//0x1e8 use disconnect

	aw_delay(1000);	//make some delay


	auto_set_timing_para(para);	//calculate DRAM timing base on DRAM_FREQ

	mctl_sys_init(para);				//system resource configuration

	reg_val = mctl_channel_init(0,para);	//channel 0 initial, tpr13[0] decide whether space scan.

	if(reg_val == 0)											//channel 0 unavailable
		return 0;

	if(para->dram_tpr13 & 0x1)						//do not scan space
	{
		if( (((para->dram_para2)>>8)&0xf)==1 )	//user define only one channel use
		{
			//disable channel 1
			reg_val = mctl_read_w(MC_CR);
			reg_val |= (0x1<<22);					//The value must be logic 1 when only one channel used
			reg_val &= ~(0x1<<19);				//dual channel disable
			mctl_write_w(MC_CR,reg_val);

			//disable channel 1 global clock
			reg_val = mctl_read_w(MC_CCR);
			reg_val &= ~(0x1U<<31);
			mctl_write_w(MC_CCR, reg_val);

		}
		else																		//user define two channel use
		{
			reg_val = mctl_channel_init(1,para);
			if(reg_val == 0)											//channel 1 unavailable
				return 0;
		}
	}
	else																	//check channel 1 available
	{
		reg_val = mctl_channel_init(1,para);
		if(reg_val==0)											//channel 1 unavailable
		{
			paraconfig(&(para->dram_para2), 0xfU<<8, 0x1U<<8);//change parameter to 1 channel

			//disable channel 1
			reg_val = mctl_read_w(MC_CR);
			reg_val |= (0x1<<22);					//The value must be logic 1 when only one channel used
			reg_val &= ~(0x1<<19);				//dual channel disable
			mctl_write_w(MC_CR,reg_val);

			//disable channel 1 global clock
			reg_val = mctl_read_w(MC_CCR);
			reg_val &= ~(0x1U<<31);
			mctl_write_w(MC_CCR, reg_val);
		}

		//*******************************
		//row and column width scan
		//*******************************
		//set DRAMC to one channel
		reg_val = mctl_read_w(MC_CR);
		reg_val |= (0x1<<22);					//The value must be logic 1 when only one channel used
		reg_val &= ~(0x1<<19);				//dual channel disable
		mctl_write_w(MC_CR,reg_val);

		aw_delay(10000);	//make some delay

		//preset data into DRAM(64Bytes)
		for(i=0; i<64; i++)
			mctl_write_w(0x20000000 + 4*i, (i%2)?(0x20000000 + 4*i):(~(0x20000000 + 4*i)));

		//fix column(pagesize) to scan row width
		reg_val = mctl_read_w(MC_CR);
		reg_val &= ~(0xf<<8);				//
		reg_val |= (0x6<<8);				//512B pagesize
		reg_val |= (0xf<<4);				//row to max
		mctl_write_w(MC_CR,reg_val);

		aw_delay(200);	//make some delay after remapping

		//row detect
		for(i=11;i<=16;i++)
		{
			tmp = 0x20000000 + (1<<(i+3+9));//row-bank-column
			ret = 0;
			for(j=0;j<64;j++)
			{
				if(mctl_read_w(0x20000000 + j*4) == mctl_read_w(tmp + j*4))
				{
					ret++;
				}
			}
			if(ret == 64)
			{
				break;
			}
		}
		if(i >= 16)
			i = 16;

		paraconfig(&(para->dram_para1), 0xffU<<20, i<<20);//row width confirmed

		//fix row width, scan column(pagesize)
		reg_val = mctl_read_w(MC_CR);
		reg_val &= ~(0xf<<4);				//
		reg_val |= (0xa<<4);				//set row to 11bit
		reg_val &= ~(0xf<<8);				//
		reg_val |= (0xa<<8);				//max pagesize
		mctl_write_w(MC_CR,reg_val);

		aw_delay(200);	//make some delay after remapping

		//column(pagesize) detect
		for(i=9;i<=13;i++)
		{
			tmp = 0x20000000 + (0x1U<<i);//column
			ret = 0;
			for(j=0;j<64;j++)
			{
				if(mctl_read_w(0x20000000 + j*4) == mctl_read_w(tmp + j*4))
				{
					ret++;
				}
			}
			if(ret == 64)
			{
				break;
			}
		}
		if(i >= 13)
			i = 13;

		if(i==9)
			i = 0;
		else
			i = (0x1U<<(i-10));

		paraconfig(&(para->dram_para1), 0xfU<<16, i<<16);//pagesize confirmed

		//******************************
		//row and column confirmed
		//******************************
	}

	mctl_com_init(para);

	aw_delay(200);	//make some delay after remapping

	ret_val = DRAMC_get_dram_size();

	paraconfig(&(para->dram_para1), 0xffffU<<0, ret_val);//dram space update

	paraconfig(&(para->dram_tpr13), 0x3U<<0, (0x3U<<0) );	//after space try, confirm the size and timing

	dram_dbg("dram size=%dMBytes!\n", ret_val);

	//return DRAMC_get_dram_size();
	return ret_val;

#endif
}







