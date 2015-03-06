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

#include "mctl_reg.h"
#include "mctl_hal.h"

#ifdef LINUX_CONFIG
	 #include <mach/sys_config.h>
#endif

static int resetflag = 0;

__dram_para_t *dram_para;

static unsigned int mctl_sys_init(__dram_para_t *dram_para);
static unsigned int mctl_reset_release(void);
static unsigned int mctl_dll_init(unsigned int ch_index, __dram_para_t *para);
static unsigned int mctl_channel_init(unsigned int ch_index, __dram_para_t *para);
static unsigned int mctl_com_init(__dram_para_t *para);
static unsigned int mctl_port_cfg(void);
static int ss_bonding_id(void);
static int pmu_bonding_id(void);
static void paraconfig(unsigned int *para, unsigned int mask, unsigned int value);
//=====================p2wi adjust pmu dc5 voltage===========================
#define P2WI_CTRL	(0x01f03400 + 0x00)
#define P2WI_CCR	(0x01f03400 + 0x04)
#define P2WI_INTE	(0x01f03400 + 0x08)
#define P2WI_STAT	(0x01f03400 + 0x0c)
#define P2WI_DADDR0	(0x01f03400 + 0x10)
#define P2WI_DADDR1	(0x01f03400 + 0x14)
#define P2WI_DLEN	(0x01f03400 + 0x18)
#define P2WI_DATA0	(0x01f03400 + 0x1c)
#define P2WI_DATA1	(0x01f03400 + 0x20)
#define P2WI_LCR	(0x01f03400 + 0x24)
#define P2WI_PMCR	(0x01f03400 + 0x28)
int p2wi_init(void)
{
	unsigned rval;
	int time = 0xfffff;

	/* enable gpio clock and reset*/
	rval = mctl_read_w(0x01f01400 + 0x28);
	rval |= (1 << 3) | (1 << 0);
	mctl_write_w(0x01f01400 + 0x28, rval);
	rval = mctl_read_w(0x01f01400 + 0xb0);
	rval |= (1 << 3) | (1 << 0);
	mctl_write_w(0x01f01400 + 0xb0, rval);

	/* config io */
	rval = mctl_read_w(0x1f02c00 + 0x00);
	rval &= ~0x77;
	rval |= 0x33;
	mctl_write_w(0x1f02c00 + 0x00, rval);

	/* config clock to 1.5MHz */
	//mctl_write_w(P2WI_CCR, 0x103);
	/* 一开始pmu是用twi模式通信，因此需要把p2wi降频后，发送转换序列号，把pmu 从twi模式转换成p2wi模式，最后才把频率提上去，不然有几率出现转换失败的情况。*/
	/* config clock to 400kHz */
	mctl_write_w(P2WI_CCR, 0x10e);

	/* switch pmu to p2wi mode */
	mctl_write_w(P2WI_PMCR, 0x803e3e68);
	while((mctl_read_w(P2WI_PMCR) & 0x80000000) && time--);
	if (mctl_read_w(P2WI_PMCR) & 0x80000000) {
	#ifdef PW2I_PRINK
			dram_dbg("Switching PMU to P2WI mode failed\n");
	#endif
		return -1;
	}

	/* config clock to 1.5MHz */
	mctl_write_w(P2WI_CCR, 0x103);
	return 0;
}

int p2wi_read(unsigned int addr, unsigned int *val)
{
	unsigned time = 0xfffff;
	unsigned istat = 0;

	mctl_write_w(P2WI_DADDR0, addr);
	mctl_write_w(P2WI_DLEN, 0x10);
	mctl_write_w(P2WI_CTRL, 0x80);
	do {
		istat = mctl_read_w(P2WI_STAT);
	} while (!(istat & 3) && time--);

	mctl_write_w(P2WI_STAT, istat);
	if (istat & 2)
		return -1;

	*val = mctl_read_w(P2WI_DATA0) & 0xff;
	return 0;
}

int p2wi_write(unsigned int addr, unsigned int val)
{
	unsigned time = 0xfffff;
	unsigned istat = 0;

	mctl_write_w(P2WI_DADDR0, addr);
	mctl_write_w(P2WI_DATA0, val);
	mctl_write_w(P2WI_DLEN, 0);
	mctl_write_w(P2WI_CTRL, 0x80);
	do {
		istat = mctl_read_w(P2WI_STAT);
	} while (!(istat & 3) && time--);
	mctl_write_w(P2WI_STAT, istat);
	if (istat & 2)
		return -1;

	return 0;
}
#define AW1636_DCDC2_VOL_CTRL	(0x22)
#define AW1636_DCDC3_VOL_CTRL	(0x23)
#define AW1636_DCDC4_VOL_CTRL	(0x24)
#define AW1636_IC_ID_REG		(0x3)
#define AW1636_DCDC5_VOL_CTRL	(0x25)
int set_ddr_voltage(__dram_para_t *dram_para)
{
	int ret = -1;
	unsigned int vol_set = 0;
	unsigned voltage_mV = 1200;

	//adjust dc5
	//DDR2 || DDR3 || DDR3L
	if((dram_para->dram_type == 2) || (dram_para->dram_type == 3) || (dram_para->dram_type == 31))
	{
		voltage_mV = 1350;
		vol_set = (voltage_mV - 1000) / 50;
		ret = p2wi_write(AW1636_DCDC5_VOL_CTRL, vol_set);
		if (ret) {
#ifdef PW2I_PRINK
			dram_dbg("set ddr voltage 1350mV failed\n");
#endif
			return -1;
		}
		__msdelay(1);
	}

	//DDR2 || DDR3
	if((dram_para->dram_type == 2)  || (dram_para->dram_type == 3))
	{
		voltage_mV = 1500;
		vol_set = (voltage_mV - 1000) / 50;
		ret = p2wi_write(AW1636_DCDC5_VOL_CTRL, vol_set);
		if (ret) {
#ifdef PW2I_PRINK
			dram_dbg("set ddr voltage 1500mV failed\n");
#endif
			return -1;
		}
		__msdelay(1);
	}

	//DDR2
	if(dram_para->dram_type == 2)
	{
		voltage_mV = 1800;
		vol_set = (voltage_mV - 1000) / 50;
		ret = p2wi_write(AW1636_DCDC5_VOL_CTRL, vol_set);
		if (ret) {
#ifdef PW2I_PRINK
			dram_dbg("set ddr voltage 1800mV failed\n");
#endif
			return -1;
		}
		__msdelay(1);
	}

#ifdef PW2I_PRINK
	dram_dbg("ddr vol %dmV\n", voltage_mV);
#endif

	return 0;
}

int set_cpu_voltage(void)
{
	int ret = -1;
	unsigned int cpu_reg = 0;
	unsigned int pmu_id = 0;
//	ret = p2wi_init();
//	if (ret)
//		return -1;
	ret = p2wi_read(AW1636_IC_ID_REG, &pmu_id);
	if (ret) {
#ifdef PW2I_PRINK
		dram_dbg("Get pmu ID failed\n");
#endif
		return -1;
	}
	if (pmu_id == 6 || pmu_id == 7 || pmu_id == 0x17)
	{
#ifdef PW2I_PRINK
		dram_dbg("Found PMU:AXP221\n");
#endif
	}else {
#ifdef PW2I_PRINK
		dram_dbg("Unknown PMU type, id %d\n", pmu_id);
#endif
		return -1;
	}
	ret = p2wi_read(AW1636_DCDC3_VOL_CTRL, &cpu_reg);

	//	ret = p2wi_write(AW1636_DCDC2_VOL_CTRL, (1260-600)/20);
	//	ret = p2wi_write(AW1636_DCDC3_VOL_CTRL, (1260-600)/20);
	ret = p2wi_write(AW1636_DCDC4_VOL_CTRL, (1200-600)/20);
	//	ret = p2wi_write(AW1636_DCDC5_VOL_CTRL, (1600-600)/20);

	if (ret)
	{
		return -1;
	}


	return 0;
}


//===========================================================================

