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

//#include "boot0_i.h"
#include "dram_i.h"
#include "types.h"
#include "mctl_hal.h"

#ifdef SUN7I_HOMLET
#include "axp15_ctrl.h"
#endif

char dram_driver_version[8]={'1','.','1','5'};
//typedef  boot_dram_para_t		__dram_para_t;
__dram_para_t *dram_para;

__u32 super_standby_flag = 0;

#pragma arm section rwdata="RW_KRNL_CLK",code="CODE_KRNL_CLK",rodata="RO_KRNL_CLK",zidata="ZI_KRNL_CLK"

extern __u32 get_cyclecount (void);
void mctl_delay(__u32 dly)
{
	__u32	i;
	for(i=0; i<dly; i++){};
}
/*
*********************************************************************************************************
*                                   DRAM INIT
*
* Description: dram init function
*
* Arguments  : para     dram config parameter
*
*
* Returns    : result
*
* Note       :
*********************************************************************************************************
*/
void mctl_ddr3_reset(void)
{
	__u32 reg_val;

	reg_val = mctl_read_w(SDR_CR);
	reg_val &= ~(0x1<<12);
	mctl_write_w(SDR_CR, reg_val);
	mctl_delay(0x100);
	reg_val = mctl_read_w(SDR_CR);
	reg_val |= (0x1<<12);
	mctl_write_w(SDR_CR, reg_val);
}

void mctl_set_drive(void)
{
	__u32 reg_val;

	reg_val = mctl_read_w(SDR_CR);
	reg_val |= (0x6<<12);
	reg_val |= 0xFFC;
	reg_val &= ~0x3;
	reg_val &= ~(0x3<<28);
//	reg_val |= 0x7<<20;
	mctl_write_w(SDR_CR, reg_val);
}

void mctl_itm_disable(void)
{
	__u32 reg_val = 0x0;

	reg_val = mctl_read_w(SDR_CCR);
	reg_val |= 0x1<<28;
	reg_val &= ~(0x1U<<31);          //danielwang, 2012-05-18
	mctl_write_w(SDR_CCR, reg_val);
}

void mctl_itm_enable(void)
{
	__u32 reg_val = 0x0;

	reg_val = mctl_read_w(SDR_CCR);
	reg_val &= ~(0x1<<28);
	mctl_write_w(SDR_CCR, reg_val);
}

void mctl_enable_dll0(__u32 phase)
{
    mctl_write_w(SDR_DLLCR0, (mctl_read_w(SDR_DLLCR0) & ~(0x3f<<6)) | (((phase>>16)&0x3f)<<6));
	mctl_write_w(SDR_DLLCR0, (mctl_read_w(SDR_DLLCR0) & ~0x40000000) | 0x80000000);

	mctl_delay(0x100);
	//delay_us(10);

	mctl_write_w(SDR_DLLCR0, mctl_read_w(SDR_DLLCR0) & ~0xC0000000);

	mctl_delay(0x1000);
	//delay_us(10);

	mctl_write_w(SDR_DLLCR0, (mctl_read_w(SDR_DLLCR0) & ~0x80000000) | 0x40000000);
	mctl_delay(0x1000);
	//delay_us(100);
}

void mctl_enable_dllx(__u32 phase)
{
	__u32 i = 0;
    __u32 reg_val;
    __u32 dll_num;
    __u32	dqs_phase = phase;

	reg_val = mctl_read_w(SDR_DCR);
	reg_val >>=6;
	reg_val &= 0x7;
	if(reg_val == 3)
		dll_num = 5;
	else
		dll_num = 3;

    for(i=1; i<dll_num; i++)
	{
		mctl_write_w(SDR_DLLCR0+(i<<2), (mctl_read_w(SDR_DLLCR0+(i<<2)) & ~(0xf<<14)) | ((dqs_phase&0xf)<<14));
		mctl_write_w(SDR_DLLCR0+(i<<2), (mctl_read_w(SDR_DLLCR0+(i<<2)) & ~0x40000000) | 0x80000000);
		dqs_phase = dqs_phase>>4;
	}

	mctl_delay(0x100);
	//delay_us(10);

    for(i=1; i<dll_num; i++)
	{
		mctl_write_w(SDR_DLLCR0+(i<<2), mctl_read_w(SDR_DLLCR0+(i<<2)) & ~0xC0000000);
	}

	mctl_delay(0x1000);
	//delay_us(10);

    for(i=1; i<dll_num; i++)
	{
		mctl_write_w(SDR_DLLCR0+(i<<2), (mctl_read_w(SDR_DLLCR0+(i<<2)) & ~0x80000000) | 0x40000000);
	}
	mctl_delay(0x1000);
	//delay_us(100);
}

void mctl_disable_dll(void)
{
	__u32 reg_val;

	reg_val = mctl_read_w(SDR_DLLCR0);
	reg_val &= ~(0x1<<30);
	reg_val |= 0x1U<<31;
	mctl_write_w(SDR_DLLCR0, reg_val);

	reg_val = mctl_read_w(SDR_DLLCR1);
	reg_val &= ~(0x1<<30);
	reg_val |= 0x1U<<31;
	mctl_write_w(SDR_DLLCR1, reg_val);

	reg_val = mctl_read_w(SDR_DLLCR2);
	reg_val &= ~(0x1<<30);
	reg_val |= 0x1U<<31;
	mctl_write_w(SDR_DLLCR2, reg_val);

	reg_val = mctl_read_w(SDR_DLLCR3);
	reg_val &= ~(0x1<<30);
	reg_val |= 0x1U<<31;
	mctl_write_w(SDR_DLLCR3, reg_val);

	reg_val = mctl_read_w(SDR_DLLCR4);
	reg_val &= ~(0x1<<30);
	reg_val |= 0x1U<<31;
	mctl_write_w(SDR_DLLCR4, reg_val);
}

