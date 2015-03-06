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
//	Allwinner Technology, All Right Reserved. 2006-2013 Copyright (c)
//
//	File: 				mctl_hal.c
//
//	Description:  This file implements basic functions for AW1671 DRAM controller
//
//	History:       2014/04/20		LHK			0.10	Initial version
//		           	 2014/04/22		LHK			0.20  add dram size scan
//			 					 2014/04/24		LHK			0.30  add master priority
//								 2014/04/26		LHK			0.40  add dram PLL	EMC function and change PLL ddr1 setting
//								 2014/05/07		LHK			0.50  add DLL-OFF for auto gate mode
//                                            add more delay at setting MC_CLKEN for two rank
//                 2014/05/08		LHK			0.60  fix master priority ,change tccd from 4 to 2
//                 2014/05/10		LHK			0.70  fix dual rank odt setting
//                 2014/05/15		LHK			0.80  add watch dog for reboot test according to A23
//                 2014/05/20		LHK			0.90  delete dll off function for Mira ddr
//                 2014/05/21		LHK			1.00  add watch dog for onther branch
//                 2014/05/26		LHK			1.10  fix rank to rank timing for miracast
//                                            add read/write priority
//                 2014/05/26		LHK			1.20  change pll-ddr0 setting
//                 2014/06/12		LHK			1.30  change pll-ddr1/pll-ddr0 setting for black screen and sscg
//                                            fix dram_para1
//                 2014/07/4		LHK			1.40  change pll-ddr1 setting for stability
//                                            add support 1*8 DDR mode
//                 2014/08/12		LHK			1.50  enlarge tXS from 0x8 to 0x10 for black screen music
//                 2014/09/15		LHK			1.60  change mctl_sys_init for uboot-fail-efex
//****************************************************************************************************

#include "mctl_hal.h"
#include "mctl_reg.h"
#include "mctl_par.h"

//***********************************************************************************************
//	void local_delay (unsigned int n)
//
//  Description:	delay n time
//
//	Return Value:	None
//***********************************************************************************************
void local_delay (unsigned int n)
{
	//while(n--);
	__msdelay(n);

}
void paraconfig(unsigned int *para, unsigned int mask, unsigned int value)
{
	*para &= ~(mask);
	*para |= value;
}
static void watchdog_disable(void)
{
	/* disable watchdog */
	mctl_write_w(0,0x1c20C00 + 0xB8);

	return ;
}