//--------------------------------------------external function definition-------------------------------------------
//*****************************************************************************
//	unsigned int mctl_init()
//  Description:	DRAM Controller Initialize Procession
//
//	Arguments:		None
//
//	Return Value:	0: Fail    others: dram size
//*****************************************************************************
unsigned int DRAMC_init(__dram_para_t *para)
{
	unsigned int dram_size = 0;
	unsigned int reg_val;
	unsigned int size_lock = 0;
	unsigned int size_max = 2048;
	unsigned int i, j;
	unsigned int tmp;
	unsigned int ret = 0;
	unsigned int bus_num = 0;
	unsigned int bank_num = 0;
	unsigned int column_num = 0;
	unsigned int row_num = 0;
	unsigned int rank_num = 0;
	unsigned int ch_num = 0;
	unsigned int page_size = 0;
    unsigned int time;

  	if(!para)
    	return 0;

    //NAND_Print("****************************************************************\n");
	//NAND_Print("NAND_ClkRequest, Open MBUS CLK0 for DRAM!!!!!!!!!!!!!!!!!!!!! \n");
	//NAND_Print("NAND_ClkRequest, Open MBUS CLK1 for DRAM!!!!!!!!!!!!!!!!!!!!! \n");
	*(volatile unsigned int *)(0x01c20000 + 0x15c) = 0x82000001U;
	*(volatile unsigned int *)(0x01c20000 + 0x160) = 0x81000000U;
	//NAND_Print("Reg 0x01c2015c: 0x%x\n", *(volatile __u32 *)(0x01c2015c));
	//NAND_Print("Reg 0x01c20160: 0x%x\n", *(volatile __u32 *)(0x01c20160));
	//NAND_Print("****************************************************************\n");


	if((para->dram_tpr13 & (0x1<<9)) == (0<<9))//size lock
		size_lock = 1;

	if(((para->dram_tpr13 & (0x3<<3)) == (0x1<<3))) //A31s
	{
		if((para->dram_tpr13 & (0x1<<9)) == (0x0<<9))
			size_max = 1024;
	}

	if(!resetflag)
	{
		mctl_reset_release();
	  	mctl_sys_init(para);
	} else
	{
	    //turn off SCLK
	    reg_val = mctl_read_w(SDR_COM_CCR);
	    reg_val &= ~(0x7<<0);
	    mctl_write_w(SDR_COM_CCR, reg_val);

	    //turn off SDRPLL
	    reg_val = mctl_read_w(SDR_COM_CCR);
	    reg_val |= (0x3<<3);
	    mctl_write_w(SDR_COM_CCR, reg_val);

	    //gate off DRAMC AHB clk
	    reg_val = mctl_read_w(CCM_AHB1_GATE0_CTRL);
	    reg_val &=~(0x1<<14);
	    mctl_write_w(CCM_AHB1_GATE0_CTRL, reg_val);

	    //turn off PLL5
	    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
	    reg_val &= ~(0x1U<<31);
	    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

	    //PLL5 configuration update(validate PLL5)
	    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
	    reg_val |= 0x1U<<20;
	    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

	    time = 0xffffff;
	    while((mctl_read_w(CCM_PLL5_DDR_CTRL) & (0x1U<<20)) && (time--)){};

	    //config PLL5 DRAM CLOCK: PLL5 = (24*N*K)/M
	    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
	    reg_val &= ~((0x3<<0) | (0x3<<4) | (0x1F<<8));
	    reg_val |= ((0x0<<0) | (0x1<<4));	//K = M = 2;
	    reg_val |= (((para->dram_clk)/24-1)<<0x8);//N
	    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

	    reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
	    reg_val &= ~(0xf<<8);
	    reg_val |= 0x1<<8;
	    mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);

	    reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
	    reg_val |= 0x1U<<16;
	    mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);

	    time = 0xffffff;
	    while((mctl_read_w(CCM_DRAMCLK_CFG_CTRL) & (0x1<<16)) && (time--)){};

	    //PLL5 enable
	    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
	    reg_val |= 0x1U<<31;
	    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

	    //PLL5 configuration update(validate PLL5)
	    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
	    reg_val |= 0x1U<<20;
	    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

	    time = 0xffffff;
	    while((mctl_read_w(CCM_PLL5_DDR_CTRL) & (0x1U<<20)) && (time--)){};

	    while((!(mctl_read_w(CCM_PLL5_DDR_CTRL) & (0x1U<<28))) && (time--)){};

	    //turn on DRAMC AHB clk
	    reg_val = mctl_read_w(CCM_AHB1_GATE0_CTRL);
	    reg_val |= (0x1<<14);
	    mctl_write_w(CCM_AHB1_GATE0_CTRL, reg_val);
	    //8x8; dram = 20.5mA; sys = 83.1mA
	    //turn on SDRPLL
	    reg_val = mctl_read_w(SDR_COM_CCR);
	    reg_val &= ~(0x3<<3);
	    mctl_write_w(SDR_COM_CCR, reg_val);

	    __msdelay(10);
	}
	if(para->dram_clk >= 200)
	{
		mctl_dll_init(0, para);

		if((para->dram_tpr13 & (0x1u<<31)) == (0x1u<<31))
		{
			if(((para->dram_para2>>8)&0xF) == 0x2)
			{
				mctl_dll_init(1, para);
			}
		}
		else
		{
			if((((para->dram_tpr13>>3)&0x3) != 1))//not A31s
			{
				if(!(((para->dram_tpr13 & (0x1u<<31)) == (0x1u<<31)) && (((para->dram_para2>>8)&0xF) == 0x1)))
					mctl_dll_init(1, para);
			}
		}
	}else
	{
		reg_val = mctl_read_w(SDR_DLLGCR);
		reg_val |= (0x1<<23) | (0x1<<15);
		reg_val &= ~(0x1<<19);
		mctl_write_w(SDR_DLLGCR, reg_val);

		reg_val = mctl_read_w(SDR_ACDLLCR);
		reg_val |= (0x1<<23) | (0x1<<15);
		reg_val &= ~(0x1<<19);
		mctl_write_w(SDR_ACDLLCR, reg_val);

		if((para->dram_tpr13 & (0x1u<<31)) == (0x1u<<31))
		{
			if(((para->dram_para2>>8)&0xF) == 0x2)
			{
				reg_val = mctl_read_w(0x1000 + SDR_DLLGCR);
				reg_val |= (0x1<<23) | (0x1<<15);
				reg_val &= ~(0x1<<19);
				mctl_write_w(0x1000 + SDR_DLLGCR, reg_val);

				reg_val = mctl_read_w(0x1000 + SDR_ACDLLCR);
				reg_val |= (0x1<<23) | (0x1<<15);
				reg_val &= ~(0x1<<19);
				mctl_write_w(0x1000 + SDR_ACDLLCR, reg_val);
			}
		}
		else
		{
			if((((para->dram_tpr13>>3)&0x3) != 1))//not A31s
			{
				if(!(((para->dram_tpr13 & (0x1u<<31)) == (0x1u<<31)) && (((para->dram_para2>>8)&0xF) == 0x1)))
				{
					reg_val = mctl_read_w(0x1000 + SDR_DLLGCR);
					reg_val |= (0x1<<23) | (0x1<<15);
					reg_val &= ~(0x1<<19);
					mctl_write_w(0x1000 + SDR_DLLGCR, reg_val);

					reg_val = mctl_read_w(0x1000 + SDR_ACDLLCR);
					reg_val |= (0x1<<23) | (0x1<<15);
					reg_val &= ~(0x1<<19);
					mctl_write_w(0x1000 + SDR_ACDLLCR, reg_val);
				}
			}
		}
	}