void mctl_configure_hostport(void)
{
	__u32 i;
	__u32 hpcr_value[32] = {
		0x00000301,0x00000301,0x00000301,0x00000301,
		0x00000301,0x00000301,0x00000301,0x00000301,
		0x0,       0x0,       0x0,       0x0,
		0x0,       0x0,       0x0,       0x0,
		0x00001031,0x00001031,0x00000735,0x00001035,
		0x00001035,0x00000731,0x00001031,0x00000735,
		0x00001035,0x00001031,0x00000731,0x00001035,
		0x00001031,0x00000301,0x00000301,0x00000731,
	};

	for(i=0; i<8; i++)
	{
		mctl_write_w(SDR_HPCR + (i<<2), hpcr_value[i]);
	}

	for(i=16; i<28; i++)
	{
		mctl_write_w(SDR_HPCR + (i<<2), hpcr_value[i]);
	}

	mctl_write_w(SDR_HPCR + (29<<2), hpcr_value[i]);
	mctl_write_w(SDR_HPCR + (31<<2), hpcr_value[i]);
}

void mctl_setup_dram_clock(__u32 clk)
{
    __u32 reg_val;

    //setup DRAM PLL
    reg_val = mctl_read_w(DRAM_CCM_SDRAM_PLL_REG);

    //DISABLE PLL before configuration by yeshaozhen at 20130724
    reg_val &= ~(0x1<<31);  	//PLL disable before configure
    mctl_write_w(DRAM_CCM_SDRAM_PLL_REG, reg_val);
	mctl_delay(0x200);
    //delay_us(20);
    //20130724 end

    reg_val &= ~0x3;
    reg_val |= 0x1;                                             //m factor
    reg_val &= ~(0x3<<4);
    reg_val |= 0x1<<4;                                          //k factor
    reg_val &= ~(0x1f<<8);
    reg_val |= ((clk/24)&0x1f)<<8;                              //n factor
    reg_val &= ~(0x3<<16);
    reg_val |= 0x1<<16;                                         //p factor
    reg_val &= ~(0x1<<29);                                      //clock output disable
    reg_val |= (__u32)0x1<<31;                                  //PLL En
    mctl_write_w(DRAM_CCM_SDRAM_PLL_REG, reg_val);
	mctl_delay(0x100000);
	//delay_us(10000);
	reg_val = mctl_read_w(DRAM_CCM_SDRAM_PLL_REG);
	reg_val |= 0x1<<29;
    mctl_write_w(DRAM_CCM_SDRAM_PLL_REG, reg_val);

	//setup MBUS clock
    reg_val = (0x1U<<31) | (0x1<<24) | (0x1<<0) | (0x1<<16);
    mctl_write_w(DRAM_CCM_MUS_CLK_REG, reg_val);

    //open DRAMC AHB & DLL register clock
    //close it first
    reg_val = mctl_read_w(DRAM_CCM_AHB_GATE_REG);
    reg_val &= ~(0x3<<14);
    mctl_write_w(DRAM_CCM_AHB_GATE_REG, reg_val);
	//delay_us(10);
	mctl_delay(0x100);
    //then open it
    reg_val |= 0x3<<14;
    mctl_write_w(DRAM_CCM_AHB_GATE_REG, reg_val);
	//delay_us(10);
	mctl_delay(0x100);

}

#define RTC_BASE			0x01c20c00
#define SDR_GP_REG0		    (RTC_BASE + 0x120)
#define SDR_GP_REG1         (RTC_BASE + 0x124)
#define SDR_GP_REG2         (RTC_BASE + 0x128)

