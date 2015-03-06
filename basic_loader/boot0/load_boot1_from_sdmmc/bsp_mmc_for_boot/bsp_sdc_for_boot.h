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

#ifndef _BSP_SDC_FOR_BOOT_H_
#define _BSP_SDC_FOR_BOOT_H_

/*
************************************************************************************************************************
*                       SPMMC_PhyInit
*
*Description: SDMMC的物理操作初始化
*
*Arguments  :   none
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_PhyInit(__u32 card_no, __u32 bus_width);
/*
************************************************************************************************************************
*                       SDMMC_PhyExit
*
*Description: SDMMC的物理操作退出
*
*Arguments  :   none
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_PhyExit(__u32 card_no);
/*
************************************************************************************************************************
*                       SDMMC_PhyRead
*
*Description: SDMMC的物理读数据
*
*Arguments  :   标准块设备操作
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_PhyRead(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no);
/*
************************************************************************************************************************
*                       SDMMC_PhyWrite
*
*Description: SDMMC的物理写数据
*
*Arguments  :   标准块设备操作
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_PhyWrite(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no);

__s32 SDMMC_PhyDiskSize(__u32 card_no);

__s32 SDMMC_PhyErase(__u32 block, __u32 nblock, __u32 card_no);



/*
************************************************************************************************************************
*                       SDMMC_PhyInit
*
*Description: SDMMC的逻辑操作初始化
*
*Arguments  :   none
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_LogicalInit(__u32 card_no, __u32 card_offset, __u32 bus_width);
/*
************************************************************************************************************************
*                       SDMMC_LogicalExit
*
*Description: SDMMC的逻辑操作初始化
*
*Arguments  :   none
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_LogicalExit(__u32 card_no);
/*
************************************************************************************************************************
*                       SDMMC_LogicalRead
*
*Description: SDMMC的逻辑读数据
*
*Arguments  :   标准块设备操作
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/__s32 SDMMC_LogicalRead(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no);

/*
************************************************************************************************************************
*                       SDMMC_LogicalWrite
*
*Description: SDMMC的逻辑写数据
*
*Arguments  :   标准块设备操作
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_LogicalWrite(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no);


__s32 SDMMC_LogicalDiskSize(__u32 card_no);

__s32 SDMMC_LogicaErase(__u32 block, __u32 nblock, __u32 card_no);


#endif //#ifndef _BSP_SDC_FOR_BOOT_H_
