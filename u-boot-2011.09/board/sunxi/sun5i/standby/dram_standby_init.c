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
#include "dram_i.h"
#include <asm/arch/dram.h>

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
	standby_delay(0x100);
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
    //  reg_val |= 0x7<<20;
    mctl_write_w(SDR_CR, reg_val);
}


void mctl_itm_disable(void)
{
    __u32 reg_val = 0x0;

	reg_val = mctl_read_w(SDR_CCR);
	reg_val |= 0x1<<28;
	reg_val &= ~(0x1U<<31);          //danielwang, 2012-06-12
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
    mctl_write_w(SDR_DLLCR0, (mctl_read_w(SDR_DLLCR0) & (~0x40000000)) | 0x80000000);

	standby_delay(0x100);

    mctl_write_w(SDR_DLLCR0, mctl_read_w(SDR_DLLCR0) & ~0xC0000000);
	standby_delay(0x1000);

    mctl_write_w(SDR_DLLCR0, (mctl_read_w(SDR_DLLCR0) & (~0x80000000)) | 0x40000000);
    standby_delay(0x1000);
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
		mctl_write_w(SDR_DLLCR0+(i<<2), (mctl_read_w(SDR_DLLCR0+(i<<2)) & (~(0xf<<14))) | ((dqs_phase&0xf)<<14));
		mctl_write_w(SDR_DLLCR0+(i<<2), (mctl_read_w(SDR_DLLCR0+(i<<2)) & (~0x40000000)) | 0x80000000);
		dqs_phase = dqs_phase>>4;
	}

	standby_delay(0x100);

    for(i=1; i<dll_num; i++)
    {
        mctl_write_w(SDR_DLLCR0+(i<<2), mctl_read_w(SDR_DLLCR0+(i<<2)) & ~0xC0000000);
    }

	standby_delay(0x1000);

    for(i=1; i<dll_num; i++)
    {
        mctl_write_w(SDR_DLLCR0+(i<<2), (mctl_read_w(SDR_DLLCR0+(i<<2)) & (~0x80000000)) | 0x40000000);
    }
    standby_delay(0x1000);
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
    
    //DISABLE PLL before configuration by yeshaozhen at 20130717
    reg_val &= ~(0x1<<31);  	//PLL disable before configure
    mctl_write_w(DRAM_CCM_SDRAM_PLL_REG, reg_val);
    standby_delay(0x1000);
    //20130717 end
    
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
    standby_delay(0x100000);
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
    standby_delay(0x100);
    //then open it
    reg_val |= 0x3<<14;
    mctl_write_w(DRAM_CCM_AHB_GATE_REG, reg_val);
    standby_delay(0x100);
    
}

