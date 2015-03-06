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


#include  <stdlib.h>


#define  MASK_LOW4      0xf
#define  NEGATIVE       1
#define  POSITIVE       0
#define  HEX_x          'x'
#define  HEX_X          'X'








/*
******************************************************************************************************************
*
*Function Name : int_to_string_dec
*
*Description : This function is to convert an 'int' data 'input' to a string in decimalism, and the string
*              converted is in 'str'.
*
*Input : int input : 'int' data to be converted.
*        char * str : the start address of the string storing the converted result.
*
*Output : void
*
*call for :
*
*Others : None at present.
*
*******************************************************************************************************************
*/



//void _sys_exit( void )
//{
//	return;
//}



//#pragma arm section  code="int_to_string_dec"
void int_to_string_dec( int input , char * str)
{
	char stack[12];
	char sign_flag = POSITIVE ;      // 'sign_flag indicates wheater 'input' is positive or negative, default
	int i ;                           // value is 'POSITIVE'.
	int j ;



	if( input == 0 )
	{
		str[0] = '0';
		str[1] = '\0';                   // 'str' must end with '\0'
		return ;
	}

	if( input < 0 )                      // If 'input' is negative, 'input' is assigned to its absolute value.
	{
		sign_flag = NEGATIVE ;
		input = -input ;
	}

	for( i = 0; input > 0; ++i )
	{
		stack[i] = input%10 + '0';      // characters in reverse order are put in 'stack' .
		input /= 10;
	}                                   // at the end of 'for' loop, 'i' is the number of characters.


    j = 0;
	if( sign_flag == NEGATIVE )
		str[j++] = '-';		            // If 'input' is negative, minus sign '-' is placed in the head.
	for( --i  ; i >= 0; --i, ++j )
		str[j] = stack[i];
	str[j] = '\0';				        // 'str' must end with '\0'
}







/*
******************************************************************************************************************
*
*Function Name : Uint_to_string_dec
*
*Description : This function is to convert an 'unsigned int' data 'input' to a string in decimalism, and the
*              string converted is in 'str'.
*
*Input : int input : 'unsigned int' data to be converted.
*        char * str : the start address of the string storing the converted result.
*
*Output : void
*
*call for :
*
*Others : None at present.
*
*******************************************************************************************************************
*/
//#pragma arm section  code="Uint_to_string_dec"
void Uint_to_string_dec( unsigned int input, char * str )
{
	char stack[11] ;
	int i ;
	int j ;


	if( input == 0 )
	{
		str[0] = '0';
		str[1] = 0;                   // 'str' must end with '\0'
		return ;
	}

	for( i = 0; input > 0; ++i )
	{
		stack[i] = input%10 + '0';       // characters in reverse order are put in 'stack' .
		input /= 10;
	}                                    // at the end of 'for' loop, 'i' is the number of characters.

	for( --i, j = 0; i >= 0; --i, ++j )
		str[j] = stack[i];
	str[j] = '\0';		                 // 'str' must end with '\0'
}








/*
******************************************************************************************************************
*
*Function Name : int_to_string_hex
*
*Description : This function is to convert an 'int' data 'input' to a string in hex, and the string
*              converted is in 'str'.
*
*Input : int input : 'int' data to be converted.
*        char * str : the start address of the string storing the converted result.
*        int hex_flag : hex_flag is just a option to distinguish whether hex format is '0f' or '0F'.
*
*Output : void
*
*call for :
*
*Others : None at present.
*
*******************************************************************************************************************
*/
//#pragma arm section  code="int_to_string_hex"
void int_to_string_hex( int input, char * str, int hex_flag )
{
	int i;
	static char base[] = "0123456789abcdef";


	for( i = 9; i > 1; --i )
	{
		str[i] = base[input&MASK_LOW4];
		input >>= 4;
	}

	str[0] = '0';
	str[1] = 'x';                         // Hex format shoud start with "0x" or "0X".
	str[10] = '\0';                        // 'str' must end with '\0'

	return;
}




