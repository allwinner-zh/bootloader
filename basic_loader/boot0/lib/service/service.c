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

#include "..\lib_i.h"
#include "string.h"
#include "..\..\load_Boot1_from_nand\nand_for_boot.h"
#include "..\..\Load_boot1_from_sdmmc\Bsp_mmc_for_boot\mmc.h"


extern const boot0_file_head_t  BT0_head;


__bool STATE_SingleChipOp( void )
{
	return 1;
}



__u8  *get_page_buf( void )
{
	return (__u8 *)PAGE_BUF_FOR_BOOT0;
}


void set_dram_para(void *dram_addr , __u32 dram_size)
{
	__dram_para_t    *dram_para = (__dram_para_t *)dram_addr;
	uboot_file_head  *uboot_buf = (uboot_file_head *)UBOOT_BASE;
#if defined (CONFIG_ARCH_SUN7I) || defined(CONFIG_ARCH_SUN5I)
	dram_para->dram_size = dram_size;
#else
	dram_para->dram_para1 &= 0xffff0000;
	dram_para->dram_para1 |= (dram_size & 0xffff);
#endif
	memcpy((void *)uboot_buf->boot_data.dram_para, dram_addr, 32 * sizeof(int));

	return;
}





//void  delay( __u32 ms )       // CPU frequence is 120Mhz
//{
//	__u32 count;
//	__u32 i;
//
//	for( i = 0, count = ms * 2000;  i < count;  i++ )
//		;
//}

__s32 BOOT_NandGetPara(boot_nand_para_t *param)
{
#ifndef A39_FPGA_PLATFORM

#ifdef NAND_DEBUG
        msg("ChannelCnt = %x\n", param->ChannelCnt);
        msg("ChipCnt = %x\n", param->ChipCnt);
        msg("ChipConnectInfo = %x\n", param->ChipConnectInfo);
        msg("RbCnt = %x\n", param->RbCnt);
        msg("RbConnectInfo= %x\n", param->RbConnectInfo);
        msg("RbConnectMode = %x\n", param->RbConnectMode);
        msg("BankCntPerChip = %x\n", param->BankCntPerChip);
        msg("DieCntPerChip = %x\n", param->DieCntPerChip);
        msg("PlaneCntPerDie = %x\n", param->PlaneCntPerDie);
        msg("SectorCntPerPage = %x\n", param->SectorCntPerPage);
        msg("PageCntPerPhyBlk = %x\n", param->PageCntPerPhyBlk);
        msg("BlkCntPerDie = %x\n", param->BlkCntPerDie);
        msg("OperationOpt = %x\n", param->OperationOpt);
        msg("FrequencePar = %x\n", param->FrequencePar);
        msg("EccMode = %x\n", param->EccMode);
        msg("NandChipId = %x, %x\n", param->NandChipId[0], param->NandChipId[4]);
        msg("ValidBlkRatio = %x\n", param->ValidBlkRatio);
        msg("good_block_ratio = %x\n", param->good_block_ratio);
        msg("ReadRetryType = %x\n", param->ReadRetryType);
        msg("DDRType = %x\n", param->DDRType);
#endif
        memcpy( (void *)param, BT0_head.prvt_head.storage_data, sizeof(boot_nand_para_t));

#else
	param->ChipCnt = 2;
	param->ChipConnectInfo = 5;
	param->RbCnt = 1;
	param->RbConnectInfo = 1;
	param->RbConnectMode = 3;
	param->BankCntPerChip = 1;
	param->DieCntPerChip = 1;
	param->PlaneCntPerDie = 2;
	param->SectorCntPerPage = 0x10;
	param->PageCntPerPhyBlk = 0x100;
	param->BlkCntPerDie = 0x800;
	param->OperationOpt = 0x48;
	param->FrequencePar = 0x1f;
	param->EccMode = 2;
	memset(param->NandChipId, 0xff, 8);
	param->NandChipId[0] = 0xad;
	param->NandChipId[1] = 0xd7;
	param->NandChipId[2] = 0x94;
	param->NandChipId[3] = 0x9a;
	param->ValidBlkRatio = 900;
	param->good_block_ratio = 960;
	param->ReadRetryType = 0;
	param->DDRType = 0;
#endif
	return 0;
}


