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

#ifndef _UART_H_
#define _UART_H_


#include "hd_scrpt.h"





/*
*********************************************************************************************************
*   UART define          < maintained by Gary >
*********************************************************************************************************
*/
  /* port */
#define UART0                       0
#define UART1                       1
#define UART2                       2
#define UART3                       3
#define UART_OFFSET                 0x400

#define UART0_IRQ_NO                1
#define UART1_IRQ_NO                2
#define UART2_IRQ_NO                3
#define UART3_IRQ_NO                4

  /* offset */
#define UART_REG_o_RBR              0x00
#define UART_REG_o_THR              0x00
#define UART_REG_o_DLL              0x00
#define UART_REG_o_DLH              0x04
#define UART_REG_o_IER              0x04
#define UART_REG_o_IIR              0x08
#define UART_REG_o_FCR              0x08
#define UART_REG_o_LCR              0x0C
#define UART_REG_o_MCR              0x10
#define UART_REG_o_LSR              0x14
#define UART_REG_o_MSR              0x18
#define UART_REG_o_SCH              0x1C
#define UART_REG_o_USR              0x7C
#define UART_REG_o_TFL              0x80
#define UART_REG_o_RFL              0x84
#define UART_REG_o_HALT             0xA4
  /* registers */
#define UART_REGS_BASE              UART0_REGS_BASE
    /* UART 0 */
#define URT0_REG_RBR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_RBR  )
#define URT0_REG_THR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_THR  )
#define URT0_REG_DLL                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_DLL  )
#define URT0_REG_DLH                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_DLH  )
#define URT0_REG_IER                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_IER  )
#define URT0_REG_IIR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_IIR  )
#define URT0_REG_FCR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_FCR  )
#define URT0_REG_LCR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_LCR  )
#define URT0_REG_MCR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_MCR  )
#define URT0_REG_LSR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_LSR  )
#define URT0_REG_MSR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_MSR  )
#define URT0_REG_SCH                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_SCH  )
#define URT0_REG_USR                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_USR  )
#define URT0_REG_TFL                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_TFL  )
#define URT0_REG_RFL                __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_RFL  )
#define URT0_REG_HALT               __REG( UART_REGS_BASE + UART0 * UART_OFFSET + UART_REG_o_HALT )
	/* UART 1 */
#define URT1_REG_RBR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_RBR  )
#define URT1_REG_THR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_THR  )
#define URT1_REG_DLL                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_DLL  )
#define URT1_REG_DLH                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_DLH  )
#define URT1_REG_IER                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_IER  )
#define URT1_REG_IIR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_IIR  )
#define URT1_REG_FCR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_FCR  )
#define URT1_REG_LCR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_LCR  )
#define URT1_REG_MCR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_MCR  )
#define URT1_REG_LSR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_LSR  )
#define URT1_REG_MSR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_MSR  )
#define URT1_REG_SCH                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_SCH  )
#define URT1_REG_USR                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_USR  )
#define URT1_REG_TFL                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_TFL  )
#define URT1_REG_RFL                __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_RFL  )
#define URT1_REG_HALT               __REG( UART_REGS_BASE + UART1 * UART_OFFSET + UART_REG_o_HALT )
  /* registers' another definition */
#define UART_REG_RBR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_RBR  )
#define UART_REG_THR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_THR  )
#define UART_REG_DLL(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_DLL  )
#define UART_REG_DLH(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_DLH  )
#define UART_REG_IER(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_IER  )
#define UART_REG_IIR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_IIR  )
#define UART_REG_FCR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_FCR  )
#define UART_REG_LCR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_LCR  )
#define UART_REG_MCR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_MCR  )
#define UART_REG_LSR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_LSR  )
#define UART_REG_MSR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_MSR  )
#define UART_REG_SCH(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_SCH  )
#define UART_REG_USR(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_USR  )
#define UART_REG_TFL(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_TFL  )
#define UART_REG_RFL(port)          __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_RFL  )
#define UART_REG_HALT(port)         __REG( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_HALT )

#define UART_RBR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_RBR  )
#define UART_THR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_THR  )
#define UART_DLL(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_DLL  )
#define UART_DLH(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_DLH  )
#define UART_IER(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_IER  )
#define UART_IIR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_IIR  )
#define UART_FCR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_FCR  )
#define UART_LCR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_LCR  )
#define UART_MCR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_MCR  )
#define UART_LSR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_LSR  )
#define UART_MSR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_MSR  )
#define UART_SCH(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_SCH  )
#define UART_USR(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_USR  )
#define UART_TFL(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_TFL  )
#define UART_RFL(port)                   ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_RFL  )
#define UART_HALT(port)                  ( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_HALT )


#endif    /*  #ifndef _UART_H_  */
