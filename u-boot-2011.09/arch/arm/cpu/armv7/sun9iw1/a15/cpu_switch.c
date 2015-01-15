/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Young <guoyingyang@allwinnertech.com>
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
#include <sys_config.h>
#include <asm/arch/timer.h>
#include <asm/arch/gpio.h>
#include <asm/arch/cpu_switch.h>

/* you can get current run_cpu idnum
	cluster0 : 0,1,2,3
	cluster1 : 4,5,6,7
	*/
/*
int get_core_id(void)
{
	return ((get_cluster_id() * 4) + get_cpu_id());
}
*/
unsigned static int sunxi_chip_rev(void)
{
	/* sun9iw1p1 chip revsion init */
	if ((readl(RPRCM_BASE + 0x190) >> 0x3) & 0x1) {
		return   SUN9I_REV_B;
	} else {
		return  SUN9I_REV_A;
	}
}

static int sun9i_ca15_power_switch_set(unsigned int cluster, unsigned int cpu, u32 enable)
{
	if (sunxi_chip_rev() >= SUN9I_REV_B) {
		if (enable) {
			if (0x00 == readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				printf("%s: power switch enable already\n", __func__);
				return 0;
			}
			/* de-active cpu power clamp */
			writel(0xFE, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(20);

			writel(0xF8, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(10);

			writel(0xE0, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(10);

			writel(0x80, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(10);

			writel(0x00, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(20);
			while(0x00 != readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				;
			}
		} else {
			if (0xFF == readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				printf("%s: power switch disable already\n", __func__);
				return 0;
			}
			writel(0xFF, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(30);
			while(0xFF != readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				;
			}
		}
	} else {
		if (enable) {
			if (0xFF == readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				printf("%s: power switch enable already\n", __func__);
				return 0;
			}
			writel(0x01, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(20);

			writel(0x07, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(10);

			writel(0x1F, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(10);

			writel(0x7F, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(10);

			writel(0xFF, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(20);
			while(0xFF != readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				;
			}
		} else {
			if (0x00 == readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				printf("%s: power switch disable already\n", __func__);
				return 0;
			}
			writel(0x00, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
			__usdelay(30);
			while(0x00 != readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
				;
			}
		}
	}
	return 0;
}


static int sun9i_ca7_power_switch_set(unsigned int cluster, unsigned int cpu, u32 enable)
{
	if (enable) {
		if (0x00 == readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			printf("%s: power switch enable already\n", __func__);
			return 0;
		}
		/* de-active cpu power clamp */
		writel(0xFE, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		__usdelay(20);

		writel(0xF8, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		__usdelay(10);

		writel(0xE0, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		__usdelay(10);

		writel(0x80, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		__usdelay(10);

		writel(0x00, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		__usdelay(20);
		while(0x00 != readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			;
		}
	} else {
		if (0xFF == readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			printf("%s: power switch disable already\n", __func__);
			return 0;
		}
		writel(0xFF, RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		__usdelay(30);
		while(0xFF != readl(RPRCM_BASE + SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			;
		}
	}
	return 0;
}

static int sun9i_cpu_power_switch_set(unsigned int cluster, unsigned int cpu, u32 enable)
{
	int ret;
	if (cluster == A15_CLUSTER) {
		ret = sun9i_ca15_power_switch_set(cluster, cpu, enable);
	} else {
		ret = sun9i_ca7_power_switch_set(cluster, cpu, enable);
	}
	return ret;
}

int sun9i_cpu_power_set(unsigned int cluster, unsigned int cpu, u32 enable)
{
	unsigned int value;

	if (enable) {
		/*
		 * power-up cpu core process
		 */
		printf("sun9i power-up cluster-%d cpu-%d\n", cluster, cpu);

		/* if boot cpu, should enable boot cpu hotplug first.*/
		//if (SUN9I_IS_BOOT_CPU(cluster, cpu)) {
		//	sun9i_boot_cpu_hotplug_enable(1);
		//}

		/* assert cpu core reset */
		value  = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(1<<cpu));
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		__usdelay(10);

		/* assert cpu power-on reset */
		value  = readl(RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value &= (~(1<<cpu));
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		__usdelay(10);

		/* L1RSTDISABLE hold low */
		if (cluster == A7_CLUSTER) {
			/* L1RSTDISABLE control bit just use for A7_CLUSTER,
			 * the A15_CLUSTER default reset by hardware when power-up,
			 * software can't control it.
			 */
			value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
			value &= ~(1<<cpu);
			writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
		}

		/* release power switch */
		sun9i_cpu_power_switch_set(cluster, cpu, 1);

		/* clear power-off gating */
		value = readl(RPRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		value &= (~(0x1<<cpu));
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		__usdelay(20);

		/* de-assert cpu power-on reset */
		value  = readl(RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value |= ((1<<cpu));
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		__usdelay(10);

		/* de-assert core reset */
		value  = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value |= (1<<cpu);
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		__usdelay(10);

		printf("sun9i power-up cluster-%d cpu-%d already\n", cluster, cpu);
	} else {
		/*
		 * power-down cpu core process
		 */
		printf("sun9i power-down cluster-%d cpu-%d\n", cluster, cpu);

		/* enable cpu power-off gating */
		value = readl(RPRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		value |= (1 << cpu);
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		__usdelay(20);

		/* active the power output switch */
		sun9i_cpu_power_switch_set(cluster, cpu, 0);

		/* if boot cpu, should disable boot cpu hotplug.*/
		//if (SUN9I_IS_BOOT_CPU(cluster, cpu)) {
		//	sun9i_boot_cpu_hotplug_enable(0);
		//}
	//	printf("sun9i power-down cpu%d ok.\n", cpu);
	}
	return 0;
}


int sun9i_cluster_power_set(unsigned int cluster, u32 enable)
{
	unsigned int value;
	int          i;

	if (enable) {
		printf("sun9i power-up cluster-%d\n", cluster);

		/* active ACINACTM */
		value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));
		value |= (1<<0);
		writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));

		/* assert cluster cores resets */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(0xF<<0));   /* Core Reset    */
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		__usdelay(10);

		/* assert cluster cores power-on reset */
		value = readl(RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value &= (~(0xF));
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		__usdelay(10);

		/* assert cluster resets */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(0x1<<24));  /* SOC DBG Reset */
		value &= (~(0xF<<16));  /* Debug Reset   */
		value &= (~(0x1<<12));  /* HReset        */
		value &= (~(0x1<<8));   /* L2 Cache Reset*/
		if (cluster == A7_CLUSTER) {
			value &= (~(0xF<<20));  /* ETM Reset     */
		} else {
			value &= (~(0xF<<4));   /* Neon Reset   */
		}
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		__usdelay(10);

		/* Set L2RSTDISABLE LOW */
		if (cluster == A7_CLUSTER) {
			value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
			value &= (~(0x1<<4));
			writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
		} else {
			value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
			value &= (~(0x1<<0));
			writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL0(cluster));
		}

		__usdelay(1000);

		/* clear cluster power-off gating */
		value = readl(RPRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		value &= (~(0x1<<4));
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		__usdelay(20);

		/* de-active ACINACTM */
		value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));
		value &= (~(1<<0));
		writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));

		/* de-assert cores reset */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value |= (0x1<<24);  /* SOC DBG Reset */
		value |= (0xF<<16);  /* Debug Reset   */
		value |= (0x1<<12);  /* HReset        */
		value |= (0x1<<8);   /* L2 Cache Reset*/
		if (cluster == A7_CLUSTER) {
			value |= (0xF<<20);  /* ETM Reset     */
		} else {
			value |= (0xF<<4);   /* Neon Reset   */
		}
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
                __usdelay(20);

		/* de-assert cores power-on reset */
		value = readl(RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value |= (0xF);
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		__usdelay(60);

		/* de-assert cores reset */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value |= (0xF<<0);   /* Core Reset    */
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
                __usdelay(20);

		printf("sun9i power-up cluster-%d ok\n", cluster);

	} else {

		printf("sun9i power-down cluster-%d\n", cluster);

		/* active ACINACTM */
		value = readl(CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));
		value |= (1<<0);
		writel(value, CPUCFG_BASE + SUNXI_CLUSTER_CTRL1(cluster));

		/* assert cluster cores resets */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(0xF<<0));   /* Core Reset    */
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		__usdelay(10);

		/* assert cluster cores power-on reset */
		value = readl(RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		value &= (~(0xF));
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWRON_RESET(cluster));
		__usdelay(10);

		/* assert cluster resets */
		value = readl(CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		value &= (~(0x1<<24));  /* SOC DBG Reset */
		value &= (~(0xF<<16));  /* Debug Reset   */
		value &= (~(0x1<<12));  /* HReset        */
		value &= (~(0x1<<8));   /* L2 Cache Reset*/
		if (cluster == A7_CLUSTER) {
			value &= (~(0xF<<20));  /* ETM Reset     */
		} else {
			value &= (~(0xF<<4));   /* Neon Reset   */
		}
		writel(value, CPUCFG_BASE + SUNXI_CPU_RST_CTRL(cluster));
		__usdelay(10);

		/* enable cluster and cores power-off gating */
		value = readl(RPRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		value |= (1<<4);
		value |= (0xF<<0);
		writel(value, RPRCM_BASE + SUNXI_CLUSTER_PWROFF_GATING(cluster));
		__usdelay(20);

		/* disable cluster cores power switch */
		for (i = 0; i < 4; i++) {
			sun9i_cpu_power_switch_set(cluster, i, 0);
		}
		printf("sun9i power-down cluster-%d ok\n", cluster);
	}

	return 0;
}

#if 1
static int a15_power_onoff(special_gpio_cfg a15_power_gpio)
{
	normal_gpio_set_t pwr_en_gpio;
	if(a15_power_gpio.mul_sel == 1)
	{
		printf("enbale a15 power\n");
		pwr_en_gpio.port = a15_power_gpio.port;
		pwr_en_gpio.port_num = a15_power_gpio.port_num;
		pwr_en_gpio.mul_sel = a15_power_gpio.mul_sel;
		pwr_en_gpio.pull = 0xff;
		pwr_en_gpio.drv_level = 0xff;
		if(a15_power_gpio.data != 0)
		{
			pwr_en_gpio.data = 1;
		}
		else
		{
			pwr_en_gpio.data = 0;
		}

		if(boot_set_one_gpio(&pwr_en_gpio, 1))
		{
			printf("a15 external power enabel failed\n");
			return -1;
		}
		__msdelay(2);
		return 0;
	}
	return 0;
}

void switch_to_a15(special_gpio_cfg a15_power_gpio)
{
	u32 addr = 0;
	u32 ret = 0;
    printf("get cpu id  \n");
   // while(*(volatile uint *)0 != 0x0123);
	//ret = get_cpu_id();
	//asm volatile("blx %0"::"r" (get_cpu_id));
	//asm volatile("mov %0, r0 ":"=r" (ret):);

	//printf("before  cpu id is %d \n",ret);
	//save_runtime_context((u32 *)0x08100000);
	a15_power_onoff(a15_power_gpio);

	asm volatile("mov r0, #0x08100000 "::);
	asm volatile("blx %0"::"r" (save_runtime_context));
	printf("save_runtime \n");
	/* map brom address to 0x0 */
	//__usdelay(1000);
	//ret = get_cpu_id();
	asm volatile("blx %0"::"r" (get_cpu_id));
	asm volatile("mov %0, r0 ":"=r" (ret):);
	printf("core id is %d \n",ret);
	if(ret != 4)
	{
		/*resumed cpu can go to this addr if you set func addr to this reg*/
		addr = (u32)jump_to_resume;
		writel(addr, R_CPU_SOFT_ENTRY_REG);
		sun9i_cluster_power_set(1,1);
		sun9i_cpu_power_set(1, 0, 1);
		asm("wfi": : : "memory","cc");
	}
	else if(ret == 4)
	{
		sun9i_cluster_power_set(0,0);
        sun9i_cpu_power_set(0, 0, 0);
	}
}
#endif
