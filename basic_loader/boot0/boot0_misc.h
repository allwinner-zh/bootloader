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

#ifndef  __boot0_misc_h
#define  __boot0_misc_h

#include "boot0_i.h"

#define BOOT0_PRVT_HEAD_VERSION         "1230"    // X.X.XX
#define BOOT0_FILE_HEAD_VERSION         "1230"    // X.X.XX

#define BOOT0_VERSION                   "1230"    // X.X.XX



#define PAGE_BUF_FOR_BOOT0              ( EGON2_DRAM_BASE + SZ_1M )

#define DEBUG

#ifdef DEBUG
#   define msg(fmt,args...)				UART_printf2(fmt ,##args)
#else
#   define msg(fmt,args...)
#endif

#define HERE                            msg("file:%s, line:%u.\n", __FILE__, __LINE__);


#define OK                    0
#define ERROR                 1



#endif     //  ifndef __boot0_misc_h

/* end of boot0_misc.h */
