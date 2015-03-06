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
		EXPORT  mmu_set_ttb
		EXPORT  mmu_set_domain_access
		EXPORT  mmu_flush_TLB
		EXPORT  mmu_set_smp

		AREA  mmu_asm, CODE, READONLY
		CODE32

begin





;*******************************************************************************
;函数名称: mmu_set_ttb
;函数原型: void mmu_set_ttb( uint32 ttb_base_addr )
;函数功能: set TTB
;入口参数: ttb base address
;返 回 值: void
;备    注:
;*******************************************************************************
mmu_set_ttb
	MCR p15, 0, r0, c2, c0, 0 ; Write TTB register
	MOV pc, lr


;*******************************************************************************
;函数名称: mmu_change_domain_access
;函数原型: s32   mmu_change_domain_access( uint32 value, uint32 mask )
;函数功能: Change Domain Access Register Values
;入口参数: value
;          mask
;返 回 值: void
;备    注:
;*******************************************************************************
mmu_set_domain_access
	MRC     p15, 0, r0, c3, c0, 0     ; Read Domain Access Control Register
	LDR     r0, =0x55555555           ; Initialize every domain entry to b01 (client)
	MCR     p15, 0, r0, c3, c0, 0     ; Write Domain Access Control Register
    MOV     pc, lr



;*******************************************************************************
;函数名称: mmu_flush_TLB
;函数原型: void   mmu_flush_TLB( void )
;函数功能: clear TLB
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
mmu_flush_TLB
    MOV r0, #0
    MCR p15, 0, r0, c8, c7, 0 ; flush both the TLBs
    MOV pc, lr

;*******************************************************************************
;函数名称: mmu_flush_TLB
;函数原型: void   mmu_flush_TLB( void )
;函数功能: clear TLB
;入口参数: void
;返 回 值: void
;备    注:
;*******************************************************************************
mmu_set_smp
    mrc  p15, 0, r0, c1, c0, 1
    orr  r0, r0, #(1 << 6)
    mcr  p15, 0, r0, c1, c0, 1

	END   ; end of mmu_asm.s
