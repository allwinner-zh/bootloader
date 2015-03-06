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

//#define FPGA_PLATFORM
//#define LPDDR2_FPGA_S2C_2CS_2CH
#define DDR3_32B
#define TEST_MEM 0x40000000

#ifdef DRAM_PRINK_ENABLE
#  define dram_dbg(fmt,args...)	UART_printf2(fmt ,##args)
#else
#  define dram_dbg(fmt,args...)
#endif

typedef struct __DRAM_PARA
{
	//normal configuration
	unsigned int        dram_clk;
	unsigned int        dram_type;		//dram_type			DDR2: 2				DDR3: 3				LPDDR2: 6	DDR3L: 31
    unsigned int        dram_zq;
    unsigned int		dram_odt_en;

	//control configuration
	unsigned int		dram_para1;
    unsigned int		dram_para2;

	//timing configuration
	unsigned int		dram_mr0;
    unsigned int		dram_mr1;
    unsigned int		dram_mr2;
    unsigned int		dram_mr3;
    unsigned int		dram_tpr0;
    unsigned int		dram_tpr1;
    unsigned int		dram_tpr2;
    unsigned int		dram_tpr3;
    unsigned int		dram_tpr4;
    unsigned int		dram_tpr5;
   	unsigned int		dram_tpr6;

    //reserved for future use
    unsigned int		dram_tpr7;
    unsigned int		dram_tpr8;
    unsigned int		dram_tpr9;
    unsigned int		dram_tpr10;
    unsigned int		dram_tpr11;
    unsigned int		dram_tpr12;
    unsigned int		dram_tpr13;

}__dram_para_t;

extern __dram_para_t *dram_para;

extern void __msdelay(unsigned int delay);

extern unsigned int DRAMC_init(__dram_para_t *para);
extern unsigned int DRAMC_init_auto(__dram_para_t *para);
extern signed int init_DRAM(int type, void *para);
//extern uint32 mctl_basic_test(void);
//extern uint32 mctl_stable_test(void);
extern int p2wi_read(unsigned int addr, unsigned int *val);
extern int p2wi_write(unsigned int addr, unsigned int val);

extern int mdfs_save_value(__dram_para_t *dram_para);

#endif  //_MCTL_HAL_H










