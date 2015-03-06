;/*
; * (C) Copyright 2007-2015
; * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
; * Jerry Wang <wangflord@allwinnertech.com>
; *
; * See file CREDITS for list of people who contributed to this
; * project.
; *
; * This program is free software; you can redistribute it and/or
; * modify it under the terms of the GNU General Public License as
; * published by the Free Software Foundation; either version 2 of
; * the License, or (at your option) any later version.
; *
; * This program is distributed in the hope that it will be useful,
; * but WITHOUT ANY WARRANTY; without even the implied warranty of
; * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
; * GNU General Public License for more details.
; *
; * You should have received a copy of the GNU General Public License
; * along with this program; if not, write to the Free Software
; * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
; * MA 02111-1307 USA
; */
;
    IF :LNOT::DEF:__egon_def_inc__
	GBLA  __egon_def_inc__

    INCLUDE  egon_mem_distribute.inc


EGON2_DRAM_BASE       equ          0x40000000
EGON2_DRAM_SIZE       equ          0x02000000
EGON2_SRAM_BASE       equ          0x0
EGON2_SRAM_SIZE       equ          ( 32 * 1024 )
EGON2_RESET_BASE      equ          0xFFFF0000



BOOT0_BASE            equ          ( EGON2_SRAM_BASE                       )
BOOT1_BASE            equ          ( EGON2_DRAM_BASE  + EGON2_BOOT1_OFFSET )
FEL_BASE              equ          ( EGON2_RESET_BASE + 0x20               )






	ENDIF     ;;; IF :LNOT::DEF:__egon_def_inc__

	END       ;;; end of egon_def.inc
