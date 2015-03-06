/*
* (C) Copyright 2007-2013
* Allwinner Technology Co., Ltd. <www.allwinnertech.com>
* Martin zheng <zhengjiewen@allwinnertech.com>
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
#ifndef  __brom_h
#define  __brom_h


/******************************************************************************/
/*                              file head of Brom                             */
/******************************************************************************/
typedef struct _brom_file_head
{	
	__u32  jump_instruction;   // one intruction jumping to real code
	__u8   magic[8];           // ="eGON.BRM", not C-style string.
	__u32  Brom_head_size;     // the size of brom_file_head_t
	__u8   file_head_vsn[4];   // the version of brom_file_head_t
	__u8   Brom_vsn[4];        // Brom version
	__u8   platform[8];        // platform information
}brom_file_head_t;

#define BROM_FILE_HEAD_VERSION         "1100"     // X.X.XX
#define BROM_MAGIC                     "eGON.BRM"


#endif     //  ifndef __brom_h

/* end of brom.h */
