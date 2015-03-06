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
//	Description:  This file implements basic functions for AW1650 DRAM controller
//
//	History:
//				2013/07/11		CPL				0.1		Initial version
//				2013/08/06		CPL				0.2		add PLL delay to 10ms and sysconfig paramters config & master config
//				2013/08/09		CPL				0.3		turn off write leveling and modify
//				2013/08/13		CPL				0.4	  update CK zq value to tpr11 for standby support
//				2013/08/21		CPL				0.5		support low freq & added power down when idle & SWON turn to 0x3FFFF
//				2013/09/04		CPL				0.6		add power down control and Rodt window control
//				2013/09/12		CPL				0.7		add frequency extend and read phase tune
//				2013/09/24		CPL				0.8		support 8bit DDR
//				2013/12/05		YSZ				0.9		disable RW Deskew train & RW eye train in DRAM initial
//        2014/01/16    ZW        1.0   updata frequency extend
//        2014/09/15    ZW        1.1   add dqs and clk delay
//*********************************************************************************************************
#include "mctl_reg.h"
#include "mctl_hal.h"

//static int resetflag = 0;

__dram_para_t *dram_para;


static void watchdog_disable(void)
{
	/* disable watchdog */
	mctl_write_w(0x1c20C00 + 0xB8, 0);

	return ;
}

static void watchdog_enable(void)
{
	/* enable watchdog */
	mctl_write_w(0x1c20C00 + 0xB4, 1);
	mctl_write_w(0x1c20C00 + 0xB8, 1);

	return ;
}

//=====================p2wi adjust pmu dc5 voltage===========================
#if 0

#define P2WI_CTRL		(0x01f03400 + 0x00)
#define P2WI_CCR		(0x01f03400 + 0x04)
#define P2WI_INTE		(0x01f03400 + 0x08)
#define P2WI_STAT		(0x01f03400 + 0x0c)
#define P2WI_DADDR0	(0x01f03400 + 0x10)
#define P2WI_DADDR1	(0x01f03400 + 0x14)
#define P2WI_DLEN		(0x01f03400 + 0x18)
#define P2WI_DATA0	(0x01f03400 + 0x1c)
#define P2WI_DATA1	(0x01f03400 + 0x20)
#define P2WI_LCR		(0x01f03400 + 0x24)
#define P2WI_PMCR		(0x01f03400 + 0x28)
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
	mctl_write_w(P2WI_CCR, 0x103);

	/* switch pmu to p2wi mode */
	mctl_write_w(P2WI_PMCR, 0x803e3e68);
	while((mctl_read_w(P2WI_PMCR) & 0x80000000) && time--);
	if (mctl_read_w(P2WI_PMCR) & 0x80000000) {
#ifdef PW2I_PRINK
		dram_dbg("Switching PMU to P2WI mode failed\n");
#endif
		return -1;
	}
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
#define AW1636_IC_ID_REG			(0x3)
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

	// DDR2 || DDR3
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

	// DDR2
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
	ret = p2wi_init();
	if (ret)
		return -1;
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

		// ret = p2wi_write(AW1636_DCDC2_VOL_CTRL, (1260-600)/20);
		// ret = p2wi_write(AW1636_DCDC3_VOL_CTRL, (1260-600)/20);
	ret = p2wi_write(AW1636_DCDC4_VOL_CTRL, (1200-600)/20);
		// ret = p2wi_write(AW1636_DCDC5_VOL_CTRL, (1600-600)/20);

	if (ret)
	{
		return -1;
	}

	return 0;
}

#endif
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
	unsigned int i,j;
	unsigned int tmp;
	unsigned int row_num;
	unsigned int reg_val;
	unsigned int ret;
	unsigned int byte;

  	if(!para)
    	return 0;

	//clk configuration
	mctl_sys_init(para);

	//channel init
	mctl_ch_init(para);

	//common init
	mctl_com_init(para);

	if(((mctl_read_w(MP_DXGCR(1)))&0x1) == 0)
		byte = 0;
	else
		byte = 1;

	if((para->dram_tpr13 & (0x1u<<31)) == 0)
	{
		 for(i=0; i<64; i++)
			 mctl_write_w(0x40000000 + 4*i, (i%2)?(0x40000000 + 4*i):(~(0x40000000 + 4*i)));

		//row detect
		for(i=11;i<=16;i++)
		{
			tmp = 0x40000000 + (1<<(i+10+byte+3));
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

		reg_val = mctl_read_w(MC_CR);
		reg_val &= ~(0xF<<4);
		reg_val |= (row_num-1)<<4;
		mctl_write_w(MC_CR, reg_val);

		para->dram_para1 &= ~(0xFFF);
		para->dram_para1 |= ((tmp - 0x40000000)>>20);
		para->dram_para1 &= ~(0xFF<<16);
		para->dram_para1 |= row_num<<16;
		if(byte)
		{
			para->dram_para2 &= ~(0xF<<0);
		}
		else
		{
			para->dram_para2 &= ~(0xF<<0);
			para->dram_para2 |= (0x2<<0);
		}
		para->dram_tpr13 |= (0x1u<<31);
	}
	 else
	 {
		 reg_val = mctl_read_w(MC_CR);
		 reg_val &= ~(0xF<<4);
		 reg_val |= (((para->dram_para1>>16)&0xFF)-1)<<4;
		 mctl_write_w(MC_CR, reg_val);
	 }

	host_config();

	return (para->dram_para1 & 0xFFF);

}