__s32 DRAMC_init_config(__dram_para_t *para)
{
    __u32 reg_val;
		__u32 hold_flag = 0;
    __u8  reg_value;
    __s32 ret_val;
    __u32 i;

//    hold_flag = mctl_read_w(SDR_DPCR);

	//check input dram parameter structure
    if(!para)
	{
    	//dram parameter is invalid
    	return 0;
	}

	//close AHB clock for DRAMC,--added by yeshaozhen at 20130724
	reg_val = mctl_read_w(DRAM_CCM_AHB_GATE_REG);
	reg_val &= ~(0x3<<14);
	mctl_write_w(DRAM_CCM_AHB_GATE_REG, reg_val);
	//delay_us(10);
	mctl_delay(0x100);
	//20130724 AHB close end

	//setup DRAM relative clock
	mctl_setup_dram_clock(para->dram_clk);

	mctl_set_drive();

	//dram clock off
	DRAMC_clock_output_en(0);

	mctl_itm_disable();
	mctl_enable_dll0(para->dram_tpr3);

	//2 ranks config,liuke added 20140722
	if (para->dram_rank_num == 2)
	{
		reg_val = mctl_read_w(SDR_CR);	//2 ranks,remap cs1,odt1
		reg_val &= ~(0x3u<<30);
		reg_val |= (0x0u<<30);
		mctl_write_w(SDR_CR, reg_val);
	}
	else
	{
		reg_val = mctl_read_w(SDR_CR);
		reg_val &= ~(0x3u<<30);
		reg_val |= (0x3u<<30);
		mctl_write_w(SDR_CR, reg_val);
	}

	//configure external DRAM
	reg_val = 0;
	if(para->dram_type == 3)
		reg_val |= 0x1;
	reg_val |= (para->dram_io_width>>3) <<1;

	if(para->dram_chip_density == 256)
		reg_val |= 0x0<<3;
	else if(para->dram_chip_density == 512)
		reg_val |= 0x1<<3;
	else if(para->dram_chip_density == 1024)
		reg_val |= 0x2<<3;
	else if(para->dram_chip_density == 2048)
		reg_val |= 0x3<<3;
	else if(para->dram_chip_density == 4096)
		reg_val |= 0x4<<3;
	else if(para->dram_chip_density == 8192)
		reg_val |= 0x5<<3;
	else
		reg_val |= 0x0<<3;
	reg_val |= ((para->dram_bus_width>>3) - 1)<<6;
	reg_val |= (para->dram_rank_num -1)<<10;
	reg_val |= 0x1<<12;
	reg_val |= ((0x1)&0x3)<<13;
	mctl_write_w(SDR_DCR, reg_val);

    //SDR_ZQCR1 set bit24 to 1
    reg_val  = mctl_read_w(SDR_ZQCR1);
    reg_val |= (0x1<<24) | (0x1<<1);
    if(para->dram_tpr4 & 0x2)
    {
    	reg_val &= ~((0x1<<24) | (0x1<<1));
    }
    mctl_write_w(SDR_ZQCR1, reg_val);

    //read odt enable;  liuke edit for read odt support,20140518
    reg_val = mctl_read_w(SDR_ODTCR);
    reg_val &=~(0xff);
    if(para->dram_odt_en)		//for odt enable
    {
    	reg_val |= (1<<0);		//1 rank config
    	if (para->dram_rank_num == 2)	//2 ranks config
    		reg_val |= (1<<4);
    }
    mctl_write_w(SDR_ODTCR, reg_val);

  	//dram clock on
  	DRAMC_clock_output_en(1);

#ifdef SUN7I_HOMLET
	//clear dram flag for box application
	mctl_write_w(SDR_DPCR, 0x16510000);
	while(mctl_read_w(SDR_DPCR) & 0x1);	//wait for flag cleared
		//flag cleared
#endif
    hold_flag = mctl_read_w(SDR_DPCR);
    if(hold_flag == 0) //normal branch
    {
	    //set odt impendance divide ratio
	    reg_val=((para->dram_zq)>>8)&0xfffff;
	    reg_val |= ((para->dram_zq)&0xff)<<20;
	    reg_val |= (para->dram_zq)&0xf0000000;
	    reg_val |= (0x1u<<31);
	    mctl_write_w(SDR_ZQCR0, reg_val);

	    while( !((mctl_read_w(SDR_ZQSR)&(0x1u<<31))) );

	}

	    //Set CKE Delay to about 1ms
	    reg_val = mctl_read_w(SDR_IDCR);
	    reg_val |= 0x1ffff;
	    mctl_write_w(SDR_IDCR, reg_val);

//      //dram clock on
//      DRAMC_clock_output_en(1);
    //reset external DRAM when CKE is Low
    //reg_val = mctl_read_w(SDR_DPCR);
    if(hold_flag == 0) //normal branch
	{
		//reset ddr3
		mctl_ddr3_reset();
	}
	else
	{
		//setup the DDR3 reset pin to high level
		reg_val = mctl_read_w(SDR_CR);
		reg_val |= (0x1<<12);
		mctl_write_w(SDR_CR, reg_val);
	}

	mctl_delay(0x10);
    while(mctl_read_w(SDR_CCR) & (0x1U<<31)) {};

    mctl_enable_dllx(para->dram_tpr3);
    //set I/O configure register
//    reg_val = 0x00cc0000;
//   reg_val |= (para->dram_odt_en)&0x3;
//  reg_val |= ((para->dram_odt_en)&0x3)<<30;
//    mctl_write_w(SDR_IOCR, reg_val);

    //liuke edit for read odt support,20140518
    if(para->dram_odt_en)
    {
    reg_val = (0x3u<<30) | (0xcc<<16) | (0x3<<0);			//DQ and DQS DQ pins odt on for default
    reg_val &= ~((para->dram_tpr5 & 0x3)<<30 | (para->dram_tpr5 & 0x3));	//dram_tpr5 bit1:0 for DQS/DQ odt disable
    mctl_write_w(SDR_IOCR, reg_val);
    }

	//set refresh period
	DRAMC_set_autorefresh_cycle(para->dram_clk);

	//set timing parameters
	mctl_write_w(SDR_TPR0, para->dram_tpr0);
	mctl_write_w(SDR_TPR1, para->dram_tpr1);
	mctl_write_w(SDR_TPR2, para->dram_tpr2);

	//set mode register
	if(para->dram_type==3)			//ddr3
	{
		reg_val = 0x1<<12;
		reg_val |= (para->dram_cas - 4)<<4;
		reg_val |= 0x5<<9;
	}
	else if(para->dram_type==2)		//ddr2
	{
		reg_val = 0x2;
		reg_val |= para->dram_cas<<4;
		reg_val |= 0x5<<9;
	}
	mctl_write_w(SDR_MR, reg_val);

    mctl_write_w(SDR_EMR, para->dram_emr1);
	mctl_write_w(SDR_EMR2, para->dram_emr2);
	mctl_write_w(SDR_EMR3, para->dram_emr3);

	//set DQS window mode
	reg_val = mctl_read_w(SDR_CCR);
	reg_val |= 0x1U<<14;
	reg_val &= ~(0x1U<<17);
		//2T & 1T mode
	if(para->dram_tpr4 & 0x1)
	{
		reg_val |= 0x1<<5;
	}
	mctl_write_w(SDR_CCR, reg_val);

	//initial external DRAM
	reg_val = mctl_read_w(SDR_CCR);
	reg_val |= 0x1U<<31;
	mctl_write_w(SDR_CCR, reg_val);

	while(mctl_read_w(SDR_CCR) & (0x1U<<31)) {};

    //setup zq calibration manual
    //reg_val = mctl_read_w(SDR_DPCR);
    if(hold_flag == 1)
	{

		super_standby_flag = 1;

		    	//disable auto-fresh			//by cpl 2013-5-6
    	reg_val = mctl_read_w(SDR_DRR);
    	reg_val |= 0x1U<<31;
    	mctl_write_w(SDR_DRR, reg_val);

        reg_val = mctl_read_w(SDR_GP_REG0);
        reg_val &= 0x000fffff;
       // reg_val |= 0x17b00000;
        reg_val |= (0x1<<28) | (para->dram_zq<<20);
		mctl_write_w(SDR_ZQCR0, reg_val);

		//03-08
		reg_val = mctl_read_w(SDR_DCR);
		reg_val &= ~(0x1fU<<27);
		reg_val |= 0x12U<<27;
		mctl_write_w(SDR_DCR, reg_val);
		while( mctl_read_w(SDR_DCR)& (0x1U<<31) );

		mctl_delay(0x100);

		//dram pad hold off
		mctl_write_w(SDR_DPCR, 0x16510000);

		while(mctl_read_w(SDR_DPCR) & 0x1){}

		//exit self-refresh state
		reg_val = mctl_read_w(SDR_DCR);
		reg_val &= ~(0x1fU<<27);
		reg_val |= 0x17U<<27;
		mctl_write_w(SDR_DCR, reg_val);

		//check whether command has been executed
		while( mctl_read_w(SDR_DCR)& (0x1U<<31) );
    	//disable auto-fresh			//by cpl 2013-5-6
    	reg_val = mctl_read_w(SDR_DRR);
    	reg_val &= ~(0x1U<<31);
    	mctl_write_w(SDR_DRR, reg_val);
		mctl_delay(0x100);;

	}

	//scan read pipe value
	mctl_itm_enable();

	if(hold_flag == 0)//normal branch
    {
		if(para->dram_tpr3 & (0x1U<<31))
		{
			ret_val = DRAMC_scan_dll_para();
			if(ret_val == 0)
			{
				para->dram_tpr3 = 0x0;
				para->dram_tpr3 |= (((mctl_read_w(SDR_DLLCR0)>>6)&0x3f)<<16);
				para->dram_tpr3 |= (((mctl_read_w(SDR_DLLCR1)>>14)&0xf)<<0);
				para->dram_tpr3 |= (((mctl_read_w(SDR_DLLCR2)>>14)&0xf)<<4);
				para->dram_tpr3 |= (((mctl_read_w(SDR_DLLCR3)>>14)&0xf)<<8);
				para->dram_tpr3 |= (((mctl_read_w(SDR_DLLCR4)>>14)&0xf)<<12);
			}
		}
		else
		{
			ret_val = DRAMC_scan_readpipe();
		}

		if(ret_val < 0)
		{
			return 0;
		}
		/* save training para */ /*liuke edit 20140909 */
		para->dram_tpr6 = mctl_read_w(SDR_RSLR0);
		//dram_dbg("Save SDR_RSLR0= %x\n",para->dram_tpr6);
		para->dram_tpr7 = mctl_read_w(SDR_RDQSGR);
		//dram_dbg("Save SDR_RDQSGR = %x\n",para->dram_tpr7);
		if(para->dram_rank_num == 2)	/*rank1 parameter save */
		{
			para->dram_tpr8 = mctl_read_w(SDR_RSLR1);
			//dram_dbg("Save SDR_RSLR1 = %x\n",para->dram_tpr8);
			para->dram_tpr7 = mctl_read_w(SDR_RDQSGR1);
			//dram_dbg("Save SDR_RDQSGR1= %x\n",para->dram_tpr9);
		}
	}
	else
	{
		//Read back dqs gating value
		mctl_write_w(SDR_RSLR0,para->dram_tpr6);
		//dram_dbg("Read back SDR_RSLR0 = %x\n",para->dram_tpr6);
		mctl_write_w(SDR_RDQSGR, para->dram_tpr7);
		//dram_dbg("Read back SDR_RDQSGR = %x\n",para->dram_tpr7);
		if(para->dram_rank_num == 2)	/*rank1 parameter read back */
		{
			mctl_write_w(SDR_RSLR1, para->dram_tpr8);
			//dram_dbg("Read back SDR_RSLR1 = %x\n",para->dram_tpr8);
			mctl_write_w(SDR_RDQSGR1, para->dram_tpr9);
			//dram_dbg("Read back SDR_RDQSGR1 = %x\n",para->dram_tpr9);
		}
	}
	//configure all host port
	mctl_configure_hostport();

	return DRAMC_get_dram_size();
}