//	mctl_reset_release();

	//set COM sclk enable register
	reg_val = mctl_read_w(SDR_COM_CCR);
	reg_val |= 0x7;
	mctl_write_w(SDR_COM_CCR, reg_val);

	__msdelay(10);

	if((mctl_read_w(R_VDD_SYS_PWROFF_GATE) & 0x3) == 0)
	{
		if(set_ddr_voltage(para))
			return 0;
	}

	mctl_channel_init(0, para);

	if((para->dram_tpr13 & (0x1u<<31)) == (0x1u<<31))
	{
		if(((para->dram_para2>>8)&0xF) == 0x2)
		{
			mctl_channel_init(1, para);
		}
	}
	else
	{
		if((((para->dram_tpr13>>3)&0x3) != 1))//not A31s
		{
			if(!(((para->dram_tpr13 & (0x1u<<31)) == (0x1u<<31)) && (((para->dram_para2>>8)&0xF) == 0x1)))
				mctl_channel_init(1, para);
		}
	}

	mctl_com_init(para);

	mctl_port_cfg();

	//data training error
   	if(mctl_read_w(SDR_PGSR) & (0x3<<5))
			return 0;
   	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
   	{
   		if(mctl_read_w(SDR_PGSR + 0x1000) & (0x3<<5))
   			return 0;
   	}


	if((para->dram_tpr13 & (0x1u<<31)) == (0x0<<31))
	{
		//turn to 1 ch/1 rank/sequence mode
		ch_num = ((para->dram_para2>>8)&0xf)-1;
		rank_num = ((para->dram_para2>>12)&0xf)-1;
		reg_val = mctl_read_w(SDR_COM_CR);
		reg_val &= ~((0x1<<19) | (0xF<<8) | (0xF<<4) | (0x3<<0) | (0x3<<2));
		reg_val |= (0x1<<15) | (0xA<<8) | (0xF<<4) | (0x1<<2);
		mctl_write_w(SDR_COM_CR, reg_val);

		//column detect
		for(i=7;i<20;i++)
		{
			tmp = 0x40000000 + (1<<i);
			ret = 0;
			for(j=0;j<64;j++)
			{
				if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(tmp + j*4))
				{
					ret++;
				}
			}
			if(ret == 64)
			{
				break;
			}
		}

		bus_num = 1<<((para->dram_para2>>0)&0xF);
		column_num = i - bus_num;
		page_size = (1<<column_num)*(bus_num<<1);
		tmp = 0;
		paraconfig(&(para->dram_para1), 0xF<<16, (page_size>>10)<<16); //page size update
		while(page_size>1)
		{
			page_size >>= 1;
			tmp++;
		}
		tmp -= 3;
		reg_val = mctl_read_w(SDR_COM_CR);
		reg_val &= ~(0xF<<8);
		reg_val |= (tmp<<8);
		mctl_write_w(SDR_COM_CR, reg_val);


		//row detect
		for(i=11;i<=16;i++)
		{
			tmp = 0x40000000 + (1<<(i+column_num+bus_num));
			ret = 0;
			for(j=0;j<64;j++)
			{
				if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(tmp + j*4))
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
		row_num = i;
		reg_val = mctl_read_w(SDR_COM_CR);
		reg_val &= ~(0xF<<4);
		reg_val |= ((row_num-1)<<4);
		mctl_write_w(SDR_COM_CR, reg_val);
		paraconfig(&(para->dram_para1), 0xFF<<20, row_num<<20); //row num update

		//bank detect
		ret = 0;
		tmp = 0x40000000 + (1<<(row_num+column_num+bus_num+2));
		for(j=0;j<64;j++)
		{
			if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(tmp + j*4))
			{
				ret++;
			}
		}
		if(ret == 64)
		{
			bank_num = 2;
			paraconfig(&(para->dram_para1), 0xFu<<28, 0u<<28); //bank num = 4
		}else
		{
			bank_num = 3;
			paraconfig(&(para->dram_para1), 0xFu<<28, 1u<<28); //bank num = 8
		}

		reg_val = mctl_read_w(SDR_COM_CR);
		reg_val &= ~(0x3<<2);
		reg_val |= ((bank_num+1)&0x4);
		reg_val |= ch_num<<19;
		reg_val |= rank_num;
		reg_val &= ~(0x1<<15);
		mctl_write_w(SDR_COM_CR, reg_val);

		dram_size = (1<<(rank_num + row_num + bank_num + column_num + ch_num + bus_num - 20));
		paraconfig(&(para->dram_para1), 0xFFFF<<0, dram_size<<0);

		paraconfig(&(para->dram_tpr13), 0x1u<<31, 0x1u<<31);

		if(((((para->dram_tpr13>>3)&0x3) == 0) || (((para->dram_tpr13>>3)&0x3) == 3) || (((para->dram_tpr13>>3)&0x3) == 2)) && (size_lock == 1))//A31 & V10
		{
			if(dram_size != 2048)
			{
				mctl_write_w(SDR_COM_CR, 0);
				return 0;
			}
		}
		else if((((para->dram_tpr13>>3)&0x3) == 1))//A31s
		{
			if(dram_size > size_max)
			{
				dram_size = 1024;
				paraconfig(&(para->dram_para1), 0xFFFF<<0, dram_size<<0);
				tmp = mctl_read_w(SDR_COM_CR);
				reg_val &= ~(0xf<<4);
				reg_val |= (((tmp>>4)&0xF)-1)<<0x4;
				mctl_write_w(SDR_COM_CR, reg_val);
				paraconfig(&(para->dram_para1), 0xFF<<20, ((tmp>>4)&0xF)<<20); //row num = 16
				return (dram_size);
			}
		}else
		{
			return (dram_size);
		}
	}

	return (para->dram_para1&0xFFFF);
}

unsigned int mctl_sys_init(__dram_para_t *dram_para)
{
    unsigned int reg_val;
    unsigned int time = 0xffffff;

    //release DRAMC register reset
    reg_val = mctl_read_w(CCM_AHB1_RST_REG0);
    reg_val &= ~(0x1<<14);
    mctl_write_w(CCM_AHB1_RST_REG0, reg_val);

    //DRAMC AHB clock off
    reg_val = mctl_read_w(CCM_AHB1_GATE0_CTRL);
    reg_val &= ~(0x1<<14);
    mctl_write_w(CCM_AHB1_GATE0_CTRL, reg_val);

//    //PLL5 disable
//    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
//    reg_val &= ~(0x1U<<31);
//    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);
//
//    //PLL5 configuration update(validate PLL5)
//    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
//    reg_val |= 0x1U<<20;
//    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);
	if(dram_para->dram_tpr13 & (0x1<<21))
	{
		mctl_write_w(CCM_BASE + 0x290, 0xD1303333);  //0.4
	}

	if(dram_para->dram_tpr13 & (0x1<<22))
	{
		mctl_write_w(CCM_BASE + 0x290, 0xC8909999);	  //0.2
	}

    //config PLL5 DRAM CLOCK: PLL5 = (24*N*K)/M
    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
    reg_val &= ~((0x3<<0) | (0x3<<4) | (0x1F<<8));
    reg_val |= ((0x0<<0) | (0x1<<4));	//K = 2  M = 1;
    reg_val |= (((dram_para->dram_clk)/24-1)<<0x8);//N
	if( (dram_para->dram_tpr13 & (0x1<<21)) || (dram_para->dram_tpr13 & (0x1<<22)) )
		reg_val |= (0x1<<24);
    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

    //PLL5 enable
    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
    reg_val |= 0x1U<<31;
    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

    //PLL5 configuration update(validate PLL5)
    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
    reg_val |= 0x1U<<20;
    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

  	__msdelay(10);

    reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
  	reg_val |= (0x1U<<31);
  	mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);

  	reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
  	reg_val |= 0x1U<<16;
  	mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);

  	time = 0xffffff;
  	while((mctl_read_w(CCM_DRAMCLK_CFG_CTRL) & (0x1<<16)) && (time--)){};

	//mdfs clk = PLL6 600M / 3 = 200M
  	reg_val = mctl_read_w(CCM_MDFS_CLK_CTRL);
  	reg_val &= ~((0x3<<24) | (0x3<<16) | (0xf<<0));
  	reg_val |= (0x1u<<31) | (0x1<<24) | (0x0<<16) | (0x2<<0);
  	mctl_write_w(CCM_MDFS_CLK_CTRL, reg_val);

  	//release DRAMC register reset
  	reg_val = mctl_read_w(CCM_AHB1_RST_REG0);
  	reg_val |= 0x1<<14;
  	mctl_write_w(CCM_AHB1_RST_REG0, reg_val);

  	//DRAMC AHB clock on
  	reg_val = mctl_read_w(CCM_AHB1_GATE0_CTRL);
  	reg_val |= 0x1<<14;
  	mctl_write_w(CCM_AHB1_GATE0_CTRL, reg_val);

  	return (1);
}

unsigned int mctl_reset_release(void)
{
	unsigned int reg_val;
	unsigned int time = 0xffffff;
	//PLL5 disable
    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
    reg_val &= ~(0x1U<<31);
    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

    //PLL5 configuration update(validate PLL5)
    reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
    reg_val |= 0x1U<<20;
    mctl_write_w(CCM_PLL5_DDR_CTRL, reg_val);

  	reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
  	reg_val &= ~(0x1U<<31);
  	mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);

  	reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
  	reg_val |= 0x1U<<16;
  	mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);

  	while((mctl_read_w(CCM_DRAMCLK_CFG_CTRL) & (0x1<<16)) && (time--)){};

  	reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
  	reg_val &= ~(0xf<<8);
  	reg_val |= 0x1<<8;
  	mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);

  	reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
  	reg_val |= 0x1U<<16;
  	mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);

  	time = 0xffffff;
  	while((mctl_read_w(CCM_DRAMCLK_CFG_CTRL) & (0x1<<16)) && (time--)){};


  	return (1);
}

unsigned int mctl_dll_init(unsigned int ch_index, __dram_para_t *para)
{
	unsigned int ch_id;
	unsigned int reg_val;

	if(ch_index == 1)
		ch_id = 0x1000;
	else
		ch_id = 0x0;

   //***********************************************
   // set dram PHY register
   //***********************************************
	//reset dll
	mctl_write_w(ch_id + SDR_ACDLLCR,0x80000000);
	mctl_write_w(ch_id + SDR_DX0DLLCR,0x80000000);
	mctl_write_w(ch_id + SDR_DX1DLLCR,0x80000000);
	if(((para->dram_para2>>0)&0xF) == 1)
	{
		mctl_write_w(ch_id + SDR_DX2DLLCR,0x80000000);
		mctl_write_w(ch_id + SDR_DX3DLLCR,0x80000000);
	}

	__msdelay(1);

	//enable dll
	mctl_write_w(ch_id + SDR_ACDLLCR,0x0);
	mctl_write_w(ch_id + SDR_DX0DLLCR,0x0);
	mctl_write_w(ch_id + SDR_DX1DLLCR,0x0);
	if(((para->dram_para2>>0)&0xF) == 1)
	{
		mctl_write_w(ch_id + SDR_DX2DLLCR,0x0);
		mctl_write_w(ch_id + SDR_DX3DLLCR,0x0);
	}

	__msdelay(1);

	//release reset dll
	mctl_write_w(ch_id + SDR_ACDLLCR,0x40000000);
	mctl_write_w(ch_id + SDR_DX0DLLCR,0x40000000);
	mctl_write_w(ch_id + SDR_DX1DLLCR,0x40000000);
	if(((para->dram_para2>>0)&0xF) == 1)
	{
		mctl_write_w(ch_id + SDR_DX2DLLCR,0x40000000);
		mctl_write_w(ch_id + SDR_DX3DLLCR,0x40000000);
	}

	__msdelay(1);

	reg_val = mctl_read_w(ch_id + SDR_DX0DLLCR);
	reg_val &= ~(0xF<<14);
	reg_val |= ((para->dram_tpr13>>16)&0xF)<<14;
	mctl_write_w(ch_id + SDR_DX0DLLCR, reg_val);

	reg_val = mctl_read_w(ch_id + SDR_DX1DLLCR);
	reg_val &= ~(0xF<<14);
	reg_val |= ((para->dram_tpr13>>16)&0xF)<<14;
	mctl_write_w(ch_id + SDR_DX1DLLCR, reg_val);

	reg_val = mctl_read_w(ch_id + SDR_DX2DLLCR);
	reg_val &= ~(0xF<<14);
	reg_val |= ((para->dram_tpr13>>16)&0xF)<<14;
	mctl_write_w(ch_id + SDR_DX2DLLCR, reg_val);

	reg_val = mctl_read_w(ch_id + SDR_DX3DLLCR);
	reg_val &= ~(0xF<<14);
	reg_val |= ((para->dram_tpr13>>16)&0xF)<<14;
	mctl_write_w(ch_id + SDR_DX3DLLCR, reg_val);

	return (1);
}