void host_config(void)
{
	mctl_write_w(MCTL_COM_BASE + 0x10, 0x00000008);	//CPU
	mctl_write_w(MCTL_COM_BASE + 0x14, 0x0001000D);	//CPU 52MB/s
	mctl_write_w(MCTL_COM_BASE + 0x18, 0x00000004); //GPU
	mctl_write_w(MCTL_COM_BASE + 0x1C, 0x00000080); //GPU 512MB/s

	mctl_write_w(MCTL_COM_BASE + 0x20, 0x00000004); //DMA
	mctl_write_w(MCTL_COM_BASE + 0x24, 0x00000019); //DMA 100MB/s
	mctl_write_w(MCTL_COM_BASE + 0x28, 0x00000004); //VE
	mctl_write_w(MCTL_COM_BASE + 0x2C, 0x00000080); //VE 512MB/s
	mctl_write_w(MCTL_COM_BASE + 0x30, 0x00000004); //BE
	mctl_write_w(MCTL_COM_BASE + 0x34, 0x01010040); //BE 256MB/s

	mctl_write_w(MCTL_COM_BASE + 0x38, 0x00000004); //FE
	mctl_write_w(MCTL_COM_BASE + 0x3C, 0x0001002F); //FE 188MB/s
	mctl_write_w(MCTL_COM_BASE + 0x40, 0x00000004); //CSI
	mctl_write_w(MCTL_COM_BASE + 0x44, 0x00010020); //CSI 128MB/s
	mctl_write_w(MCTL_COM_BASE + 0x48, 0x00000004); //IEP
	mctl_write_w(MCTL_COM_BASE + 0x4C, 0x00010020); //IEP 128MB/s
	mctl_write_w(MCTL_COM_BASE + 0x50, 0x00000008); //CPUS
	mctl_write_w(MCTL_COM_BASE + 0x54, 0x00000001); //CPUS 4MB/s
	mctl_write_w(MCTL_COM_BASE + 0x58, 0x00000008); //USB0
	mctl_write_w(MCTL_COM_BASE + 0x5C, 0x00000005); //USB0 20MB/s
	mctl_write_w(MCTL_COM_BASE + 0x60, 0x00000008); //MSTG0
	mctl_write_w(MCTL_COM_BASE + 0x64, 0x00000003); //MSTG0 12MB/s
	mctl_write_w(MCTL_COM_BASE + 0x68, 0x00000008); //USB1
	mctl_write_w(MCTL_COM_BASE + 0x6C, 0x00000005); //USB1 20MB/s
	mctl_write_w(MCTL_COM_BASE + 0x70, 0x00000008); //MSTG2
	mctl_write_w(MCTL_COM_BASE + 0x74, 0x00000003); //MSTG2 12MB/s
	mctl_write_w(MCTL_COM_BASE + 0x78, 0x00000008); //NAND
	mctl_write_w(MCTL_COM_BASE + 0x7C, 0x00000004); //NAND 16MB/s
	mctl_write_w(MCTL_COM_BASE + 0x80, 0x00000008); //TestAHB
	mctl_write_w(MCTL_COM_BASE + 0x84, 0x00000002); //TestAHB 8MB/s
	mctl_write_w(MCTL_COM_BASE + 0x88, 0x00000008); //MSTG1
	mctl_write_w(MCTL_COM_BASE + 0x8C, 0x00000003); //MSTG1 12MB/s

	mctl_write_w(MCTL_COM_BASE + 0x90, 0x00010138); //1us
}

