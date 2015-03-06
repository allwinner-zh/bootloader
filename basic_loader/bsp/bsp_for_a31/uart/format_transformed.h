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


#ifndef  __format_transformed_h
#define  __format_transformed_h




#define  HEX_x   'x'
#define  HEX_X   'X'




//This function is to convert an 'int' data 'input' to a string in decimalism,
//and the string converted is in 'str'.
extern void int_to_string_dec( int input, char * str );


//This function is to convert an 'int' data 'input' to a string in hex,
//and the string converted is in 'str'.
extern void int_to_string_hex( int input, char * str, int hex_flag );


// This function is to convert an 'unsigned int' data 'input' to a string in decimalism, and
// the  string converted is in 'str'.
extern void Uint_to_string_dec( unsigned int input, char * str );






#endif     //  ifndef __format_transformed_h

/* end of format_transformed.h  */