unsigned int mctl_channel_init(unsigned int ch_index, __dram_para_t *para)
{
	unsigned int reg_val;
	unsigned int clkmhz;
	unsigned int ch_id;
	unsigned int hold_flag;
	unsigned int ch_lock = 0;
	unsigned int bus_lock = 0;

	unsigned int time = 0xffffff;

	unsigned int trefi = 0;
	unsigned int tmrd = 0;
	unsigned int trfc = 0;
	unsigned int trp = 0;
	unsigned int tprea = 0;
	unsigned int trtw = 0;
	unsigned int tal = 0;
	unsigned int tcl = 0;
	unsigned int tcwl = 0;
	unsigned int tras = 0;
	unsigned int trc = 0;
	unsigned int trcd = 0;
	unsigned int trrd = 0;
	unsigned int trtp = 0;
	unsigned int twr = 0;
	unsigned int twtr = 0;
	unsigned int texsr = 0;
	unsigned int txp = 0;
	unsigned int txpdll = 0;
	unsigned int tzqcs = 0;
	unsigned int tzqcsi = 0;
	unsigned int tdqs = 0;
	unsigned int tcksre = 0;
	unsigned int tcksrx = 0;
	unsigned int tcke = 0;
	unsigned int tmod = 0;
	unsigned int trstl = 0;
	unsigned int tzqcl = 0;
	unsigned int tmrr = 0;
	unsigned int tckesr = 0;
	unsigned int tdpd = 0;
	unsigned int tccd = 0;
	unsigned int taond = 0;
	unsigned int tfaw = 0;
	unsigned int trtodt = 0;
	unsigned int tdqsck = 0;
	unsigned int tdqsckmax = 0;
	unsigned int tdllk = 0;
	unsigned int titmsrst = 0;
	unsigned int tdlllock = 0;
	unsigned int tdllsrst = 0;
	unsigned int tdinit0 = 0;
	unsigned int tdinit1 = 0;
	unsigned int tdinit2 = 0;
	unsigned int tdinit3 = 0;



	if((para->dram_tpr13 & (0x1<<2)) == (1<<2))//bus width lock
		bus_lock = 1;

	if((para->dram_tpr13 & (0x1<<1)) == (1<<1))//channel lock
		ch_lock = 1;

	if((para->dram_tpr13 & 0x1) == 0)//auto detect
	{
		if((para->dram_type == 6) || (para->dram_type == 7))//LPDDR2 or LPDDR3
 		{
			trefi 	= 39;
			tmrd  	= 5;
			trfc	= 70;
			trp		= 10;
			tprea	= 0;
			trtw	= 2;
			tal		= 0;
			tcl		= 6;
			tcwl    = 4;
			tras	= 22;
			trc		= 32;
			trcd	= 10;
			trrd	= 5;
			trtp	= 4;
			twr		= 6;
			twtr	= 4;
			texsr	= 80;
			txp		= 4;
			txpdll	= 0;
			tzqcs	= 47;
			tzqcsi	= 0;
			tdqs	= 7;
			tcksre	= 0;
			tcksrx	= 0;
			tcke	= 3;
			tmod 	= 0;
			trstl	= 0;
			tzqcl	= 190;
			tmrr	= 2;
			tckesr	= 7;
			tdpd	= 0;
			tccd 	= 0;
			taond	= 0;
			tfaw	= 26;
			trtodt	= 0;
			tdqsck	= 1;
			tdqsckmax = 2;
			tdllk	= 512;
			titmsrst= 10;
			tdlllock = 2750;
			tdllsrst= 27;
			tdinit0 = 105600;
			tdinit1 = 53;
			tdinit2	= 5808;
			tdinit3 = 534;

			para->dram_mr0		=	0x0;
			para->dram_mr1		=	0x92;
			if(para->dram_type == 7)	//LPDDR3
				para->dram_mr1		=	0x83;

			para->dram_mr2		=	0x4;
			para->dram_mr3		=	0x2;

			para->dram_tpr0		=	tzqcsi;
			para->dram_tpr1		= 	(texsr<<22)|(tdpd<<12)|(tzqcl<<2)|(tprea<<0);
			para->dram_tpr2		=	(trfc<<23)|(trefi<<15)|(tmrr<<7)|(tzqcs<<0);
			para->dram_tpr3		=	(trstl<<25)|(tras<<19)|(trc<<13)|(txpdll<<7)|\
							(trp<<3)|(tmrd<<0);
			para->dram_tpr4		=	(tcksre<<27)|(tcksrx<<22)|(tcke<<17)|(tmod<<12)|\
							(trtw<<8)|(tal<<4)|(tcl<<0);
			para->dram_tpr5		=	(tcwl<<28)|(trcd<<24)|(trrd<<20)|(trtp<<16)|\
							(twr<<11)|(twtr<<7)|(tckesr<<3)|(txp<<0);
			para->dram_tpr6		=	(tdqs<<29) | (tdllk<<19) | (tfaw<<13) | (tdqsck<<10) |\
							(tdqsckmax<<7) | (tccd<<6) | (trtodt<<5) | (0<<4) |\
							(taond<<2);
			para->dram_tpr7		=	(tdllsrst<<0) | (tdlllock<<6) | (titmsrst<<18);
			para->dram_tpr8		=	(tdinit0<<0) | (tdinit1<<19);
			para->dram_tpr9		=	(tdinit2<<0) | (tdinit3<<17);
		}
		else if(para->dram_type == 3)//DDR3
		{
			trefi 	= 78;	//78
			tmrd  	= 4;
			trfc	= 140;
			trp		= 9;
			tprea	= 0;
			trtw	= 2;
			tal		= 0;
			tcl		= 9;
			tcwl    = 8;
			tras	= 24;
			trc		= 33;
			trcd	= 9;
			trrd	= 4;	//4
			trtp	= 5;
			twr		= 10;
			twtr	= 5;
			texsr	= 512;
			txp		= 5;
			txpdll	= 16;
			tzqcs	= 64;
			tzqcsi	= 0;
			tdqs	= 7;
			tcksre	= 7;
			tcksrx	= 7;
			tcke	= 4;
			tmod 	= 12;
			trstl	= 80;
			tzqcl	= 512;
			tmrr	= 2;
			tckesr	= 5;
			tdpd	= 0;
			tccd 	= 0;
			taond	= 0;
			tfaw	= 22;
			trtodt	= 0;
			tdqsck	= 1;
			tdqsckmax = 1;
			tdllk	= 512;
			titmsrst= 10;
			tdlllock = 2750;
			tdllsrst= 27;
			tdinit0 = 266525;
			tdinit1 = 192;
			tdinit2	= 106610;
			tdinit3 = 534;
			if(para->dram_clk <= 480)
			{
				trfc	= 115;	//115
				tras	= 18;
				trc		= 23;
				trtp	= 4;
				twr		= 8;
				twtr	= 4;
				txp		= 4;
				txpdll	= 14;
				tcksre	= 5;
				tcksrx  = 5;
				tfaw	= 20;
				tdlllock = 2250;
				tdllsrst= 23;
				tdinit0 = 217000;
				tdinit1 = 160;
				tdinit2	= 87000;
				tdinit3 = 433;
			}
			para->dram_mr0		=	0x1A00;
			{
				if(tcl >= 12)
				{
					para->dram_mr0 |= 0x1<<2;
					para->dram_mr0 |= (tcl-12)<<4;
				}
				else
				{
					para->dram_mr0 |= (tcl-4)<<4;
				}
			}
			para->dram_mr1		=	0x4;	//0x4
			if(tal != 0)
			{
				para->dram_mr1	|= (tcl - tal)<<3;
			}
			para->dram_mr2		=	(tcwl-5)<<3;
			para->dram_mr3		=	0;

			para->dram_tpr0		=	tzqcsi;
			para->dram_tpr1		= 	(texsr<<22)|(tdpd<<12)|(tzqcl<<2)|(tprea<<0);
			para->dram_tpr2		=	(trfc<<23)|(trefi<<15)|(tmrr<<7)|(tzqcs<<0);
			para->dram_tpr3		=	(trstl<<25)|(tras<<19)|(trc<<13)|(txpdll<<7)|			\
									(trp<<3)|(tmrd<<0);
			para->dram_tpr4		=	(tcksre<<27)|(tcksrx<<22)|(tcke<<17)|(tmod<<12)|		\
									(trtw<<8)|(tal<<4)|(tcl<<0);
			para->dram_tpr5		=	(tcwl<<28)|(trcd<<24)|(trrd<<20)|(trtp<<16)|\
									(twr<<11)|(twtr<<7)|(tckesr<<3)|(txp<<0);
			para->dram_tpr6		=	(tdqs<<29) | (tdllk<<19) | (tfaw<<13) | (tdqsck<<10) |	\
									(tdqsckmax<<7) | (tccd<<6) | (trtodt<<5) | (0<<4) |		\
									(taond<<2);
			para->dram_tpr7		=	(tdllsrst<<0) | (tdlllock<<6) | (titmsrst<<18);
			para->dram_tpr8		=	(tdinit0<<0) | (tdinit1<<19);
			para->dram_tpr9		=	(tdinit2<<0) | (tdinit3<<17);
		}

	}
	else	//user lock
	{
		trefi = ((para->dram_tpr2)>>15)&0xFF;
		tmrd  = ((para->dram_tpr3)>>0)&0x7;
		trfc  = ((para->dram_tpr2)>>23)&0x1FF;
		trp   = ((para->dram_tpr3)>>3)&0xF;
		tprea = ((para->dram_tpr1)>>0)&0x3;
		trtw  = ((para->dram_tpr4)>>8)&0xF;
		tal   = ((para->dram_tpr4)>>4)&0xF;
		tcl   = ((para->dram_tpr4)>>0)&0xF;
		tcwl  = ((para->dram_tpr5)>>28)&0xF;
		tras  = ((para->dram_tpr3)>>19)&0x3F;
		trc   = ((para->dram_tpr3)>>13)&0x3F;
		trcd  = ((para->dram_tpr5)>>24)&0xF;
		trrd  = ((para->dram_tpr5)>>20)&0xF;
		trtp  = ((para->dram_tpr5)>>16)&0xF;
		twr   = ((para->dram_tpr5)>>11)&0x1F;
		twtr  = ((para->dram_tpr5)>>7)&0xF;
		texsr = ((para->dram_tpr1)>>22)&0x3FF;
		txp   = ((para->dram_tpr5)>>0)&0x7;
		txpdll= ((para->dram_tpr3)>>7)&0x3F;
		tzqcs = ((para->dram_tpr2)>>0)&0x7F;
		tzqcsi= (para->dram_tpr0);
		tdqs  = ((para->dram_tpr6)>>29)&0x7;
		tcksre= ((para->dram_tpr4)>>27)&0x1F;
		tcksrx= ((para->dram_tpr4)>>22)&0x1F;
		tcke  = ((para->dram_tpr4)>>17)&0x1F;
		tmod  = ((para->dram_tpr4)>>12)&0x1F;
		trstl = ((para->dram_tpr3)>>25)&0x7F;
		tzqcl = ((para->dram_tpr1)>>2)&0x3FF;
		tmrr  = ((para->dram_tpr2)>>7)&0xFF;
		tckesr= ((para->dram_tpr5)>>3)&0xF;
		tdpd  =	((para->dram_tpr1)>>12)&0x3FF;
		tccd 	= ((para->dram_tpr6)>>6)&0x1;
		taond	= ((para->dram_tpr6)>>2)&0x3;
		tfaw	= ((para->dram_tpr6)>>13)&0x3F;
		trtodt	= ((para->dram_tpr6)>>5)&0x1;
		tdqsck	= ((para->dram_tpr6)>>10)&0x7;
		tdqsckmax = ((para->dram_tpr6)>>7)&0x7;
		tdllk	= ((para->dram_tpr6)>>19)&0x3FF;
		titmsrst= ((para->dram_tpr7)>>18)&0xF;
		tdlllock = ((para->dram_tpr7)>>6)&0xFFF;
		tdllsrst= ((para->dram_tpr7)>>0)&0x3F;
		tdinit0 = ((para->dram_tpr8)>>0)&0x7FFFF;
		tdinit1 = ((para->dram_tpr8)>>19)&0xFF;
		tdinit2	= ((para->dram_tpr9)>>0)&0x1FFFF;
		tdinit3 = ((para->dram_tpr9)>>17)&0x3FF;
	}

	//get flag of pad hold status
	reg_val = mctl_read_w(R_VDD_SYS_PWROFF_GATE);

	if(ch_index == 1)
	{
		ch_id = 0x1000;
		hold_flag = (reg_val)&0x1;
	}
	else
	{
		ch_id = 0x0;
		hold_flag = (reg_val>>1)&0x1;
	}

	//send NOP command to active CKE
	reg_val = 0x88000000 | (0x0<<20); //rank 0
	mctl_write_w(ch_id + SDR_MCMD, reg_val);

	while((mctl_read_w(ch_id + SDR_MCMD) & 0x80000000) && (time--))
		continue;

	if((para->dram_para2 & (0xF<<12)) == (0x2<<12)) //2 rank
	{
		//send NOP command to active CKE
		reg_val = 0x88000000 | (0x1<<20); //rank 1
		mctl_write_w(ch_id + SDR_MCMD, reg_val);

		time = 0xffffff;
		while((mctl_read_w(ch_id + SDR_MCMD) & 0x80000000) && (time--))
			continue;
	}

   //set PHY genereral configuration register
   reg_val = 0x01042202;	//modify 12/3
   reg_val |= 0x2<<22;
   reg_val &= ~(0x3<<12);
   if(((para->dram_para2>>12)&0xF) == 2)
  		reg_val |= (0x1<<19);
   mctl_write_w(ch_id + SDR_PGCR, reg_val);

   //set mode register
	mctl_write_w(ch_id + SDR_MR0, para->dram_mr0);
	mctl_write_w(ch_id + SDR_MR1, para->dram_mr1);
	mctl_write_w(ch_id + SDR_MR2, para->dram_mr2);
	mctl_write_w(ch_id + SDR_MR3, para->dram_mr3);

	//phy timing parameters
	reg_val = titmsrst<<18;
	reg_val |= tdlllock<<6;
	reg_val |= tdllsrst<<0;
	if(((mctl_read_w(0x01f0020c)&0x3) == 0x2) && ((para->dram_tpr13>>20)&0x1)==0)//c
		reg_val |= 0x1<<18;
	mctl_write_w(ch_id + SDR_PTR0, reg_val);

	reg_val = tdinit0<<0;
	reg_val |= tdinit1<<19;
	mctl_write_w(ch_id + SDR_PTR1, reg_val);

	reg_val = tdinit2<<0;
	reg_val |= tdinit3<<17;
	mctl_write_w(ch_id + SDR_PTR2, reg_val);

	reg_val = (tccd)<<31;	//tCCD
	reg_val |= (trc)<<25; //tRC
	reg_val |= (trrd)<<21; //tRRD
	reg_val |= (tras)<<16; //tRAS
	reg_val |= (trcd)<<12; //tRCD
	reg_val |= (trp)<<8; //tRP
	reg_val |= (twtr)<<5; //tWTR
	reg_val |= (trtp)<<2; //tRTP
	reg_val |= (tmrd)<<0; //tMRD
	mctl_write_w(ch_id + SDR_DTPR0, reg_val);

	reg_val = tdqsckmax<<27;	//tDQSCKMAX
	reg_val |= tdqsck<<24; //tdqsck
	reg_val |= trfc<<16;
	reg_val |= trtodt<<11; //trtodt
	if((para->dram_type == 6) || (para->dram_type == 7)) //LPDDR2 or LPDDR3
		reg_val |= tmod<<9;
	else
		reg_val |= (tmod-12)<<9;
	reg_val |= tfaw<<3; //tfaw
	reg_val |= 0<<2;
	reg_val |= taond<<0; //tand taofd
	mctl_write_w(ch_id + SDR_DTPR1, reg_val);

	reg_val = tdllk<<19; //tdllk
	reg_val |= tcke<<15;
	reg_val |= txpdll<<10;
	reg_val |= texsr<<0; //txs
	mctl_write_w(ch_id + SDR_DTPR2, reg_val);

	mctl_write_w(ch_id + SDR_DFITPHYUPDTYPE0, 1);	//modify 12/2

	//set PHY DDR mode
	if(para->dram_type == 2)		//DDR2
		reg_val = 0xa;
	else if(para->dram_type == 3)	//DDR3
		reg_val = 0xb;
	else if(para->dram_type == 5)	//LPDDR
		reg_val = 0x8;
	else							//LPDDR2
		reg_val = 0xc;
	mctl_write_w(ch_id + SDR_DCR, reg_val);

	//set DDR system general configuration register
	reg_val = 0xd200001b;
	if( ((para->dram_type == 3) && ((para->dram_mr1 & 0x244) != 0) )
	 || ((para->dram_type == 6) && (para->dram_mr3 != 0))
	 || ((para->dram_type == 7) && (para->dram_mr3 != 0)) )
		reg_val |= 0x1<<29;
	if((para->dram_type == 6) || (para->dram_type == 7)) //LPDDR2 or LPDDR3
	{
		reg_val |= (0x1<<8) | (0x1<<5) | (0x1<<24);
		reg_val &= ~(0x1<<25);
	}
	mctl_write_w(ch_id + SDR_DSGCR, reg_val);

	//set DATX8 common configuration register
	reg_val = 0x800;
	if((para->dram_type == 6) || (para->dram_type == 7)) //LPDDR2 or LPDDR3
	{
		reg_val = 0x910;
	}
	mctl_write_w(ch_id + SDR_DXCCR, reg_val);

	if(para->dram_odt_en == 0)
	{
		reg_val = 0x881;
	}else
	{
		if(( (mctl_read_w(0x01f0020c)&0x3) == 0x2) )//c
			reg_val = 0x3c81;
		else
			reg_val = 0x2e81;
	}

	mctl_write_w(ch_id + SDR_DX0GCR, reg_val);
	mctl_write_w(ch_id + SDR_DX1GCR, reg_val);
	if((para->dram_para2 & 0xF) == 0)
		reg_val &= ~(0x1<<0);
	mctl_write_w(ch_id + SDR_DX2GCR, reg_val);
	mctl_write_w(ch_id + SDR_DX3GCR, reg_val);

	time = 0xffffff;
	while(((mctl_read_w(ch_id + SDR_PGSR)&0x3)!= 0x3) && (time--)) {};

	//set odt impendance divide ratio
	mctl_write_w(ch_id + SDR_ZQ0CR1, para->dram_zq);

	//clear status bits
	reg_val = mctl_read_w(ch_id + SDR_PIR);
	reg_val |= 0x1<<28;
	mctl_write_w(ch_id + SDR_PIR, reg_val);

	//init external dram
	if(hold_flag)
	{
		reg_val = 0x41;
		mctl_write_w(ch_id + SDR_PIR, reg_val);

		//write DQS gating & ZQ calibration value
	}
	else
	{
		reg_val = 0xe9;
		mctl_write_w(ch_id + SDR_PIR, reg_val);
	}

	__msdelay(1);

	//wait init done
	if(!hold_flag)
	{
		time = 0xffffff;
		while(((mctl_read_w(ch_id + SDR_PGSR)&0x1F) != 0x1F) && (time--)) {};//modify 12/3
	}else
	{
		time = 0xffffff;
		while(((mctl_read_w(ch_id + SDR_PGSR)&0x1F) != 0xB) && (time--)) {};//modify 12/3
	}


	if(((para->dram_tpr13 & (0x1u<<31)) == (0<<31)) && (!hold_flag))
	{
		//rank detect
		if((para->dram_para2 & (0xF<<12)) == (0x2<<12)) //default 2 rank; if 1 rank, pass over
		{
		time = 0xffffff;
		while(((mctl_read_w(ch_id + SDR_DX0GSR0) & (0x1<<1)) != (0x1<<1)) && (time--)){} //rank1 DX0 training done
		time = 0xffffff;
		while(((mctl_read_w(ch_id + SDR_DX1GSR0) & (0x1<<1)) != (0x1<<1)) && (time--)){} //rank1 DX0 training done
		if((mctl_read_w(ch_id + SDR_DX0GSR0)&(0x1<<5)) || (mctl_read_w(ch_id + SDR_DX1GSR0)&(0x1<<5))) 	//training error
		{
			if((((para->dram_tpr13>>3)&0x3) != 1))//not A31s
			{
				if(ch_id == 0x1000)
					{
						if((mctl_read_w(SDR_DX0GSR0)&(0x1<<5)) || (mctl_read_w(SDR_DX1GSR0)&(0x1<<5)))
						{
							paraconfig(&(para->dram_para2), 0xF<<12, 1<<12); //1 rank

							reg_val = mctl_read_w(ch_id + SDR_PGCR); // 1 rank
							reg_val &= ~(0x1<<19);
							mctl_write_w(ch_id + SDR_PGCR, reg_val);
						}
					}else
					{
						paraconfig(&(para->dram_para2), 0xF<<12, 1<<12); //1 rank

						reg_val = mctl_read_w(ch_id + SDR_PGCR); // 1 rank
						reg_val &= ~(0x1<<19);
						mctl_write_w(ch_id + SDR_PGCR, reg_val);
					}
				}else
				{
					paraconfig(&(para->dram_para2), 0xF<<12, 1<<12); //1 rank

					if(ch_id == 0)
					{
						reg_val = mctl_read_w(ch_id + SDR_PGCR); // 1 rank
						reg_val &= ~(0x1<<19);
						mctl_write_w(ch_id + SDR_PGCR, reg_val);
					}
				}
			}
		}

		//channel detect
		if((((para->dram_tpr13>>3)&0x3) != 1))//not A31s
		{
			if((ch_lock == 0) && (ch_id == 0x1000))
			{
				time = 0xffffff;
				while(((mctl_read_w(ch_id + SDR_DX0GSR0) & (0x1<<0)) != (0x1<<0)) && (time--)){} //ch1 rank0 DX0 training done
				time = 0xffffff;
				while(((mctl_read_w(ch_id + SDR_DX1GSR0) & (0x1<<0)) != (0x1<<0)) && (time--)){} //ch1 rank0 DX0 training done
				if((mctl_read_w(ch_id + SDR_DX0GSR0)&(0x1<<4)) || (mctl_read_w(ch_id + SDR_DX1GSR0)&(0x1<<4))) //training error
				{
					paraconfig(&(para->dram_para2), 0xF<<8, 1<<8); //ch num = 1
					reg_val = mctl_read_w(SDR_COM_CCR);	//turn off ch1 all DLL
					reg_val |= (0x1<<4);
					reg_val &= ~(0x1<<1);
					mctl_write_w(SDR_COM_CCR, reg_val);

					return 0;
				}
			}
		}

		//bus detect
		if(bus_lock == 0)
		{
			time = 0xffffff;
			while(((mctl_read_w(ch_id + SDR_DX2GSR0) & (0x1<<0)) != (0x1<<0)) && (time--)){} //ch0 rank0 DX2 training done
			time = 0xffffff;
			while(((mctl_read_w(ch_id + SDR_DX3GSR0) & (0x1<<0)) != (0x1<<0)) && (time--)){} //ch0 rank0 DX2 training done
			if((mctl_read_w(ch_id + SDR_DX2GSR0) &  (0x1<<4)) ||  (mctl_read_w(ch_id + SDR_DX3GSR0) &  (0x1<<4)))//ch0 rank0 DX2 training error
			{
				paraconfig(&(para->dram_para2), 0xF<<0, 0<<0); //bus width = 16
				paraconfig(&(para->dram_para1), 0xF<<16, 2<<16); //page size = 2K

				mctl_write_w(ch_id + SDR_DX2DLLCR, 0xC0000000); //turn off DX2 & DX3 DLL on ch0 & ch1
				mctl_write_w(ch_id + SDR_DX3DLLCR, 0xC0000000);

				reg_val = mctl_read_w(ch_id + SDR_DX2GCR);	//disable DX2 training
				reg_val &= ~(0x1<<0);
				mctl_write_w(ch_id + SDR_DX2GCR, reg_val);

				reg_val = mctl_read_w(ch_id + SDR_DX3GCR);	//disable DX3 training
				reg_val &= ~(0x1<<0);
				mctl_write_w(ch_id + SDR_DX3GCR, reg_val);
			}
		}

		//clear status bits
		reg_val = mctl_read_w(ch_id + SDR_PIR);
		reg_val |= 0x1<<28;
		mctl_write_w(ch_id + SDR_PIR, reg_val);

		while(mctl_read_w(ch_id + SDR_PGSR) & (1<<5));


		//init external dram
		if(hold_flag)
			reg_val = 0x41;
		else
			reg_val = 0x81;
		mctl_write_w(ch_id + SDR_PIR, reg_val);

		__msdelay(1);

		//wait init done
		if(!hold_flag)
		{
			while( (mctl_read_w(ch_id + SDR_PGSR)&0x11) != 0x11) {};//modify 12/3
		}else
		{
			while( (mctl_read_w(ch_id + SDR_PGSR)&0x1F) != 0xB) {};//modify 12/3
		}

		if(mctl_read_w(ch_id + SDR_PGSR) & (0x3<<5))
			return 0;
		//end of phy parameters detect
	}


   //***********************************************
   // set dram MCTL register
   //***********************************************
	//move to configure state
	reg_val = 0x1;
	mctl_write_w(ch_id + SDR_SCTL, reg_val);
	time = 0xffffff;
	while(((mctl_read_w(ch_id + SDR_SSTAT)&0x7) != 0x1 ) && (time--)) {};

	//set memory timing regitsers
	clkmhz = para->dram_clk;
	//clkmhz = clkmhz/1000000;
	reg_val = clkmhz;
	mctl_write_w(ch_id + SDR_TOGCNT1U, reg_val);		//1us
	reg_val = clkmhz/10;
	mctl_write_w(ch_id + SDR_TOGCNT100N, reg_val);		//100ns
	mctl_write_w(ch_id + SDR_TREFI	,trefi);
	mctl_write_w(ch_id + SDR_TMRD	,tmrd);
	mctl_write_w(ch_id + SDR_TRFC	,trfc);
	mctl_write_w(ch_id + SDR_TRP	,trp | (tprea<<16));
	mctl_write_w(ch_id + SDR_TRTW	,trtw);
	mctl_write_w(ch_id + SDR_TAL	,tal);
	mctl_write_w(ch_id + SDR_TCL	,tcl);
	mctl_write_w(ch_id + SDR_TCWL	,tcwl);
	mctl_write_w(ch_id + SDR_TRAS	,tras);
	mctl_write_w(ch_id + SDR_TRC	,trc);
	mctl_write_w(ch_id + SDR_TRCD	,trcd);
	mctl_write_w(ch_id + SDR_TRRD	,trrd);
	mctl_write_w(ch_id + SDR_TRTP	,trtp);
	mctl_write_w(ch_id + SDR_TWR	,twr);
	mctl_write_w(ch_id + SDR_TWTR	,twtr);
	mctl_write_w(ch_id + SDR_TEXSR	,texsr);
	mctl_write_w(ch_id + SDR_TXP	,txp);
	mctl_write_w(ch_id + SDR_TXPDLL ,txpdll);
	mctl_write_w(ch_id + SDR_TZQCS	,tzqcs);
	mctl_write_w(ch_id + SDR_TZQCSI ,tzqcsi);
	mctl_write_w(ch_id + SDR_TDQS   ,tdqs);
	mctl_write_w(ch_id + SDR_TCKSRE ,tcksre);
	mctl_write_w(ch_id + SDR_TCKSRX ,tcksrx);
	mctl_write_w(ch_id + SDR_TCKE 	,tcke);
	mctl_write_w(ch_id + SDR_TMOD 	,tmod);
	mctl_write_w(ch_id + SDR_TRSTL  ,trstl);
	mctl_write_w(ch_id + SDR_TZQCL  ,tzqcl);
	mctl_write_w(ch_id + SDR_TMRR 	,tmrr);
	mctl_write_w(ch_id + SDR_TCKESR ,tckesr);
	mctl_write_w(ch_id + SDR_TDPD 	,tdpd);

	if( ((para->dram_type == 3) && ((para->dram_mr1 & 0x244) != 0) )
	 || ((para->dram_type == 6) && (para->dram_mr3 != 0))
	 || ((para->dram_type == 7) && (para->dram_mr3 != 0)) )
	{
		reg_val = mctl_read_w(ch_id + SDR_DFIODTCFG);
		reg_val |= (0x1<<3);
		mctl_write_w(ch_id + SDR_DFIODTCFG, reg_val);

		if((para->dram_para2 & (0xF<<12)) == (0x2<<12)) //2 ranks
		{
			reg_val = mctl_read_w(ch_id + SDR_DFIODTCFG);
			reg_val |= (0x1<<11);
			mctl_write_w(ch_id + SDR_DFIODTCFG, reg_val);
		}

		reg_val = mctl_read_w(ch_id + SDR_DFIODTCFG1);
		reg_val &= ~((0x1f<<0));
		reg_val |= (0x0<<0);
		mctl_write_w(ch_id + SDR_DFIODTCFG1, reg_val);
	}

	//select 16/32-bits mode for MCTL
	reg_val = mctl_read_w(ch_id + SDR_PPCFG);
	//if(para->dram_bus_width == 16)
	if(((para->dram_para2>>0)&0xF) == 0)
		reg_val |= 0x1;
	mctl_write_w(ch_id + SDR_PPCFG, reg_val);

	//set DFI timing registers
	if((((para->dram_tpr13>>5)&0x1) == 0) && (para->dram_type != 6) && (para->dram_type != 7)) //NOT LPDDR2 or LPDDR3
	{
		reg_val = tcwl - 0;
		mctl_write_w(ch_id + SDR_DFITPHYWRL, reg_val);
		reg_val = tcl - 1;
		mctl_write_w(ch_id + SDR_DFITRDDEN, reg_val);
	}
	else
	{
		reg_val = tcwl - 1;
		mctl_write_w(ch_id + SDR_DFITPHYWRL, reg_val);
		reg_val = tcl - 2;
		mctl_write_w(ch_id + SDR_DFITRDDEN, reg_val);
	}
	mctl_write_w(ch_id + SDR_DFITPHYRDL, 15);

	reg_val = 0x5;
	mctl_write_w(ch_id + SDR_DFISTCFG0, reg_val);

	//configure memory related attributes of mctl
	if(para->dram_type == 2)			//DDR2
		reg_val = 0x70040;
	else if(para->dram_type == 3)		//DDR3
		reg_val = 0x70061;
	else if(para->dram_type == 5)		//LPDDR
		reg_val = 0x970040;
	else if(para->dram_type == 6)		//LPDDR2
		reg_val = 0xd70040;
	else
		reg_val = 0xE70041;				//LPDDR3
	//reg_val |= ((0x10<<8) | (0x1<<16));
	mctl_write_w(ch_id + SDR_MCFG, reg_val);

	//DFI update configuration register
	reg_val = 0x2;
	mctl_write_w(ch_id + SDR_DFIUPDCFG, reg_val);

	if(hold_flag)
	{
		//move to access state
		reg_val = 0x2;
		mctl_write_w(ch_id + SDR_SCTL, reg_val);

		time = 0xffffff;
		while(((mctl_read_w(ch_id + SDR_SSTAT)&0x7) != 0x3 )&& (time--)) {};

		//move to sleep state
		reg_val = 0x3;
		mctl_write_w(ch_id + SDR_SCTL, reg_val);
		time = 0xffffff;
		while(((mctl_read_w(ch_id + SDR_SSTAT)&0x7) != 0x5 ) && (time--)) {};

		//close pad hold function
		reg_val = mctl_read_w(R_VDD_SYS_PWROFF_GATE);
		if(ch_index == 1)
			reg_val &= ~(0x1);
		else
			reg_val &= ~(0x1<<1);
		mctl_write_w(R_VDD_SYS_PWROFF_GATE, reg_val);

		//set WAKEUP command
		reg_val = 0x4;
		mctl_write_w(ch_id + SDR_SCTL, reg_val);
		time = 0xffffff;
		while(((mctl_read_w(ch_id + SDR_SSTAT)&0x7) != 0x3 ) && (time--)) {};

//		//calibration and dqs training
//		reg_val = 0x89;
//		mctl_write_w(ch_id + SDR_PIR, reg_val);
//		time = 0xffffff;
//		while(((mctl_read_w(ch_id + SDR_PGSR)&0x1) == 0x0) && (time--)) {};
	}

//	reg_val = mctl_read_w(ch_id + SDR_MCFG);
//	reg_val &= ~(0xFF<<8);
//	reg_val |= ((0x10<<8) | (0x1<<16));
//	mctl_write_w(ch_id + SDR_MCFG, reg_val);

	//move to access state
	reg_val = 0x2;
	mctl_write_w(ch_id + SDR_SCTL, reg_val);

	time = 0xffffff;
	while(((mctl_read_w(ch_id + SDR_SSTAT)&0x7) != 0x3 )&& (time--)) {};

	return (1);
}

