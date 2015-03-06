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

#include "mctl_hal.h"
#include "mctl_reg.h"
//#include "mctl_hal.h"

//#define MCTL_MDFS_ONESTEP 0x0
//#define MCTL_MDFS_TWOSTEP 0x1
unsigned int* mdfsvalue = (unsigned*)0x28000;

/*
*	mdfs_mode: 0: one step		1: two step
* 	freq_div: for one step: PLL5/freq_div = current frequency	for two step: current frequency
*	trainint_mode: default: 0 no training;			1: training
*/
unsigned int mctl_mdfs_setup(unsigned int mdfs_mode, unsigned int freq_div, unsigned int training_mode)
{
	unsigned int reg_val;
	unsigned int err_flag = 0;
	int i;
	unsigned int tmp;

	//wait for whether the past MDFS process has done
	while(mctl_read_w(SDR_COM_MDFSCR)&0x1){};

	//move to CFG
	mctl_write_w(0 + SDR_SCTL, 0x1);
	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		mctl_write_w(0x1000 + SDR_SCTL, 0x1);
	while(0x1 != (mctl_read_w(0 + SDR_SSTAT) & 0x7)){}
	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		while(0x1 != (mctl_read_w(0x1000 + SDR_SSTAT) & 0x7)){}

//	reg_val = mctl_read_w(0 + SDR_MCFG);
//	reg_val &= ~(0xff<<8);
//	mctl_write_w(0 + SDR_MCFG, reg_val);
//
//	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
//	{
//		reg_val = mctl_read_w(0x1000 + SDR_MCFG);
//		reg_val &= ~(0xff<<8);
//		mctl_write_w(0x1000 + SDR_MCFG, reg_val);
//	}

	if(0 == mdfs_mode)
	{
		__u32 fatctor_n, fatctor_k, fatctor_m;

		reg_val = mctl_read_w(CCM_PLL5_DDR_CTRL);
		fatctor_n = ((reg_val>>8)& 0x1f) + 1;
		fatctor_k = ((reg_val>>4)& 0x03) + 1;
		fatctor_m = ((reg_val>>0)& 0x03) + 1;

		reg_val = 24 * fatctor_n * fatctor_k/fatctor_m/freq_div;
		mctl_write_w(0 + SDR_TOGCNT1U, reg_val);				//1us
		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
	   	{
	   		mctl_write_w(0x1000 + SDR_TOGCNT1U, reg_val);		//1us
	   	}
	   	reg_val /= 10;
		mctl_write_w(0 + SDR_TOGCNT100N, reg_val);				//100ns
		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		{
			mctl_write_w(0x1000 + SDR_TOGCNT100N, reg_val);		//100ns
		}
	}

	//move to GO
	mctl_write_w(0 + SDR_SCTL, 0x2);
	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		mctl_write_w(0x1000 + SDR_SCTL, 0x2);
	while(0x3 != (mctl_read_w(0 + SDR_SSTAT) & 0x7)){}
	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		while(0x3 != (mctl_read_w(0x1000 + SDR_SSTAT) & 0x7)){}

	//set DRAM middle & destination clock
  	if(0 == mdfs_mode)
  	{
  		reg_val = mctl_read_w(CCM_DRAMCLK_CFG_CTRL);
	  	reg_val &= ~(0x1<<16);
		reg_val &= ~(0xf<<8);
	  	reg_val |= (freq_div-1)<<8;
		mctl_write_w(CCM_DRAMCLK_CFG_CTRL, reg_val);
  	}

	//set Master enable and Ready mask
	reg_val = 0x3ff00001;
	mctl_write_w(SDR_COM_MDFSMER, reg_val);
	reg_val = 0xfffffff8;
	mctl_write_w(SDR_COM_MDFSMRMR, reg_val);

	//set MDFS timing parameter
	reg_val = 0x258;							//3us/5ns = 600
	mctl_write_w(SDR_COM_MDFSTR0, reg_val);
	reg_val = 102400*freq_div;
	tmp = (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>8)&0x1F)+1)*24;
	//tmp *= (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>4)&0x3)+1);
	//tmp /= (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>0)&0x3)+1);
	reg_val /= tmp;
	reg_val ++;
	mctl_write_w(SDR_COM_MDFSTR1, reg_val);		//512*200/sclk =
	mctl_write_w(SDR_COM_MDFSTR2, 0x80);		//fixed value : 128
