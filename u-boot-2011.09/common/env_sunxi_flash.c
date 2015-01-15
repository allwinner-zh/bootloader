/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
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
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <mmc.h>
#include <search.h>
#include <errno.h>
#include <nand.h>
#ifdef CONFIG_ALLWINNER
#include <boot_type.h>
#include <sys_partition.h>
#endif
extern uchar default_environment[];

char * env_name_spec = "SUNXI";

#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = NULL;
#endif /* ENV_IS_EMBEDDED */

/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

const uchar sunxi_sprite_environment[] = {
#ifdef  CONFIG_SUNXI_SPRITE_ENV_SETTINGS
	CONFIG_SUNXI_SPRITE_ENV_SETTINGS
#endif
	"\0"
};


#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET 0
#endif

//loff_t env_offset = (loff_t)CONFIG_ENV_ADDR;
size_t env_size = (size_t)CONFIG_ENV_SIZE;

uchar env_get_char_spec(int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
#error No support for redundant environment on sunxi nand yet!
#endif

static void flash_use_efex_env(void)
{
	if (himport_r(&env_htab, (char *)sunxi_sprite_environment,
		    sizeof(sunxi_sprite_environment), '\0', 0) == 0) {
		error("Environment import failed: errno = %d\n", errno);
	}
	gd->flags |= GD_FLG_ENV_READY;

	return ;
}

static int flash_saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*res;
	u32     start;

	start = sunxi_partition_get_offset_byname(CONFIG_SUNXI_ENV_PARTITION);
	if(!start){
		printf("fail to find part named %s\n", CONFIG_SUNXI_ENV_PARTITION);
		return -1;
	}

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', &res, ENV_SIZE);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc   = crc32(0, env_new.data, ENV_SIZE);

	return sunxi_flash_write(start, env_size/512, &env_new);
}

int saveenv(void)
{
	printf("saveenv storage_type = %d\n", uboot_spare_head.boot_data.storage_type);
	return flash_saveenv();
}

static void flash_env_relocate_spec(int workmode)
{
#if !defined(ENV_IS_EMBEDDED)
	char buf[CONFIG_ENV_SIZE];
	u32 start;

	if((workmode & WORK_MODE_PRODUCT) && (!(workmode & WORK_MODE_UPDATE)))
	{
		flash_use_efex_env();
	}
	else
	{
		start = sunxi_partition_get_offset_byname(CONFIG_SUNXI_ENV_PARTITION);
		if(!start){
			printf("fail to find part named %s\n", CONFIG_SUNXI_ENV_PARTITION);
			use_default();
			return;
		}

		if(!sunxi_flash_read(start, CONFIG_ENV_SIZE/512, buf))
		{
			use_default();
			return;
		}
		env_import(buf, 1);
	}

#endif
}


void env_relocate_spec(void)
{
	debug("env_relocate_spec storage_type = %d\n", uboot_spare_head.boot_data.storage_type);
	flash_env_relocate_spec(uboot_spare_head.boot_data.work_mode);
}

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	set_default_env(NULL);
}
#endif

/*
 *  This is called before nand_init() so we can't read NAND to
 *  validate env data.
 */