unsigned int mctl_com_init(__dram_para_t *para)
{
	unsigned int reg_val;

	//set COM memory organization register
	reg_val = 0;
	if(((para->dram_para2>>12)&0xF) == 2)
		reg_val |= 0x1;

	if(((para->dram_para1>>28)&0xF) == 1)
		reg_val |= 0x1<<2;

	reg_val |= ((((para->dram_para1>>20)&0xFF) -1)&0xf)<<4;

	if(((para->dram_para1>>16)&0xF) == 8)
		reg_val |= 0xa<<8;
	else if(((para->dram_para1>>16)&0xF) == 4)
		reg_val |= 0x9<<8;
	else if(((para->dram_para1>>16)&0xF) == 2)
		reg_val |= 0x8<<8;
	else if(((para->dram_para1>>16)&0xF) == 1)
		reg_val |= 0x7<<8;
	else
		reg_val |= 0x6<<8;

	if(((para->dram_para2>>0)&0xF) == 1)
		reg_val |= 0x3<<12;
	else
		reg_val |= 0x1<<12;

	if(((para->dram_para2>>4)&0xF) == 1)
		reg_val |= 0x1<<15;

	if(para->dram_type != 7)	//not LPDDR3
		reg_val |= (para->dram_type)<<16;
	else
		reg_val |= 3<<16;		//LPDDR3

	if(((para->dram_para2>>8)&0xF) == 2)
		reg_val |= 0x1<<19;

	reg_val |= 0x1<<20;
	reg_val |= 0x1<<22;
	mctl_write_w(SDR_COM_CR, reg_val);

	if((((para->dram_tpr13>>5)&0x1) == 0) && (para->dram_type != 6) && (para->dram_type != 7)) //NOT LPDDR2 or LPDDR3
	{
		reg_val = mctl_read_w(SDR_COM_DBGCR);
		reg_val |= 1U << 6;
		mctl_write_w(SDR_COM_DBGCR, reg_val);
	}

	if(((para->dram_para2>>8)&0xF) == 1)//1
	{
		reg_val = mctl_read_w(SDR_COM_CCR);	//turn off ch1 all DLL
		reg_val |= (0x1<<4);
		reg_val &= ~(0x1<<1);
		mctl_write_w(SDR_COM_CCR, reg_val);

		//put PAD into power down state
		reg_val = mctl_read_w(0x1000 + SDR_ACIOCR);
		reg_val |= (0x1<<3)|(0x1<<8)|(0x3<<18);
		mctl_write_w(0x1000 + SDR_ACIOCR, reg_val);

		reg_val = mctl_read_w(0x1000 + SDR_DXCCR);
		reg_val |= (0x1<<2)|(0x1<<3);
		mctl_write_w(0x1000 + SDR_DXCCR, reg_val);

		reg_val = mctl_read_w(0x1000 + SDR_DSGCR);
		reg_val &= ~(0x1<<28);
		mctl_write_w(0x1000 + SDR_DSGCR, reg_val);

		reg_val = mctl_read_w(R_VDD_SYS_PWROFF_GATE);
		reg_val |= 0x1;
		mctl_write_w(R_VDD_SYS_PWROFF_GATE, reg_val);
	}

	return (1);
}

