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

        INCLUDE    boot0_i.inc

    	IMPORT   Boot0_C_part
		IMPORT   cpu_init_s

		PRESERVE8


        AREA  init, CODE, READONLY
        CODE32
		ENTRY



;/*********************************the begin of initializing system********************************/



initialize


	mrs r0, cpsr
	bic r0, r0, #ARMV7_MODE_MASK
	orr r0, r0, #ARMV7_SVC_MODE
	orr r0, r0, #( ARMV7_IRQ_MASK | ARMV7_FIQ_MASK )    ;// After reset, ARM automaticly disables IRQ and FIQ, and runs in SVC mode.
	bic r0, r0, #ARMV7_CC_E_BIT                         ;// set little-endian
	msr cpsr_c, r0


;// configure memory system : disable MMU,cache and write buffer; set little_endian;
	mrc p15, 0, r0, c1, c0
	bic r0, r0, #( ARMV7_C1_M_BIT | ARMV7_C1_C_BIT )  ;// disable MMU, data cache
	bic r0, r0, #( ARMV7_C1_I_BIT | ARMV7_C1_Z_BIT )  ;// disable instruction cache, disable flow prediction
	bic r0, r0, #( ARMV7_C1_A_BIT)                    ;// disable align
	mcr p15, 0, r0, c1, c0

;// set SP for C language
	ldr sp, =BOOT0_STACK_BOTTOM

;// cpu init
	bl  cpu_init_s
;/**********************************the end of initializing system*********************************/
	bl  Boot0_C_part

 	b .                                 ;// infinite loop



	END
