/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Char <yanjianbo@allwinnertech.com>
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
#include <boot_type.h>
#include <sunxi_mbr.h>
#include <sys_partition.h>
#include <securestorage.h>

#define MAGIC		"sunxi"
#define PART_NAME	"private"
#define USER_DATA_MAXSIZE				(8 * 1024)
#define USER_DATA_PARAMETER_MAX_COUNT	(30)


#define	NAME_SIZE	32
#define VALUE_SIZE	128

typedef struct {
	char magic_name[8];
	int count;
	int reserved[3];
}USER_DATA_HEAR;

typedef struct {
	char name[NAME_SIZE];
	char value[VALUE_SIZE];
	int valid;					//0: 有效  1：无效（不被更新
	int reserved[3];
}USER_PRIVATE_DATA;

char *IGNORE_ENV_VARIABLE[] = { "console",
								"root",
								"init",
								"loglevel",
								"partitions",
								"vmalloc",
								"earlyprintk",
								"ion_reserve",
							};

int USER_DATA_NUM;									//用户的环境变量个数
char USER_DATA_NAME[10][NAME_SIZE] = {{'\0'}};		//用户的环境变量（从env.fex获取）

void check_user_data(void);
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
int save_user_private_data(char *name, char *data)
{
	int j;
	unsigned int part_offset;					//分区的地址偏移量
	unsigned int part_size;						//分区的大小
	unsigned int user_data_offset;				//用户存放数据地址的偏移量
	char user_data_buffer[USER_DATA_MAXSIZE] = {0};	//
	USER_PRIVATE_DATA *user_data_p = NULL;
	USER_DATA_HEAR *user_data_head = NULL;

	if (!name || !data) {
		printf("error: the name (data) is null\n");
		return 0;
	}

	part_size = sunxi_partition_get_size_byname(PART_NAME);
	if (part_size > 0)
	{
		part_offset = sunxi_partition_get_offset_byname(PART_NAME);
		user_data_offset = part_offset + part_size - (USER_DATA_MAXSIZE >> 9);		//获得用户存放数据地址的偏移量
		if (!sunxi_flash_read(user_data_offset, USER_DATA_MAXSIZE >> 9, user_data_buffer)) {
			printf("read flash error\n");
			return 0;
		}

		user_data_head = (USER_DATA_HEAR *)user_data_buffer;
		user_data_p = (USER_PRIVATE_DATA *)(user_data_buffer + sizeof(USER_DATA_HEAR));
		if (strncmp(user_data_head->magic_name, MAGIC, 5)) {
			memset(user_data_buffer, 0xff, USER_DATA_MAXSIZE);
			strcpy(user_data_head->magic_name, MAGIC);
			user_data_head->count = 0;
			printf("init the (user) private space\n");
		}

		if (user_data_head->count > 0) {
			for (j = 0; j < user_data_head->count && j < USER_DATA_PARAMETER_MAX_COUNT; j++) {
				if (!strcmp(name, user_data_p->name)) {					//匹配数据项名称
					strcpy(user_data_p->value, data);					//更新数据
					user_data_p->valid = 1;							    //更新有效值
					printf("Saving Environment to \n");
					break;
				}
				user_data_p++;
			}
			if (j == user_data_head->count) {
					strcpy(user_data_p->name, name);					//在尾巴增加新的数据
					strcpy(user_data_p->value, data);
					user_data_p->valid = 1;
					user_data_head->count++;
					printf("Saving Environment to \n");
			}
		}
		else {
			strcpy(user_data_p->name, name);					//第一个数据
			strcpy(user_data_p->value, data);
			user_data_p->valid = 1;
			user_data_head->count++;
			printf("Saving Environment to \n");
		}

		sunxi_flash_write(user_data_offset, USER_DATA_MAXSIZE >> 9, user_data_buffer);	//回写到flash 中
		sunxi_flash_flush();

		return 0;
	}
	printf("the part isn't exist\n");
	return 0;
}


