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
#include "standby.h"
#include "dram_i.h"
#include <linux/types.h>
#include <asm/arch/dram.h>
/*
 *********************************************************************************************************
 * File    : dram.c
 * By      : Berg.Xing
 * Date    : 2011-06-01
 * Descript: dram  for AW1623 chipset
 * Update  : date          auther      ver     notes
 *     2011-06-01      			Berg        1.0     create file
 *	  2011-07-29		  Berg        1.1	  change dram_power_save_process() and dram_power_up_process()
 *	  2011-08-05		  Berg        1.2     add retraining method in power up process when failed
 *********************************************************************************************************
 */
//static __u32 hpcr_value[32];
//static 	__u32 gating = 0;

void standby_delay(int ms)
{
    volatile int i;

    for(i=ms;i>0;i--);

    return ;
}
/*
 *********************************************************************************************************
 *                 DRAM ENTER SELF REFRESH
 *
 * Description: dram enter/exit self-refresh;
 *
 * Arguments  : none
 *
 * Returns    : none
 *
 * Note       :
 *********************************************************************************************************
 */
/*
void mctl_precharge_all(void)
{
    __u32 reg_val;

    reg_val = mctl_read_w(SDR_DCR);
    reg_val &= ~(0x1fU<<27);
    reg_val |= 0x15U<<27;
    mctl_write_w(SDR_DCR, reg_val);

    //check whether command has been executed
    while( mctl_read_w(SDR_DCR)& (0x1U<<31) );
    standby_delay(0x100);
}
*/
void DRAMC_enter_selfrefresh(void)
{
    __u32 reg_val;

    //	//disable all port
    //	for(i=0; i<31; i++)
    //	{
    //		DRAMC_hostport_on_off(i, 0x0);
    //	}
    //	for(i=0; i<8; i++)
    //	{
    //		mctl_write_w(SDR_HPCR + (i<<2), 0);
    //	}
    //
    //	for(i=16; i<28; i++)
    //	{
    //		mctl_write_w(SDR_HPCR + (i<<2), 0);
    //	}
    //
    //	mctl_write_w(SDR_HPCR + (29<<2), 0);
    //	mctl_write_w(SDR_HPCR + (31<<2), 0);
    /*
    //disable auto-fresh
    reg_val = mctl_read_w(SDR_DRR);
    reg_val |= 0x1U<<31;
    mctl_write_w(SDR_DRR, reg_val);
    */
    //issue prechage all command
    //	mctl_precharge_all();

    //disable auto-fresh			//by cpl 2013-5-6
    reg_val = mctl_read_w(SDR_DRR);
    reg_val |= 0x1U<<31;
    mctl_write_w(SDR_DRR, reg_val);


    //enter into self-refresh
    reg_val = mctl_read_w(SDR_DCR);
    reg_val &= ~(0x1fU<<27);
    reg_val |= 0x12U<<27;
    mctl_write_w(SDR_DCR, reg_val);
    while( mctl_read_w(SDR_DCR)& (0x1U<<31) );
    standby_delay(0x100);

    reg_val = mctl_read_w(SDR_CR);
    reg_val &= ~(0x3<<28);
    reg_val |= 0x2<<28;
    mctl_write_w(SDR_CR, reg_val);

    //dram pad odt hold
    mctl_write_w(SDR_DPCR, 0x16510001);

    while(!(mctl_read_w(SDR_DPCR) & 0x1));
    standby_delay(0x100);

}
/*
void mctl_mode_exit(void)
{
    __u32 i;
    __u32 reg_val;

    reg_val = mctl_read_w(SDR_DCR);
    reg_val &= ~(0x1fU<<27);
    reg_val |= 0x17U<<27;
    mctl_write_w(SDR_DCR, reg_val);

    //check whether command has been executed
    while( mctl_read_w(SDR_DCR)& (0x1U<<31) );
    standby_delay(0x100);
}
*/
/*
void DRAMC_exit_selfrefresh(void)
{
    __u32 i;
    __u32 reg_val;

    //exit self-refresh state
    mctl_mode_exit();

    //issue a refresh command
    reg_val = mctl_read_w(SDR_DCR);
    reg_val &= ~(0x1fU<<27);
    reg_val |= 0x13U<<27;
    mctl_write_w(SDR_DCR, reg_val);
    while( mctl_read_w(SDR_DCR)& (0x1U<<31) );
    standby_delay(0x100);

    //enable auto-fresh
    reg_val = mctl_read_w(SDR_DRR);
    reg_val &= ~(0x1U<<31);
    mctl_write_w(SDR_DRR, reg_val);

    //enable all port
    for(i=0; i<31; i++)
    {
        DRAMC_hostport_on_off(i, 0x1);
    }
}
*/
/*
 *********************************************************************************************************
 *                 DRAM POWER DOWN
 *
 * Description: enter/exit dram power down state
 *
 * Arguments  :
 *
 * Returns    : none;
 *
 * Note       :
 *********************************************************************************************************
 */