static void watchdog_enable(void)
{
	/* enable watchdog */
	mctl_write_w(1,0x1c20C00 + 0xB4);
	mctl_write_w(1,0x1c20C00 + 0xB8);

	return ;
}
//*****************************************************************************
//	void set_master_priority(void)
//  Description:		 set master priority and bandwidth limit
//  master  0 :  CPU               master  8  :    CPUS
//  master  1 :  GPU               master  9  :    USB0
//  master  2 :  DMA               master  10 :    MSTG0
//  master  3 :   VE               master  11 :    USB1
//  master  4 :   BE               master  12 :    MSTG2
//  master  5 :   FE               master  13 :    NAND
//  master  6 :  CSI               master  14 :    TESTAHB
//  master  7 :  IEP               master  15 :    MSTG1
//*****************************************************************************
void set_master_priority(void)
{
	//set dram master access priority
	mctl_write_w(0x0,MC_MAPR);
	//set SCHED
	mctl_write_w(0x0f802f01,SCHED);
	dram_dbg("DRAM master priority setting ok.\n");
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
	unsigned int  ctrl_freq;//half speed mode :ctrl_freq=1/2 ddr_fre
	unsigned int  type;
	unsigned int  reg_val        =0;
	unsigned int  tdinit0       = 0;
	unsigned int  tdinit1       = 0;
	unsigned int  tdinit2       = 0;
	unsigned int  tdinit3       = 0;
	unsigned char t_rdata_en    = 1;    //ptimg0
	unsigned char wr_latency    = 1;	//ptimg0
	unsigned char tcl 			= 3;	//6
	unsigned char tcwl			= 3;	//6
	unsigned char tmrw			= 0;	//0
	unsigned char tmrd			= 2;	//4;
	unsigned char tmod			= 6;	//12;
	unsigned char tccd			= 2;	//4;
	unsigned char tcke			= 2;	//3;
	unsigned char trrd			= 3;	//6;
	unsigned char trcd			= 6;	//11;
	unsigned char trc			= 20;	//39;
	unsigned char tfaw			= 16;	//32;
	unsigned char tras			= 14;	//28;
	unsigned char trp			= 6;	//11;
	unsigned char twtr			= 3;	//6;
	unsigned char twr			= 8;	//15；
	unsigned char trtp			= 3;	//6;
	unsigned char txp			= 10;	//20;
	unsigned short trefi		= 98;	//195;
	unsigned short trfc		    = 128;
	unsigned char twtp			= 12;	//24;	//write to pre_charge
	unsigned char trasmax		= 27;	//54;	//54*1024ck
	unsigned char twr2rd		= 8;	//16;
	unsigned char trd2wr		= 4;	//7;
	unsigned char tckesr		= 3;	//5;
	unsigned char tcksrx		= 4;	//8;
	unsigned char tcksre		= 4;	//8;
	ctrl_freq = para->dram_clk/2;	//Controller work in half rate mode
	type      = para->dram_type;
	//add the time user define
	if(para->dram_tpr13&0x2)
	{
		dram_dbg("User define timing parameter!\n");
		//dram_tpr0
		tccd = ( (para->dram_tpr0 >> 21) & 0x7  );//[23:21]
		tfaw = ( (para->dram_tpr0 >> 15) & 0x3f );//[20:15]
		trrd = ( (para->dram_tpr0 >> 11) & 0xf  );//[14:11]
		trcd = ( (para->dram_tpr0 >>  6) & 0x1f );//[10:6 ]
		trc  = ( (para->dram_tpr0 >>  0) & 0x3f );//[ 5:0 ]
		//dram_tpr1
		txp =  ( (para->dram_tpr1 >> 23) & 0x1f );//[27:23]
		twtr = ( (para->dram_tpr1 >> 20) & 0x7  );//[22:20]
		trtp = ( (para->dram_tpr1 >> 15) & 0x1f );//[19:15]
		twr =  ( (para->dram_tpr1 >> 11) & 0xf  );//[14:11]
		trp =  ( (para->dram_tpr1 >>  6) & 0x1f );//[10:6 ]
		tras = ( (para->dram_tpr1 >>  0) & 0x3f );//[ 5:0 ]
		//dram_tpr2
		trfc  = ( (para->dram_tpr2 >> 12)& 0x1ff);//[20:12]
		trefi = ( (para->dram_tpr2 >> 0) & 0xfff);//[11:0 ]
	}//add finish
	else
	{
		dram_dbg("Auto calculate timing parameter!\n");
		if(type==3)
		{
			//dram_tpr0
			tccd=2;
			tfaw= (50*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns;
			trrd=(10*ctrl_freq)/1000 + ( ( ((10*ctrl_freq)%1000) != 0) ? 1 :0);
			if(trrd<4) trrd=4;	//max(4ck,10ns)
			trcd= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			trc	= (53*ctrl_freq)/1000 + ( ( ((50*ctrl_freq)%1000) != 0) ? 1 :0);	//50ns
			//dram_tpr1
			txp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(txp<3) txp = 3;//max(3ck,7.5ns)
			twtr= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(twtr<4) twtr=4;	//max(4ck,7,5ns)
			trtp	= (8*ctrl_freq)/1000 + ( ( ((8*ctrl_freq)%1000) != 0) ? 1 :0);	//7.5ns;
			if(trtp<4) trtp=4;	//max(4ck,7.5ns)
			twr= (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);	//15ns;
			if(twr<3) twr=3;
			trp = (15*ctrl_freq)/1000 + ( ( ((15*ctrl_freq)%1000) != 0) ? 1 :0);//15ns(10ns)
			tras= (38*ctrl_freq)/1000 + ( ( ((35*ctrl_freq)%1000) != 0) ? 1 :0);	//38ns;
			//dram_tpr2
			trefi	= ( (7800*ctrl_freq)/1000 + ( ( ((7800*ctrl_freq)%1000) != 0) ? 1 :0) )/32;//7800ns
			trfc = (350*ctrl_freq)/1000 + ( ( ((350*ctrl_freq)%1000) != 0) ? 1 :0);	//350ns;
		}
		//assign the value back to the DRAM structure
		para->dram_tpr0 = (trc<<0) | (trcd<<6) | (trrd<<11) | (tfaw<<15) | (tccd<<21) ;
		dram_dbg("para_dram_tpr0 = %x\n",para->dram_tpr0);
		para->dram_tpr1 = (tras<<0) | (trp<<6) | (twr<<11) | (trtp<<15) | (twtr<<20)|(txp<<23);
		dram_dbg("para_dram_tpr1 = %x\n",para->dram_tpr1);
		para->dram_tpr2 = (trefi<<0) | (trfc<<12);
		dram_dbg("para_dram_tpr2 = %x\n",para->dram_tpr2);
	}
	switch(type)
	{
	case 3://DDR3
		//the time we no need to calculate
		tmrw=0x0;
		tmrd=0x4;
		tmod=0xc;
		tcke=3;
		tcksrx=5;
		tcksre=5;
		tckesr=4;
		trasmax =0x18;

			tcl		= 6;	//CL   12
			tcwl	= 4;	//CWL  8
			t_rdata_en  =4;
			wr_latency  =2;
			para->dram_mr0 	= 0x1c70;//CL=11,WR=12
			para->dram_mr2  = 0x18; //CWL=8
/*
		if(para->dram_clk <= 400)
		{
			tcl		= 3;	//CL=6
			tcwl	= 3;	//CWL=5
			t_rdata_en  =1; //(CL-4(3))/2
			wr_latency  =1;
			para->dram_mr0 	= 0x1420;//CL=6,WR=6
			para->dram_mr2     = 0;    //CWL=5
		}
		else if(para->dram_clk <= 533)
		{
			tcl		= 4;	//CL   8
			tcwl	= 3;	//CWL  6
			t_rdata_en  =2;
			wr_latency  =1;
			para->dram_mr0 	= 0x1840;//CL=8,WR=8
			para->dram_mr2     = 8;	//CWL=6
		}
		else if(para->dram_clk <= 667)
		{
			tcl		= 5;	//CL    10
			tcwl	= 4;	//CWL   7
			t_rdata_en  =3;
			wr_latency  =2;
			para->dram_mr0 	= 0x1a60;//CL=10,WR=10
			para->dram_mr2  = 0x10; //CWL=7
		}
		else if(para->dram_clk <= 800)
		{
			tcl		= 6;	//CL   12
			tcwl	= 4;	//CWL  8
			t_rdata_en  =4;
			wr_latency  =2;
			para->dram_mr0 	= 0x1c70;//CL=11,WR=12
			para->dram_mr2  = 0x18; //CWL=8
		}
*/
		tdinit0	= (500*para->dram_clk) + 1;	//500us
		tdinit1	= (360*para->dram_clk)/1000 + 1;//360ns
		tdinit2	= (200*para->dram_clk) + 1;	//200us
		tdinit3	= (1*para->dram_clk) + 1;	//1us
		twtp=tcwl+2+twr;//WL+BL/2+tWR
		twr2rd= tcwl+2+twtr;//WL+BL/2+tWTR
		trd2wr= tcl+2+1-tcwl;//RL+BL/2+2-WL
		dram_dbg("tcl = %d,tcwl = %d\n",tcl,tcwl);
		break;
	default:
		break;
	}

	//set work mode register
	reg_val=mctl_read_w(MC_WORK_MODE);
	reg_val &=~((0xfff<<12)|(0xf<<0));
	reg_val |=(0x4<<20);
	reg_val |= ((para->dram_type & 0x07)<<16);//DRAM type
	reg_val |= (( ( (para->dram_para2) & 0x01 )? 0x0:0x1) << 12);	//DQ width
	reg_val |= ( (para->dram_para2)>>12 & 0x03 );	//rank
	reg_val |= ((((para->dram_para1)>>28) & 0x01) << 2);//BANK
	reg_val |= (((para->dram_tpr13>>5)&0x1)<<19);//2T or 1T
	reg_val |= (( (para->dram_para2)>>4 &0x1 )<<24);	//CS1 control
	mctl_write_w(reg_val,MC_WORK_MODE);
	//set mode register
	mctl_write_w((para->dram_mr0),DRAM_MR0);
	mctl_write_w((para->dram_mr1),DRAM_MR1);
	mctl_write_w((para->dram_mr2),DRAM_MR2);
	mctl_write_w((para->dram_mr3),DRAM_MR3);
	//set dram timing
	reg_val= (twtp<<24)|(tfaw<<16)|(trasmax<<8)|(tras<<0);
	dram_dbg("DRAM TIMING PARA0 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG0);//DRAMTMG0
	reg_val= (txp<<16)|(trtp<<8)|(trc<<0);
	dram_dbg("DRAM TIMING PARA1 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG1);//DRAMTMG1
	reg_val= (tcwl<<24)|(tcl<<16)|(trd2wr<<8)|(twr2rd<<0);
	dram_dbg("DRAM TIMING PARA2 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG2);//DRAMTMG2
	reg_val= (tmrw<<16)|(tmrd<<12)|(tmod<<0);
	dram_dbg("DRAM TIMING PARA3 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG3);//DRAMTMG3
	reg_val= (trcd<<24)|(tccd<<16)|(trrd<<8)|(trp<<0);
	dram_dbg("DRAM TIMING PARA4 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG4);//DRAMTMG4
	reg_val= (tcksrx<<24)|(tcksre<<16)|(tckesr<<8)|(tcke<<0);
	dram_dbg("DRAM TIMING PARA5 = %x\n",reg_val);
	mctl_write_w(reg_val,DRAMTMG5);//DRAMTMG5
	//set two rank timing and exit self-refresh timing
	reg_val= mctl_read_w(DRAMTMG8);
	reg_val&=~(0xff<<8);
	reg_val&=~(0xff<<0);
	reg_val|=(0x33<<8);
	reg_val|=(0x10<<0);
	mctl_write_w(reg_val,DRAMTMG8);//DRAMTMG8
	dram_dbg("DRAM TIMING PARA8 = %x\n",reg_val);
	//set phy interface time
	reg_val=(0x2<<24)|(t_rdata_en<<16)|(0x1<<8)|(wr_latency<<0);
	dram_dbg("DRAM PHY INTERFACE PARA = %x\n",reg_val);
	mctl_write_w(reg_val,PITMG0);	//PHY interface write latency and read latency configure
	//set phy time  PTR0-2 use default
	mctl_write_w(((tdinit0<<0)|(tdinit1<<20)),PTR3);
	mctl_write_w(((tdinit2<<0)|(tdinit3<<20)),PTR4);
//	mctl_write_w(0x01e007c3,PTR0);
//	mctl_write_w(0x00170023,PTR1);
//	mctl_write_w(0x00800800,PTR3);
//	mctl_write_w(0x01000500,PTR4);
	//set refresh timing
    reg_val =(trefi<<16)|(trfc<<0);
    mctl_write_w(reg_val,RFSHTMG);
}
//***********************************************************************************************
//	unsigned int ccm_set_pll_ddr_clk(u32 pll_clk)
//
//  Description:	set-pll-ddr0-clk
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
unsigned int ccm_set_pll_ddr_clk(u32 pll_clk)
{
	unsigned int n, k, m = 1,rval;
	unsigned int div;
	unsigned int mod2, mod3;
	unsigned int min_mod = 0;

	div = pll_clk/24;
	k=2;
	m=2;
	n = div;

	rval = mctl_read_w(_CCM_PLL_DDR0_REG);
	rval &= ~((0x1f << 8) | (0x3 << 4) | (0x3 << 0));
	rval = (1U << 31)  | ((n-1) << 8) | ((k-1) << 4) | (m-1);
	mctl_write_w(rval, _CCM_PLL_DDR0_REG);
	mctl_write_w(rval|(1U << 20), _CCM_PLL_DDR0_REG);
	local_delay(20);
	return (24 * n * k / m);
}
//***********************************************************************************************
//	unsigned int ccm_set_pll_ddr_clk(u32 pll_clk)
//
//  Description:	set-pll-ddr1-clk
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
unsigned int ccm_set_pll_ddr1_clk(u32 pll_clk)
{
	u32 rval;
	u32 div;

	div = pll_clk/24;
	if (div < 12) //no less than 288M
	{
		div=12;
	}
	rval = mctl_read_w(_CCM_PLL_DDR1_REG);
	rval &= ~(0x3f<<8);
	rval |=(((div-1)<<8)|(0x1U<<31));
	mctl_write_w(rval, _CCM_PLL_DDR1_REG);
	mctl_write_w(rval|(1U << 30), _CCM_PLL_DDR1_REG);
	local_delay(1);
	return 24*div;
}
//***********************************************************************************************
//	unsigned int mctl_sys_init(__dram_para_t *para)
//
//  Description:	set pll and dram clk
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
unsigned int mctl_sys_init(__dram_para_t *para)
{
#ifndef FPGA_PLATFORM
	unsigned int reg_val = 0;
	unsigned int ret_val = 0;
	unsigned int pll_div = 0;
	//controller reset
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val &= ~(0x1U<<31);
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	//trun off mbus clk gate
	reg_val = mctl_read_w(MBUS_CLK_CTL_REG);
	reg_val &=~(1U<<31);
	writel(reg_val, MBUS_CLK_CTL_REG);
	//mbus reset
	reg_val = mctl_read_w(MBUS_RESET_REG);
	reg_val &=~(1U<<31);
	writel(reg_val, MBUS_RESET_REG);
	// close AHB BUS gating
	reg_val = mctl_read_w(BUS_CLK_GATE_REG0);
	reg_val &= ~(1U<<14);
	writel(reg_val, BUS_CLK_GATE_REG0);
	//DRAM BUS reset
	reg_val = mctl_read_w(BUS_RST_REG0);
	reg_val &= ~(1U<<14);
	writel(reg_val, BUS_RST_REG0);
	//disable pll-ddr0
	reg_val = mctl_read_w(_CCM_PLL_DDR0_REG);
	reg_val &=~(1U<<31);
	writel(reg_val, _CCM_PLL_DDR0_REG);
	//disable pll-ddr1
	reg_val = mctl_read_w(_CCM_PLL_DDR1_REG);
	reg_val &=~(1U<<31);
	writel(reg_val, _CCM_PLL_DDR1_REG);
	//add dram pll EMC function
	//set pll-ddr0/pll-ddr1
	if((para->dram_tpr13>>8)&0x1)//pll-ddr1
	{
		dram_dbg("USE PLL DDR1\n");
		reg_val = mctl_read_w(_CCM_PLL_DDR_CFG_REG);
		reg_val |= (0x1<<16);
		mctl_write_w(reg_val,_CCM_PLL_DDR_CFG_REG);
		local_delay(1);
		if((para->dram_tpr8&0x1) && (para->dram_clk<=240))
		{
			ret_val=ccm_set_pll_ddr1_clk(para->dram_clk<<2);
			pll_div = 2 ;
			dram_dbg("USE PLL BYPASS\n");
		}
		else
		{
			ret_val=ccm_set_pll_ddr1_clk(para->dram_clk<<1);
			pll_div = 4;
			dram_dbg("USE PLL NORMAL\n");
		}
		dram_dbg("PLL FREQUENCE = %d MHZ\n",ret_val);

	}
	else//pll-ddr0
	{
		dram_dbg("USE PLL DDR0\n");
		reg_val = mctl_read_w(_CCM_PLL_DDR_CFG_REG);
		reg_val &= ~(0x1<<16);
		mctl_write_w(reg_val,_CCM_PLL_DDR_CFG_REG);
		local_delay(1);
		if((para->dram_tpr8&0x1) && (para->dram_clk<=240))
		{
			ret_val=ccm_set_pll_ddr_clk(para->dram_clk<<2);
			pll_div = 2 ;
			dram_dbg("USE PLL BYPASS\n");
		}
		else
		{
			ret_val=ccm_set_pll_ddr_clk(para->dram_clk);
			pll_div = 2 ;
			dram_dbg("USE PLL NORMAL\n");
		}
		dram_dbg("PLL FREQUENCE = %d MHZ\n",ret_val);
	}
	local_delay(10);
	//dram pll EMC function
	if((para->dram_tpr13>>16)&0x3f)
	{
		//********************************************
		//bit16 : 겠류0.5
		//bit17 : 겠류0.45
		//bit18 : 겠류0.4
		//bit19 : 겠류0.3
		//bit20 : 겠류0.2
		//bit21 : 겠류0.1
		//**********************************************
		if((para->dram_tpr13>>8)&0x1)
		{
				dram_dbg("DRAM PLL DDR1 frequency extend open !\n");
				if(para->dram_tpr13 & (0x1<<16))
				{

					mctl_write_w(((0x3U<<17)|(0x158U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR1_PATTERN_REG);
				}
				if(para->dram_tpr13 & (0x1<<17))
				{
					mctl_write_w((0x1999U|(0x3U<<17)|(0x135U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR1_PATTERN_REG);
				}
				if(para->dram_tpr13 & (0x1<<18))
				{
					mctl_write_w((0x3333U|(0x3<<17)|(0x120U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR1_PATTERN_REG);
				}
				else if(para->dram_tpr13 & (0x1<<19))//------------------------------------------------------------------------------------------------------------------------------------------------------------
				{
					mctl_write_w((0x6666U|(0x3U<<17)|(0xD8U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR1_PATTERN_REG);
				}
				else if(para->dram_tpr13 & (0x1<<20))//------------------------------------------------------------------------------------------------------------------------------------------------------------
				{
					mctl_write_w((0x9999U|(0x3U<<17)|(0x90U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR1_PATTERN_REG);
				}
				else if(para->dram_tpr13 & (0x1<<21))//------------------------------------------------------------------------------------------------------------------------------------------------------------
				{
					mctl_write_w((0xccccU|(0x3U<<17)|(0x48U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR1_PATTERN_REG);
				}
				reg_val = mctl_read_w(_CCM_PLL_DDR1_REG);
				reg_val |=((0x1U<<24)|(0x1U<<30));
				mctl_write_w(reg_val,_CCM_PLL_DDR1_REG);
				while(mctl_read_w(_CCM_PLL_DDR1_REG) & (0x1<<30));
		}else
		{
				dram_dbg("DRAM PLL DDR0 frequency extend open !\n");
				if(para->dram_tpr13 & (0x1<<18))
				{
					mctl_write_w((0x3333U|(0x3<<17)|(0x120U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR0_PATTERN_REG);
				}
				else if(para->dram_tpr13 & (0x1<<19))//------------------------------------------------------------------------------------------------------------------------------------------------------------
				{
					mctl_write_w((0x6666U|(0x3U<<17)|(0xD8U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR0_PATTERN_REG);
				}
				else if(para->dram_tpr13 & (0x1<<20))//------------------------------------------------------------------------------------------------------------------------------------------------------------
				{
					mctl_write_w((0x9999U|(0x3U<<17)|(0x90U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR0_PATTERN_REG);
				}
				else if(para->dram_tpr13 & (0x1<<21))//------------------------------------------------------------------------------------------------------------------------------------------------------------
				{
					mctl_write_w((0xccccU|(0x3U<<17)|(0x48U<<20)|(0x3U<<29)|(0x1U<<31)),_CCM_PLL_DDR0_PATTERN_REG);
				}
				reg_val = mctl_read_w(_CCM_PLL_DDR0_REG);
				reg_val |=((0x1U<<24)|(0x1U<<20));
				mctl_write_w(reg_val,_CCM_PLL_DDR0_REG);
				while(mctl_read_w(_CCM_PLL_DDR0_REG) & (0x1<<20));
		}

	}
	//Setup DRAM divider ,default M=2
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val &= ~(0xf<<0);
  reg_val |= ((pll_div-1)<<0);
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	reg_val |= (0x1<<16);//updata
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	while(mctl_read_w(_CCM_DRAMCLK_CFG_REG) & (0x1<<16));
	//release controller reset
	reg_val = mctl_read_w(_CCM_DRAMCLK_CFG_REG);
	reg_val |=(0x1U<<31);
	mctl_write_w(reg_val,_CCM_DRAMCLK_CFG_REG);
	//release DRAM ahb BUS RESET
	reg_val = mctl_read_w(BUS_RST_REG0);
	reg_val |= (1U<<14);
	writel(reg_val, BUS_RST_REG0);
	//open AHB gating
	reg_val = mctl_read_w(BUS_CLK_GATE_REG0);
	reg_val |= (1U<<14);
	writel(reg_val, BUS_CLK_GATE_REG0);
	//release DRAM mbus RESET
	reg_val = mctl_read_w(MBUS_RESET_REG);
	reg_val |=(1U<<31);
	writel(reg_val, MBUS_RESET_REG);
	//open mbus gating
	reg_val = mctl_read_w(MBUS_CLK_CTL_REG);
	reg_val |=(1U<<31);
	writel(reg_val, MBUS_CLK_CTL_REG);
	//enable dramc clk
	set_master_priority();
#endif
	if((para->dram_tpr8&0x1) && (para->dram_clk<=240))
		mctl_write_w(0x0000c10f,MC_CLKEN);  //bypass
	else
		mctl_write_w(0x0000400f,MC_CLKEN);	//normal
	local_delay(10);
	return (1);
}
//***********************************************************************************************
//	void mctl_com_init(__dram_para_t *para)
//
//  Description:	set ddr para and enable clk
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
void mctl_com_init(__dram_para_t *para)
{
	unsigned int reg_val;
	reg_val = 0;
	reg_val|=(0x4<<20);
	reg_val |= ((para->dram_type & 0x07)<<16);//DRAM type
	reg_val |= ((((para->dram_para1)>>28) & 0x01) << 2);//BANK
	reg_val |= ((( ( ((para->dram_para1)>>20) & 0xff) - 1) & 0xf) << 4);//Row number
	reg_val |= (( ( (para->dram_para2) & 0x01 )? 0x0:0x1) << 12);	//DQ width
	reg_val |= ( (para->dram_para2)>>12 & 0x03 );	//rank
	reg_val |= (((para->dram_tpr13>>5)&0x1)<<19);//2T or 1T
	reg_val |= (( (para->dram_para2)>>4 &0x1 )<<24);	//CS1 control
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
		break;
	}
	mctl_write_w(reg_val,MC_WORK_MODE);
	dram_dbg("DRAM work mode register value = %x\n",reg_val);
	//enable master software clk
	reg_val = mctl_read_w(MC_SWONR);
	reg_val |= 0x3FFFF;
	mctl_write_w(reg_val,MC_SWONR);
}
//***********************************************************************************************
//	unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
//
//  Description:	init setting include different timing ,MR register,phy setting...
//
//	Arguments:		DRAM parameter
//
//	Return Value:	None
//***********************************************************************************************
#ifndef FPGA_PLATFORM
unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
{
	unsigned int reg_val = 0;
	unsigned int ret_val = 0;
	unsigned int dqs_gating_mode =0;
	dqs_gating_mode = (para->dram_tpr13>>2)&0x7;
	auto_set_timing_para(para);
//****************************************************************************************************//
//setting VTC
	if((para->dram_clk>400)&&((para->dram_tpr13>>9)&0x1))
	{
		dram_dbg("DRAM VTC is enable\n");
		reg_val=mctl_read_w(PGCR0);
		reg_val |=(0x3f<<0);
		mctl_write_w(reg_val,PGCR0);
	}else
	{
		dram_dbg("DRAM VTC is disable\n");
		reg_val=mctl_read_w(PGCR0);
		reg_val &=~(0x3f<<0);
		mctl_write_w(reg_val,PGCR0);
	}
//****************************************************************************************************//
//setting ODT
	if((para->dram_clk>400)&& para->dram_odt_en)
	{
		dram_dbg("DRAM dynamic DQS/DQ ODT is on\n");
		reg_val=mctl_read_w(DXnGCR0(0));
		reg_val |= (0x3<<9);
		mctl_write_w(reg_val,DXnGCR0(0));

		reg_val=mctl_read_w(DXnGCR0(1));
		reg_val |= (0x3<<9);
		mctl_write_w(reg_val,DXnGCR0(1));

	}else
	{
		dram_dbg("DRAM DQS/DQ ODT is off\n");
		reg_val=mctl_read_w(DXnGCR0(0));
		reg_val &= ~(0x3<<9);
		mctl_write_w(reg_val,DXnGCR0(0));

		reg_val=mctl_read_w(DXnGCR0(1));
		reg_val &= ~(0x3<<9);
		mctl_write_w(reg_val,DXnGCR0(1));
	}
//****************************************************************************************************//
//setting PLL configuration
	reg_val = mctl_read_w(PLLGCR);
	if((para->dram_tpr8&0x1)==0)
	{
		if(para->dram_clk >= 480)
			reg_val |= (0x1<<18);
		else
			reg_val |= (0x3<<18);
	}else
	{
		reg_val |= ((0x1U<<29)|(0x1U<<31));
		dram_dbg("DRAM PLL POWER DOWN AND BYPASS.\n");
	}
	mctl_write_w(reg_val, PLLGCR);
//****************************************************************************************************//
//auto detect dram size setting.In this mode ,make sure set 2 rank and full DQ
	if(!(para->dram_tpr13&0x1))
	{
		dram_dbg("DRAM SIZE AUTO SCAN ENABLE.\n");
		reg_val = mctl_read_w(MC_WORK_MODE);
		reg_val |=((0x1<<12)|(0x1<<0));
		mctl_write_w(reg_val,MC_WORK_MODE);
		dram_dbg("NOW,the work mode register value is %x\n",reg_val);
		dqs_gating_mode = 1;
	}
//****************************************************************************************************//
//setting dqs gating mode :3 modes
	switch(dqs_gating_mode)
		{
			case 0://auto gate PD
				reg_val=mctl_read_w(PGCR2);
				reg_val|=(0x3<<6);
				mctl_write_w(reg_val,PGCR2);
				dram_dbg("DRAM DQS gate is PD mode.\n");
				break;
			case 1://open DQS gating
				reg_val=mctl_read_w(PGCR2);
				reg_val&=~(0x3<<6);
				mctl_write_w(reg_val,PGCR2);

				reg_val=mctl_read_w(DQSGMR);
				reg_val&=~((0x1<<8)|(0x7));
				mctl_write_w(reg_val,DQSGMR);
				dram_dbg("DRAM DQS gate is open.\n");
				break;
			case 2://auto gating PU
				reg_val=mctl_read_w(PGCR2);
				reg_val&=~(0x3<<6);
				reg_val|=(0x2<<6);
				mctl_write_w(reg_val,PGCR2);

				ret_val =((mctl_read_w(DRAMTMG2)>>16)&0x1f)-2;
				reg_val=mctl_read_w(DQSGMR);
				reg_val&=~((0x1<<8)|(0x7));
				reg_val|=((0x1<<8)|(ret_val));
				mctl_write_w(reg_val,DQSGMR);
				dram_dbg("DRAM DQSGMR value is %x\n",reg_val);

				reg_val =mctl_read_w(DXCCR);
				reg_val |=(0x1<<8);
				reg_val &=~(0x1<<12);
				mctl_write_w(reg_val,DXCCR);
				dram_dbg("DRAM DQS gate is PU mode.\n");
				break;
			default:
				break;
		}
//****************************************************************************************************//
//setting half DQ
	if((para->dram_para2)&0x01)
	{
		mctl_write_w(0x0,DXnGCR0(1));	//disable high DQ
		dram_dbg("DRAM half DQ mode,disable high DQ\n");
	}
//****************************************************************************************************//
//data training configuration
	reg_val = (mctl_read_w(MC_WORK_MODE)&0x1);
	if(reg_val)
	{
		reg_val = mctl_read_w(DTCR);
		reg_val |= (0x3<<24);
		mctl_write_w(reg_val,DTCR);  //two rank
		dram_dbg("DRAM two rank training is on,the value is %x\n",reg_val);
	}
	else
	{
		reg_val = mctl_read_w(DTCR);
		reg_val &= ~(0x3<<24);
		reg_val |= (0x1<<24);
		mctl_write_w(reg_val,DTCR);  //one rank
		dram_dbg("DRAM one rank training is on,the value is %x\n",reg_val);
	}
//****************************************************************************************************//
//zq calibration/phy init/dram init/DDL calibration/DQS gate training
	//set zq para   CA
	reg_val = ((para->dram_zq) & 0xff );
	mctl_write_w(reg_val,ZQCR1);
	//CA calibration
	mctl_write_w(0x3,PIR);
	local_delay(1);
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);
	reg_val = mctl_read_w(ZQSR0);
	reg_val &= 0x0fffffff;
	reg_val |= (1<<28);
	mctl_write_w(reg_val,ZQCR2);
	//set zq para   DX
	reg_val = ((para->dram_zq>>8) & 0xff );
	mctl_write_w(reg_val,ZQCR1);
	//accord to DQS gating mode to choose value of pir  default:1f3
	if(((para->dram_tpr8&0x1)==0) || (para->dram_clk>240))
		reg_val=0x1f3;
	else
		reg_val=0x201f3;
	if(dqs_gating_mode==1)
		reg_val|=(1<<10);  //DQS gating training
	mctl_write_w(reg_val,PIR);
	local_delay(1);
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);
//*****************************************************************************************************//
//training information
	reg_val = mctl_read_w(PGSR0);
	if((reg_val>>20)&0xff)
	{
		dram_dbg("DRAM training error,PGSR0 = %x\n",reg_val);
		reg_val = mctl_read_w(DXnGSR0(0));
		dram_dbg("DRAM low 8  DQs information,DXnGSR = %x\n",reg_val);
		reg_val = (reg_val>>24)&0x3;
		ret_val = mctl_read_w(DXnGSR0(1));
		dram_dbg("DRAM high 8 DQs information,DXnGSR = %x\n",ret_val);
		ret_val = (ret_val>>24)&0x3;
		if(!(para->dram_tpr13&0x1)&&(reg_val!=0x3))
		{
			if(reg_val==ret_val)
			{
				dram_dbg("DRAM do not have two rank,NOW set one rank\n");
				reg_val = mctl_read_w(MC_WORK_MODE);
				reg_val &=~(0x1<<0);
				mctl_write_w(reg_val,MC_WORK_MODE);
				para->dram_para2 &=~(0xf<<12);
			}
			else
			{
				if(!(reg_val&ret_val))
				{
					dram_dbg("DRAM may be 2*8 \n");
					reg_val = mctl_read_w(MC_WORK_MODE);
					reg_val |= (0x1<<24);
					reg_val &=~(0x1<<0);
					reg_val |=(0x1<<12);
					mctl_write_w(reg_val,MC_WORK_MODE);
					//single training setting
					reg_val = mctl_read_w(DTCR);
					reg_val &= ~(0x3<<24);
					reg_val |= (0x1<<24);
					mctl_write_w(reg_val,DTCR);
					//re-training
					if(((para->dram_tpr8&0x1)==0) || (para->dram_clk>240))
						reg_val=0x1f3;
					else
						reg_val=0x201f3;
					if(dqs_gating_mode==1)
						reg_val|=(1<<10);  //DQS gating training
					mctl_write_w(reg_val,PIR);
					local_delay(1);
					while((mctl_read_w(PGSR0 )&0x1) != 0x1);
					reg_val = mctl_read_w(PGSR0);
					if((reg_val>>20)&0xff)
					{
							dram_dbg("8*2 DRAM ERROR,try 8*1\n");
							//disable high DQ
					     mctl_write_w(0x0,DXnGCR0(1));
					     reg_val = mctl_read_w(MC_WORK_MODE);
					     reg_val &= ~(0x1<<24);
					     reg_val &=~(0x1<<0);
					     reg_val &=~(0x7<<12);
					     mctl_write_w(reg_val,MC_WORK_MODE);
					     //single training setting
					     reg_val = mctl_read_w(DTCR);
					     reg_val &= ~(0x3<<24);
					     reg_val |= (0x1<<24);
					     mctl_write_w(reg_val,DTCR);  //one rank
					     //re-training
					     if(((para->dram_tpr8&0x1)==0) || (para->dram_clk>240))
					     	reg_val=0x1f3;
					     else
					     	reg_val=0x201f3;
					     if(dqs_gating_mode==1)
					     	reg_val|=(1<<10);  //DQS gating training
					     mctl_write_w(reg_val,PIR);
					     local_delay(1);
					     while((mctl_read_w(PGSR0 )&0x1) != 0x1);
					     reg_val = mctl_read_w(PGSR0);
					     if((reg_val>>20)&0xff)
					     {
					     	dram_dbg("DRAM ERROR\n");
					     }else
					     {
					     	dram_dbg("DRAM 1*8 training OK\n");
					     	para->dram_para2 &= ~(0x1<<12);
					      para->dram_para2 &=~(0xf<<0);
					      para->dram_para2 |= (0x1<<0);
					     }

					}else
					{
						dram_dbg("DRAM 2*8 training OK\n");
						para->dram_para2 &= ~(0x1<<12);
						para->dram_para2 &=~(0xf<<0);
						para->dram_para2 |=(0x1<<4);
					}
				}
				else
				{
					dram_dbg("DRAM may be 1*8 \n");
					//disable high DQ
					mctl_write_w(0x0,DXnGCR0(1));
					reg_val = mctl_read_w(MC_WORK_MODE);
					reg_val &= ~(0x1<<24);
					reg_val &=~(0x1<<0);
					reg_val &=~(0x7<<12);
					mctl_write_w(reg_val,MC_WORK_MODE);
					para->dram_para2 &= ~(0x1<<12);
					para->dram_para2 &=~(0xf<<0);
					para->dram_para2 |= (0x1<<0);
					//single training setting
					reg_val = mctl_read_w(DTCR);
					reg_val &= ~(0x3<<24);
					reg_val |= (0x1<<24);
					mctl_write_w(reg_val,DTCR);  //one rank
					//re-training
					if(((para->dram_tpr8&0x1)==0) || (para->dram_clk>240))
						reg_val=0x1f3;
					else
						reg_val=0x201f3;
					if(dqs_gating_mode==1)
						reg_val|=(1<<10);  //DQS gating training
					mctl_write_w(reg_val,PIR);
					local_delay(1);
					while((mctl_read_w(PGSR0 )&0x1) != 0x1);
					reg_val = mctl_read_w(PGSR0);
					if((reg_val>>20)&0xff)
					{
						dram_dbg("DRAM ERROR\n");
					}else
					{
						dram_dbg("DRAM 1*8 training OK\n");
					}
				}
			}
		}
		else
		{
			dram_dbg("DRAM ERROR!\n");
			return 0;
		}
	}else
	{
		if(!(para->dram_tpr13&0x1))
		{
			dram_dbg("DRAM auto scan size success!\n");
			para->dram_para2 |= (0x1<<12);
			para->dram_para2 &=~(0xf<<0);
		}
	}

//*****************************************************************************************************//
	while((mctl_read_w(STATR )&0x1) != 0x1);    //check the dramc status
	//after auto scan dram size,recovery DQS gate mode
	if(!(para->dram_tpr13&0x1))
	{
		dqs_gating_mode = (para->dram_tpr13>>2)&0x7;
		dram_dbg("DRAM recovery DQS gate mode %d\n",dqs_gating_mode);
		switch(dqs_gating_mode)
		{
			case 0://close DQS gating
				reg_val=mctl_read_w(PGCR2);
				reg_val|=(0x3<<6);
				mctl_write_w(reg_val,PGCR2);
				dram_dbg("DRAM DQS gate is close.\n");
				break;
			case 1://open DQS gating
				reg_val=mctl_read_w(PGCR2);
				reg_val&=~(0x3<<6);
				mctl_write_w(reg_val,PGCR2);

				reg_val=mctl_read_w(DQSGMR);
				reg_val&=~((0x1<<8)|(0x7));
				mctl_write_w(reg_val,DQSGMR);
				dram_dbg("DRAM DQS gate is open.\n");
				break;
			case 2://auto gating
				reg_val=mctl_read_w(PGCR2);
				reg_val&=~(0x3<<6);
				reg_val|=(0x2<<6);
				mctl_write_w(reg_val,PGCR2);

				ret_val =((mctl_read_w(DRAMTMG2)>>16)&0x1f)-2;
				reg_val=mctl_read_w(DQSGMR);
				reg_val&=~((0x1<<8)|(0x7));
				reg_val|=((0x1<<8)|(ret_val));
				mctl_write_w(reg_val,DQSGMR);
				dram_dbg("DRAM DQSGMR value is %x\n",reg_val);

				reg_val =mctl_read_w(DXCCR);
				reg_val |=(0x1<<8);
				reg_val &=~(0x1<<12);
				mctl_write_w(reg_val,DXCCR);
				dram_dbg("DRAM DQS gate is auto.\n");
				break;
			default:
				break;
		}
	}
	//enable master access
	mctl_write_w(0xffffffff,MC_MAER);
	return 1;
}
#else
unsigned int mctl_channel_init(unsigned int ch_index,__dram_para_t *para)
{
	unsigned int reg_val = 0;
	reg_val=mctl_read_w(MC_WORK_MODE);
	reg_val&=~(0xfff<<12);
	reg_val|=(0x421<<12);
	mctl_write_w(reg_val,MC_WORK_MODE);

	mctl_write_w(0x00070005,RFSHTMG);

	mctl_write_w(MCTL_MR0,DRAM_MR0);
	mctl_write_w(MCTL_MR1,DRAM_MR1);
	mctl_write_w(MCTL_MR2,DRAM_MR2);
	mctl_write_w(MCTL_MR3,DRAM_MR3);

	mctl_write_w(0x01e007c3,PTR0);
	mctl_write_w(0x00170023,PTR1);
	mctl_write_w(0x00800800,PTR3);
	mctl_write_w(0x01000500,PTR4);
	mctl_write_w(0x01000081,DTCR);
	mctl_write_w(0x03808620,PGCR1);

	mctl_write_w(MCTL_PITMG0,PITMG0);

//	mctl_write_w(0x06021b02,DRAMTMG0);
//	mctl_write_w(0x00020102,DRAMTMG1);
//	mctl_write_w(0x03030306,DRAMTMG2);
//	mctl_write_w(0x00002006,DRAMTMG3);
//	mctl_write_w(0x01020101,DRAMTMG4);
//	mctl_write_w(0x05010302,DRAMTMG5);
	mctl_write_w(0x03030306,DRAMTMG2);
	mctl_write_w(0x05040405,DRAMTMG4);

	reg_val = 0x0001d3;		//PLL enable, PLL6 should be dram_clk/2
	mctl_write_w(reg_val,PIR);	//for fast simulation
	while((mctl_read_w(PGSR0 )&0x1) != 0x1);	//for fast simulation
	while((mctl_read_w(STATR )&0x1) != 0x1);	//init done
	para->dram_tpr13&=~(0x1<<0);//to detect DRAM space
	return (mctl_soft_training());
}
unsigned int mctl_soft_training(void)
{
	const unsigned int words[64] = {0x12345678, 0xaaaaaaaa, 0x55555555, 0x00000000, 0x11223344, 0xffffffff, 0x55aaaa55, 0xaa5555aa,
									0x23456789, 0x18481113, 0x01561212, 0x12156156, 0x32564661, 0x61532544, 0x62658451, 0x15564795,
									0x10234567, 0x54515152, 0x33333333, 0xcccccccc, 0x33cccc33, 0x3c3c3c3c, 0x69696969, 0x15246412,
									0x56324789, 0x55668899, 0x99887744, 0x00000000, 0x33669988, 0x66554477, 0x5555aaaa, 0x54546212,
									0x21465854, 0x66998877, 0xf0f0f0f0, 0x0f0f0f0f, 0x77777777, 0xeeeeeeee, 0x3333cccc, 0x52465621,
									0x24985463, 0x22335599, 0x78945623, 0xff00ff00, 0x00ff00ff, 0x55aa55aa, 0x66996699, 0x66544215,
									0x54484653, 0x66558877, 0x36925814, 0x58694712, 0x11223344, 0xffffffff, 0x96969696, 0x65448861,
									0x48898111, 0x22558833, 0x69584701, 0x56874123, 0x11223344, 0xffffffff, 0x99669966, 0x36544551};
	int i=0, j=0;
	unsigned int k=0;
	for(i=0; i<0x10; i++)
	{
		for(j=0; j<0x4; j++)
		{
			mctl_write_w(((3-j)<<20)|((0xf-i)<<16)|0x400f,MCTL_CTL_BASE+0xc);
			for(k=0; k<0x10; k++);
			for(k=0; k<(1<<10); k++)
			{
				mctl_write_w(words[k%64],DRAM_MEM_BASE+(k<<2));
			}

			for(k=0; k<(1<<10); k++)
			{
				if(words[k%64] != mctl_read_w(DRAM_MEM_BASE+(k<<2)))
				break;
			}

			if(k==(1<<10))
				return 1;
		}
	}
//	mctl_write_w(0x05400f,MCTL_CTL_BASE+0xc,);
	return 0;
}
#endif
/*
*******************************************************************************
*                    GET DRAM SIZE
*
* Description: Get the real DRAM Size in MB unit;
*
* Arguments  : detect the true size  min 128MB  max 2GB
*
* Returns    :128/256/512/1024...
*
* Notes      :
*
********************************************************************************
*/
unsigned int DRAMC_get_dram_size()
{
	unsigned int reg_val;
	unsigned int dram_size;
	unsigned int temp;

    reg_val = mctl_read_w(MC_WORK_MODE);

    temp = (reg_val>>8) & 0xf;	//page size code
    dram_size = (temp - 6);	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>4) & 0xf;	//row width code
    dram_size += (temp + 1);	//(1<<dram_size) * 512Bytes

    temp = (reg_val>>2) & 0x3;	//bank number code
    dram_size += (temp + 2);	//(1<<dram_size) * 512Bytes

    temp = reg_val & 0x3;	//rank number code
    dram_size += temp;	//(1<<dram_size) * 512Bytes

    dram_size = dram_size - 11;	//(1<<dram_size)MBytes

    return (1<<dram_size);
}

void auto_detect_dram_size(__dram_para_t *para)
{
	unsigned int i=0,j=0;
	unsigned int reg_val=0,ret=0,cnt=0;
	for(i=0;i<64;i++)
	{
		mctl_write_w((i%2)?(0x40000000 + 4*i):(~(0x40000000 + 4*i)),0x40000000 + 4*i);
	}
	reg_val=mctl_read_w(MC_WORK_MODE);
	paraconfig(&reg_val,0xf<<8,0x6<<8);//512B
	reg_val|=(0x1<<15);
	reg_val|=(0xf<<4);//16 row
	reg_val&=~(0x1<<0);//one rank
	mctl_write_w(reg_val,MC_WORK_MODE);
	//row detect
	for(i=11;i<=16;i++)
	{
		ret = 0x40000000 + (1<<(i+9));//row-column
		cnt = 0;
		for(j=0;j<64;j++)
		{
			if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(ret + j*4))
			{
				cnt++;
			}
		}
		if(cnt == 64)
		{
			break;
		}
	}
	if(i >= 16)
		i = 16;
	paraconfig(&(para->dram_para1), 0xffU<<20, i<<20);//row width confirmed
	//pagesize(column)detect
	reg_val=mctl_read_w(MC_WORK_MODE);
	paraconfig(&reg_val,0xf<<4,0xa<<4);//11rows
	paraconfig(&reg_val,0xf<<8,0xa<<8);//8KB
	mctl_write_w(reg_val,MC_WORK_MODE);
	for(i=9;i<=13;i++)
	{
		ret = 0x40000000 + (0x1U<<i);//column
		cnt = 0;
		for(j=0;j<64;j++)
		{
		if(mctl_read_w(0x40000000 + j*4) == mctl_read_w(ret + j*4))
			{
				cnt++;
			}
		}
		if(cnt == 64)
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
signed int init_DRAM(int type, __dram_para_t *para)
{
	unsigned int ret_val=0;
  unsigned int reg_val=0;
  unsigned int i=0;
  unsigned int dram_size = 0;
	UART_printf2("DRAM DRIVE INFO: V1.6\n");
	dram_dbg("DRAM Type =%d (2:DDR2,3:DDR3,6:LPDDR2,7:LPDDR3)\n", para->dram_type);
	dram_dbg("DRAM zq value: %x",para->dram_zq);
	UART_printf2("DRAM CLK =%d MHZ\n", para->dram_clk);

	mctl_sys_init(para);
	ret_val=mctl_channel_init(0,para);
	if(ret_val==0)
		return 0;
	if((!(para->dram_tpr13&0x1))||(para->dram_para2&0x1))
		auto_detect_dram_size(para);
	mctl_com_init(para);
	dram_size= DRAMC_get_dram_size();
	dram_dbg("DRAM SIZE =%d M\n", dram_size);
	//if auto gate mode ,turn off DRAM DLL
//	if(((para->dram_tpr13>>2)&0x7)!=1)
//	{
//	     ret_val = (mctl_read_w(MC_WORK_MODE)&0x1);
//	     reg_val = 0;
//	     if(ret_val)
//	     {
//	     		reg_val |=(0x3<<24);
//	     		reg_val |= 0x41 ;
//	     		reg_val |= (0x1U<<16);
//	     		dram_dbg("DRAM two rank DLL is off.\n");
//	     }else
//	     {
//	     	  reg_val &=~(0x3<<24);
//	     	  reg_val |= (0x1U<<24);
//	     	  reg_val |= 0x41 ;
//	     	  reg_val |= (0x1U<<16);
//	     	  dram_dbg("DRAM single rank DLL is off.\n");
//	     }
//	     mctl_write_w(reg_val,MRCTRL0);
//	     reg_val |=(0x1U<<31);
//	     mctl_write_w(reg_val,MRCTRL0);
//	     while(((mctl_read_w(MRCTRL0)>>31)&0x1) == 0x1);
//	}
	//set DRAM ODT MAP
	reg_val = (mctl_read_w(MC_WORK_MODE)&0x1);
	if(reg_val)
	{
			mctl_write_w(0x00000303,ODTMAP);
			dram_dbg("set two rank ODTMAP\n");
	}else
		{
				mctl_write_w(0x00000201,ODTMAP);
				dram_dbg("set one rank ODTMAP\n");
		}
	local_delay(1);
	/* debug auto enter self-refresh and power down and ck gating */
	if ((para->dram_tpr13>>7) & 0x1) {
		reg_val =0;
		reg_val |= (0x1<<1);
		mctl_write_w(reg_val,PWRCTL);
		dram_dbg("DRAM auto enter power down is set\n");
	}
	if ((para->dram_tpr13>>6) & 0x1) {
		reg_val =0;
		reg_val |= ((0x1<<0)|(0x1<<3));
		mctl_write_w(reg_val,PWRCTL);
		dram_dbg("DRAM auto enter self-refresh is set\n");
	}
	watchdog_enable();
	//when init dram ok ,simple test
	for(i=0;i<32;i++)
	{
			mctl_write_w(0x12345678+i,(0x40000000+i*4));//rank0
			mctl_write_w(0x87654321+i,(0x40004000+i*4));//rank1
	}
	for(i=0;i<32;i++)
	{
			if((mctl_read_w(0x40000000+i*4))!=(0x12345678+i))
			{
					//UART_printf2("DRAM simple test fail.\n");
					//watchdog_disable();
					while(1){};
					return 0;
			}
			if((mctl_read_w(0x40004000+i*4))!=(0x87654321+i))
			{
					//UART_printf2("DRAM simple test fail.\n");
					//watchdog_disable();
					while(1){};
					return 0;
			}
	}
	para->dram_para1 &=~(0xfff<<0);
	para->dram_para1|=(dram_size);
	para->dram_tpr13|=(0x1<<0);
	watchdog_disable();
	UART_printf2("DRAM simple test OK.\n");
	return dram_size;
}
unsigned int mctl_init(void)
{
	__dram_para_t dram_para;
	signed int ret_val=0;
	//set the parameter
	dram_para.dram_clk			= 480;	//240M
	dram_para.dram_type			= 3;	//dram_type	DDR2: 2	DDR3: 3	LPDDR2: 6  LPDDR3: 7
	dram_para.dram_zq			= 0x3bbb;
	dram_para.dram_odt_en       = 0;
	dram_para.dram_para1		= 0x10F20200;//according to design
	dram_para.dram_para2		= 0x0;
	dram_para.dram_mr0			= 0x1840;
	dram_para.dram_mr1			= 0x40;
	dram_para.dram_mr2			= 0x8;
	dram_para.dram_mr3			= 0x0;
	dram_para.dram_tpr0 		= 0x0048A192;
	dram_para.dram_tpr1 		= 0x01B1B18d;
	dram_para.dram_tpr2 		= 0x00076052;
	dram_para.dram_tpr3 		= 0x0;
	dram_para.dram_tpr4 		= 0x0;
	dram_para.dram_tpr5         = 0x0;
	dram_para.dram_tpr6         = 0x0;
	dram_para.dram_tpr7         = 0x0;
	dram_para.dram_tpr8         = 0x0;
	dram_para.dram_tpr9         = 0x0;
	dram_para.dram_tpr10        = 0x0;
	dram_para.dram_tpr11        = 0x0;
	dram_para.dram_tpr12        = 0x0;
	dram_para.dram_tpr13        = 0x4;//--7

   	ret_val = init_DRAM(0, &dram_para);//signed int init_DRAM(int type, void *para);
	return ret_val;
}