int do_save_user_data (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 3) {
		printf("usage: saveud <name> <data>\n");
		return 0;
	}
	if (argc == 3) {
		save_user_private_data(argv[1], argv[2]);
	}

	return 0;
}

U_BOOT_CMD(
	save_userdata,	3,	1,	do_save_user_data,
	"save user data",
	"<name> <data>\n"
);

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
int user_data_list(void)
{
	int j;
	unsigned int part_offset;					//分区的地址偏移量
	unsigned int part_size;						//分区的大小
	unsigned int user_data_offset;				//用户存放数据地址的偏移量
	char user_data_buffer[USER_DATA_MAXSIZE];	//
	USER_PRIVATE_DATA *user_data_p = NULL;
	USER_DATA_HEAR *user_data_head = NULL;

	printf("user_data_list\n");
	part_size = sunxi_partition_get_size_byname(PART_NAME);
	if (part_size > 0) {
		part_offset = sunxi_partition_get_offset_byname(PART_NAME);

		user_data_offset = part_offset + part_size - (USER_DATA_MAXSIZE >> 9);		//获得用户存放数据地址的偏移量
		if (!sunxi_flash_read(user_data_offset, USER_DATA_MAXSIZE >> 9, user_data_buffer)) {
			printf("read flash error\n");
			return 0;
		}

		user_data_head = (USER_DATA_HEAR *)user_data_buffer;
		user_data_p = (USER_PRIVATE_DATA *)(user_data_buffer + sizeof(USER_DATA_HEAR));
		
		if (strncmp(user_data_head->magic_name, MAGIC, 5)) {
			printf("the (user) private space\n");
			return 0;
		}
		
		if (user_data_head->count > 0) {
			printf("count = %d\n", user_data_head->count);
			for (j = 0; j < user_data_head->count; j++) {
				printf("%s = %s\n", user_data_p->name, user_data_p->value);
				user_data_p++;
			}
			return 0;
		}
		printf("have not data\n");
		return 0;

	}
	printf("the part isn't exist\n");
	return 0;
}

int do_check_userdata (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc > 1)
	{
		printf("error: <command>\n");
		return 0;
	}
	user_data_list();
	return 0;
}