/*
void DRAMC_enter_power_down(void)
{

    __u32 reg_val;

    reg_val = mctl_read_w(SDR_DCR);
    reg_val &= ~(0x1fU<<27);
    reg_val |= 0x1eU<<27;
    mctl_write_w(SDR_DCR, reg_val);

    //check whether command has been executed
    while( mctl_read_w(SDR_DCR)& (0x1U<<31) );
    standby_delay(0x100);
}
*/
/*
void DRAMC_exit_power_down(void)
{
    mctl_mode_exit();
}
*/
/*
 **********************************************************************************************************************
 *                 DRAM HOSTPORT CONTROL
 *
 * Description: dram host port enable/ disable
 *
 * Arguments  : __u32 port_idx		host port index   (0,1,...31)
 *				__u32 on		enable or disable (0: diable, 1: enable)
 *
 * Returns    :
 *
 * Notes      :
 *
 **********************************************************************************************************************
 */

void DRAMC_hostport_on_off(__u32 port_idx, __u32 on)
{
    __u32   reg_val;

    if(port_idx<=31)
    {
        reg_val = mctl_read_w(SDR_HPCR + (port_idx<<2));
        if(on)
            reg_val |= 0x1;
        else
            reg_val &= ~(0x1);
        mctl_write_w(SDR_HPCR + (port_idx<<2), reg_val);
    }
}
/*
 **********************************************************************************************************************
 *                 DRAM GET HOSTPORT STATUS
 *
 * Description: dram get AHB FIFO status
 *
 * Arguments  : __u32 port_idx		host port index   	(0,1,...31)
 *
 * Returns    : __u32 ret_val		AHB FIFO status 	(0: FIFO not empty ,1: FIFO empty)
 *
 * Notes      :
 *
 **********************************************************************************************************************
 */
/*
__u32 DRAMC_hostport_check_ahb_fifo_status(__u32 port_idx)
{
    __u32   reg_val;

    if(port_idx<=31)
    {
        reg_val = mctl_read_w(SDR_CFSR);
        return ( (reg_val>>port_idx)&0x1 );
    }
    else
    {
        return 0;
    }
}
*/
/*
 **********************************************************************************************************************
 *                 DRAM GET HOSTPORT STATUS
 *
 * Description: dram get AHB FIFO status
 *
 * Arguments  : 	__u32 port_idx				host port index   	(0,1,...31)
 *				__u32 port_pri_level		priority level		(0,1,2,3)
 *
 * Returns    :
 *
 * Notes      :
 *
 **********************************************************************************************************************
 */
/*
void DRAMC_hostport_setup(__u32 port_idx, __u32 port_pri_level, __u32 port_wait_cycle, __u32 cmd_num)
{
    __u32   reg_val;

    if(port_idx<=31)
    {
        reg_val = mctl_read_w(SDR_HPCR + (port_idx<<2));
        reg_val &= ~(0x3<<2);
        reg_val |= (port_pri_level&0x3)<<2;
        reg_val &= ~(0xf<<4);
        reg_val |= (port_wait_cycle&0xf)<<4;
        reg_val &= ~(0xff<<8);
        reg_val |= (cmd_num&0x3)<<8;
        mctl_write_w(SDR_HPCR + (port_idx<<2), reg_val);
    }
}
*/
/*
 *********************************************************************************************************
 *                 DRAM power save process
 *
 * Description: We can save power by disable DRAM PLL.
 *
 * Arguments  : none
 *
 * Returns    : none
 *
 * Note       :
 *********************************************************************************************************
 */