void mctl_ch_init(__dram_para_t *para)
{
	unsigned int reg_val;
//	unsigned int hclk;
	unsigned int reg_val1;
	unsigned int hold_flag = 0;
	unsigned int i;

	if(((para->dram_tpr13>>5)&0x1) == 0)	//2T
		reg_val = 0x1000040B;
	else									//1T
		reg_val = 0x0000040B;
	mctl_write_w(MP_DCR, reg_val);

	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//default DQS#:12~9 DQS:8~5 is 1100 0100,500┯
	//now     DQS#:12~9 DQS:8~5 is 1001 0001,688┯
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	if((para->dram_tpr13 & (0x1u<<22)) == 0)//------------------------------------------------------------------------------------------------------------------------------------
	{
	  reg_val=mctl_read_w( MP_DXCCR );//--------------------------------------------------------------------------------------------------------------------------------------------
	  reg_val&=~ ((0xff)<<5);//-----------------------------------------------------------------------------------------------------------------------------------------------------
	  reg_val|=(0x91)<<5;//----------------------------------------------------------------------------------------------------------------------------------------------------------
	  mctl_write_w(MP_DXCCR, reg_val);//--------------------------------------------------------------------------------------------------------------------------------------------
  }
  if((para->dram_tpr13 & (0x1u<<23)) != 0)//dram_tpr13 bit23 is 1 enable dqs clk delay
	{
		mctl_write_w(MP_DXBDLR2(1), ((((para->dram_tpr10>>20)&0xf)<<12)|(((para->dram_tpr10>>20)&0xf)<<18)));//read dqs1 delay
		mctl_write_w(MP_DXBDLR1(1), ((para->dram_tpr10>>24)&0xf)<<24);//write dqs1 delay

		mctl_write_w(MP_DXBDLR2(0), ((((para->dram_tpr10>>12)&0xf)<<12)|(((para->dram_tpr10>>12)&0xf)<<18)));//read dqs0 delay
		mctl_write_w(MP_DXBDLR1(0),  ((para->dram_tpr10>>16)&0xf)<<24);//write dqs0 delay
		mctl_write_w(MP_ACBDLR,  ((para->dram_tpr10>>28)&0xf));//clk delay
	}

	if(para->dram_clk >= 480)
		mctl_write_w(MP_DLLGCR, 0x5c000);
	else
		mctl_write_w(MP_DLLGCR, 0xdc000);
	//general configuration
	reg_val = 0x0A003E3F;
	mctl_write_w(MP_PGCR(0), reg_val);
	reg_val = 0x03008421;
	mctl_write_w(MP_PGCR(1), reg_val);

	//MR register setup
	mctl_write_w(MP_MR(0), para->dram_mr0);
	mctl_write_w(MP_MR(1), para->dram_mr1);
	mctl_write_w(MP_MR(2), para->dram_mr2);
	mctl_write_w(MP_MR(3), para->dram_mr3);

	if((para->dram_tpr13 & (1<<16)) == 0)
	{
		reg_val = mctl_read_w(MP_DXGCR(0));
		reg_val &= ~(0x7<<11);
		reg_val |= (0x4<<11);
		mctl_write_w(MP_DXGCR(0), reg_val);

		reg_val = mctl_read_w(MP_DXGCR(1));
		reg_val &= ~(0x7<<11);
		reg_val |= (0x4<<11);
		mctl_write_w(MP_DXGCR(1), reg_val);
	}

//	hclk = dram_get_ahb1_clk()/1000000;

	//phy timing registers
//	reg_val = 	(0x10<<0)       |	/*tPHYRST*/
//				((4*hclk)<<6)  	|	/*tPLLGS*/
//				(hclk<<21);			/*tPLLPD*/
//	mctl_write_w(MP_PTR(0), reg_val);

//	reg_val = 	((9*hclk)<<0)  	|	/*tPLLRST*/
//				((100*hclk)<<16);	/*tPLLLOCK*/
//	mctl_write_w(MP_PTR(1), reg_val);

	// reg_val =	(0xF<<0)		|	/*tCALON*/
				// (0xF<<5)			|	/*tCALS*/
				// (0xF<<10)		|	/*tCALH*/
				// (0x10<<15);			/*tWLDLYS*/
	reg_val = (((para->dram_tpr0>>0)&0x1F)<<0)		|	/*tCALON*/
			  (((para->dram_tpr0>>5)&0x1F)<<5)		|	/*tCALS*/
			  (((para->dram_tpr0>>10)&0x1F)<<10)	|	/*tCALH*/
			  (((para->dram_tpr0>>15)&0x1F)<<15);		/*tWLDLYS*/
	mctl_write_w(MP_PTR(2), reg_val);

	// reg_val =	(133250<<0)	    |	/*tDINIT0*/
				// (96<<20);		    /*tDINIT1*/
	reg_val =	(((para->dram_tpr1>>0)&0xFFFFF)<<0)	|	/*tDINIT0*/
				(((para->dram_tpr1>>20)&0x1FF)<<20);	/*tDINIT1*/
	mctl_write_w(MP_PTR(3), reg_val);

	// reg_val = 	(53300<<0)		|	/*tDINIT2*/
				// (171<<18);		    /*tDINIT3*/
	reg_val = 	(((para->dram_tpr2>>0)&0x3FFFF)<<0)		|	/*tDINIT2*/
				(((para->dram_tpr0>>20)&0x3FF)<<18);		    /*tDINIT3*/
	mctl_write_w(MP_PTR(4), reg_val);

	// reg_val =	(17<<26)		|	/*tRC*/
				// (2<<22)			|	/*tRRD*/
				// (12<<16)		|	/*tRAS*/
				// (5<<12)			|	/*tRCD*/
				// (5<<8)			|	/*tRP*/
				// (3<<4)			|	/*tWTR*/
				// (3<<0);				/*tRTp*/
	reg_val =	(((para->dram_tpr3>>26)&0x3F)<<26)		|	/*tRC*/
				(((para->dram_tpr3>>22)&0xF)<<22)		|	/*tRRD*/
				(((para->dram_tpr3>>16)&0x3F)<<16)		|	/*tRAS*/
				(((para->dram_tpr3>>12)&0xF)<<12)		|	/*tRCD*/
				(((para->dram_tpr3>>8)&0xF)<<8)			|	/*tRP*/
				(((para->dram_tpr3>>4)&0xF)<<4)			|	/*tWTR*/
				(((para->dram_tpr3>>0)&0xF)<<0);				/*tRTp*/
	mctl_write_w(MP_DTPR(0), reg_val);

	// reg_val =	(0<<31)			|	/*tCCD*/
				// (0<<30)			|	/*tRTW*/
				// (0<<29)			|	/*tRTODT*/
				// (256<<19)		|	/*tDLLK*/
				// (2<<15)			|	/*tCKE*/
				// (3<<10)			|	/*tXP*/
				// (256<<0);			/*tXS*/
	reg_val =	(((para->dram_tpr4>>31)&0x1)<<31)			|	/*tCCD*/
				(((para->dram_tpr4>>30)&0x1)<<30)			|	/*tRTW*/
				(((para->dram_tpr4>>29)&0x1)<<29)			|	/*tRTODT*/
				(((para->dram_tpr4>>19)&0x3FF)<<19)			|	/*tDLLK*/
				(((para->dram_tpr4>>15)&0xF)<<15)			|	/*tCKE*/
				(((para->dram_tpr4>>10)&0x1F)<<10)			|	/*tXP*/
				(((para->dram_tpr4>>0)&0x3FF)<<0);			/*tXS*/
	mctl_write_w(MP_DTPR(2), reg_val);

	//data training configuration
	reg_val = 0x01000081;
	mctl_write_w(MP_DTCR, reg_val);

	//disable high 8-bit when 8-bit mode
	if( (para->dram_clk <= 240) || ( ((para->dram_odt_en)&0x1) == 0) )
	{
		reg_val = mctl_read_w(MP_DXGCR(0));
		reg_val &= ~(0x3<<9);
		mctl_write_w(MP_DXGCR(0), reg_val);

		reg_val = mctl_read_w(MP_DXGCR(1));
		reg_val &= ~(0x3<<9);
		mctl_write_w(MP_DXGCR(1), reg_val);
	}

	if(para->dram_clk <= 240)
	{
		mctl_write_w(MP_ODTCR, 0);
		mctl_write_w(MX_ODTMAP, 0);
	}
	//clear training states last time
//	reg_val = 0x1<<27;
//	mctl_write_w(MP_PIR, reg_val);

//	if(para->dram_clk < 336) //pll bypass mode
//	{
//		reg_val = mctl_read_w(MP_PIR);
//		reg_val |= 0x1<<17;
//		mctl_write_w(MP_PIR, reg_val);
//	}

	//timing parameters configuration
	// reg_val = 	(2<<20)			|	/*refresh_margin*/
				// (1<<12)			|	/*refresh_to_x32*/
				// (7<<8);				/*refresh_burst*/
	reg_val = 	(((para->dram_tpr5>>8)&0xF)<<20)			|	/*refresh_margin*/
				(((para->dram_tpr5>>3)&0x1F)<<12)			|	/*refresh_to_x32*/
				(((para->dram_tpr5>>0)&0x7)<<8);				/*refresh_burst*/
	mctl_write_w(MX_RFSHCTL0, reg_val);

	// reg_val = 	(11<<24)		|	/*wr2pre*/
				// (11<<16)		|	/*t_faw*/
				// (3<<8)			|	/*t_ras_max*/
				// (12<<0);			/*t_ras_min*/
	reg_val = 	(((para->dram_tpr5>>12)&0x3F)<<24)		|	/*wr2pre*/
				(((para->dram_tpr5>>18)&0x3F)<<16)		|	/*t_faw*/
				(((para->dram_tpr5>>24)&0x3F)<<8)		|	/*t_ras_max*/
				(((para->dram_tpr6>>0)&0x3F)<<0);			/*t_ras_min*/
	mctl_write_w(MX_DRAMTMG(0), reg_val);

	// reg_val = 	(3<<16)			|	/*t_xp*/
				// (3<<8)			|	/*rd2pre*/
				// (17<<0);			/*t_rc*/
	reg_val = 	(((para->dram_tpr6>>6)&0x1F)<<16)			|	/*t_xp*/
				(((para->dram_tpr6>>11)&0x1F)<<8)		|	/*rd2pre*/
				(((para->dram_tpr6>>16)&0x3F)<<0);			/*t_rc*/
	mctl_write_w(MX_DRAMTMG(1), reg_val);

	// reg_val = 	(3<<24)			|	/*write_latency*/
				// (5<<16)			|	/*read_latency*/
				// (4<<8)			|	/*rd2wr*/
				// (9<0);				/*wr2rd*/
	reg_val = 	(((para->dram_tpr6>>22)&0x3F)<<24)		|	/*write_latency*/
				(((para->dram_tpr7>>0)&0x1F)<<16)			|	/*read_latency*/
				(((para->dram_tpr7>>5)&0x1F)<<8)			|	/*rd2wr*/
				(((para->dram_tpr7>>10)&0x3F)<<0);			/*wr2rd*/
	mctl_write_w(MX_DRAMTMG(2), reg_val);

	// reg_val = 	(2<<12)			|	/*t_mrd*/
				// (6<<0);				/*t_mode*/
	reg_val = 	(((para->dram_tpr6>>28)&0xF)<<12)			|	/*t_mrd*/
				(((para->dram_tpr7>>16)&0x3FF)<<0);			/*t_mode*/
	mctl_write_w(MX_DRAMTMG(3), reg_val);

	// reg_val = 	(5<<24)			|	/*trcd*/
				// (0<<16)			|	/*tccd*/
				// (2<<8)			|	/*trrd*/
				// (5<<0);				/*trp*/
	reg_val = 	(((para->dram_tpr7>>26)&0xF)<<24)			|	/*trcd*/
				(((para->dram_tpr8>>0)&0x7)<<16)			|	/*tccd*/
				(((para->dram_tpr8>>3)&0x7)<<8)				|	/*trrd*/
				(((para->dram_tpr8>>6)&0xF)<<0);				/*trp*/
	mctl_write_w(MX_DRAMTMG(4), reg_val);

	// reg_val = 	(7<<24)			|	/*t_cksrx*/
				// (4<<16)			|	/*t_cksre*/
				// (3<<8)			|	/*t_ckesr*/
				// (2<<0);				/*tcke*/
	reg_val = 	(((para->dram_tpr8>>10)&0xF)<<24)			|	/*t_cksrx*/
				(((para->dram_tpr8>>14)&0xF)<<16)			|	/*t_cksre*/
				(((para->dram_tpr8>>18)&0x3F)<<8)		|	/*t_ckesr*/
				(((para->dram_tpr8>>24)&0xF)<<0);			/*tcke*/
	mctl_write_w(MX_DRAMTMG(5), reg_val);

	reg_val = 	(0x8<<0);
	mctl_write_w(MX_DRAMTMG(8), reg_val);

	// reg_val = 	(2<<24)			|	/*t_ctrl_delay*/
				// (3<<16)			|	/*t_rdata_en*/
				// (1<<8)			|	/*tphy_wrdata*/
				// (2<<0);				/*write latency*/
	reg_val = 	(((para->dram_tpr8>>28)&0xF)<<24)			|	/*t_ctrl_delay*/
				(((para->dram_tpr9>>10)&0x1F)<<16)		|	/*t_rdata_en*/
				(((para->dram_tpr9>>5)&0x1F)<<8)			|	/*tphy_wrdata*/
				(((para->dram_tpr9>>0)&0x1F)<<0);			/*write latency*/
	mctl_write_w(MX_PITMG(0), reg_val);

	reg_val = mctl_read_w(MX_PITMG(1));
	reg_val |= 	(8<<16);			/*t_wrdata_delay*/
	mctl_write_w(MX_PITMG(1), reg_val);

	reg_val = 	(((para->dram_tpr9>>15)&0x7F)<<24)		|	/*rdwr_idle_gap*/
				 (0<<2)									|
				 (0<<1)									|
				 (0x20<<8)								|
				 (0x1<<0);
	mctl_write_w(MX_SCHED, reg_val);

	reg_val = (para->dram_mr1<<0) | (para->dram_mr0<<16);
	mctl_write_w(MX_INIT3, reg_val);
	reg_val = (para->dram_mr3<<0) | (para->dram_mr2<<16);
	mctl_write_w(MX_INIT4, reg_val);

	reg_val = 0;
	mctl_write_w(MX_PIMISC, reg_val);
	reg_val = 0x80000000;
	mctl_write_w(MX_UPD(0), reg_val);

	//setup refresh timing
	reg_val = (((para->dram_tpr9>>22)&0x3FF)<<0)	|	/*t_rfc_min*/
			  (((para->dram_tpr10>>0)&0xFFF)<<16);		/*t_rfc_nom_x32*/
	mctl_write_w(MX_RFSHTMG, reg_val);

	//master register setup
	if(((para->dram_tpr13>>5)&0x1) == 0)	//2T
		reg_val = 0x1040401;
	else									//1T
		reg_val = 0x1040001;
//	reg_val = 0x10401501;		//8-bit
	mctl_write_w(MX_MSTR, reg_val);

	if((para->dram_tpr13 & (1<<17)) == 0)
	{
		mctl_write_w(MX_PWRCTL, 0x2);
		mctl_write_w(MX_PWRTMG, 0x8001);
	}

	//mctl_write_w(MX_PWRCTL, 0x2);
	//mctl_write_w(MX_PWRTMG, 0x8001);

	//disable auto-refresh
	reg_val = 0x1;
	mctl_write_w(MX_RFSHCTL3, reg_val);

	//start controller init
	reg_val = 0x1;
	mctl_write_w(MX_PIMISC, reg_val);

	//hold flag
	reg_val = mctl_read_w(VDD_SYS_PWROFF_GATING);
	if(reg_val & 0x1)
		hold_flag = 1;

	//sdram controller reset de-assert
	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
	reg_val |= 0x1u<<31;
	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);

	//global clk enable
	reg_val = mctl_read_w(MC_CCR);
