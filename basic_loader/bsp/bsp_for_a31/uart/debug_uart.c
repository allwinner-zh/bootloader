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

#include "types.h"
#include <stdarg.h>
#include "format_transformed.h"
#include "uart_hd.h"


void UART_puts_no_newline( const char * str )
{
	while( *str != '\0' )
	{
		if( *str == '\n' )                      // if current character is '\n', insert and output '\r'
			UART_putchar( '\r' );

		UART_putchar( *str++ );
	}
}
/*
******************************************************************************************************************
*
*Function Name : UART_printf
*
*Description : This function is to formatedly output through UART, similar to ANSI C function printf().
*                This function can support and only support the following Conversion Specifiers:
*              %d		Signed decimal integer.
*              %u		Unsigned decimal integer.
*              %x		Unsigned hexadecimal integer, using hex digits 0f.
*              %X		Unsigned hexadecimal integer, using hex digits 0F.
*              %c		Single character.
*              %s		Character string.
*              %p		A pointer.
*
*Input : refer to ANSI C function printf().
*
*Output : void, different form ANSI C function printf().
*
*call for : void int_to_string_dec( __s32 input, char * str ), defined in format_transformed.c.
*           void int_to_string_hex( __s32 input, char * str );  defined in format_transformed.c.
*           void Uint_to_string_dec( __u32 input, char * str );  defined in format_transformed.c.
*           void UART_putchar( __s32 ch); defined in boot loader.
*           void UART_puts( const char * string ); defined in boot loader.
*
*Others : None at present.
*
*******************************************************************************************************************
*/
void UART_printf2( const char * str, ...)
{
	char string[13];
	char *p;
	__s32 hex_flag ;
	va_list argp;

	va_start( argp, str );

	while( *str )
	{
		if( *str == '%' )
		{
			++str;
			p = string;
			hex_flag = HEX_X;
			switch( *str )
			{
				case 'd': int_to_string_dec( va_arg( argp,  __s32 ), string );
                          UART_puts_no_newline( p );
						  ++str;
						  break;
				case 'x': hex_flag = HEX_x;	         // jump to " case 'X' "
				case 'p':
				case 'X': int_to_string_hex( va_arg( argp,  __s32 ), string, hex_flag );
						  UART_puts_no_newline( p );
                          ++str;
						  break;
				case 'u': Uint_to_string_dec( va_arg( argp,  __s32 ), string );
						  UART_puts_no_newline( p );
						  ++str;
						  break;
				case 'c': UART_putchar( va_arg( argp,  __s32 ) );
						  ++str;
						  break;
				case 's': UART_puts_no_newline( va_arg( argp, char * ) );
						  ++str;
						  break;
				default : UART_putchar( '%' );       // if current character is not Conversion Specifiers 'dxpXucs',
						  UART_putchar( *str );         // output directly '%' and current character, and then
						  ++str;                        // let 'str' point to next character.
			}
		}

		else
		{
			if( *str == '\n' )                      // if current character is '\n', insert and output '\r'
				UART_putchar( '\r' );

			UART_putchar( *str++ );
		}
	}

	va_end( argp );
}