/*
__u32 mctl_ahb_reset(void)
{

    __u32 reg_val;

    reg_val = mctl_read_w(DRAM_CCM_AHB_GATE_REG);
    reg_val &=~(0x3<<14);
    mctl_write_w(DRAM_CCM_AHB_GATE_REG,reg_val);
    standby_delay(0x10);

    reg_val = mctl_read_w(DRAM_CCM_AHB_GATE_REG);
    reg_val |=(0x3<<14);
    mctl_write_w(DRAM_CCM_AHB_GATE_REG,reg_val);

    return 0;
}
*/
/*
   __s32 DRAMC_retraining(void)
   {
   __u32 i;
   __u32 reg_val;
   __u32 ret_val;
   __u32 reg_dcr, reg_drr, reg_tpr0, reg_tpr1, reg_tpr2, reg_mr, reg_emr, reg_emr2, reg_emr3;
   __u32 reg_zqcr0, reg_iocr, reg_ccr, reg_zqsr;

//remember register value
reg_dcr = mctl_read_w(SDR_DCR);
reg_drr = mctl_read_w(SDR_DRR);
reg_tpr0 = mctl_read_w(SDR_TPR0);
reg_tpr1 = mctl_read_w(SDR_TPR1);
reg_tpr2 = mctl_read_w(SDR_TPR2);
reg_mr = mctl_read_w(SDR_MR);
reg_emr = mctl_read_w(SDR_EMR);
reg_emr2 = mctl_read_w(SDR_EMR2);
reg_emr3 = mctl_read_w(SDR_EMR3);
reg_zqcr0 = mctl_read_w(SDR_ZQCR0);
reg_iocr = mctl_read_w(SDR_IOCR);
reg_tpr3 = (mctl_read_w(SDR_DLLCR1)>>14) & 0xf;
reg_tpr3 |= ((mctl_read_w(SDR_DLLCR2)>>14) & 0xf)<<4;
reg_tpr3 |= ((mctl_read_w(SDR_DLLCR3)>>14) & 0xf)<<8;
reg_tpr3 |= ((mctl_read_w(SDR_DLLCR4)>>14) & 0xf)<<12;
reg_ccr = mctl_read_w(SDR_CCR);
reg_zqsr = mctl_read_w(SDR_ZQSR);
while(1){
mctl_ahb_reset();
mctl_ddr3_reset();
mctl_set_drive();

mctl_itm_disable();

mctl_enable_dll0();

//set CCR value
mctl_write_w(SDR_CCR, reg_ccr);

//configure external DRAM
mctl_write_w(SDR_DCR, reg_dcr);

//set ZQ value
reg_val = reg_zqsr&0xfffff;
reg_val |= 0x1<<30;
reg_val |= 0x1<<28;
reg_val |= reg_zqcr0&(0xff<<20);
reg_val |= reg_zqcr0&(0x1<<29);
mctl_write_w(SDR_ZQCR0, reg_val);

//dram clock on
DRAMC_clock_output_en(1);


standby_delay(0x10);
while(mctl_read_w(SDR_CCR) & (0x1U<<31)) {};

mctl_enable_dllx(reg_tpr3);

//set I/O configure register
mctl_write_w(SDR_IOCR, reg_iocr);

//set refresh period
mctl_write_w(SDR_DRR, reg_drr);

//set timing parameters
mctl_write_w(SDR_TPR0, reg_tpr0);
mctl_write_w(SDR_TPR1, reg_tpr1);
mctl_write_w(SDR_TPR2, reg_tpr2);

//set mode register
mctl_write_w(SDR_MR, reg_mr);
mctl_write_w(SDR_EMR, reg_emr);
mctl_write_w(SDR_EMR2, reg_emr2);
mctl_write_w(SDR_EMR3, reg_emr3);

//initial external DRAM
reg_val = mctl_read_w(SDR_CCR);
reg_val |= 0x1U<<31;
mctl_write_w(SDR_CCR, reg_val);
while(mctl_read_w(SDR_CCR) & (0x1U<<31)) {};

//dram pad hold release
mctl_write_w(SDR_DPCR, 0x16510000);
standby_delay(0x10000);

//scan read pipe value
mctl_itm_enable();
ret_val = DRAMC_scan_readpipe();

//configure all host port
mctl_configure_hostport();

if(ret_val == 0)
    return 0;
    }
}
*/
#define SAVE_SDR_ZQSR_TO_RTC


