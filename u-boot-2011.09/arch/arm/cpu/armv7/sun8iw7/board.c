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
#include <asm/armv7.h>
#include <asm/io.h>
#include <pmu.h>
#include <asm/arch/timer.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/key.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/efuse.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <smc.h>
/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nannd flash, mmc2.
 * We check where we boot from by checking the config
 * of the gpio pin.
 */
DECLARE_GLOBAL_DATA_PTR;

extern int sunxi_clock_get_axi(void);
extern int sunxi_clock_get_ahb(void);
extern int sunxi_clock_get_apb1(void);
extern int sunxi_clock_get_pll6(void);


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
#ifndef CONFIG_FPGA
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

void sunxi_set_cpux_voltage(void);

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
        sunxi_set_cpux_voltage();
		sunxi_clock_set_corepll(uboot_spare_head.boot_data.run_clock, 0);
	}

	pll1 = sunxi_clock_get_corepll();

	tick_printf("PMU: pll1 %d Mhz,PLL6=%d Mhz\n", pll1,sunxi_clock_get_pll6());
    printf("AXI=%d Mhz,AHB=%d Mhz, APB1=%d Mhz \n", sunxi_clock_get_axi(),sunxi_clock_get_ahb(),sunxi_clock_get_apb1());

#ifdef CONFIG_SUNXI_AXP_MAIN
    axp_set_charge_vol_limit();
    axp_set_all_limit();
    axp_set_hardware_poweron_vol();

	axp_set_power_supply_output();

	power_limit_init();
#endif
//read chipid and thermal sensor
    sid_read();

    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          : power_off
