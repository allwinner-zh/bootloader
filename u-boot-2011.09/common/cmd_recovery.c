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
#include <sys_partition.h>
#include <sunxi_cmd.h>


static  int  do_recovery(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int  ret;
	char load_addr[16];
	uint addr;
	char *const flash_argv[4] = { "sunxi_flash", "read", load_addr, "recovery"};
	char *const boota_argv[2] = { "boota", load_addr};

	if(argc > 2)
	{
		printf("too much args, the most args is 2\n");

		return -1;
	}
	if(argc == 2)
	{
		addr = (uint)simple_strtoul(argv[1], NULL, 16);
		if(addr < 0x40000000)
		{
			addr = 0x40007800;
		}
	}
	else
	{
		addr = 0x40007800;
	}
	sprintf(load_addr, "%x", addr);

	ret = do_sunxi_flash(0, 0, 4, flash_argv);
	if(ret<0)
	{
		printf("something bad in recovery function\n");
	}
	do_boota(0, 0, 2, boota_argv);

	return -1;
}


U_BOOT_CMD(
	recovery,	2,	1,	do_recovery,
	"sunxi recovery function",
	"[parmeters 0]: option, the address to load recovery img\n"
	"NULL\n"
);