__s32 DRAMC_init_proc_ex(__dram_para_t *para)
{
    __u32 i;
    __s32 ret = 0;

    for(i=0; i<3; i++)
    {
        ret = DRAMC_init_config(para);
        if(ret) break;
    }

    return ret;
}
/* liuke edit 20140903,this branch no use any more */
/*
__s32 DRAMC_init(__dram_para_t *para)
{
    __u32 reg_val;
		__u32 err_flag = 0;
    __s32 ret_val = 0;
    __u32 i,m;
    __u32 bonding_id;
    __u8  dram_space_try = 0;	//try dram space when "1"

	//check input dram parameter structure
    if(!para)
		{
    	//dram parameter is invalid
    	return 0;
		}

    dram_space_try = (para->dram_tpr3 & 0x10);	//try dram space when bit 4 of tpr3 asserted, SDR_DPCR must be 0 when bit asserted

    if(dram_space_try)	//SW branch, space try
  	{
			//get bonding ID
			reg_val = mctl_read_w(DRAM_CCM_AHB_GATE_REG);
			reg_val |= 0x1<<5;
			mctl_write_w(DRAM_CCM_AHB_GATE_REG, reg_val);
			mctl_write_w(DRAM_CCM_SS_CLK_REG, 0x80000000);

			reg_val = mctl_read_w(DRAM_SS_CTRL_REG);
			bonding_id = (reg_val >>16)&0x7;

			//try 16/32 bus width
			para->dram_type = 3;
			para->dram_rank_num = 1;
			para->dram_chip_density = 8192;
			para->dram_io_width	= 16;
			para->dram_bus_width = 32;
			if(!DRAMC_init_proc_ex(para))
			{
				para->dram_bus_width = 16;
				if(!DRAMC_init_proc_ex(para))
				{
					return 0;
				}
			}

			for(m =0x50000000; m<0xC0000000; m+=0x10000000)
			{
				for(i=0; i<32; i++)
				{
					if(mctl_read_w(m + i*4) != mctl_read_w(0x40000000 + i*4))
						break;
				}
				if(i == 32)
				{
					err_flag = 1;
					para->dram_size = (m - 0x40000000)>>20;
					para->dram_chip_density = (m - 0x40000000)>>17;
					if(para->dram_bus_width == 32)
						para->dram_chip_density>>=1;
					break;

				}
			}

			if(m == 0xC0000000)
			{
				para->dram_size = 2048;
				para->dram_chip_density = 8192;

			}

			reg_val = mctl_read_w(SDR_DCR);
			reg_val &= ~(0x7<<3);
			if(para->dram_chip_density == 256)
				reg_val |= 0x0<<3;
			else if(para->dram_chip_density == 512)
				reg_val |= 0x1<<3;
			else if(para->dram_chip_density == 1024)
				reg_val |= 0x2<<3;
			else if(para->dram_chip_density == 2048)
				reg_val |= 0x3<<3;
			else if(para->dram_chip_density == 4096)
				reg_val |= 0x4<<3;
			else if(para->dram_chip_density == 8192)
				reg_val |= 0x5<<3;
			else
				reg_val |= 0x0<<3;
			mctl_write_w(SDR_DCR, reg_val);

			mctl_configure_hostport();

			ret_val	= 1;
  	}
    else			//boot branch
  	{
  		ret_val	= DRAMC_init_config(para);
  	}

  	return ret_val;
}
*/
/*
__s32 DRAMC_init_EX(__dram_para_t *para)
{
	__u32 i = 0;
	__s32 ret_val = 0;

	for(i=0; i<3; i++)
	{
	  ret_val = DRAMC_init(para);
	  if(ret_val) break;
	}

	return ret_val;
}
*/
/*
*********************************************************************************************************
*                                   DRAM EXIT
*
* Description: dram exit;
*
* Arguments  : none;
*
* Returns    : result;
*
* Note       :
*********************************************************************************************************
*/
__s32 DRAMC_exit(void)
{
    return 0;
}

