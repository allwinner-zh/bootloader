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
//#define __FPGA__

#include "types.h"
#include "bsp.h"

#define   UART_OFFSET   0x400
#define   TxFIFOSize    1024
//UART Baudrate Control
#define   UART_BAUD    115200      // Baud rate for UART
                                   // Compute the divisor factor
// UART Line Control Parameter
#define   PARITY       0           // Parity: 0,2 - no parity; 1 - odd parity; 3 - even parity
#define   STOP         0           // Number of Stop Bit: 0 - 1bit; 1 - 2(or 1.5)bits
#define   DLEN         3           // Data Length: 0 - 5bits; 1 - 6bits; 2 - 7bits; 3 - 8bits


#define SERIAL_CHAR_READY()     ( UART_REG_LSR(port) & ( 1 << 0 ) )
#define SERIAL_READ_CHAR()      UART_REG_RBR(port)
#define SERIAL_READ_STATUS()    ( UART_REG_LSR(port) & 0xFF )
#define SERIAL_WRITE_STATUS()	UART_REG_LSR(port)
#define SERIAL_WRITE_READY()	( UART_REG_LSR(port) & ( 1 << 6 ) )
#define SERIAL_WRITE_CHAR(c)	( ( UART_REG_THR(port) ) = ( c ) )


static  __u32  port = 0;


void UART_open( __s32 uart_port, void  *uart_ctrl, __u32 apb_freq )
{
  	__u32   temp=0, i;
  	__u32   uart_clk;
  	__u32   lcr;
  	volatile unsigned int   *reg;

	port = uart_port;

	// config clock
	if(port > 7)
	{
		return ;
	}
	//reset
	reg = (volatile unsigned int *)(0x06000400 + 0x1B4);
	*reg &= ~(1 << (16 + port));
	for( i = 0; i < 100; i++ );
	*reg |=  (1 << (16 + port));
	//gate
	reg = (volatile unsigned int *)(0x06000400 + 0x194);
	*reg &= ~(1 << (16 + port));
	for( i = 0; i < 100; i++ );
	*reg |=  (1 << (16 + port));

	(*(volatile unsigned int *)0x01c202D8) |= (1 << (16 + port));
	// config uart gpio
	// config tx gpio
	boot_set_gpio((void *)uart_ctrl, 2, 1);
    // Set Baudrate
    uart_clk = ( apb_freq + 8*UART_BAUD ) / (16*UART_BAUD);
    lcr = UART_REG_LCR(port);
    UART_REG_HALT(port) = 1;
    UART_REG_LCR(port) = lcr | 0x80;
    UART_REG_DLH(port) = uart_clk>>8;
    UART_REG_DLL(port) = uart_clk&0xff;
    UART_REG_LCR(port) = lcr & (~0x80);
	UART_REG_HALT(port) = 0;
    // Set Lin Control Register
    temp = ((PARITY&0x03)<<3) | ((STOP&0x01)<<2) | (DLEN&0x03);
    UART_REG_LCR(port) = temp;

    // Disable FIFOs
    UART_REG_FCR(port) = 0x06;
}


void UART_close( void )
{
	return ;
}



//char UART_getchar( __u32 timeout )
//{
//	__u32  i;
//
//	if( timeout == 0 )           // infinite wait
//	{
//		while( !SERIAL_CHAR_READY() )
//			;
//		return SERIAL_READ_CHAR();
//	}
//	else
//	{
//		for( i = 0; i < timeout; i++ )
//		{
//			if(SERIAL_CHAR_READY())
//				break;
//		}
//		if( i == timeout )
//			return 0;
//		else
//			return SERIAL_READ_CHAR();
//	}
//}




void UART_putchar(char c)
{
	while (!SERIAL_WRITE_READY())
	  ;                       /* nothing */

	SERIAL_WRITE_CHAR(c);
}
