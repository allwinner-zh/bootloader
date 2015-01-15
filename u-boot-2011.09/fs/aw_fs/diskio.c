/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/
#include <common.h>
#include <mmc.h>
#include "diskio.h"		/* FatFs lower layer API */
#ifndef CONFIG_ALLWINNER
#include "usbdisk.h"	/* Example: USB drive control */
#include "atadrive.h"	/* Example: ATA drive control */
#include "sdcard.h"		/* Example: MMC/SDC contorl */
#endif
#include <sys_partition.h>
/* Definitions of physical drive number for each media */
#define ATA		0
#define MMC		1
#define USB		2

extern char PART_NAME[3][16];

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
#ifndef CONFIG_ALLWINNER
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case ATA :
		result = ATA_disk_initialize();

		// translate the reslut code here

		return stat;

	case MMC :
		result = MMC_disk_initialize();

		// translate the reslut code here

		return stat;

	case USB :
		result = USB_disk_initialize();

		// translate the reslut code here
		return stat;
	}
	return STA_NOINIT;
#else
	return 0;
#endif
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
#ifndef CONFIG_ALLWINNER
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case ATA :
		result = ATA_disk_status();

		// translate the reslut code here

		return stat;

	case MMC :
		result = MMC_disk_status();

		// translate the reslut code here

		return stat;

	case USB :
		result = USB_disk_status();

		// translate the reslut code here

		return stat;
	}
	return STA_NOINIT;
#else
	return 0;
#endif
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read_fs (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	DWORD count		/* Number of sectors to read (1..128) */
)
{
#ifndef CONFIG_ALLWINNER
	DRESULT res;
#endif
	int result;
	unsigned int start_block;
#ifndef CONFIG_ALLWINNER
	switch (pdrv) {
	case ATA :
		// translate the arguments here

		result = ATA_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	}
	return RES_PARERR;
#else
//	result = sunxi_test_mmc_read(sector+0x12000, count, buff);
	start_block = sunxi_partition_get_offset_byname(PART_NAME[pdrv]);
	if (!start_block)
	{
		printf("[disk_read_fs] no the partition\n");
		return RES_ERROR;
	}
//	printf("read part %s\n", PART_NAME[pdrv]);
//	result = sunxi_test_nand_read((unsigned int)(sector+start_block),(unsigned int)count,buff);
	result = sunxi_flash_read(sector+start_block,count, buff);
	if(!result)
	{
		printf("read all error: start=%lx, addr=0x%x count=0x%x\n", sector, (unsigned int)buff,(unsigned int)count);

		return 1;
	}
	return RES_OK;
#endif
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#define _USE_WRITE 1
#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	DWORD count			/* Number of sectors to write (1..128) */
)
{
//	DRESULT res;
	int result;
	int start_block;
#ifndef CONFIG_ALLWINNER
	switch (pdrv) {
	case ATA :
		// translate the arguments here

		result = ATA_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}
	return RES_PARERR;
#else
	start_block = sunxi_partition_get_offset_byname(PART_NAME[pdrv]);

	if (!start_block)
	{
		printf("[disk_write] no the partition\n");
		return RES_ERROR;
	}
//	printf("write part %s\n", PART_NAME[pdrv]);
//	result = sunxi_test_nand_write(sector+start_block,count,buff);
	result = sunxi_flash_write(sector+start_block,count,(void *)buff);
	if(!result){
		printf("[%s] err in sector %lx count %x buff %x\n",__func__,sector,(unsigned int)count,(unsigned int)buff);
		return RES_ERROR;
	}
	return RES_OK;
#endif
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
#ifndef CONFIG_ALLWINNER
	DRESULT res;
	int result;

	switch (pdrv) {
	case ATA :
		// pre-process here

		result = ATA_disk_ioctl(cmd, buff);

		// post-process here

		return res;

	case MMC :
		// pre-process here

		result = MMC_disk_ioctl(cmd, buff);

		// post-process here

		return res;

	case USB :
		// pre-process here

		result = USB_disk_ioctl(cmd, buff);

		// post-process here

		return res;
	}
	return RES_PARERR;
#else
	return RES_OK;
#endif
}
#endif
