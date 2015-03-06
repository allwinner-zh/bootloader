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

#ifndef  __adv_NF_func_h
#define  __adv_NF_func_h


typedef enum
{
	ADV_NF_OK               =0,
	ADV_NF_FIND_OK          =0,
	ADV_NF_NO_NEW_BAD_BLOCK =0,
	ADV_NF_ERROR              ,
	ADV_NF_NO_FIND_ERR        ,
	ADV_NF_OVERTIME_ERR       ,
	ADV_NF_LACK_BLKS          ,
	ADV_NF_NEW_BAD_BLOCK      ,
}adv_nf_errer_e;


extern __s32 load_and_check_in_one_blk( __u32 blk_num, void *buf, __u32 size,
                                        __u32 blk_size, const char *magic );

extern __s32 load_in_many_blks( __u32 start_blk, __u32 last_blk_num, void *buf,
						        __u32 size, __u32 blk_size, __u32 *blks );

extern __s32 write_in_one_blk( __u32 blk_num, void *buf, __u32 size, __u32 blk_size );

extern __s32 write_in_many_blks( __u32 start_blk, __u32 last_blk_num, void *buf,
					             __u32 size, __u32 blk_size, __u32 * blks );







#endif     //  ifndef __adv_NF_func_h


/* end of adv_NF_func.h  */
