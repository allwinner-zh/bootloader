/*
 * (C) Copyright 2012
 *     wangflord@allwinnertech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */

#include "boot0_i.h"
#include "spinor/sunxi_spinor.h"
#include "load_boot1_from_spinor.h"

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int load_boot1_from_spinor(void)
{
	__u32 length;
	uboot_file_head  *bfh;

	if(spinor_init())
	{
		msg("spinor init fail\n");

		return ERROR;
	}
	/* 载入当前块最前面512字节的数据到SRAM中，目的是获取文件头 */
	if(spinor_read(BOOT1_START_SECTOR_IN_SPINOR, 1, (void *)UBOOT_BASE ) )
	{
		msg("the first data is error\n");

		goto __load_boot1_from_spinor_fail;
	}
	msg("Succeed in reading Boot1 file head.\n");

	/* 察看是否是文件头 */
	if( check_magic( (__u32 *)UBOOT_BASE, UBOOT_MAGIC ) != CHECK_IS_CORRECT )
	{
		msg("ERROR! Add %u doesn't store head of Boot1 copy.\n", BOOT1_START_SECTOR_IN_SPINOR );

		goto __load_boot1_from_spinor_fail;
	}

	bfh = (uboot_file_head *) UBOOT_BASE;
	length =  bfh->boot_head.length;
	msg("The size of uboot is %x.\n", length );
	if( ( length & ( 512 - 1 ) ) != 0 ) 	// length必须是NF_SECTOR_SIZE对齐的
	{
		msg("the boot1 is not aligned by %x\n", bfh->boot_head.align_size);

		goto __load_boot1_from_spinor_fail;
	}

	if(spinor_read(BOOT1_START_SECTOR_IN_SPINOR, length/512, (void *)UBOOT_BASE ))
	{
		msg("spinor read data	error\n");

		goto __load_boot1_from_spinor_fail;
	}
	bfh->boot_data.storage_type = 3;

	return OK;

__load_boot1_from_spinor_fail:

	return ERROR;
}



