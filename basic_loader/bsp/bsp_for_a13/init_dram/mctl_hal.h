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

#ifndef __MCTL_HAL_H__
#define __MCTL_HAL_H__

typedef struct _boot_dram_para_t
{
    unsigned int           dram_baseaddr;
    unsigned int           dram_clk;
    unsigned int           dram_type;
    unsigned int           dram_rank_num;
    unsigned int           dram_chip_density;
    unsigned int           dram_io_width;
    unsigned int		    dram_bus_width;
    unsigned int           dram_cas;
    unsigned int           dram_zq;
    unsigned int           dram_odt_en;
    unsigned int 			dram_size;
    unsigned int           dram_tpr0;
    unsigned int           dram_tpr1;
    unsigned int           dram_tpr2;
    unsigned int           dram_tpr3;
    unsigned int           dram_tpr4;
    unsigned int           dram_tpr5;
    unsigned int 			dram_emr1;
    unsigned int           dram_emr2;
    unsigned int           dram_emr3;
}__dram_para_t;

#endif  //__MCTL_HAL_H__