unsigned int mctl_port_cfg(void)
{
	unsigned int reg_val;

	//enable DRAM AXI clock for CPU access
	reg_val = mctl_read_w(CCM_AXI_GATE_CTRL);
	reg_val |= 0x1;
	mctl_write_w(CCM_AXI_GATE_CTRL, reg_val);

	//master configuration
	mctl_write_w(0x01c62010, 0x0400302);
	mctl_write_w(0x01c62014, 0x1000307);
	mctl_write_w(0x01c62018, 0x0400302);
	mctl_write_w(0x01c6201c, 0x1000307);
	mctl_write_w(0x01c62020, 0x1000307);
	mctl_write_w(0x01c62028, 0x1000303);

	mctl_write_w(0x01c62030, 0x1000303);
	mctl_write_w(0x01c62034, 0x0400310);
	mctl_write_w(0x01c62038, 0x1000307);
	mctl_write_w(0x01c6203c, 0x1000303);
	mctl_write_w(0x01c62040, 0x1800303);
	mctl_write_w(0x01c62044, 0x1800303);
	mctl_write_w(0x01c62048, 0x1800303);
	mctl_write_w(0x01c6204C, 0x1800303);
	mctl_write_w(0x01c62050, 0x1000303);

	mctl_write_w(0x01c6206C, 0x00000002);

	mctl_write_w(0x01c62070, 0x00000310);
	mctl_write_w(0x01c62074, 0x00400310);
	mctl_write_w(0x01c62078, 0x00400310);
	mctl_write_w(0x01c6207C, 0x00000307);
	mctl_write_w(0x01c62080, 0x00000317);
	mctl_write_w(0x01c62084, 0x00000307);

  	return (1);
}

