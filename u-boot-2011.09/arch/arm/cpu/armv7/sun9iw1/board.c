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
#include <asm/arch/clock.h>
#include <asm/arch/efuse.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <smc.h>
#include <asm/arch/cpu_switch.h>
/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nannd flash, mmc2.
 * We check where we boot from by checking the config
 * of the gpio pin.
 */
DECLARE_GLOBAL_DATA_PTR;

static void config_cpu_map(void);
static int  sunxi_scan_VF_table(uint table_num, uint boot_clock);
static int  sunxi_get_soc_bin(void);

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

	return 0;
}

struct bias_set
{
	int  vol;
	int  index;
};

int power_config_gpio_bias(void)
{
	char gpio_bias[32], gpio_name[32];
	char *gpio_name_const="pa_bias";
	char port_index;
	char *axp=NULL, *supply=NULL, *vol=NULL;
	uint main_hd;
	uint bias_vol_set;
	int  index, ret, i;
	uint port_bias_addr;
	uint vol_index, config_type;
	int  pmu_vol;
	struct bias_set bias_vol_config[8] =
		{ {1800, 0}, {2500, 6}, {2800, 9}, {3000, 0xa}, {3300, 0xd}, {0, 0} };

	main_hd = script_parser_fetch_subkey_start("gpio_bias");

	index = 0;
	while(1)
	{
		memset(gpio_bias, 0, 32);
		memset(gpio_name, 0, 32);
		ret = script_parser_fetch_subkey_next(main_hd, gpio_name, (int *)gpio_bias, &index);
		if(!ret)
		{
			lower(gpio_name);
			lower(gpio_bias);

			port_index = gpio_name[1];
			gpio_name[1] = 'a';
			if(strcmp(gpio_name_const, gpio_name))
			{
				printf("invalid gpio bias name %s\n", gpio_name);

				continue;
			}
			gpio_name[1] = port_index;
			i=0;
			axp = gpio_bias;
			while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
			{
				i++;
			}
			gpio_bias[i++]='\0';

			if(!strcmp(axp, "constant"))
			{
				config_type = 1;
			}
			else if(!strcmp(axp, "floating"))
			{
				printf("ignore %s bias config\n", gpio_name);

				continue;
			}
			else
			{
				config_type = 0;
			}

			if(config_type == 0)
			{
				supply = gpio_bias + i;
				while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
				{
					i++;
				}
				gpio_bias[i++]='\0';
			}

			printf("supply=%s\n", supply);
			vol = gpio_bias + i;
			while( (gpio_bias[i]!=':') && (gpio_bias[i]!='\0') )
			{
				i++;
			}

			bias_vol_set = simple_strtoul(vol, NULL, 10);
			for(i=0;i<5;i++)
			{
				if(bias_vol_config[i].vol == bias_vol_set)
				{
					break;
				}
			}
			if(i==5)
			{
				printf("invalid gpio bias set vol %d, at name %s\n", bias_vol_set, gpio_name);

				break;
			}
			vol_index = bias_vol_config[i].index;

			if((port_index >= 'a') && (port_index <= 'h'))
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_PIO_BASE + 0x300 + 0x4 * (port_index - 'a');
			}
			else if(port_index == 'j')
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_PIO_BASE + 0x300 + 0x4 * (port_index - 'a');
			}
			else if((port_index == 'l') || (port_index == 'm'))
			{
				//获取寄存器地址
				port_bias_addr = SUNXI_R_PIO_BASE + 0x300 + 0x4 * (port_index - 'l');
			}
			else
			{
				printf("invalid gpio port at name %s\n", gpio_name);

				continue;
			}
			printf("axp=%s, supply=%s, vol=%d\n", axp, supply, bias_vol_set);
			if(config_type == 1)
			{
				smc_writel(vol_index, port_bias_addr);
			}
			else
			{
				pmu_vol = axp_probe_supply_status_byname(axp, supply);
				if(pmu_vol < 0)
				{
					printf("sunxi board read %s %s failed\n", axp, supply);

					continue;
				}

				if(pmu_vol > bias_vol_set)	//pmu实际电压超过需要设置的电压
				{
					//电压降低到需要电压
					axp_set_supply_status_byname(axp, supply, bias_vol_set, 1);
					//设置寄存器
					smc_writel(vol_index, port_bias_addr);
				}
				else if(pmu_vol < bias_vol_set)	//pmu实际电压低于需要设置的电压
				{
					//设置寄存器
					smc_writel(vol_index, port_bias_addr);
					//把pmu电压调整到需要的电压
					axp_set_supply_status_byname(axp, supply, bias_vol_set, 1);
				}
				else
				{
					//如果实际电压等于需要设置电压，直接设置即可
					smc_writel(vol_index, port_bias_addr);
				}
			}
			printf("reg addr=0x%x, value=0x%x, pmu_vol=%d\n", port_bias_addr, smc_readl(port_bias_addr), bias_vol_set);
		}
		else
		{
			printf("config gpio bias voltage finish\n");

			break;
		}
	}

	return 0;
}

