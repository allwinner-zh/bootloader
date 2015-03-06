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

#include "types.h"
#include "boot0_v2.h"

#pragma  arm section  rodata="fes1_head"

#define   DDR3_USED

const boot0_file_head_t  fes1_head = {
	                                  {
	      /* jump_instruction */          ( 0xEA000000 | ( ( ( sizeof( boot0_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int ) - 2 ) & 0x00FFFFFF ) ),
							   		      BOOT0_MAGIC,
							   		      STAMP_VALUE,
							   		      32,
							   		      sizeof( boot_file_head_t ),
							   		      BOOT_PUB_HEAD_VERSION,
							   		      FES1_DRAM_RET_ADDR,
							   		      FES1_RUN_ADDR,
							   		      EGON_VERSION,
							   		      {
							   		      	0, 0, '2','.','0','.','0',0
							   		      },
							 	      },
#ifdef  DDR3_USED
							 	      {
							 	          sizeof( boot0_private_head_t ),
							 	          { 0 },
							 	          { 0 },
							 	          0,
										  {
										  	{ 2, 22, 4, 1, 1, 0, 0, 0},
										  	{ 2, 23, 4, 1, 1, 0, 0, 0}
										  },
							 	          0,
							 	          { 0 },
							 	          { 0 },
							 	          { 0 }
							 	      }
#else
							 	      {
							 	          sizeof( boot0_private_head_t ),
							 	          BOOT0_PRVT_HEAD_VERSION,
							 	          { 0x40000000,
							 	            1024,
							 	            180,
							 	            1,
							 	            1,
							 	            0,
							 	            (__dram_type_e)1,
							 	            16,
							 	            10,
							 	            14,
							 	            4,
							 	            3,
							 	            0,
							 	            16,
							 	            1024
							 	          },
							 	          0,
										  {
										  	{ 2, 22, 4, 1, 1, 0, 0, 0},
										  	{ 2, 23, 4, 1, 1, 0, 0, 0}
										  },
							 	          0,
							 	          { 0 },
							 	          { 0 },
							 	          { 0 }
							 	      }
#endif
							 	  };



/*******************************************************************************
*
*                  关于Boot_file_head中的jump_instruction字段
*
*  jump_instruction字段存放的是一条跳转指令：( B  BACK_OF_Boot_file_head )，此跳
*转指令被执行后，程序将跳转到Boot_file_head后面第一条指令。
*
*  ARM指令中的B指令编码如下：
*          +--------+---------+------------------------------+
*          | 31--28 | 27--24  |            23--0             |
*          +--------+---------+------------------------------+
*          |  cond  | 1 0 1 0 |        signed_immed_24       |
*          +--------+---------+------------------------------+
*  《ARM Architecture Reference Manual》对于此指令有如下解释：
*  Syntax :
*  B{<cond>}  <target_address>
*    <cond>    Is the condition under which the instruction is executed. If the
*              <cond> is ommitted, the AL(always,its code is 0b1110 )is used.
*    <target_address>
*              Specified the address to branch to. The branch target address is
*              calculated by:
*              1.  Sign-extending the 24-bit signed(wro's complement)immediate
*                  to 32 bits.
*              2.  Shifting the result left two bits.
*              3.  Adding to the contents of the PC, which contains the address
*                  of the branch instruction plus 8.
*
*  由此可知，此指令编码的最高8位为：0b11101010，低24位根据Boot_file_head的大小动
*态生成，所以指令的组装过程如下：
*  ( sizeof( boot_file_head_t ) + sizeof( int ) - 1 ) / sizeof( int )
*                                              求出文件头占用的“字”的个数
*  - 2                                         减去PC预取的指令条数
*  & 0x00FFFFFF                                求出signed-immed-24
*  | 0xEA000000                                组装成B指令
*
*******************************************************************************/

