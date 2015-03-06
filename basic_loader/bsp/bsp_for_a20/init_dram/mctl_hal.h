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

//#include  "types.h"
#include  "types.h"
#include  "bsp.h"
#include  "dram_for_debug.h"
#include  "interinc/egon_def.h"

#ifdef DRAM_PRINK_ENABLE
#  define dram_dbg(fmt,args...)	UART_printf2(fmt ,##args)
#else
#  define dram_dbg(fmt,args...)
#endif

typedef struct __DRAM_PARA
{
    __u32           dram_baseaddr;
    __u32           dram_clk;
    __u32           dram_type;
    __u32           dram_rank_num;
    __u32           dram_chip_density;
    __u32           dram_io_width;
    __u32		    dram_bus_width;
    __u32           dram_cas;
    __u32           dram_zq;
    __u32           dram_odt_en;
    __u32 			dram_size;
    __u32           dram_tpr0;
    __u32           dram_tpr1;
    __u32           dram_tpr2;
    __u32           dram_tpr3;
    __u32           dram_tpr4;
    __u32           dram_tpr5;
    __u32 			dram_emr1;
    __u32           dram_emr2;
    __u32           dram_emr3;
	__u32           dram_tpr6;
	__u32           dram_tpr7;
	__u32           dram_tpr8;
	__u32           dram_tpr9;
}__dram_para_t;
extern __dram_para_t *dram_para;


//extern boot_dram_para_t *dram_para;

#endif  //_MCTL_HAL_H
