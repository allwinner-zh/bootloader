;/*
; * (C) Copyright 2007-2013
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

#define  ARMV7_USR_MODE        0x10
#define  ARMV7_FIQ_MODE        0x11
#define  ARMV7_IRQ_MODE        0x12
#define  ARMV7_SVC_MODE        0x13
#define  ARMV7_MON_MODE        0x16
#define  ARMV7_ABT_MODE        0x17
#define  ARMV7_UND_MODE        0x1b
#define  ARMV7_SYSTEM_MODE     0x1f
#define  ARMV7_MODE_MASK       0x1f
#define  ARMV7_FIQ_MASK        0x40
#define  ARMV7_IRQ_MASK        0x80


#define   ARMV7_THUMB_MASK    (1<<5)
#define   ARMV7_END_MASK      (1<<9)

#define   ARMV7_IT_MASK       ((0x3f<<10)|(0x03<<25))
#define   ARMV7_GE_MASK       (0x0f<<16)
#define   ARMV7_JAVA_MASK     (1<<24)

#define   ARMV7_QFLAG_BIT     (1 << 27)
#define   ARMV7_CC_V_BIT      (1 << 28)
#define   ARMV7_CC_C_BIT      (1 << 29)
#define   ARMV7_CC_Z_BIT      (1 << 30)
#define   ARMV7_CC_N_BIT      (1 << 31)
#define   ARMV7_CC_E_BIT      (1 << 9)

#define   ARMV7_C1_M_BIT         ( 1 << 0 )
#define   ARMV7_C1_A_BIT         ( 1 << 1 )
#define   ARMV7_C1_C_BIT         ( 1 << 2 )
#define   ARMV7_C1_Z_BIT         ( 1 << 11 )
#define   ARMV7_C1_I_BIT         ( 1 << 12 )


	.globl cpux_s_start
cpux_s_start:
@;//disable all interrupts
@;//set processor mode as SVC
	mrs r0, cpsr
	bic r0, r0, #ARMV7_MODE_MASK
	orr r0, r0, #ARMV7_SVC_MODE
	orr r0, r0, #(ARMV7_FIQ_MASK  | ARMV7_IRQ_MASK )      @;// After reset, ARM automaticly disables IRQ and FIQ, and runs in SVC mode.
	bic r0, r0, #ARMV7_CC_E_BIT                           @;// set little-endian
	msr cpsr_c, r0


@;// configure memory system : disable MMU,cache and write buffer; set little_endian; set high interrupt vector table
	mrc p15, 0, r0, c1, c0
	bic r0, r0, #( ARMV7_C1_M_BIT | ARMV7_C1_C_BIT )  @;// disable MMU, data cache
	bic r0, r0, #( ARMV7_C1_I_BIT | ARMV7_C1_Z_BIT )  @;// disable instruction cache, disable flow prediction
	bic r0, r0, #( ARMV7_C1_A_BIT)                    @;// disable align
	mcr p15, 0, r0, c1, c0

	mrs r0, cpsr
    bic r0, r0, #ARMV7_MODE_MASK
    orr r0, r0, #ARMV7_SVC_MODE
    msr cpsr_c, r0

@;/**********************************the end of initializing system*********************************/

@;// set SP for SVC mode
	mrc p15, 0, r0, c0, c0, 5
	lsl	r0, r0, #10
	add	sp, r0, #0x41000000

	b  cpux_c_start

 	b .                                 @;// infinite loop

