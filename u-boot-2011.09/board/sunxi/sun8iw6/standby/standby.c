/*
 * (C) Copyright 2007-2013
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
#include <common.h>
#include "standby_i.h"
#include "standby.h"
#include "sunxi_board.h"
#include <asm/arch/gic.h>


DECLARE_GLOBAL_DATA_PTR;


static int boot_early_standby_mode(void);


#if 0
	#define STANDBY_DEBUG(c)  standby_serial_putc(c)
#else
	#define STANDBY_DEBUG(c)
#endif

void standby_serial_putc(char c)
{
	__u32 reg_val;

	do
	{
		reg_val = *(volatile unsigned int *)(0x01c2807C);
	}	while (!(reg_val & 0x02));

	*(volatile unsigned int *)(0x01c28000) = c;
}

int boot_standby_mode(void)
{
	int status;

	//axp_set_suspend_chgcur();
	boot_store_sp();
	boot_set_sp();

	STANDBY_DEBUG('A');
	status = boot_early_standby_mode();
	STANDBY_DEBUG('[');

	boot_restore_sp();
	STANDBY_DEBUG('{');

	return status;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int boot_early_standby_mode(void)
{

   return 0;

}


