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
#include <asm/io.h>
#include <pmu.h>
#include <asm/arch/timer.h>
#include <asm/arch/key.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>
/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nannd flash, mmc2.
 * We check where we boot from by checking the config
 * of the gpio pin.
 */
DECLARE_GLOBAL_DATA_PTR;

u32 get_base(void)
{

	u32 val;

	__asm__ __volatile__("mov %0, pc \n":"=r"(val)::"memory");
	val &= 0xF0000000;
	val >>= 28;
	return val;
}

/* do some early init */
void s_init(void)
{
	watchdog_disable();
}

void reset_cpu(ulong addr)
{
	watchdog_enable();
#ifndef CONFIG_A50_FPGA
loop_to_die:
	goto loop_to_die;
#endif
}

void v7_outer_cache_enable(void)
{
	return ;
}

void v7_outer_cache_inval_all(void)
{
	return ;
}

void v7_outer_cache_flush_range(u32 start, u32 stop)
{
	return ;
}

void enable_caches(void)
{
    icache_enable();
    dcache_enable();
}

void disable_caches(void)
{
    icache_disable();
	dcache_disable();
}

int display_inner(void)
{
	tick_printf("version: %s\n", uboot_spare_head.boot_head.version);

	return 0;
}

int script_init(void)
{
    uint offset, length;
	char *addr;

	offset = uboot_spare_head.boot_head.uboot_length;
	length = uboot_spare_head.boot_head.length - uboot_spare_head.boot_head.uboot_length;
	addr   = (char *)CONFIG_SYS_TEXT_BASE + offset;

    debug("script offset=%x, length = %x\n", offset, length);

	if(length)
	{
		memcpy((void *)SYS_CONFIG_MEMBASE, addr, length);
		script_parser_init((char *)SYS_CONFIG_MEMBASE);
	}
	else
	{
		script_parser_init(NULL);
	}
#if defined(CONFIG_SUNXI_SCRIPT_REINIT)
	{
		void *tmp_target_buffer = (void *)(CONFIG_SYS_TEXT_BASE - 0x01000000);

		memset(tmp_target_buffer, 0, 1024 * 1024);
		memcpy(tmp_target_buffer, (void *)CONFIG_SYS_TEXT_BASE, uboot_spare_head.boot_head.length);
	}
#endif
	return 0;
}

int power_source_init(void)
{
	int pll1;
	int dcdc3_vol;

	if(script_parser_fetch("power_sply", "dcdc3_vol", &dcdc3_vol, 1))
	{
		dcdc3_vol = 1200;
	}
	if(axp_probe() > 0)
	{
		axp_probe_factory_mode();
		if(!axp_probe_power_supply_condition())
		{
			if(!axp_set_supply_status(0, PMU_SUPPLY_DCDC3, dcdc3_vol, -1))
			{
				tick_printf("PMU: dcdc3 %d\n", dcdc3_vol);
				sunxi_clock_set_corepll(uboot_spare_head.boot_data.run_clock, 0);
			}
			else
			{
				printf("axp_set_dcdc3 fail\n");
			}
		}
		else
		{
			printf("axp_probe_power_supply_condition error\n");
		}
	}
	else
	{
		printf("axp_probe error\n");
	}

	pll1 = sunxi_clock_get_corepll();

	tick_printf("PMU: pll1 %d Mhz\n", pll1);

    axp_set_charge_vol_limit();
    axp_set_all_limit();
    axp_set_hardware_poweron_vol();

	axp_set_power_supply_output();

	power_limit_init();

	return 0;
}