//    if(para->dram_clk < 336) //pll bypass mode
//    	reg_val |= (0x1<<19) | (0x1<<22);
//    else
    	reg_val |= (0x1<<19);
	mctl_write_w(MC_CCR, reg_val);

	reg_val1 = 0x00000;
//    if(para->dram_clk < 336) //pll bypass mode
//	   reg_val1 = 0x20000;

	//zq calibration
	if(hold_flag == 0)
	{
		mctl_write_w(MP_ZQCR1(0), (para->dram_zq>>8)&0xFF);//CK DRV & CA exclude ck drv

		reg_val =reg_val1 | 0x3;
		mctl_write_w(MP_PIR, reg_val);	//CTLDINIT

		for(i=0; i<1000; i++){}

		while((mctl_read_w(MP_PGSR(0))&0x9) != 0x9);	//

		para->dram_tpr11 = mctl_read_w(MP_ZQSR0(0));

		reg_val = mctl_read_w(MP_ZQSR0(0));
		reg_val |=0x1<<28;
		mctl_write_w(MP_ZQCR2, reg_val);

		mctl_write_w(MP_ZQCR1(0), (para->dram_zq)&0xFF);	//DX ODT   &  DX DRV


//		if(para->dram_clk < 336) //pll bypass mode
//			reg_val = reg_val1 |  0xFDE3;
//		else
		reg_val = reg_val1 |  0x5F3;	//reg_val = reg_val1 |  0xFDF3;	//0xFDF3 simple test ok  0x1D3
		mctl_write_w(MP_PIR, reg_val);

		for(i=0; i<1000; i++){}

		//wait for DLL Lock & init done
		while((mctl_read_w(MP_PGSR(0))&0x3) != 0x3);

		//for 8bit try
		if(mctl_read_w(MP_DXGSR0(1)) & (1<<24))
		{
			mctl_write_w(MP_DXGCR(1), 0);
			reg_val = mctl_read_w(MP_DXGCR(1));
			reg_val|=(0x7)<<4;//set DX1 Power Down Driver
			mctl_write_w(MP_DXGCR(1), reg_val);

//			mctl_write_w(MP_ZQCR1(0), (para->dram_zq>>8)&0xFF);//CK DRV & CA exclude ck drv
//
//			reg_val =reg_val1 | 0x3;
//			mctl_write_w(MP_PIR, reg_val);	//CTLDINIT
//
//			for(i=0; i<1000; i++){}
//
//			while((mctl_read_w(MP_PGSR(0))&0x9) != 0x9);	//
//
//			para->dram_tpr11 = mctl_read_w(MP_ZQSR0(0));
//
//			reg_val = mctl_read_w(MP_ZQSR0(0));
//			reg_val |=0x1<<28;
//			mctl_write_w(MP_ZQCR2, reg_val);

			mctl_write_w(MP_ZQCR1(0), (para->dram_zq)&0xFF);	//DX ODT   &  DX DRV

	//		if(para->dram_clk < 336) //pll bypass mode
	//			reg_val = reg_val1 |  0xFDE3;
	//		else
			reg_val = reg_val1 |  0x5F3;	//reg_val = reg_val1 |  0xFDF3;	//0xFDF3 simple test ok  0x1D3
			mctl_write_w(MP_PIR, reg_val);

			for(i=0; i<10000; i++){}

			reg_val = mctl_read_w(MX_MSTR);
			reg_val |= 0x1<<12;
			mctl_write_w(MX_MSTR, reg_val);
		}

		//delay the dqs
		reg_val = mctl_read_w(MP_DXLCDLR1(0)); //15ps per step
		if(para->dram_odt_en & (1u<<31))  //minus delay
		{
			if( ((reg_val>>8)&0xFF) <= ((para->dram_odt_en>>8)&0xFF) )
				reg_val &= ~(0xFF<<8);
			else
				reg_val -= (para->dram_odt_en & 0x0000ff00);

		}								//plus delay
		else
		{
			if( (((reg_val>>8)&0xFF) + ((para->dram_odt_en>>8)&0xFF)) >= 0xFF )
				reg_val |= (0xFF<<8);
			else
				reg_val += (para->dram_odt_en & 0x0000ff00);
		}
		mctl_write_w(MP_DXLCDLR1(0), reg_val);

		reg_val = mctl_read_w(MP_DXLCDLR1(1));	//15ps per step
		if(para->dram_odt_en & (1u<<31))  //minus delay
		{
			if( ((reg_val>>8)&0xFF) <= ((para->dram_odt_en>>8)&0xFF) )
				reg_val &= ~(0xFF<<8);
			else
				reg_val -= (para->dram_odt_en & 0x0000ff00);

		}								//plus delay
		else
		{
			if( (((reg_val>>8)&0xFF) + ((para->dram_odt_en>>8)&0xFF)) >= 0xFF )
				reg_val |= (0xFF<<8);
			else
				reg_val += (para->dram_odt_en & 0x0000ff00);
		}
		mctl_write_w(MP_DXLCDLR1(1), reg_val);
		//test end

	}else{
	   // reg_val = mctl_read_w(MP_PIR);
		if(para->dram_clk < 336) //pll bypass mode
			reg_val =reg_val1 | 0x40000161;
		else
			reg_val =reg_val1 | 0x40000171;
	    mctl_write_w(MP_PIR, reg_val);

	    //wait for DLL Lock & init done
	    while((mctl_read_w(MP_PGSR(0))&0x1) != 0x1);
	}

	//init done
	while((mctl_read_w(MX_STATR)&0x1) != 0x1);

    reg_val = 0x08003e3f;
	//	if(hold_flag)
	//		reg_val = 0x08003e3f;
	mctl_write_w(MP_PGCR(0), reg_val);

	if(!hold_flag)
	{
		//enable auto-refresh
		reg_val = 0x0;
		mctl_write_w(MX_RFSHCTL3, reg_val);
	}else
	{
		//set selfrefresh command
  		reg_val = mctl_read_w(MX_PWRCTL);
  		reg_val |= 0x1<<0;
  		mctl_write_w(MX_PWRCTL, reg_val);

  		//confirm dram controller has enter selfrefresh
  		while(((mctl_read_w(MX_STATR)&0x7) != 0x3)){}

		reg_val = mctl_read_w(MP_DSGCR);
  		reg_val |= (0x1<<28);
  		mctl_write_w(MP_DSGCR, reg_val);

  		reg_val = mctl_read_w(MP_DXCCR);
  		reg_val &= ~((0x1<<3)|(0x1<<4));
  		mctl_write_w(MP_DXCCR, reg_val);

  		reg_val = mctl_read_w(MP_ACIOCR);
  		reg_val &= ~((0x1<<3)|(0x1<<8)|(0x1<<18));
  		mctl_write_w(MP_ACIOCR, reg_val);

	  	//pad hold
	  	reg_val = mctl_read_w(VDD_SYS_PWROFF_GATING);
  		reg_val &= ~(0x1<<0);
  		mctl_write_w(VDD_SYS_PWROFF_GATING, reg_val);

  		//set selfrefresh command
  		reg_val = mctl_read_w(MX_PWRCTL);
  		reg_val &= ~(0x1<<0);
  		mctl_write_w(MX_PWRCTL, reg_val);

  		//confirm dram controller has exit from selfrefresh
  		while(((mctl_read_w(MX_STATR)&0x7) != 0x1)){}
	}

}