//	reg_val = 0x100;
//	mctl_write_w(SDR_COM_MDFSTR3, reg_val);

	//training_mode: 0: no training  1: training
	if(0 == training_mode)
	{
		for(i=0;i<8;i++)
		{
			mctl_write_w(SDR_COM_MDFSGCR + 4*i, mdfsvalue[(freq_div-1)*8 + i]);
		}
	}

	//start hardware MDFS
	if(0 == mdfs_mode)
	{
		reg_val = mctl_read_w(SDR_COM_MDFSCR);
		reg_val >>= 2;
		reg_val &= ((0x1<<4) | (0x1<<8));
		reg_val |= 0x5 | (0x2u<<30);

		tmp = (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>8)&0x1F)+1)*24;
		tmp *= (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>4)&0x3) + 1);
		tmp /= (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>0)&0x3) + 1);

		if((tmp/freq_div) < 200)
			reg_val |= 0x1<<10;
		if((tmp/freq_div) <= 120)
			reg_val |= 0x1<<6;
		if(training_mode)
			reg_val &= ~(0x1<<2);
		mctl_write_w(SDR_COM_MDFSCR, reg_val);
	}
	else
	{
		reg_val = mctl_read_w(SDR_COM_MDFSCR);
		reg_val >>= 2;
		reg_val &= ((0x1<<4) | (0x1<<8));
		reg_val |= 0x7 | (0x2u<<30);
		if(freq_div < 200)
			reg_val |= 0x1<<10;
		if(freq_div <= 120)
			reg_val |= 0x1<<6;
		if(training_mode)
			reg_val &= ~(0x1<<2);
		mctl_write_w(SDR_COM_MDFSCR, reg_val);
	}

	//wait for whether the past MDFS process is done
	while(mctl_read_w(SDR_COM_MDFSCR)&0x1){};

	if(1 == training_mode)
	{
		//PIR training
		mctl_write_w(0 + SDR_PIR, 0x81);
		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		{
			mctl_write_w(0x1000 + SDR_PIR, 0x81);
		}

		while(0x11 != (mctl_read_w(0 + SDR_PGSR)&0x11)){};
		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		{
			while(0x11 != (mctl_read_w(0x1000 + SDR_PGSR)&0x11)){};
		}
		if(mctl_read_w(0 + SDR_PGSR) & (0x3<<5))
		{
			dram_dbg("0 + SDR_PGSR = 0x%x\n", mctl_read_w(0 + SDR_PGSR));
			//clear status bits
			reg_val = mctl_read_w(0 + SDR_PIR);
			reg_val |= 0x1<<28;
			mctl_write_w(0 + SDR_PIR, reg_val);

			err_flag = 1;
		}
		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		{
		if(mctl_read_w(0x1000 + SDR_PGSR) & (0x3<<5))
		{
			dram_dbg("0x1000 + SDR_PGSR = 0x%x\n", mctl_read_w(0x1000 + SDR_PGSR));
				//clear status bits
				reg_val = mctl_read_w(0x1000 + SDR_PIR);
				reg_val |= 0x1<<28;
				mctl_write_w(0x1000 + SDR_PIR, reg_val);

				err_flag = 1;
			}
		}
	}

	//move to CFG
	mctl_write_w(0 + SDR_SCTL, 0x1);
	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		mctl_write_w(0x1000 + SDR_SCTL, 0x1);
	while(0x1 != (mctl_read_w(0 + SDR_SSTAT) & 0x7)){}
	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		while(0x1 != (mctl_read_w(0x1000 + SDR_SSTAT) & 0x7)){}

//	reg_val = mctl_read_w(0 + SDR_MCFG);
//	reg_val |= (0x10<<8);
//	mctl_write_w(0 + SDR_MCFG, reg_val);
//
//	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
//	{
//		reg_val = mctl_read_w(0x1000 + SDR_MCFG);
//		reg_val |= (0x10<<8);
//		mctl_write_w(0x1000 + SDR_MCFG, reg_val);
//	}

	//move to GO
	mctl_write_w(0 + SDR_SCTL, 0x2);
	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		mctl_write_w(0x1000 + SDR_SCTL, 0x2);
	while(0x3 != (mctl_read_w(0 + SDR_SSTAT) & 0x7)){}
	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		while(0x03 != (mctl_read_w(0x1000 + SDR_SSTAT) & 0x7)){}

	if(!err_flag)
		return 0;
	else
		return 1;
}