static void __disable_unused_mode(void)
{
	writel(1, 0x01C08014);
	writel(0, 0x01C08010);
	writel(1, 0x01C08014);
}

int check_a15_flag(void);
void get_boot_cpu_flag(void);


int power_source_init(void)
{
	int dcdc3_vol;
	int dcdc3_vol_d;

	if(axp_probe() > 0)
	{
		if(!axp_probe_power_supply_condition())
		{
			if(!(uboot_spare_head.boot_data.reserved[0] & 0xff00))	//a7启动
			{
				if(script_parser_fetch("power_sply", "dcdc3_vol", &dcdc3_vol, 1))
				{
					dcdc3_vol_d = 1200;
				}
				else
				{
					dcdc3_vol_d = dcdc3_vol%10000;
				}
				printf("try to set dcdc3 to %d mV\n", dcdc3_vol_d);
				if(!axp_set_supply_status(0, PMU_SUPPLY_DCDC3, dcdc3_vol_d, -1))
				{
					tick_printf("PMU: dcdc3 %d\n", dcdc3_vol_d);
				}
				else
				{
					printf("axp_set_dcdc3 fail\n");
				}
			}
			else												//a15启动
			{
				int table_num, core_vol;

				table_num = sunxi_get_soc_bin();				//找到对应cpu版本的vf表
				/*scan cpu VF table*/                           //根据给定频率，找到对应电压
				core_vol = sunxi_scan_VF_table(table_num,uboot_spare_head.boot_data.run_clock);
				if(core_vol <= 0)
				{
					printf("can not find the voltage which match the dedicated a15 freq\n");
				}
				else            /*set core frequency and vol*/
			    {
			    	//set cpuB vol
				    if(!axp_set_supply_status_byname("axp806","dcdca_vol",core_vol,1))		//调整对应c1的设定电压
				    {
				    	tick_printf("PMU: axp806 dcdca %d\n", core_vol);
				    	core_vol += 1000000;
				    	script_parser_patch("slave_power_sply","dcdca_vol",&core_vol,1);

			    		sunxi_clock_set_C1corepll(uboot_spare_head.boot_data.run_clock, core_vol);
				    }
				    else
				    {
				    	printf("axp_set_dcdca fail\n");
				    }
			    }
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

	config_cpu_map();

    if(!(uboot_spare_head.boot_data.reserved[0] & 0xff00))
	    tick_printf("PMU: pll1 %d Mhz\n", sunxi_clock_get_corepll());
    else
        tick_printf("PMU : pll2 %d Mhz \n",sunxi_clock_get_C1corepll());

    axp_set_charge_vol_limit();
    axp_set_all_limit();
    axp_set_hardware_poweron_vol();

	__disable_unused_mode();

	axp_set_power_supply_output();
	axp_slave_set_power_supply_output();

	power_config_gpio_bias();
    power_limit_init();
    return 0;
}


static int sunxi_get_soc_bin(void)
{
	unsigned int sunxi_soc_chipid;
	u32 type = 0;

	/* sun9iw1p1 soc chip id init */
	sunxi_soc_chipid = readl(SUNXI_SID_BASE + 0x200 + 0x4);
	type = (sunxi_soc_chipid >> 14) & 0x3f;
	printf("chip id type is %d\n",type);
	switch (type)
	{
		case 0b000000: //default
			return 0;
		case 0b000001: //normal
			return 1;
		case 0b000011: //
			return 0;
		case 0b000111:
			return 2;
		default:
			return 0;
	}
}

static int sunxi_scan_VF_table(uint table_num, uint boot_clock)
{
	char vf_table[16];
	char freq_lel[16];
	char volt_lel[16];
	uint volt, set_clock;
	uint max_freq, min_freq;
	uint freq, lel_num;
	int  i;

	if(boot_clock == 0)
	{
		printf("boot_clock is Invalid \n");
		return -1;
	}

	memset(vf_table , 0x00, 16);
	memset(freq_lel, 0x00,16);
	memset(volt_lel , 0x00,16);
	sprintf(vf_table ,"%s%d","vf_table",table_num);
	printf("======= %s =====\n",vf_table);

	if(script_parser_fetch(vf_table,"B_max_freq",(int*)&max_freq,sizeof(int)/4))
	{
		printf("can not find B_max_freq from script \n");
		return -1;
	}
	printf("max_freq is %d \n",max_freq);
	if(script_parser_fetch(vf_table,"B_min_freq",(int*)&min_freq,sizeof(int)/4))
	{
		printf("can not find B_min_freq from script \n");
		return -1;
	}
	printf("min_freq is %d \n",min_freq);

	set_clock = boot_clock * 1000000 ;
	if(set_clock > max_freq)
		set_clock = max_freq;
	if(set_clock < min_freq)
		set_clock = min_freq;
	printf("try to set clock to %d MHZ\n",set_clock/1000000);

	if(script_parser_fetch(vf_table,"B_LV_count",(int*)&lel_num,sizeof(int)/4))
	{
		printf("can not find B_LV_count from script \n");
		return -1;
	}
	printf("lel_num is %d\n",lel_num);
	for(i = 1; i <= lel_num; i++)
	{
		sprintf(freq_lel,"%s%d%s","B_LV",i,"_freq");
		if(script_parser_fetch(vf_table,freq_lel,(int*)&freq,sizeof(int)/4))
		{
			printf("can not find B_LV_count from script \n");
			return -1;
		}
		printf("==== %s = %d ====\n",freq_lel,freq);
		if(set_clock >= freq)
		{
			sprintf(volt_lel,"%s%d%s","B_LV",i,"_volt");
			if(script_parser_fetch(vf_table,volt_lel,(int*)&volt,sizeof(int)/4))
			{
				printf("can not find volt_lel from script \n");
				return -1;
			}
			printf("find volt_lel = %d \n",volt);
			return volt;
		}
	}

	return -1;
}


static void config_cpu_map(void)
{
    uint boot_a15[8] = {0x100,0x101,0x102,0x103,0x00,0x01,0x02,0x03};
    uint boot_a7[8] = {0x00,0x01,0x02,0x03,0x100,0x101,0x102,0x103};

	if(uboot_spare_head.boot_data.reserved[0] & 0xff00)		//当前使用的是a15启动
    {
        printf("change to boot_cpu : A15 \n");
        script_parser_patch("cpu_logical_map","cpu0",(void *)&boot_a15[0],1);
        script_parser_patch("cpu_logical_map","cpu1",(void *)&boot_a15[1],1);
        script_parser_patch("cpu_logical_map","cpu2",(void *)&boot_a15[2],1);
        script_parser_patch("cpu_logical_map","cpu3",(void *)&boot_a15[3],1);
        script_parser_patch("cpu_logical_map","cpu4",(void *)&boot_a15[4],1);
        script_parser_patch("cpu_logical_map","cpu5",(void *)&boot_a15[5],1);
        script_parser_patch("cpu_logical_map","cpu6",(void *)&boot_a15[6],1);
        script_parser_patch("cpu_logical_map","cpu7",(void *)&boot_a15[7],1);
    }
    else 											    //当前使用的是a7启动
    {
        printf("change to boot_cpu : A7 \n");
        script_parser_patch("cpu_logical_map","cpu0",(void *)&boot_a7[0],1);
        script_parser_patch("cpu_logical_map","cpu1",(void *)&boot_a7[1],1);
        script_parser_patch("cpu_logical_map","cpu2",(void *)&boot_a7[2],1);
        script_parser_patch("cpu_logical_map","cpu3",(void *)&boot_a7[3],1);
        script_parser_patch("cpu_logical_map","cpu4",(void *)&boot_a7[4],1);
        script_parser_patch("cpu_logical_map","cpu5",(void *)&boot_a7[5],1);
        script_parser_patch("cpu_logical_map","cpu6",(void *)&boot_a7[6],1);
        script_parser_patch("cpu_logical_map","cpu7",(void *)&boot_a7[7],1);
    }

	return ;
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
	volatile uint reg_val;

    do
    {
	    smc_writel((1<<16) | (SUNXI_RUN_EFEX_FLAG<<8), SUNXI_RPRCM_BASE + 0x1f0);
	    smc_writel((1<<16) | (SUNXI_RUN_EFEX_FLAG<<8) | (1U<<31), SUNXI_RPRCM_BASE + 0x1f0);
	    __usdelay(10);
	    CP15ISB;
	    CP15DMB;
	    smc_writel((1<<16) | (SUNXI_RUN_EFEX_FLAG<<8), SUNXI_RPRCM_BASE + 0x1f0);
	    reg_val = smc_readl(SUNXI_RPRCM_BASE + 0x1f0);
    }
    while((reg_val & 0xff) != SUNXI_RUN_EFEX_FLAG);
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
	volatile uint reg_val;

    do
    {
	    smc_writel((1<<16),            SUNXI_RPRCM_BASE + 0x1f0);
	    smc_writel((1<<16) | (1U<<31), SUNXI_RPRCM_BASE + 0x1f0);
	    __usdelay(10);
	    CP15ISB;
	    CP15DMB;
	    smc_writel((1<<16), SUNXI_RPRCM_BASE + 0x1f0);
	    reg_val = smc_readl(SUNXI_RPRCM_BASE + 0x1f0);
    }
    while((reg_val & 0xff) != 0);
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
void sunxi_set_rtc_flag(u8 flag)
{
	volatile uint reg_val;

    do
    {
	    smc_writel((1<<16) | (flag<<8), SUNXI_RPRCM_BASE + 0x1f0);
	    smc_writel((1<<16) | (flag<<8) | (1U<<31), SUNXI_RPRCM_BASE + 0x1f0);
	    __usdelay(10);
	    CP15ISB;
	    CP15DMB;
	    smc_writel((1<<16) | (flag<<8), SUNXI_RPRCM_BASE + 0x1f0);
	    reg_val = smc_readl(SUNXI_RPRCM_BASE + 0x1f0);
    }
    while((reg_val & 0xff) != flag);
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
int sunxi_probe_securemode(void)
{
	uint reg_val;

	reg_val = readl(CCM_PLL1_C0_CTRL);
	if(!reg_val)  //读到数据全是0，那么只能是使能secure的normal模式
	{
		if(uboot_spare_head.boot_data.secureos_exist==1)	//如果是1，由sbromsw传递，表示存在安全系统，否则没有
		{
			gd->securemode = SUNXI_SECURE_MODE_WITH_SECUREOS;
			printf("secure mode: with secureos\n");
		}
		else
		{
			gd->securemode = SUNXI_SECURE_MODE_NO_SECUREOS;		//不存在安全系统
			printf("secure mode: no secureos\n");
		}
	}
	else		 //读到数据非0，那么只能是未使能secure
	{
		gd->securemode = SUNXI_NORMAL_MODE;
		printf("normal mode\n");
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
//	int mode;
//
//	if(gd->securemode == SUNXI_NORMAL_MODE)
//	{
//		mode = sid_probe_security_mode();
//		if(!mode)
//		{
//			sid_set_security_mode();
//		}
//	}

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
int sunxi_get_securemode(void)
{
	return gd->securemode;
}

