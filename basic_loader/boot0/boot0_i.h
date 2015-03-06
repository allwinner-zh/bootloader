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
#ifndef  __boot0_i_h
#define  __boot0_i_h


#include "types.h"
#include "storage_media_cfg.h"
#include "bsp.h"
#include "dram_for_debug.h"
#include "boot0_v2.h"
#include "spare_head.h"
#include "boot0_misc.h"
#include "drv/boot0_drv.h"
#include "sys_op/sys_op.h"
#include "lib/lib.h"
#include "load_Boot1_from_nand/load_Boot1_from_nand.h"
#include "load_boot1_from_sdmmc/load_boot1_from_sdmmc.h"

extern const boot0_file_head_t  BT0_head;

#endif     //  ifndef __boot0_i_h

/* end of boot0_i.h */
