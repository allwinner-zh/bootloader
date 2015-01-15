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
#include <mmc.h>
#include <sunxi_nand.h>
#include <boot_type.h>
#include "sunxi_flash.h"
#include "sys_config.h"
#include "sys_partition.h"
#ifdef CONFIG_SUNXI_SPINOR
#include <asm/arch/spinor.h>
#endif


int sunxi_flash_init_uboot(int verbose);
extern int sunxi_card_fill_boot0_magic(void);
/*
************************************************************************************************************
*
*                                             function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/

static int
sunxi_null_op(unsigned int start_block, unsigned int nblock, void *buffer){
	return 0;
}

static int
sunxi_null_erase(int erase, void *mbr_buffer)
{
	return 0;
}

static uint
sunxi_null_size(void){
	return 0;
}

static int
sunxi_null_init(int flag){
	return -1;
}

static int
sunxi_null_exit(int force){
	return -1;
}

static int
sunxi_null_flush(void){
	return 0;
}

static int
sunxi_null_force_erase(void){
    return 0;
}

#ifdef CONFIG_SUNXI_SPINOR
static int
sunxi_null_datafinish(void){
	return 0;
}
#endif


/*
************************************************************************************************************
*
*                                             function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/

int (* sunxi_flash_init_pt)(int stage) = sunxi_null_init;
int (* sunxi_flash_read_pt) (uint start_block, uint nblock, void *buffer) = sunxi_null_op;
//int (* sunxi_flash_read_sequence) (uint start_block, uint nblock, void *buffer) = sunxi_null_op;
int (* sunxi_flash_write_pt)(uint start_block, uint nblock, void *buffer) = sunxi_null_op;
uint (* sunxi_flash_size_pt)(void) = sunxi_null_size;
int (* sunxi_flash_exit_pt) (int force) = sunxi_null_exit;
int (* sunxi_flash_flush_pt) (void) = sunxi_null_flush;
int (* sunxi_flash_phyread_pt) (unsigned int start_block, unsigned int nblock, void *buffer) = sunxi_null_op;
int (* sunxi_flash_phywrite_pt)(unsigned int start_block, unsigned int nblock, void *buffer) = sunxi_null_op;

int (* sunxi_sprite_init_pt)(int stage) = sunxi_null_init;
int (* sunxi_sprite_read_pt) (uint start_block, uint nblock, void *buffer) = sunxi_null_op;
int (* sunxi_sprite_write_pt)(uint start_block, uint nblock, void *buffer) = sunxi_null_op;
int (* sunxi_sprite_erase_pt)(int erase, void *mbr_buffer) = sunxi_null_erase;
uint (* sunxi_sprite_size_pt)(void) = sunxi_null_size;
int (* sunxi_sprite_exit_pt) (int force) = sunxi_null_exit;
int (* sunxi_sprite_flush_pt)(void) = sunxi_null_flush;
int (* sunxi_sprite_force_erase_pt)(void)  = sunxi_null_force_erase;
int (* sunxi_sprite_phyread_pt) (unsigned int start_block, unsigned int nblock, void *buffer) = sunxi_null_op;
int (* sunxi_sprite_phywrite_pt)(unsigned int start_block, unsigned int nblock, void *buffer) = sunxi_null_op;
#ifdef CONFIG_SUNXI_SPINOR
int (* sunxi_sprite_datafinish_pt) (void) = sunxi_null_datafinish;
#endif


static struct mmc *mmc_boot, *mmc_sprite;
/*
************************************************************************************************************
*
*                                             function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/


/*
************************************************************************************************************
*
*                                             function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/
static int
sunxi_flash_mmc_read(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("mmcboot read: start 0x%x, sector 0x%x\n", start_block, nblock);

	return mmc_boot->block_dev.block_read(mmc_boot->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
					nblock, buffer);
}

//int
//sunxi_mmc_read_sequence(unsigned int start_block, unsigned int nblock, void *buffer){

//	int status;

//	status = mmc_boot->block_dev.block_int_read(mmc_boot->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
//					nblock, buffer);

//	return status;
//}


static int
sunxi_flash_mmc_write(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("mmcboot write: start 0x%x, sector 0x%x\n", start_block, nblock);

	return mmc_boot->block_dev.block_write(mmc_boot->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
					nblock, buffer);
}

static uint
sunxi_flash_mmc_size(void){

	return mmc_boot->block_dev.lba;
}

static int
sunxi_flash_mmc_init(int stage){
	return 0;
}

static int
sunxi_flash_mmc_exit(int force){
	return 0;
}

int sunxi_flash_mmc_phyread(unsigned int start_block, unsigned int nblock, void *buffer)
{
	return mmc_boot->block_dev.block_read_mass_pro(mmc_boot->block_dev.dev, start_block, nblock, buffer);
}

int sunxi_flash_mmc_phywrite(unsigned int start_block, unsigned int nblock, void *buffer)
{
	return mmc_boot->block_dev.block_write_mass_pro(mmc_boot->block_dev.dev, start_block, nblock, buffer);
}

int sunxi_flash_mmc_secread( int item, unsigned char *buf, unsigned int nblock)
{
	return mmc_boot->block_dev.block_read_secure(2, item, (u8 *)buf, nblock);
}

int sunxi_flash_mmc_secwrite( int item, unsigned char *buf, unsigned int nblock)
{
	return mmc_boot->block_dev.block_write_secure(2, item, (u8 *)buf, nblock);
}

int sunxi_sprite_mmc_secwrite(int item ,unsigned char *buf,unsigned int nblock)
{
	if(mmc_sprite->block_dev.block_write_secure(2, item, (u8 *)buf, nblock) >=0)
        return 0;
    else
        return -1;
}

int sunxi_sprite_mmc_secread(int item ,unsigned char *buf,unsigned int nblock)
{
    if(mmc_sprite->block_dev.block_read_secure(2, item, (u8 *)buf, nblock) >=0)
        return 0;
    else
        return -1;
}


#ifdef CONFIG_SUNXI_SPINOR
static int
sunxi_flash_spinor_read(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("spinor read: start 0x%x, sector 0x%x\n", start_block, nblock);

    return spinor_read(start_block + CONFIG_SPINOR_LOGICAL_OFFSET, nblock, buffer);
}

//static int
//sunxi_flash_spinor_write(unsigned int start_block, unsigned int nblock, void *buffer)
//{
//	debug("spinor write: start 0x%x, sector 0x%x\n", start_block, nblock);

//	return spinor_write(start_block + CONFIG_SPINOR_LOGICAL_OFFSET, nblock, buffer);
//}

static int
sunxi_sprite_spinor_write(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("burn spinor write: start 0x%x, sector 0x%x\n", start_block, nblock);

	return spinor_sprite_write(start_block, nblock, buffer);
}


static uint
sunxi_flash_spinor_size(void){

	return spinor_size();
}

static int sunxi_flash_spinor_erase(int erase,void *mbr_buffer)
{
	return spinor_erase_all_blocks(erase);
}

static int
sunxi_flash_spinor_init(int stage)
{
	return spinor_init(stage);
}

static int
sunxi_flash_spinor_exit(int force)
{
	return spinor_exit(force);
}

static int
sunxi_flash_spinor_datafinish(void)
{
	return spinor_datafinish();
}

static int
sunxi_flash_spinor_flush(void)
{
	return spinor_flush_cache();
}
#endif


/*
************************************************************************************************************
*
*                                             function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/
static int
sunxi_sprite_mmc_read(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("mmcsprite read: start 0x%x, sector 0x%x\n", start_block, nblock);

	return mmc_sprite->block_dev.block_read(mmc_sprite->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
					nblock, buffer);
}

static int
sunxi_sprite_mmc_write(unsigned int start_block, unsigned int nblock, void *buffer)
{
	debug("mmcsprite write: start 0x%x, sector 0x%x\n", start_block, nblock);

	return mmc_sprite->block_dev.block_write(mmc_sprite->block_dev.dev, start_block + CONFIG_MMC_LOGICAL_OFFSET,
					nblock, buffer);
}

static int
sunxi_sprite_mmc_erase(int erase, void *mbr_buffer)
{
	return card_erase(erase, mbr_buffer);
}

static uint
sunxi_sprite_mmc_size(void){

	return mmc_sprite->block_dev.lba;
}

static int
sunxi_sprite_mmc_init(int stage){
	return 0;
}

static int
sunxi_sprite_mmc_exit(int force){
	return 0;
}

int sunxi_sprite_mmc_phyread(unsigned int start_block, unsigned int nblock, void *buffer)
{
	return mmc_sprite->block_dev.block_read_mass_pro(mmc_sprite->block_dev.dev, start_block, nblock, buffer);
}

int sunxi_sprite_mmc_phywrite(unsigned int start_block, unsigned int nblock, void *buffer)
{
	return mmc_sprite->block_dev.block_write_mass_pro(mmc_sprite->block_dev.dev, start_block, nblock, buffer);
}

int sunxi_sprite_mmc_phyerase(unsigned int start_block, unsigned int nblock, void *skip)
{
	if (nblock == 0) {
		printf("%s: @nr is 0, erase from @from to end\n", __FUNCTION__);
		nblock = mmc_sprite->block_dev.lba - start_block - 1;
	}
	return mmc_sprite->block_dev.block_mmc_erase(mmc_sprite->block_dev.dev, start_block, nblock, skip);
}

int sunxi_sprite_mmc_phywipe(unsigned int start_block, unsigned int nblock, void *skip)
{
	if (nblock == 0) {
		printf("%s: @nr is 0, wipe from @from to end\n", __FUNCTION__);
		nblock = mmc_sprite->block_dev.lba - start_block - 1;
	}
	return mmc_sprite->block_dev.block_secure_wipe(mmc_sprite->block_dev.dev, start_block, nblock, skip);
}

int sunxi_sprite_mmc_force_erase(void)
{
    return 0;
}

#ifdef CONFIG_SUNXI_SPINOR
int sunxi_sprite_setdata_finish(void)
{
	return sunxi_sprite_datafinish_pt();
}
#endif


/*
************************************************************************************************************
*
*											  function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/

int sunxi_flash_read (uint start_block, uint nblock, void *buffer)
{
	debug("sunxi flash read : start %d, sector %d\n", start_block, nblock);
	return sunxi_flash_read_pt(start_block, nblock, buffer);
}

int sunxi_flash_write(uint start_block, uint nblock, void *buffer)
{
	debug("sunxi flash write : start %d, sector %d\n", start_block, nblock);
	return sunxi_flash_write_pt(start_block, nblock, buffer);
}

uint sunxi_flash_size(void)
{
	return sunxi_flash_size_pt();
}

int sunxi_flash_exit(int force)
{
    return sunxi_flash_exit_pt(force);
}

int
sunxi_flash_flush(void)
{
	return sunxi_flash_flush_pt();
}

int sunxi_flash_phyread (uint start_block, uint nblock, void *buffer)
{
	return sunxi_flash_phyread_pt(start_block, nblock, buffer);
}

int sunxi_flash_phywrite(uint start_block, uint nblock, void *buffer)
{
	return sunxi_flash_phywrite_pt(start_block, nblock, buffer);
}




int sunxi_sprite_init(int stage)
{
	return sunxi_sprite_init_pt(stage);
}

int sunxi_sprite_read (uint start_block, uint nblock, void *buffer)
{
	return sunxi_sprite_read_pt(start_block, nblock, buffer);
}

int sunxi_sprite_write(uint start_block, uint nblock, void *buffer)
{
	return sunxi_sprite_write_pt(start_block, nblock, buffer);
}

int sunxi_sprite_erase(int erase, void *mbr_buffer)
{
	return sunxi_sprite_erase_pt(erase, mbr_buffer);
}

uint sunxi_sprite_size(void)
{
	return sunxi_sprite_size_pt();
}

int sunxi_sprite_exit(int force)
{
    return sunxi_sprite_exit_pt(force);
}

int
sunxi_sprite_flush(void)
{
	return sunxi_sprite_flush_pt();
}

int sunxi_sprite_phyread (uint start_block, uint nblock, void *buffer)
{
	return sunxi_sprite_phyread_pt(start_block, nblock, buffer);
}

int sunxi_sprite_phywrite(uint start_block, uint nblock, void *buffer)
{
	return sunxi_sprite_phywrite_pt(start_block, nblock, buffer);
}

int sunxi_sprite_force_erase(void)
{
    return sunxi_sprite_force_erase_pt();
}
/*
************************************************************************************************************
*
*											  function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/

int sunxi_flash_handle_init(void)
{
    int workmode;
	int storage_type;
	int card_no;
//	long long flash_size;

//	uboot_spare_head.boot_data.storage_type = 0;
//	if(uboot_spare_head.boot_data.storage_type)
//		uboot_spare_head.boot_data.work_mode = WORK_MODE_CARD_PRODUCT;//WORK_MODE_CARD_PRODUCT;

    workmode = uboot_spare_head.boot_data.work_mode;
#if 1
    printf("workmode = %d\n", workmode);
    debug("storage type = %d\n", uboot_spare_head.boot_data.storage_type);
#endif

	if(workmode == WORK_MODE_BOOT || workmode == WORK_MODE_SPRITE_RECOVERY)
	{
	    int nand_used, sdc0_used, sdc2_used, sdc_detmode=3;

		storage_type = uboot_spare_head.boot_data.storage_type;
		debug("storage type = %d\n", storage_type);
        if((storage_type == 1) || (storage_type == 2))
		{
		    if(2 == storage_type)
			{
			    nand_used = 0;
			    sdc2_used = 1;

				script_parser_patch("nand0_para", "nand0_used", &nand_used, 1);
		        script_parser_patch("nand1_para", "nand1_used", &nand_used, 1);
		        script_parser_patch("mmc2_para",  "sdc_used",   &sdc2_used, 1);
		        script_parser_patch("mmc2_para",  "sdc_detmode", &sdc_detmode, 1);
			}
			else
			{
				//nand_used  = 0;
			    sdc0_used  = 1;
			    //sdc2_used  = 0;

				//script_parser_patch("nand0_para", "nand0_used", &nand_used, 1);
		        //script_parser_patch("nand1_para", "nand1_used", &nand_used, 1);
		        script_parser_patch("mmc0_para",  "sdc_used",   &sdc0_used, 1);
		        script_parser_patch("mmc0_para",  "sdc_detmode", &sdc_detmode, 1);
		        //script_parser_patch("mmc2_para",  "sdc_used",   &sdc2_used, 1);
			}
			card_no = (storage_type == 1)?0:2;
			printf("MMC:	 %d\n", card_no);
			board_mmc_set_num(card_no);
			debug("set card number\n");
			board_mmc_pre_init(card_no);
			debug("begin to find mmc\n");
			mmc_boot = find_mmc_device(card_no);
			if(!mmc_boot){
				printf("fail to find one useful mmc card\n");
				return -1;
			}
			debug("try to init mmc\n");
			if (mmc_init(mmc_boot)) {
				puts("MMC init failed\n");
				return  -1;
			}
			debug("mmc %d init ok\n", card_no);

			sunxi_flash_read_pt  = sunxi_flash_mmc_read;
			sunxi_flash_write_pt = sunxi_flash_mmc_write;
			sunxi_flash_size_pt  = sunxi_flash_mmc_size;
			sunxi_flash_exit_pt  = sunxi_flash_mmc_exit;

			sunxi_flash_phyread_pt  = sunxi_flash_mmc_phyread;
			sunxi_flash_phywrite_pt = sunxi_flash_mmc_phywrite;
			sunxi_sprite_phyread_pt  = sunxi_flash_mmc_phyread;
			sunxi_sprite_phywrite_pt = sunxi_flash_mmc_phywrite;
		}
		else
		{
			return -1;
		}

		sunxi_sprite_read_pt  = sunxi_flash_read_pt;
		sunxi_sprite_write_pt = sunxi_flash_write_pt;

		sunxi_flash_init_uboot(0);
		script_parser_patch("target", "storage_type", &storage_type, 1);

		tick_printf("sunxi flash init ok\n");
#if defined(CONFIG_ARCH_SUN8IW1P1)
		if((storage_type == 0) || (storage_type == 2))	//如果是A31非卡0启动，则需要跳转检测卡0
		{
			sunxi_card_probe_mmc0_boot();
		}
#endif
        if((storage_type == 0) ||(storage_type == 2))
        {
            int sprite_next_work = 0;
            script_parser_fetch("card_boot","next_work",&sprite_next_work,1);
            if(sprite_next_work == SUNXI_UPDATA_NEXT_ACTION_SPRITE_TEST)
                sunxi_card_fill_boot0_magic();
        }
	}
	else if((workmode & WORK_MODE_PRODUCT) || (workmode == 0x30))		/* 量产模式 */
	{
		if(0)
		{
			;
		}
#ifdef CONFIG_SUNXI_SPINOR
		else if(!try_spi_nor(0)) //burn nor
		{
			printf("try nor successed \n");
			sunxi_sprite_init_pt  = sunxi_flash_spinor_init;
			sunxi_sprite_exit_pt  = sunxi_flash_spinor_exit;
			sunxi_sprite_read_pt  = sunxi_flash_spinor_read;
			sunxi_sprite_write_pt = sunxi_sprite_spinor_write;
			sunxi_sprite_erase_pt = sunxi_flash_spinor_erase;
			sunxi_sprite_size_pt  = sunxi_flash_spinor_size;
			sunxi_sprite_flush_pt = sunxi_flash_spinor_flush;
			sunxi_sprite_datafinish_pt = sunxi_flash_spinor_datafinish;
			printf("sunxi sprite has installed spi function\n");
			uboot_spare_head.boot_data.storage_type = 3;
		}
#endif
        else                                   /* burn sdcard 2 */
		{
		    board_mmc_pre_init(2);
			mmc_sprite = find_mmc_device(2);
			if(!mmc_sprite){
				printf("fail to find one useful mmc card\n");
				return -1;
			}

			if (mmc_init(mmc_sprite)) {
				puts("MMC init failed\n");
				return  -1;
			}
			sunxi_sprite_init_pt  = sunxi_sprite_mmc_init;
            sunxi_sprite_exit_pt  = sunxi_sprite_mmc_exit;
			sunxi_sprite_read_pt  = sunxi_sprite_mmc_read;
			sunxi_sprite_write_pt = sunxi_sprite_mmc_write;
			sunxi_sprite_erase_pt = sunxi_sprite_mmc_erase;
			sunxi_sprite_size_pt  = sunxi_sprite_mmc_size;
			sunxi_sprite_phyread_pt  = sunxi_sprite_mmc_phyread;
			sunxi_sprite_phywrite_pt = sunxi_sprite_mmc_phywrite;
            sunxi_sprite_force_erase_pt = sunxi_sprite_mmc_force_erase;
			debug("sunxi sprite has installed sdcard2 function\n");
			uboot_spare_head.boot_data.storage_type = 2;
	    }

		if((workmode == WORK_MODE_CARD_PRODUCT) || (workmode == 0x30))      //sdcard burn mode
		{
            board_mmc_pre_init(0);
			mmc_boot = find_mmc_device(0);
			if(!mmc_boot)
			{
				printf("fail to find one useful mmc card\n");
				return -1;
			}

			if (mmc_init(mmc_boot))
			{
				puts("MMC sprite init failed\n");
				return  -1;
			}
			else
			{
				puts("mmc init ok\n");
			}
			sunxi_flash_init_pt  = sunxi_flash_mmc_init;
			sunxi_flash_read_pt  = sunxi_flash_mmc_read;
			sunxi_flash_write_pt = sunxi_flash_mmc_write;
			sunxi_flash_size_pt  = sunxi_flash_mmc_size;
			sunxi_flash_phyread_pt  = sunxi_flash_mmc_phyread;
			sunxi_flash_phywrite_pt = sunxi_flash_mmc_phywrite;
			sunxi_flash_exit_pt  = sunxi_flash_mmc_exit;
		}
		sunxi_flash_init_uboot(0);
	}
	else if(workmode & WORK_MODE_UPDATE)		/* 升级模式 */
	{
	}
	else   /* undefined mode */
	{
	}

	return 0;
}
/*
************************************************************************************************************
*
*											  function
*
*
*
*
*
*
*
*
************************************************************************************************************
*/

