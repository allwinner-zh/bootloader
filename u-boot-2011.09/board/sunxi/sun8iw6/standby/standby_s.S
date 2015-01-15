/*
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
 */


/*
;************************************************************************************************************
;*
;*                                             function
;*
;*    函数名称：
;*
;*    参数列表：
;*
;*    返回值  ：
;*
;*    说明    ：
;*
;*
;************************************************************************************************************
*/

	.globl boot_store_sp
boot_store_sp:
	STMFD   sp!, {r0-r3}
	ldr  r1, =0xef0
	str  sp, [r1]

	mov  pc, lr
/*
;************************************************************************************************************
;*
;*                                             function
;*
;*    函数名称：
;*
;*    参数列表：
;*
;*    返回值  ：
;*
;*    说明    ：
;*
;*
;************************************************************************************************************
*/
	.globl boot_restore_sp
boot_restore_sp:
	ldr  r1, =0xef0
	ldr  sp, [r1]
	LDMFD   sp!, {r0-r3}

	mov  pc, lr
/*
;************************************************************************************************************
;*
;*                                             function
;*
;*    函数名称：
;*
;*    参数列表：
;*
;*    返回值  ：
;*
;*    说明    ：
;*
;*
;************************************************************************************************************
*/
	.globl boot_set_sp
boot_set_sp:
	ldr  sp, =0x7000
	mov  pc, lr

/*
;************************************************************************************************************
;*
;*                                             function
;*
;*    函数名称：
;*
;*    参数列表：
;*
;*    返回值  ：
;*
;*    说明    ：
;*
;*
;************************************************************************************************************
*/
	.globl boot_halt
boot_halt:
	wfi

	mov  pc, lr



