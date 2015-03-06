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
    IF :LNOT::DEF:__ARM9_INC_
	GBLA  __ARM9_INC_

ARMV7_USR_MODE        EQU    0x10
ARMV7_FIQ_MODE        EQU    0x11
ARMV7_IRQ_MODE        EQU    0x12
ARMV7_SVC_MODE        EQU    0x13
ARMV7_MON_MODE        EQU 	 0x16
ARMV7_ABT_MODE        EQU    0x17
ARMV7_UND_MODE        EQU    0x1b
ARMV7_SYSTEM_MODE     EQU    0x1f
ARMV7_MODE_MASK       EQU    0x1f
ARMV7_FIQ_MASK        EQU    0x40
ARMV7_IRQ_MASK        EQU    0x80



;//coprocessor CP15
;//C1
ARMV7_THUMB_MASK    EQU (1<<5)
ARMV7_END_MASK      EQU (1<<9)

ARMV7_IT_MASK       EQU ((0x3f<<10)|(0x03<<25))
ARMV7_GE_MASK       EQU (0x0f<<16)
ARMV7_JAVA_MASK     EQU (1<<24)

ARMV7_QFLAG_BIT     EQU (1 << 27)
ARMV7_CC_V_BIT      EQU (1 << 28)
ARMV7_CC_C_BIT      EQU (1 << 29)
ARMV7_CC_Z_BIT      EQU (1 << 30)
ARMV7_CC_N_BIT      EQU (1 << 31)
ARMV7_CC_E_BIT      EQU (1 << 9)

ARMV7_C1_M_BIT      EQU    ( 1 << 0 )
ARMV7_C1_A_BIT      EQU    ( 1 << 1 )
ARMV7_C1_C_BIT      EQU    ( 1 << 2 )
ARMV7_C1_Z_BIT      EQU    ( 1 << 11 )
ARMV7_C1_I_BIT      EQU    ( 1 << 12 )


BOOT0_STACK_BOTTOM      EQU     ( 0x0000F000 )


    endif
    end