void mctl_sys_init(__dram_para_t *para)
{
	unsigned int reg_val;

	//turn off dram ahb1 gating
	reg_val = mctl_read_w(CCM_AHB1_GATE0_REG);
	reg_val &= ~(0x1<<14);
	mctl_write_w(CCM_AHB1_GATE0_REG, reg_val);

	//put DRAMC into reset states
	reg_val = mctl_read_w(CCM_AHB1_RST_REG0);
	reg_val &= ~(0x1<<14);
	mctl_write_w(CCM_AHB1_RST_REG0, reg_val);

	//sdram controller reset
	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
	reg_val &= ~(0x1u<<31);
	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);

	//configuration update
	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
	reg_val |= (0x1u<<16);
	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);

	//wait for configuration update
	while(mctl_read_w(CCM_DRAM_CFG_REG) & (0x1<<16)){}

	//turn off DDR-PLL
	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
	reg_val &= ~(0x1u<<31);
	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);

	//configuration update
	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
		reg_val |= (0x1u<<20);
	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);

	//wait for configuration update
		while(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<20)){}

//********************************************
//bit21:bit18=0001:pll 겠류0.4
//bit21:bit18=0010:pll 겠류0.3
//bit21:bit18=0100:pll 겠류0.2
//bit21:bit18=1000:pll 겠류0.1
//**********************************************
	if(para->dram_tpr13 & (0x1<<18))
	{
		mctl_write_w(CCM_BASE + 0x290, 0xD1303333);
	}
	else if(para->dram_tpr13 & (0x1<<19))//------------------------------------------------------------------------------------------------------------------------------------------------------------
	{
		mctl_write_w(CCM_BASE + 0x290, 0xcce06666);
	}
	else if(para->dram_tpr13 & (0x1<<20))//------------------------------------------------------------------------------------------------------------------------------------------------------------
	{
		mctl_write_w(CCM_BASE + 0x290, 0xc8909999);
	}
	else if(para->dram_tpr13 & (0x1<<21))//------------------------------------------------------------------------------------------------------------------------------------------------------------
	{
		mctl_write_w(CCM_BASE + 0x290, 0xc440cccc);
	}

	//==========DRAM PLL configuration=============
	//formula: DDR-PLL = ( 24 x N x K ) / M
	//when dram_clk >= 333MHz, DDR-PLL = 1/2 dram_clk
	//when dram_clk <  333MHz, DDR-PLL = 4 x dram_clk