U_BOOT_CMD(
	check_userdata,	1,	1,	do_check_userdata,
	"check user data",
	"<command>\n"
);

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
int update_user_data(void)
{
	int i, j;
	unsigned int part_offset;					//分区的地址偏移量
	unsigned int part_size;						//分区的大小
	unsigned int user_data_offset;				//用户存放数据地址的偏移量
	char user_data_buffer[USER_DATA_MAXSIZE];	//
	USER_PRIVATE_DATA *user_data_p = NULL;
	USER_DATA_HEAR *user_data_head = NULL;
#ifdef CONFIG_SUNXI_SECURE_STORAGE
	int data_len;
	int ret;
	char buffer[512];
	int updata_data_num = 0;
#endif

	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
	{
		return 0;
	}
	
	check_user_data();											//从env中检测用户的环境变量
	
#ifdef CONFIG_SUNXI_SECURE_STORAGE
	if(!sunxi_secure_storage_init())
	{	
		memset(buffer, 0, 512);
		for (i = 0; i < USER_DATA_NUM; i++) 
		{	
			ret = sunxi_secure_object_read(USER_DATA_NAME[i], buffer, 512, &data_len);
			if(!ret && data_len < 512) 
			{
				//sunxi_dump(buffer, data_len);
				setenv(USER_DATA_NAME[i], buffer);
				printf("updataed %s = %s\n", USER_DATA_NAME[i], buffer);					
				memset(buffer, 0, 512);
				updata_data_num++;
			}
		}
	}
	if (updata_data_num)
	{
		return 0;	
	}
#endif
		
	part_size = sunxi_partition_get_size_byname(PART_NAME);
	if (part_size > 0) {
		part_offset = sunxi_partition_get_offset_byname(PART_NAME);
		user_data_offset = part_offset + part_size - (USER_DATA_MAXSIZE >> 9);		//获得用户存放数据地址的偏移量
		if (!sunxi_flash_read(user_data_offset, USER_DATA_MAXSIZE >> 9, user_data_buffer)) {
			printf("read flash error\n");
			return 0;
		}
		
		user_data_head = (USER_DATA_HEAR *)user_data_buffer;
		user_data_p = (USER_PRIVATE_DATA *)(user_data_buffer + sizeof(USER_DATA_HEAR));
	
		if (strncmp(user_data_head->magic_name, MAGIC, 5)) { 				//校验数据是否有效
			printf("the user data'magic is bad\n");
			return 0;
		}
	
		if (user_data_head->count > 0) {
			for (i = 0; i < USER_DATA_NUM; i++) {
				user_data_p = (USER_PRIVATE_DATA *)(user_data_buffer + sizeof(USER_DATA_HEAR));
				for (j = 0; j < user_data_head->count && j < USER_DATA_PARAMETER_MAX_COUNT; j++) {
					if (!strcmp(USER_DATA_NAME[i], user_data_p->name))
					{
						setenv(user_data_p->name, user_data_p->value);
						printf("updataed %s = %s\n", user_data_p->name, user_data_p->value);
					}
					user_data_p++;
				}
			}
			return 0;
		}
		printf("not the user data to updata\n");
		return 0;
	}
	printf("the %s part isn't exist\n", PART_NAME);
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
*    note          :		 得到用户的环境变量
*
*
************************************************************************************************************
*/
void check_user_data(void)
{
	char *command_p  =NULL;
	char temp_name[32] = {'\0'};
	int i, j;

	//command_p = getenv("boot_base");  //合并nand和mmc启动参数后的方法，暂时不用
	if((uboot_spare_head.boot_data.storage_type == 1) || (uboot_spare_head.boot_data.storage_type == 2))
	{
		command_p = getenv("setargs_mmc");
	}
	else
	{
		command_p = getenv("setargs_nand");
	}
	//printf("cmd line = %s\n", command_p);
	if (!command_p) {
		printf("cann't get the boot_base from the env\n");
		return ;
	}

	while (*command_p != '\0' && *command_p != ' ') {		//过滤第一个环境变量
		command_p++;
	}
	command_p++;
	while (*command_p == ' ') {								//过滤多余的空格
		command_p++;
	}
	while (*command_p != '\0' && *command_p != ' ') { 		//过滤第二个环境变量
		command_p++;
	}
	command_p++;
	while (*command_p == ' ') {
		command_p++;
	}

	USER_DATA_NUM =0;
	while (*command_p != '\0') {
		i = 0;
		while (*command_p != '=') {
			temp_name[i++] = *command_p;
			command_p++;
		}
		temp_name[i] = '\0';
		if (i != 0) {
			for (j = 0; j < sizeof(IGNORE_ENV_VARIABLE) / sizeof(int); j++) {
				if (!strcmp(IGNORE_ENV_VARIABLE[j], temp_name)) {			//查词典库，排除系统的环境变量，得到用户的数据
					break;
				}
			}
			if (j >= sizeof(IGNORE_ENV_VARIABLE) / sizeof(int)) {
				if (!strcmp(temp_name, "mac_addr")) {						//处理mac_addr和mac不相等的情况（特殊情况）
					strcpy(USER_DATA_NAME[USER_DATA_NUM], "mac");
				}
				else {
					strcpy(USER_DATA_NAME[USER_DATA_NUM], temp_name);
				}
				USER_DATA_NUM++;
			}
		}
		while (*command_p != '\0' && *command_p != ' ') {					//下一个变量
			command_p++;
		}
		while (*command_p == ' ') {
			command_p++;
		}
	}
/*
	printf("USER_DATA_NUM = %d\n", USER_DATA_NUM);
	for (i = 0; i < USER_DATA_NUM; i++) {
		printf("user data = %s\n", USER_DATA_NAME[i]);
	}
*/
}