int mdfs_save_value(__dram_para_t *dram_para)
{
	unsigned int reg_val;
	unsigned int max_index, i;
	unsigned int ret = 0;
	unsigned int clk_bk;
	unsigned int low_freq;

	//PLL6
	reg_val = mctl_read_w(0x01c20000 + 0x28);
	reg_val &= ~(0x1u<<31);
	mctl_write_w(0x01c20000 + 0x28, reg_val);

	reg_val = mctl_read_w(0x01f01444);
	reg_val &= ~(0xffU<<24);
	reg_val |= 0xa7U<<24;
	mctl_write_w(0x01f01444, reg_val);

	reg_val = mctl_read_w(0x01f01444);
	reg_val &= ~(0xf<<15);
	reg_val |= 0x7<<16;
	mctl_write_w(0x01f01444, reg_val);

	reg_val = mctl_read_w(0x01c20000 + 0x28);
	reg_val |= (0x1u<<31);
	mctl_write_w(0x01c20000 + 0x28, reg_val);

//#if 1		//turn off MDFS scan
if(dram_para->dram_tpr12 & 0x1)//open MDFS scan table function
{
    memset(mdfsvalue, 0, 512);

//	reg_val = (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>8)&0x1F)+1)*24;
//	reg_val *= (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>4)&0x3) + 1);
//	reg_val /= (((mctl_read_w(CCM_PLL5_DDR_CTRL)>>0)&0x3) + 1);
//	if(reg_val < 450)
//	{
//		i = 0;
//		max_index = 8;
//	}
//	else
	{
		i = 1;
		max_index = 16;
	}

	for(;i<max_index;i++)
	{
		//if((i!=1) && (i!=2) && (i!=3) && (i!=5) && (i!=7) && (i!=11) && (i!=11) && (i!=12) && (i!=15))
		//	continue;
		//setup mdfs: one step, div factor, training
			ret = mctl_mdfs_setup(0, i+1, 1);
			if(ret == 0)
				dram_para->dram_tpr12 |= (0x1<<(i+1));
			else
				dram_para->dram_tpr12 &= ~(0x1<<(i+1));
//		dram_dbg("0 + SDR_DX0DQSTR = 0x%x\n", mctl_read_w(0 + SDR_DX0DQSTR));
//		dram_dbg("0 + SDR_DX1DQSTR = 0x%x\n", mctl_read_w(0 + SDR_DX1DQSTR));
//		dram_dbg("0 + SDR_DX2DQSTR = 0x%x\n", mctl_read_w(0 + SDR_DX2DQSTR));
//		dram_dbg("0 + SDR_DX3DQSTR = 0x%x\n", mctl_read_w(0 + SDR_DX3DQSTR));
//		dram_dbg("0x1000 + SDR_DX0DQSTR = 0x%x\n", mctl_read_w(0x1000 + SDR_DX0DQSTR));
//		dram_dbg("0x1000 + SDR_DX1DQSTR = 0x%x\n", mctl_read_w(0x1000 + SDR_DX1DQSTR));
//		dram_dbg("0x1000 + SDR_DX2DQSTR = 0x%x\n", mctl_read_w(0x1000 + SDR_DX2DQSTR));
//		dram_dbg("0x1000 + SDR_DX3DQSTR = 0x%x\n", mctl_read_w(0x1000 + SDR_DX3DQSTR));

		{
		//set DQS gate value
		reg_val = (((mctl_read_w(SDR_DX0DQSTR)>>12)&0x3)<<0) 	| \
				  (((mctl_read_w(SDR_DX0DQSTR)>>0)&0x7)<<4)  	| \
				  (((mctl_read_w(SDR_DX1DQSTR)>>12)&0x3)<<8) 	| \
				  (((mctl_read_w(SDR_DX1DQSTR)>>0)&0x7)<<12) 	| \
				  (((mctl_read_w(SDR_DX2DQSTR)>>12)&0x3)<<16) 	| \
				  (((mctl_read_w(SDR_DX2DQSTR)>>0)&0x7)<<20) 	| \
				  (((mctl_read_w(SDR_DX3DQSTR)>>12)&0x3)<<24) 	| \
				  (((mctl_read_w(SDR_DX3DQSTR)>>0)&0x7)<<28);
		//mctl_write_w(SDR_COM_MDFSGCR + 4*0, reg_val);
		mdfsvalue[i * 8 + 0] = reg_val;

		reg_val = (((mctl_read_w(SDR_DX0DQSTR)>>14)&0x3)<<0) 	| \
				  (((mctl_read_w(SDR_DX0DQSTR)>>3)&0x7)<<4)  	| \
				  (((mctl_read_w(SDR_DX1DQSTR)>>14)&0x3)<<8) 	| \
				  (((mctl_read_w(SDR_DX1DQSTR)>>3)&0x7)<<12) 	| \
				  (((mctl_read_w(SDR_DX2DQSTR)>>14)&0x3)<<16) 	| \
				  (((mctl_read_w(SDR_DX2DQSTR)>>3)&0x7)<<20) 	| \
				  (((mctl_read_w(SDR_DX3DQSTR)>>14)&0x3)<<24) 	| \
				  (((mctl_read_w(SDR_DX3DQSTR)>>3)&0x7)<<28);
		//mctl_write_w(SDR_COM_MDFSGCR + 4*1, reg_val);
		mdfsvalue[i * 8 + 1] = reg_val;

		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		{
			reg_val = (((mctl_read_w(0x1000 + SDR_DX0DQSTR)>>12)&0x3)<<0) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX0DQSTR)>>0)&0x7)<<4)  	| \
					  (((mctl_read_w(0x1000 + SDR_DX1DQSTR)>>12)&0x3)<<8) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX1DQSTR)>>0)&0x7)<<12) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX2DQSTR)>>12)&0x3)<<16) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX2DQSTR)>>0)&0x7)<<20) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX3DQSTR)>>12)&0x3)<<24) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX3DQSTR)>>0)&0x7)<<28);
			//mctl_write_w(SDR_COM_MDFSGCR + 4*2, reg_val);
			mdfsvalue[i * 8 + 2] = reg_val;

			reg_val = (((mctl_read_w(0x1000 + SDR_DX0DQSTR)>>14)&0x3)<<0) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX0DQSTR)>>3)&0x7)<<4)  	| \
					  (((mctl_read_w(0x1000 + SDR_DX1DQSTR)>>14)&0x3)<<8) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX1DQSTR)>>3)&0x7)<<12) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX2DQSTR)>>14)&0x3)<<16) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX2DQSTR)>>3)&0x7)<<20) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX3DQSTR)>>14)&0x3)<<24) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX3DQSTR)>>3)&0x7)<<28);
			//mctl_write_w(SDR_COM_MDFSGCR + 4*3, reg_val);
			mdfsvalue[i * 8 + 3] = reg_val;
		}

		reg_val = (((mctl_read_w(SDR_DX0DQSTR)>>12)&0x3)<<0) 	| \
				  (((mctl_read_w(SDR_DX0DQSTR)>>0)&0x7)<<4)  	| \
				  (((mctl_read_w(SDR_DX1DQSTR)>>12)&0x3)<<8) 	| \
				  (((mctl_read_w(SDR_DX1DQSTR)>>0)&0x7)<<12) 	| \
				  (((mctl_read_w(SDR_DX2DQSTR)>>12)&0x3)<<16) 	| \
				  (((mctl_read_w(SDR_DX2DQSTR)>>0)&0x7)<<20) 	| \
				  (((mctl_read_w(SDR_DX3DQSTR)>>12)&0x3)<<24) 	| \
				  (((mctl_read_w(SDR_DX3DQSTR)>>0)&0x7)<<28);
		//mctl_write_w(SDR_COM_MDFSGCR + 4*4, reg_val);
		mdfsvalue[i * 8 + 4] = reg_val;

		reg_val = (((mctl_read_w(SDR_DX0DQSTR)>>14)&0x3)<<0) 	| \
				  (((mctl_read_w(SDR_DX0DQSTR)>>3)&0x7)<<4)  	| \
				  (((mctl_read_w(SDR_DX1DQSTR)>>14)&0x3)<<8) 	| \
				  (((mctl_read_w(SDR_DX1DQSTR)>>3)&0x7)<<12) 	| \
				  (((mctl_read_w(SDR_DX2DQSTR)>>14)&0x3)<<16) 	| \
				  (((mctl_read_w(SDR_DX2DQSTR)>>3)&0x7)<<20) 	| \
				  (((mctl_read_w(SDR_DX3DQSTR)>>14)&0x3)<<24) 	| \
				  (((mctl_read_w(SDR_DX3DQSTR)>>3)&0x7)<<28);
		//mctl_write_w(SDR_COM_MDFSGCR + 4*5, reg_val);
		mdfsvalue[i * 8 + 5] = reg_val;

		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		{
			reg_val = (((mctl_read_w(0x1000 + SDR_DX0DQSTR)>>12)&0x3)<<0) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX0DQSTR)>>0)&0x7)<<4)  	| \
					  (((mctl_read_w(0x1000 + SDR_DX1DQSTR)>>12)&0x3)<<8) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX1DQSTR)>>0)&0x7)<<12) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX2DQSTR)>>12)&0x3)<<16) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX2DQSTR)>>0)&0x7)<<20) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX3DQSTR)>>12)&0x3)<<24) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX3DQSTR)>>0)&0x7)<<28);
			//mctl_write_w(SDR_COM_MDFSGCR + 4*6, reg_val);
			mdfsvalue[i * 8 + 6] = reg_val;

			reg_val = (((mctl_read_w(0x1000 + SDR_DX0DQSTR)>>14)&0x3)<<0) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX0DQSTR)>>3)&0x7)<<4)  	| \
					  (((mctl_read_w(0x1000 + SDR_DX1DQSTR)>>14)&0x3)<<8) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX1DQSTR)>>3)&0x7)<<12) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX2DQSTR)>>14)&0x3)<<16) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX2DQSTR)>>3)&0x7)<<20) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX3DQSTR)>>14)&0x3)<<24) 	| \
					  (((mctl_read_w(0x1000 + SDR_DX3DQSTR)>>3)&0x7)<<28);
			//mctl_write_w(SDR_COM_MDFSGCR + 4*7, reg_val);
			mdfsvalue[i * 8 + 7] = reg_val;
		}