//*****************************************************************************
//	signed int init_DRAM(int type)
//  Description:	System init dram
//
//	Arguments:		type:	0: no lock		1: get the fixed parameters & auto detect & lock
//
//	Return Value:	0: fail
//					others: pass
//*****************************************************************************
signed int init_DRAM(int type, void *para)
{
	signed int ret_val;
	int cpu_id, pmu_id;
	unsigned int low_clk = 0;
	unsigned int factor = 0;

	dram_para = (__dram_para_t *)para;

	if(dram_para->dram_tpr13 & (0x1u<<31))
	{
		if( (dram_para->dram_tpr12 & 0x2) && !(dram_para->dram_tpr12 & (0x1<<18)) )
		{
			//low_clk = 24 * ((dram_para->dram_tpr12>>20)&0xF);
			factor = (dram_para->dram_clk*2) / dram_para->dram_tpr11;
			low_clk = (dram_para->dram_clk*2) / factor;
			dram_para->dram_tpr12 |= (dram_para->dram_clk /24)<<24;
			dram_para->dram_clk = low_clk;
			dram_para->dram_tpr12 |= 0x1<<18;
		}
	}

	dram_dbg("[DRAM]ver 1.08 clk = %d\n", dram_para->dram_clk  );

	ret_val = p2wi_init();
	if (ret_val)
		return 0;

	set_cpu_voltage();

	/**
	 * CPU bonding ID:  0: A31	1: A31S		2: A3X PHONE
	 * PMU bonding ID:  0: AXP221	1: AXP221S
	 * A31 - AXP221
	 * A31S- AXP221 & AXP221S
	 */
	cpu_id = ss_bonding_id();
	pmu_id = pmu_bonding_id();
	if ((!cpu_id && pmu_id == 1) || cpu_id < 0 || pmu_id < 0) {
		dram_dbg("pmu does not match with cpu, pmuid %d, cpuid %d\n", pmu_id, cpu_id);
		return 0;
	}
	dram_dbg("cpu %d pmu %d\n", cpu_id, pmu_id);

	if((dram_para->dram_tpr13 & (0x1u<<31)) == (0x0<<31))//auto detect parameters
	{
		if(((dram_para->dram_tpr13 & (0x1u<<31)) == 0) && !(mctl_read_w(R_VDD_SYS_PWROFF_GATE)&0x3))
		{
			paraconfig(&(dram_para->dram_para2), 0xF<<0, 1<<0); //bus width = 32
			paraconfig(&(dram_para->dram_para1), 0xF<<16, 4<<16); //page size = 4K
			paraconfig(&(dram_para->dram_para2), 0xF<<12, 2<<12); //rank num = 2
			paraconfig(&(dram_para->dram_para2), 0xF<<8, 2<<8); //ch num = 2
			paraconfig(&(dram_para->dram_para1), 0xFF<<20, 16<<20); //row num = 16
			paraconfig(&(dram_para->dram_para1), 0xFu<<28, 1u<<28); //bank num = 8
			paraconfig(&(dram_para->dram_para2), 0xFu<<4, 0u<<4); //access mode = interleave
			paraconfig(&(dram_para->dram_para1), 0xFFFF<<0, 0x800<<0);
		}

		if(type == 1)
		{
			dram_para->dram_tpr13 |= ((0x1<<1) | (0x1<<2)); //ch/bus lock
		}else
		{
			dram_para->dram_tpr13 |= (0x1<<9); //size no lock
		}

		if((cpu_id == 0) || (cpu_id == 3) || (cpu_id == 2)) //A31 & V10
		{
			dram_para->dram_tpr13 |= 0x1<<7; //2GB max
			dram_para->dram_tpr13 &= ~(0x3<<3);
			dram_para->dram_tpr13 |= (cpu_id<<3);
			paraconfig(&(dram_para->dram_para2), 0xF<<8, 2<<8); //ch num = 2

		}else if((cpu_id == 1))  //A31S
		{
			dram_para->dram_tpr13 |= 0x2<<7; //1GB max
			dram_para->dram_tpr13 &= ~(0x3<<3);
			dram_para->dram_tpr13 |= (cpu_id<<3);
			paraconfig(&(dram_para->dram_para2), 0xF<<8, 1<<8);	//ch_num = 1
		}

	    if((mctl_read_w(SDR_PGSR) & (0x3f<<0)) == 0x1F)
	        resetflag = 1;
	}

	ret_val = DRAMC_init(dram_para);

	return ret_val;
}