//	if(para->dram_clk >= 336)	//PLL normal mode
	{
		reg_val = ((para->dram_clk/24 - 1)<<8);			//N
		reg_val |= 0<<4;								//K = 1
		reg_val |= (2-1)<<0;							//M = 2
		reg_val |= (0x1u<<31);
	}
//	else						//PLL bypass mode
//	{
//		reg_val = (((para->dram_clk<<1)/24 - 1)<<8);	//N
//		reg_val |= 1<<4;								//K = 2
//		reg_val |= (0)<<0;								//M = 1
//		reg_val |= (0x1u<<31);
//	}
	if( para->dram_tpr13 & (0xf<<18) )//------------------------------------------------------------------------------------------------------------------------------
		reg_val |= 0x1<<24;
	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);

	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
	reg_val |= 0x1<<20;
	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);

	while(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<20)){}

	//wait for stable ONLY FOR IC
	//while(!(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<28))){}

	__msdelay(10);	//adjust PLL delay to 10ms

	//put DRAMC out from reset states
	reg_val = mctl_read_w(CCM_AHB1_RST_REG0);
	reg_val |= (0x1<<14);
	mctl_write_w(CCM_AHB1_RST_REG0, reg_val);

	//turn on dram ahb1 gating
	reg_val = mctl_read_w(CCM_AHB1_GATE0_REG);
	reg_val |= (0x1<<14);
	mctl_write_w(CCM_AHB1_GATE0_REG, reg_val);

