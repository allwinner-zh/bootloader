/*
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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

#define  UART_REGS_BASE    			UART0_REGS_BASE

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


#define UART_REG_RBR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_RBR  )
#define UART_REG_THR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_THR  )
#define UART_REG_DLL(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_DLL  )
#define UART_REG_DLH(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_DLH  )
#define UART_REG_IER(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_IER  )
#define UART_REG_IIR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_IIR  )
#define UART_REG_FCR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_FCR  )
#define UART_REG_LCR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_LCR  )
#define UART_REG_MCR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_MCR  )
#define UART_REG_LSR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_LSR  )
#define UART_REG_MSR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_MSR  )
#define UART_REG_SCH(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_SCH  )
#define UART_REG_USR(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_USR  )
#define UART_REG_TFL(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_TFL  )
#define UART_REG_RFL(port)          readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_RFL  )
#define UART_REG_HALT(port)         readl( UART_REGS_BASE + port * UART_OFFSET + UART_REG_o_HALT )


#endif    /*  #ifndef _UART_H_  */
