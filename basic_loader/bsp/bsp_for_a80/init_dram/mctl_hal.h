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
#ifndef   _MCTL_HAL_H
#define   _MCTL_HAL_H

#include  "types.h"
#include  "bsp.h"
#include  "dram_for_debug.h"

#define  DRAM_PRINK_ENABLE

#ifdef DRAM_PRINK_ENABLE
#  define dram_dbg(fmt,args...)	UART_printf2(fmt ,##args)
#else
#  define dram_dbg(fmt,args...)
#endif

typedef struct __DRAM_PARA
{
	//normal configuration
	unsigned int        dram_clk;
	unsigned int        dram_type;		//dram_type			DDR2: 2				DDR3: 3		LPDDR2: 6	LPDDR3: 7	DDR3L: 31
	//unsigned int        lpddr2_type;	//LPDDR2 type		S4:0	S2:1 	NVM:2
    unsigned int        dram_zq;		//[7:0] for CK/CA  [15:8] for DX0/1  [23:16] for DX2/3
    unsigned int		dram_odt_en;

	//control configuration
	unsigned int		dram_para1;	//[31:30]-channel_enable [29:28]-channel_number [27:24]-bank_size [23:16]-row_width [15:8]-bus_width [7:4]-rank [3:0]-page_size
    unsigned int		dram_para2;	//[2]-single_chip_DQ_width [1:0]-single_chip_density

    //ZQ
//    unsigned int		dram_zq0pr;	//for CK/CA
//    unsigned int		dram_zq1pr;	//for DX0/1
//    unsigned int		dram_zq2pr;	//for DX2/3

	//timing configuration
	unsigned int		dram_mr0;
    unsigned int		dram_mr1;
    unsigned int		dram_mr2;
    unsigned int		dram_mr3;
    unsigned int		dram_tpr0;
    unsigned int		dram_tpr1;
    unsigned int		dram_tpr2;
    unsigned int		dram_tpr3;
    unsigned int		dram_tpr4;	//[0]-1T/2T [4]-half_dq
    unsigned int		dram_tpr5;
   	unsigned int		dram_tpr6;	//[0]-wake_branch
   	//unsigned int		dram_user_lock;

    //reserved for future use
    unsigned int		dram_tpr7;
    unsigned int		dram_tpr8;
    unsigned int		dram_tpr9;
    unsigned int		dram_tpr10;
    unsigned int		dram_tpr11;
    unsigned int		dram_tpr12;
    unsigned int		dram_tpr13;

}__dram_para_t;

#define DRAM_MAX_SIZE_MB		(MCTL_PAGE_SIZE*MCTL_BANK_SIZE*(0x1<<(MCTL_ROW_WIDTH-10))*MCTL_CHANNEL_NUM)  //in MB

extern unsigned int mctl_init(void);
extern unsigned int mctl_set_emrs(unsigned int emrs_id, unsigned int emrs_val);
extern unsigned int mctl_scan_readpipe(unsigned int clk);

//extern void mctl_self_refresh_entry(unsigned int channel_num);
extern void mctl_self_refresh_entry(unsigned int ch_index);
//extern void mctl_self_refresh_exit(unsigned int channel_num);
extern void mctl_self_refresh_exit(unsigned int ch_index);
extern void mctl_pad_hold(void);
extern void mctl_pad_release(void);
extern unsigned int mctl_sys_init_setfreq(unsigned int dram_freq);
extern unsigned int mctl_init_setreq(unsigned int dram_freq);
extern void mctl_power_down_entry(void);
extern void mctl_power_down_exit(void);
extern void mctl_precharge_all(void);
extern void mctl_deep_sleep_entry(void);
extern void mctl_deep_sleep_exit(void);

extern void mctl_setup_ar_interval(unsigned int clk);
extern void mctl_DLL_reset(void);
extern void mctl_DLL_enable(void);
extern void mctl_DLL_disable(void);
extern void mctl_hostport_control(unsigned int enable);
extern unsigned int mctl_init_dram(void);


extern void mctl_host_port_cfg(unsigned int port_no, unsigned int cfg);
extern void mctl_power_save_process(void);
extern unsigned int mctl_power_up_process(void);
extern unsigned int mctl_ahb_reset(void);

extern unsigned int mctl_sys_init(__dram_para_t *para);
extern void auto_set_timing_para(__dram_para_t *para);
extern signed int init_DRAM(int type, __dram_para_t *para);

extern unsigned int mctl_reset_release(void);

extern int dram_test_simple(void);

#endif  //_MCTL_HAL_H