//	if(para->dram_clk < 336)	//PLL bypass mode
//	{
//		reg_val = mctl_read_w(MC_CCR);
//		reg_val &= ~(0xffff);
//		reg_val |= 0x7<<16;
//		mctl_write_w(MC_CCR, reg_val);
//	}
}


void mctl_com_init(__dram_para_t *para)
{
	unsigned int reg_val;

	if(para->dram_tpr13 & (0x1u<<31))
	{
		reg_val = 0;
		reg_val |= 0x3<<16;		//DDR3
		reg_val |= ((para->dram_para2>>4)&0xF)<<15;		//interleave mode
		reg_val |= 0x1<<12;		//16-bit data width
		reg_val |= 0x8<<8;		//2K-byte
		reg_val |= (((para->dram_para1>>16)&0xFF) - 1)<<4;		//16-bit row addr
		reg_val |= ((para->dram_para1>>24)&0xF)<<2;		//8 banks
	}
	else
	{
		reg_val = 0;
		reg_val |= 0x3<<16;		//DDR3
		reg_val |= 0<<15;		//interleave mode
		reg_val |= 0x1<<12;		//16-bit data width
		reg_val |= 0x8<<8;		//2K-byte
		reg_val |= 0xF<<4;		//16-bit row addr
		reg_val |= 0x1<<2;		//8 banks
	}
	mctl_write_w(MC_CR, reg_val);

	if(((mctl_read_w(MP_DXGCR(1)))&0x1) == 0)//8bit use page_size is 1KB
	{
		reg_val = mctl_read_w(MC_CR);
		reg_val &= ~(0xF<<8);
		reg_val |= 7<<8;
		reg_val &=~(0x1<<12);//8-bit data width mode
		mctl_write_w(MC_CR, reg_val);
	}

	reg_val = mctl_read_w(MC_SWONR);
	//reg_val |= (0x3<<10);
	reg_val |= 0x3FFFF;
	mctl_write_w(MC_SWONR, reg_val);

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
//	int cpu_id, pmu_id;
	unsigned int low_clk = 0;
	unsigned int factor = 0;
	unsigned int dram_value = 0;
	unsigned int i = 0;

	dram_para = (__dram_para_t *)para;

	/*
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
	*/

	dram_dbg("[DRAM]ver 1.1 clk = %d\n", dram_para->dram_clk);

	//watchdog_enable();
	ret_val = DRAMC_init(dram_para);

	__msdelay(10);
	//for(i = 0 ;i <1000000;i++);
	watchdog_enable();

	mctl_write_w(0x40000000,0x00123456);
	mctl_write_w(0x40000100,0x00123456);
	mctl_write_w(0x40001000,0x00123456);
	mctl_write_w(0x40010000,0x00123456);
	mctl_write_w(0x40100000,0x00123456);
	mctl_write_w(0x40020000,0x00123456);
	mctl_write_w(0x40002000,0x00123456);
	mctl_write_w(0x40000200,0x00123456);

	__asm("dmb");
	dram_value = mctl_read_w(0x40000000);
	dram_value += mctl_read_w(0x40000100);
	dram_value += mctl_read_w(0x40001000);
	dram_value += mctl_read_w(0x40010000);
	dram_value += mctl_read_w(0x40100000);
	dram_value += mctl_read_w(0x40020000);
	dram_value += mctl_read_w(0x40002000);
	dram_value += mctl_read_w(0x40000200);
	if(dram_value == (0x00123456 * 8) )
	{
		dram_dbg("dram ok\n");
		__asm("dmb");
		watchdog_disable();
	}
	else
	{
		dram_dbg("dram bad\n");
		for(;;);
	}

	//dram_dbg("[DRAM]ver 0.5 tpr13 = 0x%x\n", dram_para->dram_tpr13);

	return ret_val;
}

