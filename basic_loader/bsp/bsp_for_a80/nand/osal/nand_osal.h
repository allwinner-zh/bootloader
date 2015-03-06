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
#ifndef __NAND_OSAL_H__
#define __NAND_OSAL_H__

#include <string.h>
#include "types.h"
#include "boot0_i.h"

#define __OS_EBASE_SYSTEM__
#define __OS_NAND_DBG__

#define NAND_IO_BASE_ADDR  0x01c03000
#define   NAND_IO_BASE_ADDR0   0x1c03000

#ifndef   CONFIG_ARCH_SUN9IW1P1
#define   NAND_IO_BASE_ADDR1   0x1c05000
#else
#define   NAND_IO_BASE_ADDR1   0x1c04000
#endif

extern void *NAND_IORemap(unsigned int base_addr, unsigned int size);

//USE_SYS_CLK
extern int NAND_ClkRequest(unsigned int nand_index);
extern void NAND_ClkRelease(unsigned int nand_index);
extern int NAND_SetClk(unsigned int nand_index, unsigned int nand_clk);
extern int NAND_GetClk(unsigned int nand_index);

extern void NAND_PIORequest(unsigned int nand_index);
extern void NAND_PIORelease(unsigned int nand_index);

extern void NAND_EnRbInt(void);
extern void NAND_ClearRbInt(void);
extern int NAND_WaitRbReady(void);
extern int NAND_WaitDmaFinish(void);
extern void NAND_RbInterrupt(void);

extern void* NAND_Malloc(unsigned int Size);
extern void NAND_Free(void *pAddr, unsigned int Size);
extern int  NAND_Print(const char * str, ...);

//define the memory set interface
#define MEMSET(x,y,z)            			memset((x),(y),(z))

//define the memory copy interface
#define MEMCPY(x,y,z)                   	memcpy((x),(y),(z))

//define the memory alocate interface
#define MALLOC(x)                       	NAND_Malloc((x))

//define the memory release interface
#define FREE(x,size)                    	NAND_Free((x),(size))

//define the message print interface
#define PRINT(...)							NAND_Print(__VA_ARGS__)


#endif //__NAND_OSAL_H__
