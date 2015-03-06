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

#ifndef  __check_h
#define  __check_h



#define CHECK_IS_WRONG             1
#define CHECK_IS_CORRECT           0


extern __s32 check_magic( __u32 *mem_base, const char *magic );
extern __s32 check_sum  ( __u32 *mem_base, __u32 size );
extern __s32 check_file ( __u32 *mem_base, __u32 size, const char *magic );







#endif // end of #ifndef __check_h

/* end of check.h  */
