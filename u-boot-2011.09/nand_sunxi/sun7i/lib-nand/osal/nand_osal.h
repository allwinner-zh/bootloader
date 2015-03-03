/*
 * (C) Copyright 2007-2013
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

#include <common.h>

#ifndef __int64
	#define __int64 __u64
#endif
#ifndef uchar
typedef unsigned char   uchar;
#endif
#ifndef uint16
typedef unsigned short  uint16;
#endif
#ifndef uint32
typedef unsigned int    uint32;
#endif
#ifndef sint32
typedef  int            sint32;
#endif
#ifndef uint64
typedef __int64         uint64;
#endif
#ifndef sint16
typedef short           sint16;
#endif
#ifndef UINT8
typedef unsigned char   UINT8;
#endif
#ifndef UINT32
typedef unsigned int    UINT32;
#endif
#ifndef SINT32
typedef  signed int     SINT32;
#endif

#define __OS_NAND_DBG__
#define __OS_LINUX_SYSTEM__
#define __OS_USE_VADDR__

#define NAND_WRITE_BACK_ALL_LOG_BLK

#define __OS_NAND_SUPPORT_RB_INT__
#ifdef __OS_NAND_SUPPORT_RB_INT__
    #define __OS_NAND_SUPPORT_INT__
#endif

#define __OS_NAND_SUPPORT_DMA_INT__
#ifdef __OS_NAND_SUPPORT_DMA_INT__
    #define __OS_NAND_SUPPORT_INT__
#endif

#define __FPGA_TEST__

extern void *NAND_IORemap(unsigned int base_addr, unsigned int size);

//USE_SYS_CLK
extern int NAND_ClkRequest(void);
extern void NAND_ClkRelease(void);
extern int NAND_SetClk(__u32 nand_clk);
extern int NAND_GetClk(void);
extern int NAND_AHBEnable(void);
extern int NAND_ClkEnable(void);
extern void NAND_AHBDisable(void);
extern void NAND_ClkDisable(void);




extern __s32 NAND_CleanFlushDCacheRegion(__u32 buff_addr, __u32 len);
extern __u32 NAND_DMASingleMap(__u32 rw, __u32 buff_addr, __u32 len);
extern __u32 NAND_DMASingleUnmap(__u32 rw, __u32 buff_addr, __u32 len);
extern __u32 NAND_VA_TO_PA(__u32 buff_addr);
extern void* NAND_RequestDMA(void);
extern __s32 NAND_DMAConfigStart(int rw, unsigned int buff_addr, int len);

extern void NAND_PIORequest(void);
extern void NAND_PIORelease(void);


extern void NAND_EnRbInt(void);
extern void NAND_ClearRbInt(void);
extern int NAND_WaitRbReady(void);
extern __s32 NAND_WaitDmaFinish(void);

extern void NAND_Memset(void* pAddr, unsigned char value, unsigned int len);
extern void NAND_Memcpy(void* pAddr_dst, void* pAddr_src, unsigned int len);
extern void* NAND_Malloc(unsigned int Size);
extern void NAND_Free(void *pAddr, unsigned int Size);
extern int NAND_Print(const char * str, ...);

extern __u32 NAND_GetIOBaseAddrCH0(void);
extern __u32 NAND_GetIOBaseAddrCH1(void);
extern __u32 NAND_GetNandIDNumCtrl(void);
extern __u32 NAND_GetNandExtPara(__u32 para_num);
//extern __s32 NAND_UpdatePhyArch(void);

extern int NAND_PhysicLockInit(void);
extern int NAND_PhysicLock(void);
extern int NAND_PhysicUnLock(void);
extern int NAND_PhysicLockExit(void);

extern __u32 NAND_Get_nandp0(void);
extern __u32 NAND_Getidnumberctl(void);

extern void NAND_ShowEnv(__u32 type, char *name, __u32 len, __u32 *val);

#define NAND_IO_BASE_ADDR0		( NAND_GetIOBaseAddrCH0() )
#define NAND_IO_BASE_ADDR1		( NAND_GetIOBaseAddrCH1() )

#define NAND_IO_BASE_ADDR		( 0x01c03000 )


//#define MAX_NFC_CH				( NAND_GetMaxChannelCnt() )
#define PLATFORM				(NAND_GetPlatform())
//define the memory set interface
#define MEMSET(x,y,z)            			NAND_Memset((x),(y),(z))

//define the memory copy interface
#define MEMCPY(x,y,z)                   	NAND_Memcpy((x),(y),(z))

//define the memory alocate interface
#define MALLOC(x)                       	NAND_Malloc((x))

//define the memory release interface
#define FREE(x,size)                    	NAND_Free((x),(size))

//define the message print interface
#define PRINT(...)							NAND_Print(__VA_ARGS__)

#define DBUG_MSG(...)

#define DBUG_INF(...)                       NAND_Print(__VA_ARGS__)


#endif //__NAND_OSAL_H__