/*
*********************************************************************************************************
*                                   CHECK DDR READPIPE
*
* Description: check ddr readpipe;
*
* Arguments  : none
*
* Returns    : result, 0:fail, 1:success;
*
* Note       :
*********************************************************************************************************
*/
__s32 DRAMC_scan_readpipe(void)
{
	__u32 reg_val;

	//Clear Error Flag
	reg_val = mctl_read_w(SDR_CSR);
	reg_val &= ~(0x1<<20);
	mctl_write_w(SDR_CSR, reg_val);

	//data training trigger
	reg_val = mctl_read_w(SDR_CCR);
	reg_val |= 0x1<<30;
	mctl_write_w(SDR_CCR, reg_val);

	//check whether data training process is end
	while(mctl_read_w(SDR_CCR) & (0x1<<30)) {};

	//check data training result
	reg_val = mctl_read_w(SDR_CSR);
	if(reg_val & (0x1<<20))
	{
		return -1;
	}

	return (0);
}


/*
*********************************************************************************************************
*                                   SCAN DDR DLL Parameters
*
* Description: Scan DDR DLL Parameters;
*
* Arguments  : __dram_para_t
*
* Returns    : result, -1:fail, 0:success;
*
* Note       :
*********************************************************************************************************
*/
#define MCTL_DQS_DLY_COUNT     7
#define MCTL_CLK_DLY_COUNT     15
#define MCTL_SCAN_PASS		   0