int ss_bonding_id(void)
{
	int reg_val;
	int id;
	//enable SS working clock
	reg_val = mctl_read_w(0x01c20000 + 0x9C); //CCM_SS_SCLK_CTRL
	//24MHz
	reg_val &= ~(0x3<<24);
	reg_val &= ~(0x3<<16);
	reg_val &= ~(0xf);
	reg_val |= 0x0<<16;
	reg_val |= 0;
	reg_val |= 0x1U<<31;
	mctl_write_w(0x01c20000 + 0x9C, reg_val);

	//enable SS AHB clock
	reg_val = mctl_read_w(0x01c20000 + 0x60); //CCM_AHB1_GATE0_CTRL
	reg_val |= 0x1<<5;		//SS AHB clock on
	mctl_write_w(0x01c20000 + 0x60, reg_val);

	//release SS reset
	reg_val = mctl_read_w(0x01c20000 + 0x2c0);
	reg_val |= 0x1<<5;
	mctl_write_w(0x01c20000 + 0x2c0, reg_val);

	reg_val = mctl_read_w(0x01C15000 + 0x00); //SS_CTL
	reg_val >>=16;
	reg_val &=0x3;
	mctl_write_w(0x01C15000 + 0x00,reg_val);

	id = reg_val;

	reg_val = mctl_read_w(0x01C15000 + 0x00); //SS_CTL
	reg_val &= ~0x1;
	mctl_write_w(0x01C15000 + 0x00,reg_val);

	//0&3: A31	1: A31S		2: v10
	return id;
}

int pmu_bonding_id(void)
{
	unsigned int id = 0;
	int ret;
	ret = p2wi_write(0xff, 1);
	if (ret)
		return -1;
	ret = p2wi_read(0x29, &id);
	if (ret)
		return -1;
	ret = p2wi_write(0xff, 0);
	if (ret)
		return -1;
	return (int)id;
}


void paraconfig(unsigned int *para, unsigned int mask, unsigned int value)
{
	*para &= ~(mask);
	*para |= value;
}

