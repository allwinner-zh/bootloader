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

#ifndef  __basic_nf_func_h
#define  __basic_nf_func_h

#define BOOT0_START_BLK_NUM             0
#define BLKS_FOR_BOOT0                  2
#define BOOT1_START_BLK_NUM             BLKS_FOR_BOOT0

#define BLKS_FOR_BOOT1_IN_16K_BLK_NF    32
#define BLKS_FOR_BOOT1_IN_32K_BLK_NF    8
#define BLKS_FOR_BOOT1_IN_128K_BLK_NF   5
#define BLKS_FOR_BOOT1_IN_256K_BLK_NF   5
#define BLKS_FOR_BOOT1_IN_512K_BLK_NF   5
#define BLKS_FOR_BOOT1_IN_1M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_2M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_4M_BLK_NF     5
#define BLKS_FOR_BOOT1_IN_8M_BLK_NF     5

extern __u32 NF_BLOCK_SIZE;
extern __u32 NF_BLK_SZ_WIDTH;
extern __u32 NF_PAGE_SIZE;
extern __u32 NF_PG_SZ_WIDTH;
extern __u32 BOOT1_LAST_BLK_NUM;
extern __u32 page_with_bad_block;


#define NF_SECTOR_SIZE                  512U
#define NF_SCT_SZ_WIDTH                 9U
#define OOB_BUF_SIZE_PER_SECTOR         4


enum
{
	NF_OK         = 0,
	NF_GOOD_BLOCK = 0,
	NF_OVERTIME_ERR  ,
	NF_ECC_ERR       ,
	NF_BAD_BLOCK     ,
	NF_ERASE_ERR     ,
	NF_PROG_ERR      ,
	NF_NEW_BAD_BLOCK ,
	NF_LACK_BLKS     ,
	NF_ERROR     = -1,
	NF_ERR_COUNT              // always the last one
};


#define MAX_PAGE_SIZE         SZ_8K
#define BAD_BLK_FLAG          0



extern __s32  NF_open ( void );
extern __s32  NF_close( void );
extern __s32  NF_read ( __u32 sector_num, void *buffer, __u32 N );
extern __s32  NF_write( __u32 sector_num, void *buffer, __u32 N );
extern __s32  NF_erase( __u32 blk_num );
extern __s32  NF_read_status ( __u32 blk_num );
extern __s32  NF_mark_bad_block( __u32 blk_num );
extern __s32  NF_verify_block( __u32 blk_num );




#endif     //  ifndef __basic_nf_func_h


/* end of basic_nf_func.h  */