//		//set Impedance value
//		reg_val = (((mctl_read_w(SDR_ZQ0SR0)>>0)&0x1F)<<0)| \
//				  (((mctl_read_w(SDR_ZQ0SR0)>>5)&0x1F)<<5)| \
//				  (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>0)&0x1F)<<16)| \
//				  (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>5)&0x1F)<<21);
//		//mctl_write_w(SDR_COM_MDFSIVR + 4*0, reg_val);
//		mdfsvalue[i][8] = reg_val;
//
//		if(mctl_read_w(SDR_ZQ0CR1)>>8)
//		{
//			reg_val = (((mctl_read_w(SDR_ZQ0SR0)>>10)&0x1F)<<0)| \
//				 	  (((mctl_read_w(SDR_ZQ0SR0)>>20)&0x1F)<<5);
//		}else
//		{
//			reg_val = (((mctl_read_w(SDR_ZQ0SR0)>>0)&0x1F)<<0)| \
//				  	  (((mctl_read_w(SDR_ZQ0SR0)>>5)&0x1F)<<5);
//		}
//
//		if(mctl_read_w(0x1000 + SDR_ZQ0CR1)>>8)
//		{
//			reg_val |= (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>10)&0x1F)<<16)| \
//				  	   (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>20)&0x1F)<<21);
//		}else
//		{
//			reg_val |= (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>0)&0x1F)<<16)| \
//				  	   (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>5)&0x1F)<<21);
//		}
//		//mctl_write_w(SDR_COM_MDFSIVR + 4*1, reg_val);
//		mdfsvalue[i][9] = reg_val;
//
//		reg_val = (((mctl_read_w(SDR_ZQ0SR0)>>0)&0x1F)<<0)| \
//				  (((mctl_read_w(SDR_ZQ0SR0)>>5)&0x1F)<<5)| \
//				  (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>0)&0x1F)<<16)| \
//				  (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>5)&0x1F)<<21);
//		//mctl_write_w(SDR_COM_MDFSIVR + 4*2, reg_val);
//		mdfsvalue[i][10] = reg_val;
//
//		if(mctl_read_w(SDR_ZQ0CR1)>>8)
//		{
//			reg_val = (((mctl_read_w(SDR_ZQ0SR0)>>10)&0x1F)<<0)| \
//				 	  (((mctl_read_w(SDR_ZQ0SR0)>>20)&0x1F)<<5);
//		}else
//		{
//			reg_val = (((mctl_read_w(SDR_ZQ0SR0)>>0)&0x1F)<<0)| \
//				  	  (((mctl_read_w(SDR_ZQ0SR0)>>5)&0x1F)<<5);
//		}
//
//		if(mctl_read_w(0x1000 + SDR_ZQ0CR1)>>8)
//		{
//			reg_val |= (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>10)&0x1F)<<16)| \
//				  	   (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>20)&0x1F)<<21);
//		}else
//		{
//			reg_val |= (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>0)&0x1F)<<16)| \
//				  	   (((mctl_read_w(0x1000 + SDR_ZQ0SR0)>>5)&0x1F)<<21);
//		}
//		//mctl_write_w(SDR_COM_MDFSIVR + 4*3, reg_val);
//		mdfsvalue[i][11] = reg_val;
		}
	}

	//return to original freq
	mctl_mdfs_setup(0, 2, 0);
	dram_para->dram_tpr12 |= 0x1u<<31;
}//end of MDFS scan function
//#endif