__s32 DRAMC_init(boot_dram_para_t *para)
{
    __u32 reg_val;
	__u32 hold_flag = 0;
   // __u8  reg_value;
    __s32 ret_val;  

    //check input dram parameter structure
    if(!para)
    {
        //dram parameter is invalid
        return 0;
    }
    
    //close AHB clock for DRAMC,--added by yeshaozhen at 20130715
    reg_val = mctl_read_w(DRAM_CCM_AHB_GATE_REG);
    reg_val &= ~(0x3<<14);
    mctl_write_w(DRAM_CCM_AHB_GATE_REG, reg_val);
		standby_delay(0x200);
		//20130715 AHB close end

    //setup DRAM relative clock
    mctl_setup_dram_clock(para->dram_clk);


    mctl_set_drive();

    //dram clock off
    DRAMC_clock_output_en(0);


    mctl_itm_disable();
    mctl_enable_dll0(para->dram_tpr3);

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

    //dram clock on
    DRAMC_clock_output_en(1);
    
    hold_flag = mctl_read_w(SDR_DPCR);
    if(hold_flag == 0) //normal branch
    {
    //set odt impendance divide ratio
    reg_val=((para->dram_zq)>>8)&0xfffff;
    reg_val |= ((para->dram_zq)&0xff)<<20;
    reg_val |= (para->dram_zq)&0xf0000000;
    reg_val |= (0x1u<<31);
    mctl_write_w(SDR_ZQCR0, reg_val);
    
    while( !((mctl_read_w(SDR_ZQSR)&(0x1<<31))) );

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
    
    standby_delay(0x10);
    while(mctl_read_w(SDR_CCR) & (0x1U<<31)) {};

    mctl_enable_dllx(para->dram_tpr3);
    //set I/O configure register
//    reg_val = 0x00cc0000;
//   reg_val |= (para->dram_odt_en)&0x3;
//  reg_val |= ((para->dram_odt_en)&0x3)<<30;
//    mctl_write_w(SDR_IOCR, reg_val);

    //set refresh period
    DRAMC_set_autorefresh_cycle(para->dram_clk);

    //set timing parameters
    mctl_write_w(SDR_TPR0, para->dram_tpr0);
    mctl_write_w(SDR_TPR1, para->dram_tpr1);
    mctl_write_w(SDR_TPR2, para->dram_tpr2);

    //set mode register
    if(para->dram_type==3)                          //ddr3
    {
        reg_val = 0x1<<12;
        reg_val |= (para->dram_cas - 4)<<4;
        reg_val |= 0x5<<9;
    }
    else if(para->dram_type==2)                 //ddr2
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
    
//  while(1);

    //setup zq calibration manual
    //reg_val = mctl_read_w(SDR_DPCR);
    if(hold_flag == 1)
    {

//      super_standby_flag = 1;

    	//disable auto-fresh			//by cpl 2013-5-6
    	reg_val = mctl_read_w(SDR_DRR);
    	reg_val |= 0x1U<<31;
    	mctl_write_w(SDR_DRR, reg_val);


        reg_val = mctl_read_w(SDR_GP_REG0);
        reg_val &= 0x000fffff;
        mctl_write_w(SDR_GP_REG0, reg_val);
        //reg_val |= 0x17b00000;
        reg_val |= (0x1<<28) | (para->dram_zq<<20);
        mctl_write_w(SDR_ZQCR0, reg_val);
        
        //03-08
        reg_val = mctl_read_w(SDR_DCR);
        reg_val &= ~(0x1fU<<27);
        reg_val |= 0x12U<<27;
        mctl_write_w(SDR_DCR, reg_val);
        while( mctl_read_w(SDR_DCR)& (0x1U<<31) );
        
        standby_delay(0x100);

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
        standby_delay(0x100);;
    
//        //issue a refresh command
//        reg_val = mctl_read_w(SDR_DCR);
//        reg_val &= ~(0x1fU<<27);
//        reg_val |= 0x13U<<27;
//        mctl_write_w(SDR_DCR, reg_val);
//        
//        while( mctl_read_w(SDR_DCR)& (0x1U<<31) );
//
//        mem_delay(0x100);
    }

    //scan read pipe value
    mctl_itm_enable();
    
    if(hold_flag == 0)//normal branch
    {
    ret_val = DRAMC_scan_readpipe();
    
    if(ret_val < 0)
    {
        return 0;
	    }
    
    }else	//super standby branch
    {
    	//write back dqs gating value
        reg_val = mctl_read_w(SDR_GP_REG1);
        mctl_write_w(SDR_RSLR0, reg_val);

        reg_val = mctl_read_w(SDR_GP_REG2);
        mctl_write_w(SDR_RDQSGR, reg_val);

      //mctl_write_w(SDR_RSLR0, dqs_value_save[0]);
	  //mctl_write_w(SDR_RDQSGR, dqs_value_save[1]);
    }
    //configure all host port
    mctl_configure_hostport();

    return DRAMC_get_dram_size();
}



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
void DRAMC_set_autorefresh_cycle(__u32 clk)
{
    __u32 reg_val;
//  __u32 dram_size;
    __u32 tmp_val;

//  dram_size = mctl_read_w(SDR_DCR);
//  dram_size >>=3;
//  dram_size &= 0x7;

//  if(clk < 600)
    {
//      if(dram_size<=0x2)
//          tmp_val = (131*clk)>>10;
//      else
//          tmp_val = (336*clk)>>10;
        reg_val = 0x83;
        tmp_val = (7987*clk)>>10;
        tmp_val = tmp_val*9 - 200;
        reg_val |= tmp_val<<8;
        reg_val |= 0x8<<24;
        mctl_write_w(SDR_DRR, reg_val);
    }
//  else
//   {
//      mctl_write_w(SDR_DRR, 0x0);
//   }
}

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

__s32 init_DRAM(boot_dram_para_t *boot0_para)
{
    __u32 i;
    __s32 ret_val;

    if(boot0_para->dram_clk > 2000)
    {
//          boot0_para->dram_clk = mem_uldiv(boot0_para->dram_clk, 1000000);
            //boot0_para.dram_clk /= 1000000;
    }

    ret_val = 0;
    i = 0;
    while( (ret_val == 0) && (i<4) )
    {
            ret_val = DRAMC_init(boot0_para);
            i++;
    }
    return ret_val;
}
__s32 dram_exit(void)
{
    return 0;
}

__s32 dram_get_size(void)
{
    return DRAMC_get_dram_size();
}

void dram_set_clock(int clk)
{
    return mctl_setup_dram_clock(clk);
}

void dram_set_drive(void)
{
    mctl_set_drive();
}

void dram_set_autorefresh_cycle(__u32 clk)
{
    DRAMC_set_autorefresh_cycle(clk);
}

__s32 dram_scan_readpipe(void)
{
    return DRAMC_scan_readpipe();
}

