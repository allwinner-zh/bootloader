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
#include <asm/arch/cpu.h>
#include <asm/arch/key.h>
#include <asm/arch/sys_proto.h>
#include <pmu.h>
#include <sys_config.h>
#include <smc.h>

int sunxi_key_init(void)
{
    uint reg_val;

	reg_val = smc_readl(SUNXI_KEY_ADC_CRTL);
    reg_val &= ~((7<<1) | (0xffU << 24));
	reg_val |=  LRADC_HOLD_EN;
	reg_val |=  LRADC_EN;
	smc_writel(reg_val, SUNXI_KEY_ADC_CRTL);

	/* disable all key irq */
	smc_writel(0, SUNXI_KEY_ADC_INTC);
	smc_writel(0x1f1f, SUNXI_KEY_ADC_INTS);

	return 0;
}

int sunxi_key_exit(void)
{
    smc_writel(0, SUNXI_KEY_ADC_CRTL);
	/* disable all key irq */
	smc_writel(0, SUNXI_KEY_ADC_INTC);
	smc_writel(0x1f1f, SUNXI_KEY_ADC_INTS);

	return 0;
}


int sunxi_key_read(void)
{
	u32 ints;
	int key = -1;
    int keyen_flag = 1;

	if( !script_parser_fetch("key_detect_en","keyen_flag",&keyen_flag,1) )
	{
		if(!keyen_flag)
			return -1;
	}

	ints = smc_readl(SUNXI_KEY_ADC_INTS);
	/* clear the pending data */
	smc_writel(ints & 0x1f, SUNXI_KEY_ADC_INTS);
	/* if there is already data pending,
	 read it */
	if( ints & ADC0_KEYDOWN_PENDING)
	{
		if(ints & ADC0_DATA_PENDING)
		{
			key = smc_readl(SUNXI_KEY_ADC_DATA0) & 0x3f;
			if(!key)
			{
				key = -1;
			}
		}
	}
	else if(ints & ADC0_DATA_PENDING)
	{
		key = smc_readl(SUNXI_KEY_ADC_DATA0) & 0x3f;
		if(!key)
		{
			key = -1;
		}
	}
//#ifdef DEBUG
	if(key > 0)
		printf("key pressed value=0x%x\n", key);
//#endif

	return key;
}

int do_key_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct sunxi_lradc *sunxi_key_base = (struct sunxi_lradc *)SUNXI_LRADC_BASE;
	int power_key;

	puts("press a key:\n");
	sunxi_key_base->ints = 0x1f1f;

    while(!ctrlc())
    {
		sunxi_key_read();
		power_key = axp_probe_key();
		if(power_key > 0)
		{
			break;
		}
	}

	return 0;

}

U_BOOT_CMD(
	key_test, 1, 0,	do_key_test,
	"Test the key value\n",
	""
);
