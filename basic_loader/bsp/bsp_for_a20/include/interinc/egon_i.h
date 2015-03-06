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
#ifndef  __egon_i_h
#define  __egon_i_h


#define EPDK_CHIP_BDD0                  0
#define EPDK_CHIP_AF0B                  1
#define EPDK_CHIP_AF0C                  2
#define EPDK_CHIP_AFA0                  3
#define EPDK_CHIP_AFAA                  4


#define BOOT0_IN_NF           // BOOT0_IN_NF, BOOT0_IN_SF, BOOT0_IN_IE
#define BOOT1_IN_NF           // BOOT1_IN_NF, BOOT1_IN_SF, BOOT1_IN_IE
#define KENEL_IN_NF           // KENEL_IN_RAM, KENEL_IN_NF


#if SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_NAND_FLASH
	#define BOOT0_ALIGN_SIZE  NF_ALIGN_SIZE
#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SPI_NOR_FLASH
	#define BOOT0_ALIGN_SIZE  BOOT_SECTOR_SIZE
#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SD_CARD
    #define BOOT0_ALIGN_SIZE  BOOT_SECTOR_SIZE
#else
	#error The storage media has not been defined.
#endif


#if SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_NAND_FLASH
	#define BOOT1_ALIGN_SIZE  NF_ALIGN_SIZE
#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SPI_NOR_FLASH
	#define BOOT1_ALIGN_SIZE  BOOT_SECTOR_SIZE
#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SD_CARD
	#define BOOT1_ALIGN_SIZE  BOOT_SECTOR_SIZE
#else
	#error The storage media has not been defined.
#endif


#define BOOT_PUB_HEAD_VERSION           "1100"    // X.X.XX
#define EGON_VERSION                    "1100"    // X.X.XX



#endif     //  ifndef __egon_i_h

/* end of egon_i.h */