#if 0
int ss_bonding_id(void)
{
	int reg_val;
	int id = 0;
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

	// enable SS AHB clock
	reg_val = mctl_read_w(0x01c20000 + 0x60); //CCM_AHB1_GATE0_CTRL
	reg_val |= 0x1<<5;		//SS AHB clock on
	mctl_write_w(0x01c20000 + 0x60, reg_val);

	// release SS reset
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

	//0: A31	1: A31S		2: A3X PHONE
	return id;
}

int pmu_bonding_id(void)
{
	unsigned int id = 0;
	return (int)id;
}
#endif

void paraconfig(unsigned int *para, unsigned int mask, unsigned int value)
{
	*para &= ~(mask);
	*para |= value;
}

unsigned int dram_get_ahb1_clk(void)
{
	unsigned int src;
	unsigned int rval;
	unsigned int ahb1pdiv;
	unsigned int ahb1div;

	rval = mctl_read_w(CCM_AHB1_APB1_CFG);
	src = 0x3 & (rval >> 12);
	ahb1pdiv = (0x3 & (rval >> 6)) + 1;
	ahb1div = 1 << (0x3 & (rval >> 4));
	switch (src) {
		case 0:
			src = 32000;
			break;
		case 1:
			src = 24000000;
			break;
		case 2:
			src = dram_get_axi_clk();
			break;
		case 3:
			src = dram_get_pll_periph_clk() / ahb1pdiv;
			break;
	}
	return src / ahb1div;
}

unsigned int dram_get_axi_clk(void)
{
	unsigned int src;
	unsigned int rval;
	unsigned int axidiv;
	rval = mctl_read_w(CCM_CPU_AXI_CFG);
	src = 0x3 & (rval >> 16);
	axidiv = (0x7 & (rval >> 0)) + 1;
	switch (src) {
		case 0:
			src = 32000;
			break;
		case 1:
			src = 24000000;
			break;
		case 2:
		case 3:
			src = dram_get_pll_cpux_clk();
			break;
	}
	return src / axidiv;
}

unsigned int dram_get_pll_periph_clk(void)
{
	u32 rval = 0;
	u32 n, k;
	rval = mctl_read_w(CCM_PLL_PERH_CTRL);
	n = (0x1f & (rval >> 8)) + 1;
	k = (0x3 & (rval >> 4)) + 1;
	return (24000000 * n * k)>>1;
}

unsigned int dram_get_pll_cpux_clk(void)
{
	unsigned int rval = 0;
	unsigned int n, k, m;
	rval = readl(CCM_PLL_CPUX_CTRL);
	n = (0x1f & (rval >> 8)) + 1;
	k = (0x3 & (rval >> 4)) + 1;
	m = (0x3 & (rval >> 0)) + 1;
	return 24000000 * n * k / m;
}

void mctl_init(void)
{
	__dram_para_t parameters = {
			552,
			3,
			0xBB7B,
			1,
			0x1102400,
			0,
			0x1A50,
			0x4,
			0x10,
			0,
			0xAB83DEF,
			0x6052080,
			0xD034,
			0x448C5533,
			0x08010D00,
			0x340B20F,
			0x20D118CC,
			0x14062485,
			0x220D1D52,
			0x1E078C22,
			0x3C,
			0,
			0,
			0x30000		//0x10000
	};

	init_DRAM(0, &parameters);
}