*
*    parmeters     :
*
*    return        :
*
*    note          : power off system
*
*
************************************************************************************************************
*/
int power_off(void)
{
	volatile unsigned int reg_val;
	//set PL05 musel output
	reg_val = readl(R_PIO_BASE + 0x00);
	reg_val &= ~(0xF << 20);
	reg_val |= (0x1 << 20);
	writel(reg_val, R_PIO_BASE + 0x00);

	//set PL05 low to set vcc-io power off
	reg_val = readl(R_PIO_BASE + 0x10);
	reg_val |= (0x1 << 5);
	writel(reg_val, R_PIO_BASE + 0x10);

	//set PL08,PL09 musel output
	reg_val = readl(R_PIO_BASE + 0x04);
	reg_val &= ~(0xFF);
	reg_val |= 0x11;
	writel(reg_val, R_PIO_BASE + 0x04);

	//set PL08,PL09 low to set  power off
	reg_val = readl(R_PIO_BASE + 0x10);
	reg_val &= ~(0x3 << 8);
	writel(reg_val, R_PIO_BASE + 0x10);
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void sunxi_set_fel_flag(void)
{
	writel(SUNXI_RUN_EFEX_FLAG, RTC_GENERAL_PURPOSE_REG(2));
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void sunxi_clear_fel_flag(void)
{
	writel(0, RTC_GENERAL_PURPOSE_REG(0));
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
#ifdef CONFIG_SUNXI_SECURE_SYSTEM
int sunxi_probe_securemode(void)
{
	uint reg_val;

	writel(0xffff, CONFIG_SYS_SRAMA2_BASE);
	reg_val = readl(CONFIG_SYS_SRAMA2_BASE);
	if(!reg_val)  //读到数据全是0，那么只能是使能secure的normal模式
	{
		if(uboot_spare_head.boot_data.secureos_exist == 1)	//如果是1，由sbromsw传递，表示存在安全系统，否则没有
		{
			gd->securemode = SUNXI_SECURE_MODE_WITH_SECUREOS;
			printf("normal mode: with secureos\n");
		}
		else
		{
			gd->securemode = SUNXI_SECURE_MODE_NO_SECUREOS;		//不存在安全系统
			printf("normal mode: no secureos\n");
		}
	}
	else		 //读到数据非0，那么只能是未使能secure
	{
            if(uboot_spare_head.boot_data.secureos_exist == 0 )
            {
                printf("SUNXI_SECURE_MODE  \n");
                gd->securemode = SUNXI_SECURE_MODE;
            }
            else
            {
                printf("SUNXI_NORMAL_MODE   \n");
		gd->securemode = SUNXI_NORMAL_MODE;
            }
	    printf("already secure mode\n");
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_set_secure_mode(void)
{
	int mode;
    int secure_bit = 0;
	if(gd->securemode == SUNXI_NORMAL_MODE)
	{
		printf("normal mode\n");
        if(!script_parser_fetch("platform","secure_bit",&secure_bit,1))
        {
            if(secure_bit == 1)
            {
                printf("/*===ready to burn secure bit==== */\n");
                gd->securemode = SUNXI_SECURE_MODE_NO_SECUREOS;
				mode = sid_probe_security_mode();
				if(!mode)
				{
					sid_set_security_mode();
				}
                printf("burn secure bit success \n");
            }
        }
	}

	return 0;
}
int sunxi_get_securemode(void)
{
	return gd->securemode;
}
#endif

#define VOL2REG(v) (u8)(((v - 680) / 10) | 0x80)
#define PMU_ADDR 		0x65
#define VOL_REG_ADDR	0x01
extern int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);
void sunxi_set_cpux_voltage_by_i2c(uint volt)
{
	u8 change = VOL2REG(volt);
	debug("=== volt %d = %x ===\n",volt,change);
	if(i2c_write(PMU_ADDR, VOL_REG_ADDR, 1, (uchar *)&change, 1))
	{
		printf("sunxi_set_cpux_voltage_by_i2c error \n");
	}
	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :sunxi_set_cpux_voltage
*
*    parmeters     :void
*
*    return        :void
*
*    note          :
*
*
************************************************************************************************************
*/
void sunxi_set_cpux_voltage(void)
{
	uint max_freq = 0;
	uint min_freq = 0;
	uint lel_num = 0;
	uint volt = 0;
    char freq_lel[16];
    char volt_lel[16];
	uint pmuic_type = 0;
	uint freq = 0;
	uint i = 0;
	uint set_clock = uboot_spare_head.boot_data.run_clock * 1000000;

    memset(freq_lel, 0x00,16);
    memset(volt_lel , 0x00,16);

//get max_freq
	if(script_parser_fetch("dvfs_table", "max_freq", (int*)&max_freq, 1))
	{
		printf("sunxi_set_cpux_voltage error : can not find max_freq from dvfs_table  \n");
		return ;
	}
	debug("max_freq is %d \n",max_freq);
	if(script_parser_fetch("dvfs_table", "min_freq", (int*)&min_freq, 1))
	{
		printf("sunxi_set_cpux_voltage error : can not find min_freq from dvfs_table  \n");
		return ;
	}
	debug("min_freq is %d \n",min_freq);
	
	if(max_freq < set_clock)
		set_clock = max_freq;
	if(min_freq > set_clock)
		set_clock = min_freq;

	uboot_spare_head.boot_data.run_clock  = set_clock / 1000000;
//compare sys_config bootclock
   if(script_parser_fetch("dvfs_table","LV_count",(int*)&lel_num,sizeof(int)/4))
   {
           printf("can not find LV_count from script \n");
           return ;
   }
   debug("lel_num is %d\n",lel_num);
   for(i = lel_num; i >= 0;i--)
   {
        sprintf(freq_lel,"%s%d%s","LV",i,"_freq");
        if(script_parser_fetch("dvfs_table",freq_lel,(int*)&freq,sizeof(int)/4)) 
        {
                printf("can not find B_LV_count from script \n");
                return ;
        }
        debug("==== %s = %d ====\n",freq_lel,freq);

        if(set_clock <= freq)
        {
             sprintf(volt_lel,"%s%d%s","LV",i,"_volt");
             if(script_parser_fetch("dvfs_table",volt_lel,(int*)&volt,sizeof(int)/4))
             {
                     printf("can not find volt_lel from script \n");
                     return ;
             }
             debug("find volt_lel = %d \n",volt);
			 break;
        }
   }

//get pmu_type
   if(script_parser_fetch("dvfs_table","pmuic_type",(int*)&pmuic_type,sizeof(int)/4))
   {
		printf("can not find pmuic_type form script  \n");
		return ;
   }

//type 8016 i2c or gpio or none
	if(pmuic_type == 2)
	{
		debug("I2C type \n");
		sunxi_set_cpux_voltage_by_i2c(volt);	
	}
	else if(pmuic_type == 1)
	{
		debug("gpio type \n");
		gpio_request_simple("dvfs_table",NULL);
	}
	return ;
}
