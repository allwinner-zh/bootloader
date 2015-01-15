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
#include <spare_head.h>
#include <axp_power.h>
#include <android_misc.h>
#include <sunxi_mbr.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <fastboot.h>
#include <pmu.h>
#include <asm/arch/timer.h>
#include <asm/arch/key.h>
#include <asm/arch/dma.h>
#include <sunxi_board.h>
#include <serial.h>
#include <asm/arch/usb.h>
#if defined(CONFIG_SUNXI_I2C)
	#include <i2c.h>
#elif defined(CONFIG_SUNXI_P2WI)
	#include <p2wi.h>
#elif defined(CONFIG_SUNXI_RSB)
	#include <rsb.h>
#else
#endif

#if defined(CONFIG_SUNXI_RTC)
	#include <rtc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int loglel_change_flag __attribute__((section(".data"))) = 0;
extern int update_user_data(void);
extern void jump_to(unsigned int entey_addr);
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
void sunxi_flush_allcaches(void)
{
	icache_disable();

	flush_dcache_all();
	dcache_disable();
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
int __mmc_exit(void)
{
	return 0;
}
int mmc_exit(void)
	__attribute__((weak, alias("__mmc_exit")));


void sunxi_board_close_source(void)
{
//	axp_set_vbus_limit_dc();
	mmc_exit();
	timer_exit();

	sunxi_key_exit();
#ifdef CONFIG_SUN6I
	p2wi_exit();
#endif
	sunxi_flash_exit(1);	//强制关闭FLASH
	sunxi_sprite_exit(1);
	sunxi_dma_exit();
	disable_interrupts();
	interrupt_exit();

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
int sunxi_board_restart(int next_mode)
{
	if(!next_mode)
	{
		next_mode = PMU_PRE_SYS_MODE;
	}
	printf("set next mode %d\n", next_mode);
	axp_set_next_poweron_status(next_mode);
	board_display_set_exit_mode(0);
#ifdef CONFIG_SUNXI_DISPLAY
	drv_disp_exit();
#endif
	sunxi_board_close_source();
	reset_cpu(0);

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
int sunxi_board_shutdown(void)
{
#if defined(CONFIG_SUNXI_RTC)
	printf("rtc disable\n");
    rtc_disable();
#endif
	printf("set next system normal\n");
	axp_set_next_poweron_status(0x0);

	board_display_set_exit_mode(0);
#ifdef CONFIG_SUNXI_DISPLAY
    drv_disp_exit();
#endif
	sunxi_flash_exit(1);	//强制关闭FLASH
	sunxi_sprite_exit(1);
	disable_interrupts();
	interrupt_exit();

	tick_printf("power off\n");
	axp_set_hardware_poweroff_vol();
	axp_set_power_off();
#if defined(CONFIG_ARCH_SUN8IW7P1)
	power_off();
#endif
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
int sunxi_board_run_fel(void)
{
#if defined(CONFIG_SUN6I) || defined(CONFIG_ARCH_SUN8IW3P1)|| defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN7I)|| defined(CONFIG_ARCH_SUN8IW8P1)
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = SUNXI_RUN_EFEX_FLAG;
#elif defined(CONFIG_ARCH_SUN9IW1P1) || defined(CONFIG_ARCH_SUN8IW7P1) || defined(CONFIG_ARCH_SUN8IW6P1)
	sunxi_set_fel_flag();
#endif
	printf("set next system status\n");

	axp_set_next_poweron_status(PMU_PRE_SYS_MODE);

	board_display_set_exit_mode(0);
#ifdef CONFIG_SUNXI_DISPLAY
	drv_disp_exit();
#endif
	printf("sunxi_board_close_source\n");
	sunxi_board_close_source();

	sunxi_flush_allcaches();
#if defined(CONFIG_ARCH_SUN5I)
	printf("jump to fel_base\n");
	jump_to(FEL_BASE);
#else
	printf("reset cpu\n");

	reset_cpu(0);
#endif

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
int sunxi_board_run_fel_eraly(void)
{
#if defined(CONFIG_SUN6I) || defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1)|| defined(CONFIG_ARCH_SUN7I)||defined(CONFIG_ARCH_SUN8IW8P1)
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = SUNXI_RUN_EFEX_FLAG;
#elif defined(CONFIG_ARCH_SUN9IW1P1) || defined(CONFIG_ARCH_SUN8IW7P1) || defined(CONFIG_ARCH_SUN8IW6P1)
	sunxi_set_fel_flag();
#endif
	printf("set next system status\n");
    axp_set_next_poweron_status(PMU_PRE_SYS_MODE);
	timer_exit();
	sunxi_key_exit();
#ifdef CONFIG_SUN6I
	p2wi_exit();
#endif
	sunxi_dma_exit();
#if defined(CONFIG_ARCH_SUN5I)
	printf("jump to fel_base\n");
	jump_to(FEL_BASE);
#else
	printf("reset cpu\n");
//#if defined(CONFIG_ARCH_SUN9IW1P1)
//	*( volatile unsigned int *)(0x008000e0) = 0x16aa0000;
//#endif
	reset_cpu(0);
#endif
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
void sunxi_update_subsequent_processing(int next_work)
{
	printf("next work %d\n", next_work);
	switch(next_work)
	{
		case SUNXI_UPDATE_NEXT_ACTION_REBOOT:	//重启
        case SUNXI_UPDATA_NEXT_ACTION_SPRITE_TEST:
            printf("SUNXI_UPDATE_NEXT_ACTION_REBOOT\n");
			//do_reset(NULL, 0, 0, NULL);
			sunxi_board_restart(0);

			break;
		case SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN:	//关机
			printf("SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN\n");
			//do_shutdown(NULL, 0, 0, NULL);
			sunxi_board_shutdown();

			break;
		case SUNXI_UPDATE_NEXT_ACTION_REUPDATE:
			printf("SUNXI_UPDATE_NEXT_ACTION_REUPDATE\n");
			sunxi_board_run_fel();			//进行量产

			break;
		case SUNXI_UPDATE_NEXT_ACTION_BOOT:
		case SUNXI_UPDATE_NEXT_ACTION_NORMAL:
		default:
			printf("SUNXI_UPDATE_NEXT_ACTION_NULL\n");
			break;
	}

	return ;
}

//(SUNXI_MBR_MAX_PART_COUNT * (16*2 + 2))
#define PARTITION_SETS_MAX_SIZE	 1024
void fastboot_partition_init(void)
{
	fastboot_ptentry fb_part;
	int index, part_total;
	char partition_sets[PARTITION_SETS_MAX_SIZE];
	char part_name[16];
	char *partition_index = partition_sets;
	int offset = 0;
	int temp_offset = 0;
	int storage_type = uboot_spare_head.boot_data.storage_type;

	printf("--------fastboot partitions--------\n");
	part_total = sunxi_partition_get_total_num();
	if((part_total <= 0) || (part_total > SUNXI_MBR_MAX_PART_COUNT))
	{
		printf("mbr not exist\n");

		return ;
	}
	printf("-total partitions:%d-\n", part_total);
	printf("%-12s  %-12s  %-12s\n", "-name-", "-start-", "-size-");

	memset(partition_sets, 0, PARTITION_SETS_MAX_SIZE);

	for(index = 0; index < part_total && index < SUNXI_MBR_MAX_PART_COUNT; index++)
	{
		sunxi_partition_get_name(index, &fb_part.name[0]);
		fb_part.start = sunxi_partition_get_offset(index) * 512;
		fb_part.length = sunxi_partition_get_size(index) * 512;
		fb_part.flags = 0;
		printf("%-12s: %-12x  %-12x\n", fb_part.name, fb_part.start, fb_part.length);

		memset(part_name, 0, 16);
		if(!storage_type)
		{
			sprintf(part_name, "nand%c", 'a' + index);
		}
		else
		{
			if(index == 0)
			{
				strcpy(part_name, "mmcblk0p2");
			}
			else if( (index+1)==part_total)
			{
				strcpy(part_name, "mmcblk0p1");
			}
			else
			{
				sprintf(part_name, "mmcblk0p%d", index + 4);
			}
		}

		temp_offset = strlen(fb_part.name) + strlen(part_name) + 2;
		if(temp_offset >= PARTITION_SETS_MAX_SIZE)
		{
			printf("partition_sets is too long, please reduces partition name\n");
			break;
		}
		fastboot_flash_add_ptn(&fb_part);
		sprintf(partition_index, "%s@%s:", fb_part.name, part_name);
		offset += temp_offset;
		partition_index = partition_sets + offset;
	}

	partition_sets[offset-1] = '\0';
	partition_sets[PARTITION_SETS_MAX_SIZE - 1] = '\0';
	printf("-----------------------------------\n");

	setenv("partitions", partition_sets);
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
*    note          :  把目标字符串dest_buf(格式：空格隔开，比如"AAA BBB C DD EEE"，但是不包括tab
*                     中的goal子字符串，替换成replace
*                     如
*					  sunxi_str_replace("abc def gh", "def", "replace")
*					  执行的结果是  "abc def gh"变成了 "abc replace gh"
*					  如果找不到不够，则不替换
*					  必须保证空间足够
************************************************************************************************************
*/
#ifndef CONFIG_SUNXI_SPINOR_PLATFORM
static int sunxi_str_replace(char *dest_buf, char *goal, char *replace)
{
	char tmp[128];
	char tmp_str[16];
	int  goal_len, rep_len, dest_len;
	int  i, j, k;

	if( (goal == NULL) || (dest_buf == NULL))
	{
		return -1;
	}

	memset(tmp, 0, 128);
	strcpy(tmp, dest_buf);

	goal_len = strlen(goal);
	dest_len = strlen(dest_buf);

	if(replace != NULL)
	{
		rep_len = strlen(replace);
	}
	else
	{
		rep_len = 0;
	}
	j = 0;
	for(i=0;tmp[i];)
	{
		//找出空格字符
		k = 0;
		while(((tmp[i] != ' ') && (tmp[i] != 0) )|| (tmp[i+1] == ' '))
		{
			tmp_str[k++] = tmp[i];
			i ++;
			if(i >= dest_len)
				break;
		}
		i ++;
		//开始找出一个完整的字符串
		tmp_str[k] = 0;
		if(!strcmp(tmp_str, goal))
		{
			if(rep_len)
			{
				strcpy(dest_buf + j, replace);
				if(tmp[j + goal_len])
				{
					memcpy(dest_buf + j + rep_len, tmp + j + goal_len, dest_len - j - goal_len);
					dest_buf[dest_len - goal_len + rep_len] = 0;
				}
			}
			else
			{
				if(tmp[j + goal_len])
				{
					memcpy(dest_buf + j, tmp + j + goal_len, dest_len - j - goal_len);
					dest_buf[dest_len - goal_len + rep_len] = 0;
				}
			}

			return 0;
		}
		j = i;
	}

	return 0;

}
#define    ANDROID_NULL_MODE            0
#define    ANDROID_FASTBOOT_MODE		1
#define    ANDROID_RECOVERY_MODE		2
#define    USER_SELECT_MODE 			3
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
static int detect_other_boot_mode(void)
{
    int ret1, ret2;
	int key_high, key_low;
	int keyvalue;
	int user_mode_used = 0;
	keyvalue = gd->key_pressd_value;
	printf("key %d\n", keyvalue);
	script_parser_fetch("platform", "user_mode_used", &user_mode_used, 1);
	if(user_mode_used)
	{
		if(keyvalue >0)
		{
			return USER_SELECT_MODE;
		}
	}
	else
	{
	    ret1 = script_parser_fetch("recovery_key", "key_max", &key_high, 1);
		ret2 = script_parser_fetch("recovery_key", "key_min", &key_low,  1);
		if((ret1) || (ret2))
		{
			printf("cant find rcvy value\n");
		}
		else
		{
			printf("recovery key high %d, low %d\n", key_high, key_low);
			if((keyvalue >= key_low) && (keyvalue <= key_high))
			{
				printf("key found, android recovery\n");

				return ANDROID_RECOVERY_MODE;
			}
		}
	    ret1 = script_parser_fetch("fastboot_key", "key_max", &key_high, 1);
		ret2 = script_parser_fetch("fastboot_key", "key_min", &key_low, 1);
		if((ret1) || (ret2))
		{
			printf("cant find fstbt value\n");
		}
		else
		{
			printf("fastboot key high %d, low %d\n", key_high, key_low);
			if((keyvalue >= key_low) && (keyvalue <= key_high))
			{
				printf("key found, android fastboot\n");
				return ANDROID_FASTBOOT_MODE;
			}
		}
	}
	return ANDROID_NULL_MODE;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :	check_debug_mode
*
*    parmeters     :
*
*    return        :
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/

static void check_debug_mode(void)
{
    //if enter debug mode,set loglevel = 8
    char change_env_data[32];
    char *env_concole = "ttyS";
    int baud = 115200;
    int port_id = 0;

    if(!loglel_change_flag)
        return ;
    memset(change_env_data,0,32);
    sprintf(change_env_data, "%d",8);
    setenv("loglevel",change_env_data);
    if(script_parser_fetch("force_uart_para","force_uart_port",&port_id,sizeof(int)/4))
    {
        printf("card0_print_para port_id fetch error\n");
        return ;
    }
    memset(change_env_data,0,32);
    strcat(change_env_data,"ttyS");
    sprintf(change_env_data,"%s%d%s%d",env_concole,port_id,",",baud);
    setenv("console",change_env_data);

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
int check_android_misc(void)
{
	int   mode;
	int	  pmu_value;
	u32   misc_offset = 0;
	char  misc_args[2048];
	char  misc_fill[2048];
	char  boot_commond[128];
	static struct bootloader_message *misc_message;

	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
	{
		return 0;
	}
	if(gd->force_shell)
	{
		char delaytime[8];

		sprintf(delaytime, "%d", 3);
		setenv("bootdelay", delaytime);
	}
    //if enter debug mode,set loglevel = 8
    check_debug_mode();

   memset(boot_commond, 0x0, 128);
	strcpy(boot_commond, getenv("bootcmd"));
	printf("base bootcmd=%s\n", boot_commond);
	//判断存储介质
	if((uboot_spare_head.boot_data.storage_type == 1) || (uboot_spare_head.boot_data.storage_type == 2))
	{
		sunxi_str_replace(boot_commond, "setargs_nand", "setargs_mmc");
		printf("bootcmd set setargs_mmc\n");
	}
	else
	{
		printf("bootcmd set setargs_nand\n");
	}
	misc_message = (struct bootloader_message *)misc_args;
	memset(misc_args, 0x0, 2048);
	memset(misc_fill, 0xff, 2048);
	mode = detect_other_boot_mode();
	if(mode == ANDROID_NULL_MODE)
	{
		pmu_value = axp_probe_pre_sys_mode();
		if(pmu_value == PMU_PRE_FASTBOOT_MODE)
		{
			puts("PMU : ready to enter fastboot mode\n");
			strcpy(misc_message->command, "bootloader");
		}
		else if(pmu_value == PMU_PRE_RECOVERY_MODE)
		{
			puts("PMU : ready to enter recovery mode\n");
			strcpy(misc_message->command, "boot-recovery");
		}
		else
		{
			misc_offset = sunxi_partition_get_offset_byname("misc");
			debug("misc_offset = %x\n",misc_offset);
			if(!misc_offset)
			{
				printf("no misc partition is found\n");
			}
			else
			{
				printf("misc partition found\n");
				sunxi_flash_read(misc_offset, 2048/512, misc_args); //read misc partition data
			}
		}
	}
	else if(mode == ANDROID_RECOVERY_MODE)
	{
		strcpy(misc_message->command, "boot-recovery");
	}
	else if( mode == ANDROID_FASTBOOT_MODE)
	{
		strcpy(misc_message->command, "bootloader");
	}
	//最终统一判断命令
	if(!loglel_change_flag)   //add by young,if you want to enter debug_mode ,so do enter boot_normal
	{
		if(!strcmp(misc_message->command, "efex"))
		{
			/* there is a recovery command */
			puts("find efex cmd\n");
			sunxi_flash_write(misc_offset, 2048/512, misc_fill);
			sunxi_board_run_fel();

			return 0;
		}

		if(!strcmp(misc_message->command, "boot-resignature"))
		{
			puts("find boot-resignature cmd\n");
			sunxi_flash_write(misc_offset, 2048/512, misc_fill);
			sunxi_oem_op_lock(SUNXI_LOCKING, NULL, 1);
		}
		else if(!strcmp(misc_message->command, "boot-recovery"))
		{
			if(!strcmp(misc_message->recovery, "sysrecovery"))
			{
				puts("recovery detected, will sprite recovery\n");
				strncpy(boot_commond, "sprite_recovery", sizeof("sprite_recovery"));
				sunxi_flash_write(misc_offset, 2048/512, misc_fill);
			}
			else
			{
				puts("Recovery detected, will boot recovery\n");
				sunxi_str_replace(boot_commond, "boot_normal", "boot_recovery");
			}
			/* android recovery will clean the misc */
		}
		else if(!strcmp(misc_message->command, "bootloader"))
		{
			puts("Fastboot detected, will boot fastboot\n");
			sunxi_str_replace(boot_commond, "boot_normal", "boot_fastboot");
			if(misc_offset)
				sunxi_flash_write(misc_offset, 2048/512, misc_fill);
		}
		else if(!strcmp(misc_message->command, "usb-recovery"))
		{
			puts("Recovery detected, will usb recovery\n");
			sunxi_str_replace(boot_commond, "boot_normal", "boot_recovery");
		}
	}
	if(!strcmp(misc_message->command ,"debug_mode"))
	{
		puts("debug_mode detected ,will enter debug_mode");
		if(!change_to_debug_mode())
		{
			check_debug_mode();
		}
			sunxi_flash_write(misc_offset,2048/512,misc_fill);
	}
	setenv("bootcmd", boot_commond);

	printf("to be run cmd=%s\n", boot_commond);
	return 0;

}
#endif
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
int board_late_init(void)
{
	fastboot_partition_init();

#ifdef  CONFIG_ARCH_HOMELET
	respond_physical_key_action();
#endif
#ifndef CONFIG_SUNXI_SPINOR_PLATFORM
	check_android_misc();
#endif
#ifdef  CONFIG_ARCH_HOMELET
	update_user_data();
#endif
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

int check_uart_input(void)
{
	int c = 0;
	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
	{
	    return 0;
	}
	if(tstc())
	{
		c = getc();
		printf("0x%x\n", c);
	}
	else
	{
		puts("no key input\n");
	}

	if(c == '2')
	{
		return -1;
	}
	else if(c == '3')
	{
		sunxi_key_init();
		do_key_test(NULL, 0, 1, NULL);
	}
	else if(c == 's')		//shell mode
	{
		gd->force_shell = 1;
                gd->debug_mode = 1;
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
#define   KEY_DELAY_MAX          (8)
#define   KEY_DELAY_EACH_TIME    (40)
#define   KEY_MAX_COUNT_GO_ON    ((KEY_DELAY_MAX * 1000)/(KEY_DELAY_EACH_TIME))
int check_update_key(void)
{
	int ret;
	int fel_key_max;
	int power_plug_count = 0;
	int new_power_status = 0;
	int old_power_status = 0;
	int time_tick = 0 ;
	int fel_key_mode = 0;
	int user_mode_used = 0;
    gd->key_pressd_value = 0;
	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
	{
	    return 0;
	}
	old_power_status = axp_probe_power_source();
	sunxi_key_init();
//detect user_mode exist
	script_parser_fetch("platform", "user_mode_used", &user_mode_used, 1);
	if(user_mode_used)
	{
		printf("user_mode found\n");
		fel_key_mode = 2;
	}

//detect fel_key exist
    else
    {
		ret = script_parser_fetch("fel_key", "fel_key_max", &fel_key_max, 1);
	    if(ret)
	    {
	    	printf("fel key old mode\n");
			fel_key_mode = 1;
		}
		else
		{
			printf("fel key new mode\n");
		}
    }
	printf("run key detect\n");

	sunxi_key_read();
	__msdelay(10);

	if(!fel_key_mode)
	{
		int key_value;
		int fel_key_max, fel_key_min;
		time_tick = 0;
	    key_value = sunxi_key_read();  		//读取按键信息
	    if(key_value < 0)             				//没有按键按下
	    {
	        printf("no key found\n");
	        return 0;
	    }
	   gd->key_pressd_value = key_value;
		ret = script_parser_fetch("fel_key", "fel_key_max", &fel_key_max, 1);
	    if(ret)
	    {
	    	printf("fel key max not found\n");

	    	return 0;
	    }

		ret = script_parser_fetch("fel_key", "fel_key_min", &fel_key_min, 1);
	    if(ret)
	    {
	    	printf("fel key min not found\n");

	    	return 0;
	    }

		if((key_value <= fel_key_max) && (key_value >= fel_key_min))
		{
			printf("fel key detected\n");

			return 1;
		}

		printf("fel key value %d is not in the range from %d to %d\n", key_value, fel_key_min, fel_key_max);
		//old_usb_plug_status = usb_probe_vbus_type();
		//add by guoyingyang
		while(sunxi_key_read() > 0) //press key and not loosen
		{
			time_tick++;
			new_power_status = axp_probe_power_source();
			__msdelay(KEY_DELAY_EACH_TIME);
			if(new_power_status != old_power_status)
			{
				power_plug_count++;
				old_power_status = new_power_status;
			}
			if(power_plug_count == 3)
			{
				change_to_debug_mode();
				break;
			}
			if(time_tick > KEY_MAX_COUNT_GO_ON)
			{
				printf("time out\n");
				break;
			}
		}

		return 0;
	}
	else if(fel_key_mode == 1)
	{
	    int count;
	    int value_old, value_new, value_cnt;
	    int new_key, new_key_flag;

	    time_tick = 0;
	    count = 0;
	    value_cnt = 0;
	    new_key = 0;
	    new_key_flag = 0;
	    ret = sunxi_key_read();  				//读取按键信息
	    if(ret < 0)             				//没有按键按下
	    {
	        printf("no key found\n");
	        return 0;
	    }
	    else
	    {
	    	value_old = ret;
	    }
			gd->key_pressd_value = ret;
	    while(1)
	    {
	        time_tick ++;
	        ret = axp_probe_key();  			//获取power按键信息
			new_power_status = axp_probe_power_source(); //detect vbus status
			printf("new_power_status = %d \n",new_power_status);
	        if(ret > 0)              	  		//检测到POWER按键按下
	        {
	            count ++;
	        }
			if(new_power_status != old_power_status)
			{
				debug("=======flag change========\n");
				power_plug_count ++;
				old_power_status = new_power_status;
			}
	        __msdelay(KEY_DELAY_EACH_TIME);
	        ret = sunxi_key_read();  			//读取按键信息
	        if(ret < 0)             			//没有按键按下
	        {
	            printf("key not pressed anymore\n");
	            if(count == 1)
	            {
	            	if(new_key >= 2)
	            	{
	            		printf("1\n");
	            		printf("force to debug mode\n");
                                return -1;
	            	}
	            }

				return 0;
	        }
	        else
	        {
	        	value_new = ret;
	        	if(value_old == value_new)
	        	{
	        		value_cnt ++;
	        		if(new_key_flag == 1)
	        		{
	        			new_key ++;
	        			new_key_flag ++;
	        		}
	        		else if(!new_key_flag)
	        		{
	        			new_key_flag ++;
	        		}
	        	}
	        	else
	        	{
	        		new_key_flag = 0;
	        		value_old = value_new;
	        	}
	        }

	        if(count == 3)
	        {
	        	printf("you can unclench the key to update now\n");
	            return -1;
	        }

			if((!count) && (power_plug_count == 3))
			{
				change_to_debug_mode();
				return 0;
			}

	        if((!count) && (time_tick >= KEY_MAX_COUNT_GO_ON))
	        {
	            printf("timeout, but no power key found\n");

	            return 0;
	        }
	    }
	}
	else
	{
		int user_key_value;
		user_key_value = sunxi_key_read();		 //读取按键信息
		if(user_key_value < 0)					 //没有按键按下
		{
			printf("no user_key found\n");
		}
		else
		{
			gd->key_pressd_value = user_key_value;
		}
		return 0;
	}
}

#ifndef CONFIG_SUNXI_SPINOR_PLATFORM
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
*    note          :    作用：初始化用户配置的gpio
*
*
************************************************************************************************************
*/
int gpio_control(void)
{
	int ret;
	int used;

	ret = script_parser_fetch("boot_init_gpio", "used", &used, sizeof(int) / 4);
	if (!ret && used) {
			puts("user_gpio config\n");
			gpio_request_ex("boot_init_gpio", NULL);
			puts("user_gpio ok\n");
			return 0;
	}
	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :	usb-recovery 写入misc分区对应的命令
*
*    parmeters     :
*
*    return        :
*
*    note          :	yanjianbo@allwinnertech.com
*
*
************************************************************************************************************
*/
int write_usb_recovery_to_misc(void)
{
	u32   misc_offset = 0;
	char  misc_args[2048];
	static struct bootloader_message *misc_message;
	int ret;

	memset(misc_args, 0x0, 2048);
	misc_message = (struct bootloader_message *)misc_args;

	misc_offset = sunxi_partition_get_offset_byname("misc");
	if(!misc_offset)
	{
		printf("no misc partition\n");
		return 0;
	}
	ret = sunxi_flash_read(misc_offset, 2048/512, misc_args);
	if (!ret)
	{
		printf("error: read misc partition\n");
		return 0;
	}
	strcpy(misc_message->command, "usb-recovery");
	sunxi_flash_write(misc_offset, 2048/512, misc_args);
	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :	一键恢复的按键检测
*
*    parmeters     :
*
*    return        :
*
*    note          :	yanjianbo@allwinnertech.com
*
*
************************************************************************************************************
*/
#define ONEKEY_USB_RECOVERY_MODE			(0x01)
#define ONEKEY_SPRITE_RECOVERY_MODE			(0x02)
#define USB_RECOVERY_KEY_VALUE				(0x81)
#define SPRITE_RECOVERY_KEY_VALUE			(0X82)

int check_physical_key_early(void)
{
	user_gpio_set_t	gpio_recovery;
	__u32 gpio_hd;
	int ret;
	int gpio_value = 0;
	int used = 0;
	int mode = 0;

	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
	{
		return 0;
	}

	ret = script_parser_fetch("recovery_para", "used", (int *)&used, sizeof(int) / 4);
	if (ret || !used)
	{
		printf("[recovery] no use\n");
		return 0;
	}
	ret = script_parser_fetch("recovery_para", "recovery_key", (int *)&gpio_recovery, sizeof(user_gpio_set_t) / 4);
  if (!ret)
	{
		gpio_recovery.mul_sel = 0;		//强制设置成输入
		gpio_hd = gpio_request(&gpio_recovery, 1);
		if (gpio_hd)
		{
			int time;
			gpio_value = 0;
			for(time = 0; time < 4; time++)
			{
				gpio_value += gpio_read_one_pin_value(gpio_hd, 0);
				__msdelay(5);
			}
			if (!gpio_value)
			{
				printf("[box recovery] find the key\n");
				script_parser_fetch("recovery_para", "mode", (int *)&mode, sizeof(int) / 4);
				if (mode == ONEKEY_USB_RECOVERY_MODE)
				{
					gd->key_pressd_value = USB_RECOVERY_KEY_VALUE;
				}
				else if (mode == ONEKEY_SPRITE_RECOVERY_MODE)
				{
					gd->key_pressd_value = SPRITE_RECOVERY_KEY_VALUE;
					uboot_spare_head.boot_data.work_mode = WORK_MODE_SPRITE_RECOVERY;
				}
				else
				{
					printf("[recovery] no option for one key recovery's mode (%d)\n", mode);
				}
			}
		}
	}
	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :	按键检测后的事件处理
*
*    parmeters     :
*
*    return        :
*
*    note          :	yanjianbo@allwinnertech.com
*
*
************************************************************************************************************
*/
void respond_physical_key_action(void)
{
	int key_value;
	key_value = gd->key_pressd_value;

	if (key_value == USB_RECOVERY_KEY_VALUE)
	{
		printf("[box recovery] set to one key usb recovery\n");
		write_usb_recovery_to_misc();
	}
	else if (key_value == SPRITE_RECOVERY_KEY_VALUE)
	{
		printf("[box recovery] set to one key sprite recovery\n");
		//setenv("bootcmd", "sprite_recovery");
	}
}
#endif

#ifdef CONFIG_ARCH_SUN9IW1P1
#define UART_GATE_CTRL  (0x06000400 + 0x194)
#else
#define UART_GATE_CTRL  (0x01c20000 + 0x6c)
#endif

#if !defined(CONFIG_ARCH_SUN7I) || !defined(CONFIG_ARCH_SUN5I)
#ifdef CONFIG_ARCH_SUN9IW1P1
#define UART_RST_CTRL   (0x06000400 + 0x1B4)
#else
#define UART_RST_CTRL   (0x01c20000 + 0x02D8)
#endif
#endif
/*
************************************************************************************************************
*
*                                             function
*
*    name          :	modify_uboot_uart
*
*    parmeters     :
*
*    return        :
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/
int modify_uboot_uart(void)
{
    script_gpio_set_t fetch_cfg_gpio[2];
    u32  reg = 0;
    int uart_port_id = 0;
//disable uart0
    if(script_parser_fetch("uart_para","uart_debug_rx",(int *)(&fetch_cfg_gpio[0]),sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error: can't find card0_rx \n");
        return -1;
    }
	fetch_cfg_gpio[0].mul_sel = 0;
    if(script_parser_patch("uart_para","uart_debug_rx",(void*)&fetch_cfg_gpio[0],sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error : can't patch uart_debug_rx\n");
        return -1;
    }
    //config uart_tx
    if(script_parser_fetch("uart_para","uart_debug_tx",(int *)(&fetch_cfg_gpio[1]),sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error: can't find card0_tx \n");
        return -1;
    }
	fetch_cfg_gpio[1].mul_sel = 0;
    if(script_parser_patch("uart_para","uart_debug_tx",(void*)&fetch_cfg_gpio[1],sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error : can't patch uart_debug_tx\n");
        return -1;
    }
	//disable uart0
	gpio_request_simple("uart_para",NULL);
    //port_id
    if(script_parser_fetch("force_uart_para","force_uart_port",(int *)(&uart_port_id),sizeof(int)/4))
    {
        printf("debug_mode_error: can't find card0_tx \n");
        return -1;
    }
	if(script_parser_patch("uart_para","uart_debug_port",(int *)(&uart_port_id),sizeof(int)/4))
    {
        printf("debug_mode_error: can't find card0_tx \n");
        return -1;
    }
	if(script_parser_fetch("force_uart_para","force_uart_tx",(int *)(&fetch_cfg_gpio[0]),sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error: can't find card0_tx \n");
        return -1;
    }
    if(script_parser_patch("uart_para","uart_debug_tx",(void*)&fetch_cfg_gpio[0],sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error : can't patch uart_debug_tx\n");
        return -1;
    }
	if(script_parser_fetch("force_uart_para","force_uart_rx",(int *)(&fetch_cfg_gpio[1]),sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error: can't find card0_tx \n");
        return -1;
    }
    if(script_parser_patch("uart_para","uart_debug_rx",(void*)&fetch_cfg_gpio[1],sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error : can't patch uart_debug_tx\n");
        return -1;
    }

    printf("uart_port_id = %d\n",uart_port_id);
    uboot_spare_head.boot_data.uart_port = uart_port_id;
    //reset
#ifdef UART_RST_CTRL
	reg = readl(UART_RST_CTRL);
	reg &= ~(1 << (16 + uart_port_id));
	reg |=  (1 << (16 + uart_port_id));
        writel(reg,UART_RST_CTRL);
#endif
    //gate
	reg = readl(UART_GATE_CTRL);
    reg &= ~(1 << (16 + uart_port_id));
	reg |=  (1 << (16 + uart_port_id));
    writel(reg,UART_GATE_CTRL);
	//enable card0
	gpio_request_simple("uart_para",NULL);
    serial_init();
	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :	modify_system_uart
*
*    parmeters     :
*
*    return        : -1 :fail   0:success
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/

int modify_system_uart(void)
{
    script_gpio_set_t fetch_cfg_gpio[2];
    int uart_port_id = 0;
    char uartname[16] ;
    char uart_data[8] ;
    int sdc0_used = 0,uart_used = 1;

    if(script_parser_fetch("force_uart_para","force_uart_rx",(int *)(&fetch_cfg_gpio[0]),sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error: can't find force_uart_rx \n");
        return -1;
    }
    if(script_parser_fetch("force_uart_para","force_uart_tx",(int *)(&fetch_cfg_gpio[1]),sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error: can't find force_uart_tx \n");
        return -1;
    }
    if(script_parser_fetch("force_uart_para","force_uart_port",(int *)(&uart_port_id),sizeof(int)/4))
    {
        printf("debug_mode_error: can't find card0_tx \n");
        return -1;
    }
    memset(uartname,0,16);
    memset(uart_data,0,8);
#if defined (CONFIG_ARCH_SUN7I) ||defined(CONFIG_ARCH_SUN8IW1P1)
    strcat(uartname,"uart_para");
#else
	strcat(uartname,"uart");
#endif
    sprintf(uart_data,"%d",uart_port_id);
    strcat(uartname,uart_data);
    printf("the uartname is %s  \n",uartname);
    if(script_parser_patch(uartname,"uart_used",(int *)(&uart_used),sizeof(int)/4))
    {
        printf("debug_mode_error : can't find patch uart_used\n");
        return -1;
    }
    if(script_parser_patch(uartname,"uart_port",(int *)(&uart_port_id),sizeof(int)/4))
    {
        printf("debug_mode_error : can't find uart_debug_port \n");
        return -1;
    }
    if(script_parser_patch(uartname,"uart_rx",(void*)&fetch_cfg_gpio[0],sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error : can't patch uart_debug_rx\n");
        return -1;
    }
    if(script_parser_patch(uartname,"uart_tx",(void*)&fetch_cfg_gpio[1],sizeof(script_gpio_set_t)/4))
    {
        printf("debug_mode_error : can't patch uart_debug_rx\n");
        return -1;
    }
//disable card0 init in linux
    if(script_parser_patch("mmc0_para","sdc_used",(int *)(&sdc0_used),sizeof(int)/4))
    {
        printf("debug_mode_error :can not patch sdc_used \n");
        return -1;
    }
    return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :	change_to_debug_mode
*
*    parmeters     :
*
*    return        : -1 :fail   0:success
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/
int change_to_debug_mode(void)
{
    printf("enter debug mode\n");
    if(modify_uboot_uart())
    {
		printf("debug_mode_error : fail to modify uboot uart\n");
		return -1;
    }
    if(modify_system_uart())
    {
		printf("debug_mode_error: fail to modify system uart\n");
		return -1;
    }
//if enter debug mode ,set system can print message
    loglel_change_flag = 1;
    return 0;
}

#ifdef CONFIG_READ_LOGO_FOR_KERNEL
/*
************************************************************************************************************
*
*                                             function
*
*    name          :	sunxi_read_bootlogo
*
*    parmeters     :
*
*    return        : -1 :fail   0:success
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/
void sunxi_read_bootlogo(char *part_name)
{
    int ret = 0;
    u32 rblock = 0;
    u32 start_block = 0;
    uint addr;
#if defined(CONFIG_SUNXI_LOGBUFFER)
    addr = (uint)(CONFIG_SYS_SDRAM_BASE + gd->ram_size - SUNXI_DISPLAY_FRAME_BUFFER_SIZE);
#else
    addr = (uint)(SUNXI_DISPLAY_FRAME_BUFFER_ADDR);
#endif
    printf("addr = %x \n",addr);
    start_block = sunxi_partition_get_offset_byname((const char *)part_name);
    if(start_block == 0 )
        return ;
    rblock = sunxi_partition_get_size_byname((const char *)part_name);
    ret = sunxi_flash_read(start_block, rblock, (void *)addr);
    if(ret != 0)
    {
        gd->fb_base = addr ;
        printf("sunxi_read_bootlogo: read bootlogo partition successful \n");
    }
    else
    {
        gd->fb_base = 0;
        printf("sunxi_read_bootlogo: read bootlogo partition fail \n");
    }
    return;
}
#endif

/*
************************************************************************************************************
*
*                                             function
*
*    name          :	get_debugmode_flag
*
*    parmeters     :
*
*    return        : 
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/
int get_debugmode_flag(void)
{
    int debug_mode = 0;
    if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
    {
    	gd->debug_mode = 1;
        return 0;
    }
    if(!script_parser_fetch("platform", "debug_mode",&debug_mode, 1))
        gd->debug_mode = debug_mode;
    else
        gd->debug_mode = 1;
    return 0;
}
