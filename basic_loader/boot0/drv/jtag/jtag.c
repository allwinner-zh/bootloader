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

#ifndef  __jtag_c
#define  __jtag_c


#include "..\\boot0_drv_i.h"


//*****************************************************************************
//	void jtag_init(uint32 port_id)
//  Description:	Select different JTAG port and enable JTAG
//
//	Arguments:		uint32 port_id   1: GPIOB port, 0: GPIOA port
//
//	Return Value:	None
//*****************************************************************************
void jtag_init(normal_gpio_cfg *jtag_gpio)
{
	boot_set_gpio(jtag_gpio, 6, 1);
}


/*------------------------------------------platform end------------------------------------------*/



#endif     //  ifndef __jtag_c

/* end of jtag.c */
