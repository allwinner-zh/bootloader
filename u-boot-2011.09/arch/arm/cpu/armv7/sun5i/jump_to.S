/*
;*****************************************************************************************************************
;                                                         eGON
;                                         the Embedded GO-ON Bootloader System
;
;                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
;											       All Rights Reserved
;
; File Name : func_in_asm.s
;
; Author : Gary.Wang
;
; Version : 1.1.0
;
; Date : 2009.09.14
;
; Description :
;
; Others : None at present.
;
;
; History :
;
;  <Author>        <time>       <version>      <description>
;
; Gary.Wang       2009.09.14      1.1.0        build the file
;
;******************************************************************************************************************
*/


/*
;*******************************************************************************
;函数名称: jump_to
;函数原型: void jump_to( __u32 entry_addr )
;函数功能: 跳转到entry_addr处执行
;入口参数: entry_addr(r0)       入口地址或者目标地址
;返 回 值: void
;备    注:
;*******************************************************************************
*/
	.globl jump_to
jump_to:
	mov pc, r0