__s32 DRAMC_scan_dll_para(void)
{
    const __u32 DQS_DLY[MCTL_DQS_DLY_COUNT] = {0x3, 0x2, 0x1, 0x0, 0xE, 0xD, 0xC};  //36~144
    const __u32 CLK_DLY[MCTL_CLK_DLY_COUNT] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38};
    __u32 clk_dqs_count[MCTL_CLK_DLY_COUNT>MCTL_DQS_DLY_COUNT?MCTL_CLK_DLY_COUNT:MCTL_DQS_DLY_COUNT];
    __u32 dqs_i, clk_i;
    __u32 max_val, min_val;
    __u32 dqs_index, clk_index;

    //Find DQS_DLY Pass Count for every CLK_DLY
    for(clk_i = 0; clk_i < MCTL_CLK_DLY_COUNT; clk_i ++)
    {
    	clk_dqs_count[clk_i] = 0;
    	mctl_write_w(SDR_DLLCR0, (mctl_read_w(SDR_DLLCR0) & ~(0x3f<<6)) | ((CLK_DLY[clk_i] & 0x3f) << 6));
    	for(dqs_i = 0; dqs_i < MCTL_DQS_DLY_COUNT; dqs_i ++)
    	{
    		mctl_write_w(SDR_DLLCR1, (mctl_read_w(SDR_DLLCR1) & ~(0xf<<14)) | ((DQS_DLY[dqs_i] & 0xf) << 14));
    		mctl_write_w(SDR_DLLCR2, (mctl_read_w(SDR_DLLCR2) & ~(0xf<<14)) | ((DQS_DLY[dqs_i] & 0xf) << 14));
    		mctl_write_w(SDR_DLLCR3, (mctl_read_w(SDR_DLLCR3) & ~(0xf<<14)) | ((DQS_DLY[dqs_i] & 0xf) << 14));
    		mctl_write_w(SDR_DLLCR4, (mctl_read_w(SDR_DLLCR4) & ~(0xf<<14)) | ((DQS_DLY[dqs_i] & 0xf) << 14));
    		//delay_us(1000);
			if(DRAMC_scan_readpipe()==MCTL_SCAN_PASS)
    		{
    			clk_dqs_count[clk_i] ++;
    		}
    	}
    }

    //Test DQS_DLY Pass Count for every CLK_DLY from up to down
    for(dqs_i = MCTL_DQS_DLY_COUNT; dqs_i > 0; dqs_i--)
    {
    	max_val = MCTL_CLK_DLY_COUNT;
    	min_val = MCTL_CLK_DLY_COUNT;
    	for(clk_i=0; clk_i<MCTL_CLK_DLY_COUNT; clk_i++)
    	{
    		if(clk_dqs_count[clk_i]==dqs_i)
    		{
    			max_val = clk_i;
    			if(min_val==MCTL_CLK_DLY_COUNT)
    			{
    				min_val = clk_i;
    			}
    		}
    	}
    	if(max_val < MCTL_CLK_DLY_COUNT)
    	{
    		break;
    	}
    }
    if(!dqs_i)
    {
    	//Fail to Find a CLK_DLY
    	mctl_write_w(SDR_DLLCR0, mctl_read_w(SDR_DLLCR0) & ~(0x3f<<6));
    	mctl_write_w(SDR_DLLCR1, mctl_read_w(SDR_DLLCR1) & ~(0xf<<14));
    	mctl_write_w(SDR_DLLCR2, mctl_read_w(SDR_DLLCR2) & ~(0xf<<14));
    	mctl_write_w(SDR_DLLCR3, mctl_read_w(SDR_DLLCR3) & ~(0xf<<14));
    	mctl_write_w(SDR_DLLCR4, mctl_read_w(SDR_DLLCR4) & ~(0xf<<14));
	//	delay_us(1000);
    	return DRAMC_scan_readpipe();
    }

    //Find the middle index of CLK_DLY
    clk_index = (max_val + min_val) >> 1;
    if((max_val == (MCTL_CLK_DLY_COUNT-1))&&(min_val>0))
    {
    	//if CLK_DLY[MCTL_CLK_DLY_COUNT] is very good, then the middle value can be more close to the max_val
    	clk_index = (MCTL_CLK_DLY_COUNT + clk_index) >> 1;
    }
    else if((max_val < (MCTL_CLK_DLY_COUNT-1))&&(min_val==0))
    {
    	//if CLK_DLY[0] is very good, then the middle value can be more close to the min_val
    	clk_index >>= 1;
    }
    if(clk_dqs_count[clk_index] < dqs_i)
    {
    	//if the middle value is not very good, can take any value of min_val/max_val
    	clk_index = min_val;
    }

    //Find the middle index of DQS_DLY for the CLK_DLY got above, and Scan read pipe again
    mctl_write_w(SDR_DLLCR0, (mctl_read_w(SDR_DLLCR0) & ~(0x3f<<6)) | ((CLK_DLY[clk_index] & 0x3f) << 6));
    max_val = MCTL_DQS_DLY_COUNT;
    min_val = MCTL_DQS_DLY_COUNT;
    for(dqs_i = 0; dqs_i < MCTL_DQS_DLY_COUNT; dqs_i++)
    {
    	clk_dqs_count[dqs_i] = 0;
    	mctl_write_w(SDR_DLLCR1, (mctl_read_w(SDR_DLLCR1) & ~(0xf<<14)) | ((DQS_DLY[dqs_i] & 0xf) << 14));
    	mctl_write_w(SDR_DLLCR2, (mctl_read_w(SDR_DLLCR2) & ~(0xf<<14)) | ((DQS_DLY[dqs_i] & 0xf) << 14));
    	mctl_write_w(SDR_DLLCR3, (mctl_read_w(SDR_DLLCR3) & ~(0xf<<14)) | ((DQS_DLY[dqs_i] & 0xf) << 14));
    	mctl_write_w(SDR_DLLCR4, (mctl_read_w(SDR_DLLCR4) & ~(0xf<<14)) | ((DQS_DLY[dqs_i] & 0xf) << 14));
		//delay_us(1000);
    	if(DRAMC_scan_readpipe()==MCTL_SCAN_PASS)
    	{
    		clk_dqs_count[dqs_i] = 1;
    		max_val = dqs_i;
    		if(min_val == MCTL_DQS_DLY_COUNT) min_val = dqs_i;
    	}
    }
    if(max_val < MCTL_DQS_DLY_COUNT)
    {
    	dqs_index = (max_val + min_val) >> 1;
    	if((max_val == (MCTL_DQS_DLY_COUNT-1))&&(min_val>0))
    	{
    	   	//if DQS_DLY[MCTL_DQS_DLY_COUNT-1] is very good, then the middle value can be more close to the max_val
    		dqs_index = (MCTL_DQS_DLY_COUNT + dqs_index) >> 1;
    	}
    	else if((max_val < (MCTL_DQS_DLY_COUNT-1))&&(min_val==0))
    	{
    	   	//if DQS_DLY[0] is very good, then the middle value can be more close to the min_val
    		dqs_index >>= 1;
    	}
    	if(!clk_dqs_count[dqs_index])
    	{
    		//if the middle value is not very good, can take any value of min_val/max_val
    		dqs_index = min_val;
    	}
    	mctl_write_w(SDR_DLLCR1, (mctl_read_w(SDR_DLLCR1) & ~(0xf<<14)) | ((DQS_DLY[dqs_index] & 0xf) << 14));
    	mctl_write_w(SDR_DLLCR2, (mctl_read_w(SDR_DLLCR2) & ~(0xf<<14)) | ((DQS_DLY[dqs_index] & 0xf) << 14));
    	mctl_write_w(SDR_DLLCR3, (mctl_read_w(SDR_DLLCR3) & ~(0xf<<14)) | ((DQS_DLY[dqs_index] & 0xf) << 14));
    	mctl_write_w(SDR_DLLCR4, (mctl_read_w(SDR_DLLCR4) & ~(0xf<<14)) | ((DQS_DLY[dqs_index] & 0xf) << 14));
		//delay_us(1000);
		mctl_delay(0x1000);
    	return DRAMC_scan_readpipe();
    }
    else
    {
    	//Fail to Find a DQS_DLY
    	mctl_write_w(SDR_DLLCR0, mctl_read_w(SDR_DLLCR0) & ~(0x3f<<6));
    	mctl_write_w(SDR_DLLCR1, mctl_read_w(SDR_DLLCR1) & ~(0xf<<14));
    	mctl_write_w(SDR_DLLCR2, mctl_read_w(SDR_DLLCR2) & ~(0xf<<14));
    	mctl_write_w(SDR_DLLCR3, mctl_read_w(SDR_DLLCR3) & ~(0xf<<14));
    	mctl_write_w(SDR_DLLCR4, mctl_read_w(SDR_DLLCR4) & ~(0xf<<14));
		//delay_us(1000);
		mctl_delay(0x1000);
    	return DRAMC_scan_readpipe();
    }
}

