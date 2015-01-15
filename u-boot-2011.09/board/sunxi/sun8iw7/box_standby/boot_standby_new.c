/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * charles <yanjianbo@allwinnertech.com>
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
#include <sys_config.h>
#include <asm/arch/timer.h>
#include <asm/armv7.h>
#include <asm/arch/cpu.h>
#include <asm/arch/platform.h>
#include <power.h>
#include <smc.h>

DECLARE_GLOBAL_DATA_PTR;

// standby led
#define STANDBY_LED_CONT		(2)
#define STANDBY_WAIT_LED		(0)
#define STANDBY_WORK_LED		(1)
#define LED_CLOSE				(0)
#define LED_OPEN				(1)

// system start mode
#define MODE_NULL				(0x0)
#define MODE_SHUTDOWN_OS		(0x1)
#define MODE_WAIT_WAKE_UP		(0x2)
#define MODE_RUN_OS				(0xf)

// run addr
#define SCRIPT_ADDR				(CONFIG_SYS_SDRAM_BASE + 0x04000000)
#define BOOT_STANDBY_RUN_ADDR	(0x00040000)

extern void boot_standby_new_relocate(int addr, int reloc_off);
extern void sunxi_flush_allcaches(void);
extern int disable_interrupts(void);

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
static int read_start_mode(void)
{
	int mode = 0;
    mode =  smc_readl(RTC_GENERAL_PURPOSE_REG(3));
	return mode;
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
static void write_start_mode(int mode)
{
	unsigned char value = mode & 0xff;

	smc_writel(value, RTC_GENERAL_PURPOSE_REG(3));
	return ;
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
*    说明    ： index   0 :  待机灯		1：工作灯
*								status	0：	 输出低电平	1：输出高电平
*
************************************************************************************************************
*/
void standby_led_control(int index, int status)
{
	int ret;
	user_gpio_set_t	gpio_init;
	
	switch (index) {
		case STANDBY_WAIT_LED:
			ret = script_parser_fetch("boot_standby", "standby_led", (void *)&gpio_init, sizeof(gpio_init) / 4);
			if (!ret) {
				gpio_init.data = status & 0x1;
				gpio_request(&gpio_init, 1);
			}
			break;
		case STANDBY_WORK_LED:
			ret = script_parser_fetch("boot_standby", "work_led", (void *)&gpio_init, sizeof(gpio_init) / 4);
			if (!ret) {
				gpio_init.data = status & 0x1;
				gpio_request(&gpio_init, 1);
			}
			break;
		default: ;
	}
}

static void copy_script_to_sram(int addr)
{
	unsigned int length;

	length = uboot_spare_head.boot_head.length - uboot_spare_head.boot_head.uboot_length;
	
	printf("[box standby] script addr=0x%x, length = 0x%x\n", addr, length);
	
	if (length)
	{
		memcpy((void *)addr, (void *)SYS_CONFIG_MEMBASE, length);
	}
	else
	{
		printf("error: script's length is 0\n");
	}
	
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
static void init_cpus(void)
{
	//放开cpus reset位
	volatile unsigned long value;

	printf("[box standby] init_cpus\n");

	value = smc_readl(0x01F01C00 + 0x0); //R_CPUS_CFG_BASE
	value &= ~1;
	smc_writel(value, 0x01F01C00 + 0x0);
	value = smc_readl(0x01F01C00 + 0x0);
	value |= 1;
	smc_writel(value, 0x01F01C00 + 0x0);
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
static int aw_suspend_cpu_die(void)
{
	unsigned long actlr;
	
	printf("[box standby] CPU0 go to WFI\n");
	/* step1: disable cache */
	asm("mrc    p15, 0, %0, c1, c0, 0" : "=r" (actlr) );
	actlr &= ~(1<<2);
	asm("mcr    p15, 0, %0, c1, c0, 0\n" : : "r" (actlr));

	/* step2: clean and ivalidate L1 cache */
	//sunxi_flush_allcaches();

	/* step3: execute a CLREX instruction */
	asm("clrex" : : : "memory", "cc");

	/* step4: switch cpu from SMP mode to AMP mode, aim is to disable cache coherency */
	asm("mrc    p15, 0, %0, c1, c0, 1" : "=r" (actlr) );
	actlr &= ~(1<<6);
	asm("mcr    p15, 0, %0, c1, c0, 1\n" : : "r" (actlr));

	/* step5: execute an ISB instruction */
	CP15ISB;
	/* step6: execute a DSB instruction  */
	CP15DSB;

	/* step7: execute a WFI instruction */
	asm("wfi" : : : "memory", "cc");
	
	return 0;
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
static void clear_bss_for_boot_standby(int addr, int length)
{
	memset((void *)addr, 0x0, length);
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
static void do_no_thing_loop(void)
{
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
*    说明    ：rtc = 1		android通过设置RTC = 1，然后重启系统，通过进入standby等待唤醒
*							 rtc = f		关机后，下次开机直接进入系统
*							 
************************************************************************************************************
*/
void do_box_standby(void)
{
	int mode;
	int used;
	int start_type;
	int ret;
	int standby_status = 0;
	
	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
	{
		return ;
	}
	
	mode = read_start_mode();
	printf("[box standby] read rtc = 0x%x\n", mode);	
			
	ret = script_parser_fetch("box_start_os", "used", &used, sizeof(int) / 4);
	if (ret || !used)
	{
		printf("[box_start_os] mag be no use\n");
		return ;
	}
	ret = script_parser_fetch("box_start_os", "start_type", &start_type, sizeof(int) / 4);
	if (ret || start_type)
	{
		// 直接开机情况下，当检测到RTC = 1，无条件进入standby模式，否则启动系统
		if (mode != MODE_SHUTDOWN_OS)
		{
			printf("[box_start_os] mag be start_type no use\n");
			return ;
		}
	}
	if (mode == MODE_SHUTDOWN_OS)
	{
		printf("[box standby] go to standby and wake up waiting ir\n");

		write_start_mode(MODE_NULL);			//清除工作模式
		
#if (defined STANDBY_LED_CONT == 2)
		standby_led_control(STANDBY_WAIT_LED, LED_OPEN);
		standby_led_control(STANDBY_WORK_LED, LED_CLOSE);
#else
		standby_led_control(STANDBY_WAIT_LED, LED_OPEN);
#endif

		standby_status = 1;
	}
	else if (mode == MODE_RUN_OS)
	{
		printf("[box standby] start os\n");
		write_start_mode(MODE_NULL);
		
#if (defined STANDBY_LED_CONT == 2)
		standby_led_control(STANDBY_WAIT_LED, LED_CLOSE);
		standby_led_control(STANDBY_WORK_LED, LED_OPEN);
#else
		standby_led_control(STANDBY_WAIT_LED, LED_OPEN);
#endif
		return ;
	}
	else
	{
		printf("[box standby] first start, so go to standby\n");
		
		//write_start_mode(MODE_WAIT_WAKE_UP);
		
#if (defined STANDBY_LED_CONT == 2)
		standby_led_control(STANDBY_WAIT_LED, LED_OPEN);
		standby_led_control(STANDBY_WORK_LED, LED_CLOSE);
#else
		standby_led_control(STANDBY_WAIT_LED, LED_OPEN);
#endif

		standby_status = 1;
	}
	
	if (standby_status)
	{
		disable_interrupts();
		clear_bss_for_boot_standby(BOOT_STANDBY_RUN_ADDR, SRAM_A2_SIZE);
		printf("BOOT_STANDBY_RUN_ADDR:0x%x\n", BOOT_STANDBY_RUN_ADDR);
		printf("SRAM_A2_SIZE:0x%x\n", SRAM_A2_SIZE);
		printf("SCRIPT_ADDR:0x%x\n", SCRIPT_ADDR);
		copy_script_to_sram(SCRIPT_ADDR);
		printf("BOOT_STANDBY_RUN_ADDR:0x%x\n", BOOT_STANDBY_RUN_ADDR);
		boot_standby_new_relocate(BOOT_STANDBY_RUN_ADDR, gd->reloc_off);
		sunxi_flush_allcaches();
		init_cpus();
		aw_suspend_cpu_die();
	}

	while (1) {
		do_no_thing_loop();
	}
}
