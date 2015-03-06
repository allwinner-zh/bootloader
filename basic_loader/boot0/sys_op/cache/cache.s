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

		EXPORT    enable_icache
		EXPORT    disable_icache
;		EXPORT	  enable_dcache
;		EXPORT	  disable_dcache
;		EXPORT    flush_dcache
		EXPORT    flush_icache

;		EXPORT    enable_prediction


		INCLUDE   ./../../boot0_i.inc


		AREA  cache, CODE, READONLY
		CODE32

begin



;*******************************************************************************
;函数名称: enable_icache
;函数原型: void enable_icache( void )
;函数功能: enable instruction cache
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
enable_icache
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0       ; I+BTB cache invalidate
	mrc p15, 0, r0, c1, c0
	orr r0, r0, #(1 << 12)           ; enable instruction cache.
	mcr p15, 0, r0, c1, c0
	mov pc, lr


;*******************************************************************************
;函数名称: enable_icache
;函数原型: void enable_icache( void )
;函数功能: enable instruction cache
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
;enable_dcache
;    mov r0, #0
;    mcr p15, 0, r0, c7, c5, 0       ; I+BTB cache invalidate
;	mrc p15, 0, r0, c1, c0
;	orr r0, r0, #(1 << 12)           ; enable instruction cache.
;	mcr p15, 0, r0, c1, c0
;	mov pc, lr


;*******************************************************************************
;函数名称: disable_icache
;函数原型: void disable_icache( void )
;函数功能: disable instruction cache
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
disable_icache
	mov r0, #0
    mcr p15, 0, r0, c7, c5, 0       ; I+BTB cache invalidate
	mrc p15, 0, r0, c1, c0
	bic r0, r0, #(1 << 2)               ;// disable instruction cache.
	mcr p15, 0, r0, c1, c0
	mov pc, lr

;*******************************************************************************
;函数名称: disable_icache
;函数原型: void disable_icache( void )
;函数功能: disable instruction cache
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
;disable_dcache
;	mov r0, #0
;    mcr p15, 0, r0, c7, c5, 0       ; I+BTB cache invalidate
;	mrc p15, 0, r0, c1, c0
;	bic r0, r0, #(1 << 2)               ;// disable instruction cache.
;	mcr p15, 0, r0, c1, c0
;	mov pc, lr


;*******************************************************************************
;函数名称: flush_dcache
;函数原型: void flush_dcache( void )
;函数功能: flush data cache
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
;//ARMV7_FLUSH_DCACHE_ALL
;flush_dcache
;    MRC     p15, 1, r0, c0, c0, 1       ; read clidr
;    ANDS    r3, r0, #0x7000000          ; extract loc from clidr
;    MOV     r3, r3, lsr #23             ; left align loc bit field
;    BEQ     finished                    ; if loc is 0, then no need to clean
;
;    mov     r10, #0                     ; start clean at cache level 0
;loop1
;    ADD     r2, r10, r10, lsr #1        ; work out 3x current cache level
;    MOV     r1, r0, lsr r2              ; extract cache type bits from clidr
;    AND     r1, r1, #7                  ; mask of the bits for current cache only
;    CMP     r1, #2                      ; see what cache we have at this level
;    BLT     skip                        ; skip if no cache, or just i-cache
;    MCR     p15, 2, r10, c0, c0, 0      ; select current cache level in cssr
;    ISB                                 ; isb to sych the new cssr&csidr
;    MRC     p15, 1, r1, c0, c0, 0       ; read the new csidr
;    AND     r2, r1, #7                  ; extract the length of the cache lines
;    ADD     r2, r2, #4                  ; add 4 (line length offset)
;    LDR     r4, =0x3ff
;    ANDS    r4, r4, r1, lsr #3          ; find maximum number on the way size
;    CLZ     r5, r4                      ; find bit position of way size increment
;    LDR     r7, =0x7fff
;    ANDS    r7, r7, r1, lsr #13         ; extract max number of the index size
;loop2
;    MOV     r9, r4                      ; create working copy of max way size
;loop3
;    ORR     r11, r10, r9, lsl r5        ; factor way and cache number into r11
;    ORR     r11, r11, r7, lsl r2        ; factor index number into r11
;    MCR     p15, 0, r11, c7, c6, 2      ; invalidate by set/way
;    SUBS    r9, r9, #1                  ; decrement the way
;    BGE     loop3
;    SUBS    r7, r7, #1                  ; decrement the index
;    BGE     loop2
;skip
;    ADD     r10, r10, #2                ; increment cache number
;    CMP     r3, r10
;    BGT     loop1
;finished
;    MOV     r10, #0                     ; swith back to cache level 0
;    MCR     p15, 2, r10, c0, c0, 0      ; select current cache level in cssr
;    ISB
;    MOV     pc, lr

;*******************************************************************************
;函数名称: flush_dcache
;函数原型: void flush_dcache( void )
;函数功能: flush data cache
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
;ARMV7_FLUSH_ICACHE_ALL
flush_icache
    MOV     r0, #0
    MCR     p15, 0, r0, c7, c5, 0       ; I+BTB cache invalidate
    MOV     pc, lr

;*******************************************************************************
;函数名称: enable_prediction
;函数原型: void enable_prediction( void )
;函数功能: enable prediction flow
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
;ARMV7_PREDICTION_ENABLE
;enable_prediction
;	stmfd   sp!, {r0, lr}
;    bl 		rd_label0                  ; Initial Return Stack to prevent 'x' for simulation
;rd_label0
;	bl 		rd_label1
;rd_label1
;	bl 		rd_label2
;rd_label2
;	bl 		rd_label3
;rd_label3
;	bl 		rd_label4
;rd_label4
;	bl 		rd_label5
;rd_label5
;	bl 		rd_label6
;rd_label6
;	bl 		rd_label7
;rd_label7
;
;    mrc     p15, 0, r0, c1, c0, 0       ; read CP15 register 1
;    orr     r0, r0, #(0x1 <<11 )        ; Enable all forms of branch prediction
;    mcr     p15, 0, r0, c1, c0, 0       ; write CP15 register 1
;
;    ldmfd   sp!, {r0, lr}
;    mov     pc, lr



	END   ; end of cache.s