static block_dev_desc_t 	sunxi_flash_blk_dev;

block_dev_desc_t *sunxi_flash_get_dev(int dev)
{
	sunxi_flash_blk_dev.dev = dev;
	sunxi_flash_blk_dev.lba = sunxi_partition_get_size(dev);

	return ((block_dev_desc_t *) & sunxi_flash_blk_dev);
}

unsigned long  sunxi_flash_part_read(int dev_num, unsigned long start, lbaint_t blkcnt, void *dst)
{
	uint offset;

	offset = sunxi_partition_get_offset(dev_num);
	if(!offset)
	{
		printf("sunxi flash error: cant get part %d offset\n", dev_num);

		return 0;
	}
	start += offset;
#ifdef DEBUG
    printf("nand try to read from %x, length %x block\n", (int )start, (int )blkcnt);
#endif
#ifdef DEBUG
    int ret;

    ret = sunxi_flash_read((uint)start, (uint )blkcnt, dst);
    printf("read result = %d\n", ret);

	return ret;
#else
    return sunxi_flash_read((uint)start, (uint )blkcnt, dst);
#endif
}

unsigned long  sunxi_flash_part_write(int dev_num, unsigned long start, lbaint_t blkcnt, const void *dst)
{
	uint offset;

	offset = sunxi_partition_get_offset(dev_num);
	if(!offset)
	{
		printf("sunxi flash error: cant get part %d offset\n", dev_num);

		return 0;
	}
	start += offset;
#ifdef DEBUG
    printf("nand try to write from %x, length %x block\n", (int )start, (int )blkcnt);
#endif
#ifdef DEBUG
	int ret;

	ret = sunxi_flash_write((uint)start, (uint )blkcnt, (void *)dst);
	printf("read result = %d\n", ret);

	return ret;
#else
	return sunxi_flash_write((uint)start, (uint )blkcnt, (void *)dst);
#endif
}


int sunxi_flash_init_uboot(int verbose)
{
#ifdef DEBUG
    puts("sunxi flash init uboot\n");
#endif
	sunxi_flash_blk_dev.if_type = IF_TYPE_SUNXI_FLASH;
	sunxi_flash_blk_dev.part_type = PART_TYPE_DOS;
	sunxi_flash_blk_dev.dev = 0;
	sunxi_flash_blk_dev.lun = 0;
	sunxi_flash_blk_dev.type = 0;

	/* FIXME fill in the correct size (is set to 32MByte) */
	sunxi_flash_blk_dev.blksz = 512;
	sunxi_flash_blk_dev.lba = 0;
	sunxi_flash_blk_dev.removable = 0;
	sunxi_flash_blk_dev.block_read = sunxi_flash_part_read;
	sunxi_flash_blk_dev.block_write = sunxi_flash_part_write;

	return 0;
}