/*
*********************************************************************************************************
*                                   DRAM SCAN READ PIPE
*
* Description: dram scan read pipe
*
* Arguments  : none
*
* Returns    : result, 0:fail, 1:success;
*
* Note       :
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                   DRAM CLOCK CONTROL
*
* Description: dram get clock
*
* Arguments  : on   dram clock output (0: disable, 1: enable)
*
* Returns    : none
*
* Note       :
*********************************************************************************************************
*/
void DRAMC_clock_output_en(__u32 on)
{
    __u32 reg_val;

    reg_val = mctl_read_w(SDR_CR);

    if(on)
        reg_val |= 0x1<<16;
    else
        reg_val &= ~(0x1<<16);

    mctl_write_w(SDR_CR, reg_val);
}
/*
*********************************************************************************************************
* Description: Set autorefresh cycle
*
* Arguments  : clock value in MHz unit
*
* Returns    : none
*
* Note       :
*********************************************************************************************************
*/
void DRAMC_set_autorefresh_cycle(__u32 clk)
{
	__u32 reg_val;
//	__u32 dram_size;
	__u32 tmp_val;

//	dram_size = mctl_read_w(SDR_DCR);
//	dram_size >>=3;
//	dram_size &= 0x7;

//	if(clk < 600)
	{
//		if(dram_size<=0x2)
//			tmp_val = (131*clk)>>10;
//		else
//			tmp_val = (336*clk)>>10;
		reg_val = 0x83;
		tmp_val = (7987*clk)>>10;
		tmp_val = tmp_val*9 - 200;
		reg_val |= tmp_val<<8;
		reg_val |= 0x8<<24;
		mctl_write_w(SDR_DRR, reg_val);
	}
//	else
//   {
//		mctl_write_w(SDR_DRR, 0x0);
//   }
}