//	//move to CFG
//	mctl_write_w(0 + SDR_SCTL, 0x1);
//	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
//		mctl_write_w(0x1000 + SDR_SCTL, 0x1);
//	while(0x1 != (mctl_read_w(0 + SDR_SSTAT) & 0x7)){}
//	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
//		while(0x1 != (mctl_read_w(0x1000 + SDR_SSTAT) & 0x7)){}
//
//	reg_val = mctl_read_w(0 + SDR_MCFG);
//	reg_val |= (0x10<<8);
//	mctl_write_w(0 + SDR_MCFG, reg_val);
//
//	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
//	{
//		reg_val = mctl_read_w(0x1000 + SDR_MCFG);
//		reg_val |= (0x10<<8);
//		mctl_write_w(0x1000 + SDR_MCFG, reg_val);
//	}
//
//	//move to GO
//	mctl_write_w(0 + SDR_SCTL, 0x2);
//	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
//		mctl_write_w(0x1000 + SDR_SCTL, 0x2);
//	while(0x3 != (mctl_read_w(0 + SDR_SSTAT) & 0x7)){}
//	if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
//		while(0x03 != (mctl_read_w(0x1000 + SDR_SSTAT) & 0x7)){}

	if(dram_para->dram_tpr12 & 0x2)//open soft frequency switch
	{

		//clear training value buf
		for(i=0;i<128;i++)
			mdfsvalue[i] = 0;

		//save low frequency training parameters
		mdfsvalue[0] = mctl_read_w(SDR_ZQ0SR0)&0xFFFFF;
		mdfsvalue[1] = mctl_read_w(SDR_DX0DQSTR);
		mdfsvalue[2] = mctl_read_w(SDR_DX1DQSTR);
		mdfsvalue[3] = mctl_read_w(SDR_DX2DQSTR);
		mdfsvalue[4] = mctl_read_w(SDR_DX3DQSTR);
		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		{
			mdfsvalue[5] = mctl_read_w(0x1000 + SDR_ZQ0SR0)&0xFFFFF;
			mdfsvalue[6] = mctl_read_w(0x1000 + SDR_DX0DQSTR);
			mdfsvalue[7] = mctl_read_w(0x1000 + SDR_DX1DQSTR);
			mdfsvalue[8] = mctl_read_w(0x1000 + SDR_DX2DQSTR);
			mdfsvalue[9] = mctl_read_w(0x1000 + SDR_DX3DQSTR);
		}

		//high frequency training parameters
		dram_para->dram_clk = 24*((dram_para->dram_tpr12>>24)&0x1f);
		ret = init_DRAM(0, dram_para);

		mdfsvalue[10]  = mctl_read_w(SDR_ZQ0SR0)&0xFFFFF;
		mdfsvalue[11]  = mctl_read_w(SDR_DX0DQSTR);
		mdfsvalue[12] = mctl_read_w(SDR_DX1DQSTR);
		mdfsvalue[13] = mctl_read_w(SDR_DX2DQSTR);
		mdfsvalue[14] = mctl_read_w(SDR_DX3DQSTR);
		if(mctl_read_w(SDR_COM_CR)&(0x1<<19))
		{
			mdfsvalue[15] = mctl_read_w(0x1000 + SDR_ZQ0SR0)&0xFFFFF;
			mdfsvalue[16] = mctl_read_w(0x1000 + SDR_DX0DQSTR);
			mdfsvalue[17] = mctl_read_w(0x1000 + SDR_DX1DQSTR);
			mdfsvalue[18] = mctl_read_w(0x1000 + SDR_DX2DQSTR);
			mdfsvalue[19] = mctl_read_w(0x1000 + SDR_DX3DQSTR);
		}

		dram_para->dram_tpr12 |= 0x1<<19;

	}
//	{
//		int k;
//
//		for(k=0;k<512/4;k++)
//		{
//			if(!(k & 0x07))
//			{
//				dram_dbg("\n");
//			}
//			dram_dbg("%x ", mdfsvalue[k]);
//		}
//	}

	return 0;
   // mctl_mdfs_setup(0, 1, 1);
}