static __s32 backup_dram_cal_val(int standby_mode)
{
#ifdef SAVE_SDR_ZQSR_TO_RTC
    __u32 reg_val;
    //save memc ZQ value into RTC GP register
    reg_val = mctl_read_w(SDR_ZQSR)&0xfffff;
    if (standby_mode != 0)
    {
        reg_val |= 0x1<<20;             //super standby flag
    }
    mctl_write_w(SDR_GP_REG0, reg_val);
    reg_val = mctl_read_w(SDR_RSLR0);
    mctl_write_w(SDR_GP_REG1, reg_val);

    reg_val = mctl_read_w(SDR_RDQSGR);
    mctl_write_w(SDR_GP_REG2, reg_val);
    return 0;

#elif defined(SAVE_SDR_ZQSR_TO_AXP)

    __u32 value;
    __u8 reg_addr_1st = 0x0a;
    __u8 reg_addr_2nd = 0x0b;
    __u8 reg_addr_3rd = 0x0c;
    __u8 reg_val;


    value = mctl_read_w(SDR_ZQSR) & 0xfffff;
    //busy_waiting();

    reg_val = value&0xff;
    if(twi_byte_rw(TWI_OP_WR, AXP_ADDR,reg_addr_1st, &reg_val)){
        return -1;
    }

    reg_val = (value>>8)&0xff;
    if(twi_byte_rw(TWI_OP_WR, AXP_ADDR,reg_addr_2nd, &reg_val)){
        return -1;
    }

    reg_val = (value>>16)&0x0f;
    if(twi_byte_rw(TWI_OP_WR, AXP_ADDR,reg_addr_3rd, &reg_val)){
        return -1;
    }

    return 0;
#else 
#error "super standby has not save SDR_ZQSR"
#endif
}
__s32 dram_power_save_process(boot_dram_para_t* standby_dram_para)
{
    /*__u32 i;	*/
    __u32 reg_val;

#define MAX_RETRY_TIMES (5)

    __s32 retry = MAX_RETRY_TIMES;

    while((-1 == backup_dram_cal_val(0)) && --retry){
        ;
    }
    if(0 == retry){
        return -1;
    }else{
        retry = MAX_RETRY_TIMES;
    }	

    //put external SDRAM into self-fresh state
    DRAMC_enter_selfrefresh();

    //	//disable ITM
    //	mctl_itm_disable();
    //
    //	//disable and reset all DLL
    //	mctl_disable_dll();

    reg_val = mctl_read_w(DRAM_CCM_AHB_GATE_REG);
    reg_val &= ~(0x3<<14);
    mctl_write_w(DRAM_CCM_AHB_GATE_REG, reg_val);

    return 0;

}


extern __s32 init_DRAM(boot_dram_para_t *boot0_para);
__u32 dram_power_up_process(boot_dram_para_t* standby_dram_para)
{
    return init_DRAM(standby_dram_para);
}

/*
void dram_enter_selfrefresh(void)
{
    DRAMC_enter_selfrefresh();
}
*/
/*
void dram_exit_selfrefresh(void)
{
    DRAMC_exit_selfrefresh();
}
*/
/*
void dram_enter_power_down(void)
{
    DRAMC_enter_power_down();
}
*/
/*
void dram_exit_power_down(void)
{
    DRAMC_exit_power_down();
}
*/
/*
void dram_hostport_on_off(__u32 port_idx, __u32 on)
{
    DRAMC_hostport_on_off(port_idx, on);
}
*/
/*
__u32 dram_hostport_check_ahb_fifo_status(__u32 port_idx)
{
    return DRAMC_hostport_check_ahb_fifo_status(port_idx);
}
*/
/*
void dram_hostport_setup(__u32 port, __u32 prio, __u32 wait_cycle, __u32 cmd_num)
{
    DRAMC_hostport_setup(port, prio, wait_cycle, cmd_num);
}
*/