/*
**********************************************************************************************************************
*                                               GET DRAM SIZE
*
* Description: Get DRAM Size in MB unit;
*
* Arguments  : None
*
* Returns    : 32/64/128
*
* Notes      :
*
**********************************************************************************************************************
*/
__u32 DRAMC_get_dram_size(void)
{
    __u32 reg_val;
    __u32 dram_size;
    __u32 chip_den;

    reg_val = mctl_read_w(SDR_DCR);
    chip_den = (reg_val>>3)&0x7;
    if(chip_den == 0)
    	dram_size = 32;
    else if(chip_den == 1)
    	dram_size = 64;
    else if(chip_den == 2)
    	dram_size = 128;
    else if(chip_den == 3)
    	dram_size = 256;
    else if(chip_den == 4)
    	dram_size = 512;
    else
    	dram_size = 1024;

    if( ((reg_val>>1)&0x3) == 0x1)
    	dram_size<<=1;
    if( ((reg_val>>6)&0x7) == 0x3)
    	dram_size<<=1;
    if( ((reg_val>>10)&0x3) == 0x1)
    	dram_size<<=1;

    return dram_size;
}
__s32 DRAMC_to_card_init(__dram_para_t *para)
{
	__u32 i,j,cnt;
	//__u32 dram_den;
	//__u32 err_flag = 0;
	//__u32 start_adr, end_adr;
	__u32 bonding_id;
	__u32 reg_val;
	__u32 ret;

	//check whether scan dram size
	//below scan dram parameter

	//get bonding ID
	reg_val = mctl_read_w(DRAM_CCM_AHB_GATE_REG);
	reg_val |= 0x1<<5;
	mctl_write_w(DRAM_CCM_AHB_GATE_REG, reg_val);
	mctl_write_w(DRAM_CCM_SS_CLK_REG, 0x80000000);

	reg_val = mctl_read_w(DRAM_SS_CTRL_REG);
	bonding_id = (reg_val >>16)&0x7;
	if (!(para->dram_tpr4 & (1u<<31)))	//DRAM space auto detect
	{
		dram_dbg("DRAM space auto detect\n");
		para->dram_rank_num = 2;
		para->dram_chip_density = 4096;
		para->dram_io_width	= 16;
		para->dram_bus_width = 32;
		if(!DRAMC_init_proc_ex(para))
		{
			para->dram_bus_width = 16;
			if(!DRAMC_init_proc_ex(para))
			{
				para->dram_rank_num = 1;
				if(para->dram_type == 2)
					para->dram_chip_density = 4096;
				else
					para->dram_chip_density = 8192;
				para->dram_bus_width = 32;
				if(!DRAMC_init_proc_ex(para))
				{
					para->dram_bus_width = 16;
					if(!DRAMC_init_proc_ex(para))
						{
							dram_dbg("DRAM space auto scan failed!\n");
							return 0;
						}

				}
			}
		}
		dram_dbg("DRAM space auto scan succceed!\n");
		dram_dbg("para->dram_rank_num = %d\n",para->dram_rank_num);
		dram_dbg("para->dram_io_width = %d\n",para->dram_io_width);
		dram_dbg("para->dram_bus_width = %d\n",para->dram_bus_width);
		dram_dbg("para->dram_chip_density = %d Mb\n",para->dram_chip_density);

		for(i=0;i<64;i++)
		mctl_write_w(para->dram_baseaddr + 4*i,(i%2)?(para->dram_baseaddr + 4*i):(~(para->dram_baseaddr + 4*i)));

		for(i=11;i<=16;i++)	/* row detect */
		{
			ret = para->dram_baseaddr + (1<<(i + 3 + 10 + (para->dram_bus_width>>4)));	/* row-bank-column */
			cnt = 0;
			for(j=0;j<64;j++)
				if(mctl_read_w(para->dram_baseaddr + j*4) == mctl_read_w(ret + j*4))
					cnt++;
			if(cnt == 64)
				break;
		}
		if(i >= 16)
			i = 16;
		dram_dbg("row number is %d\n",i);
		para->dram_size = (1<<(i + 3 + 10 + (para->dram_bus_width>>4)))>>20;	/*Unit MB */
		para->dram_chip_density = (para->dram_size<<3)/(para->dram_bus_width/para->dram_io_width);	/*Unit Mb */
		para->dram_size <<= (para->dram_rank_num - 1);	/* for two rank */

		if (para->dram_size >= 2048)
		{
			para->dram_size = 2048;
			para->dram_chip_density = para->dram_size >> (para->dram_rank_num - 1);
			para->dram_chip_density = (para->dram_size<<3)/(para->dram_bus_width/para->dram_io_width);
		}

	}	/* for known dram size or manul config */
	else
	{
		dram_dbg("DRAM parameter config information is:\n");
		dram_dbg("dram_rank_num = %d\n",para->dram_rank_num);
		dram_dbg("dram_io_width = %d\n",para->dram_io_width);
		dram_dbg("dram_bus_width = %d\n",para->dram_bus_width);
		dram_dbg("dram_chip_density = %d Mb\n",para->dram_chip_density);
		if(!DRAMC_init_proc_ex(para))
		{
			dram_dbg("Init dram failed\n");
			return 0;
		}
		dram_dbg("Init dram success\n");
		para->dram_size = DRAMC_get_dram_size();
	}
	para->dram_tpr4 |= 1u<<31;	/* set space flag */
	//dram_dbg("para->dram_size = %d MB\n",para->dram_size);
	return para->dram_size;
}

__s32 init_DRAM(int type, __dram_para_t *boot0_para)
{
	__u32 i;
	__s32 ret_val;
//	boot_dram_para_t  boot0_para;
//    msg("dram driver version: %s\n",dram_driver_version);
//	get_boot0_dram_para( &boot0_para );
 //   print_boot0_dram_para(&boot0_para);
#ifdef SUN7I_HOMLET
//probe pmu exist
	pmu_init();
//check the pmu id num
	if(!check_pmu_id(PMU_ID_NUM))
	{
		pmu_set_ddr_vol_onoff(1);//set ddr_vol enable when check id_num success
		__msdelay(100);
	}
        else
        {
            UART_printf2("jump to fel\n");
            jump_to_standby(FEL_BASE); //if check pum_id_num error,jump to fel
        }
#endif
	if(boot0_para->dram_clk > 2000)
	{
		boot0_para->dram_clk /= 1000000;
	 }

	ret_val = 0;
	i = 0;
	UART_printf2("DRAM boot version v1.17\n");
	while( (ret_val == 0) && (i<4) )
	{

//#if SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_NAND_FLASH
//		ret_val = DRAMC_init( boot0_para );
//#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SD_CARD
//		if(!type)
//		{
			ret_val = DRAMC_to_card_init( boot0_para );
//		}
//		else
//		{	//liuke edit remove DRAMC_init branch,ues DRAMC_to_card_init only
//			ret_val = DRAMC_init( boot0_para );
//		}
//		set_boot0_dram_para( boot0_para );
//#else
//#error  The storage media of Boot1 has not been defined.
//#endif
		i++;
	}
//    print_boot0_dram_para(&boot0_para);
	return ret_val;
}
#pragma arm section
