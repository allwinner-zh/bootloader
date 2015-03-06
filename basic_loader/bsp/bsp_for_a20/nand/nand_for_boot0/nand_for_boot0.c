
/*
 * (C) Copyright 2007-2015
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

#include "types.h"
#include "bsp_nand.h"
//#include "nand_oal.h"
//#include "nand_osal.h"
//#include "nand_simple.h"
//#include "nand_physic.h"
//#include "nand_ndfc.h"

extern void NAND_Print( const char * str, ...);
extern BOOT_NandGetPara(boot_nand_para_t *nand_info);
/*
************************************************************************************************************************
*                       GET FLASH INFO
*
*Description: get some info about nand flash;
*
*Arguments  : *param     the stucture with info.
*
*Return     :   = SUCESS  get ok;
*               = FAIL    get fail.
************************************************************************************************************************
*/
__s32 NFB_GetFlashInfo(boot_flash_info_t *param)
{
    boot_nand_para_t nand_info;

    BOOT_NandGetPara(&nand_info);

	param->chip_cnt	 		= nand_info.ChipCnt;
	param->blk_cnt_per_chip = nand_info.BlkCntPerDie * nand_info.DieCntPerChip;
	param->blocksize 		= nand_info.SectorCntPerPage * nand_info.PageCntPerPhyBlk;
	param->pagesize 		= nand_info.SectorCntPerPage;
	param->pagewithbadflag  = 0 ;   // fix page 0 as bad flag page index

	return 0;
}


/*
************************************************************************************************************************
*                       INIT NAND FLASH
*
*Description: initial nand flash,request hardware resources;
*
*Arguments  : void.
*
*Return     :   = SUCESS  initial ok;
*               = FAIL    initial fail.
************************************************************************************************************************
*/
__s32 NFB_PhyInit(void)
{
	__s32 ret;

	ret = PHY_Init();
	if (ret)
	{
		NAND_Print("NB0 : nand phy init fail\n");
		return ret;
	}

	ret = BOOT_AnalyzeNandSystem();
	if (ret)
	{
		NAND_Print("NB0 : nand scan fail\n");
		return ret;
	}

	NAND_Print("NB0 : nand phy init ok\n");


	return(PHY_ChangeMode(1));
}

/*
************************************************************************************************************************
*                       RELEASE NAND FLASH
*
*Description: release  nand flash and free hardware resources;
*
*Arguments  : void.
*
*Return     :   = SUCESS  release ok;
*               = FAIL    release fail.
************************************************************************************************************************
*/
__s32 NFB_PhyExit(void)
{
	PHY_Exit();

	/* close nand flash bus clock gate */
	//NAND_CloseAHBClock();

	return 0;
}

/*
************************************************************************************************************************
*                       READ ONE SINGLE PAGE
*
*Description: read one page data from nand based on single plane;
*
*Arguments  : *readop - the structure with physical address in nand and data buffer
*
*Return     :   = SUCESS  read ok;
*               = FAIL    read fail.
************************************************************************************************************************
*/
__s32 NFB_PhyRead (struct boot_physical_param *readop)
{
	return(PHY_SimpleRead (readop));
}
