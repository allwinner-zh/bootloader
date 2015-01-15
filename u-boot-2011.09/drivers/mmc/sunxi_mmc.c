/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Aaron <leafy.myeh@allwinnertech.com>
 *
 * MMC driver for allwinner sunxi platform.
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
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/mmc.h>
#include <asm/arch/timer.h>
#include <malloc.h>
#include <mmc.h>
#include <smc.h>
#include <sys_config.h>

#include "mmc_def.h"

DECLARE_GLOBAL_DATA_PTR;
//#define SUNXI_MMCDBG
//#undef SUNXI_MMCDBG
//#define MMCINFO(fmt...)	printf("[mmc]: "fmt)
#ifndef CONFIG_ARCH_SUN7I
#define MMC_REG_FIFO_OS		(0x200)
#else
#define MMC_REG_FIFO_OS		(0x100)
#endif

#ifdef SUNXI_MMCDBG
//#define MMCDBG(fmt...)	printf("[mmc]: "fmt)

static void dumphex32(char* name, char* base, int len)
{
	__u32 i;

	MMCMSG("dump %s registers:", name);
	for (i=0; i<len; i+=4) {
		if (!(i&0xf))
			MMCMSG("\n0x%p : ", base + i);
		MMCMSG("0x%08x ", smc_readl((uint)base + i));
	}
	MMCMSG("\n");
}

/*
static void dumpmmcreg(struct sunxi_mmc *reg)
{
	printf("dump mmc registers:\n");
	printf("gctrl     0x%08x\n", reg->gctrl     );
	printf("clkcr     0x%08x\n", reg->clkcr     );
	printf("timeout   0x%08x\n", reg->timeout   );
	printf("width     0x%08x\n", reg->width     );
	printf("blksz     0x%08x\n", reg->blksz     );
	printf("bytecnt   0x%08x\n", reg->bytecnt   );
	printf("cmd       0x%08x\n", reg->cmd       );
	printf("arg       0x%08x\n", reg->arg       );
	printf("resp0     0x%08x\n", reg->resp0     );
	printf("resp1     0x%08x\n", reg->resp1     );
	printf("resp2     0x%08x\n", reg->resp2     );
	printf("resp3     0x%08x\n", reg->resp3     );
	printf("imask     0x%08x\n", reg->imask     );
	printf("mint      0x%08x\n", reg->mint      );
	printf("rint      0x%08x\n", reg->rint      );
	printf("status    0x%08x\n", reg->status    );
	printf("ftrglevel 0x%08x\n", reg->ftrglevel );
	printf("funcsel   0x%08x\n", reg->funcsel   );
	printf("dmac      0x%08x\n", reg->dmac      );
	printf("dlba      0x%08x\n", reg->dlba      );
	printf("idst      0x%08x\n", reg->idst      );
	printf("idie      0x%08x\n", reg->idie      );
	printf("cbcr      0x%08x\n", reg->cbcr      );
	printf("bbcr      0x%08x\n", reg->bbcr      );
}
*/
#else
//#define MMCDBG(fmt...)
#define dumpmmcreg(fmt...)
#define  dumphex32(fmt...)
#endif /* SUNXI_MMCDBG */

#define BIT(x)				(1<<(x))
/* Struct for Intrrrupt Information */
#define SDXC_RespErr		BIT(1) //0x2
#define SDXC_CmdDone		BIT(2) //0x4
#define SDXC_DataOver		BIT(3) //0x8
#define SDXC_TxDataReq		BIT(4) //0x10
#define SDXC_RxDataReq		BIT(5) //0x20
#define SDXC_RespCRCErr		BIT(6) //0x40
#define SDXC_DataCRCErr		BIT(7) //0x80
#define SDXC_RespTimeout	BIT(8) //0x100
#define SDXC_ACKRcv			BIT(8)	//0x100
#define SDXC_DataTimeout	BIT(9)	//0x200
#define SDXC_BootStart		BIT(9)	//0x200
#define SDXC_DataStarve		BIT(10) //0x400
#define SDXC_VolChgDone		BIT(10) //0x400
#define SDXC_FIFORunErr		BIT(11) //0x800
#define SDXC_HardWLocked	BIT(12)	//0x1000
#define SDXC_StartBitErr		BIT(13)	//0x2000
#define SDXC_AutoCMDDone	BIT(14)	//0x4000
#define SDXC_EndBitErr		BIT(15)	//0x8000
#define SDXC_SDIOInt		BIT(16)	//0x10000
#define SDXC_CardInsert		BIT(30) //0x40000000
#define SDXC_CardRemove	BIT(31) //0x80000000
#define SDXC_IntErrBit		(SDXC_RespErr | SDXC_RespCRCErr | SDXC_DataCRCErr \
				| SDXC_RespTimeout | SDXC_DataTimeout | SDXC_FIFORunErr \
				| SDXC_HardWLocked | SDXC_StartBitErr | SDXC_EndBitErr)  //0xbfc2

/* IDMA status bit field */
#define SDXC_IDMACTransmitInt	BIT(0)
#define SDXC_IDMACReceiveInt	BIT(1)
#define SDXC_IDMACFatalBusErr	BIT(2)
#define SDXC_IDMACDesInvalid	BIT(4)
#define SDXC_IDMACCardErrSum	BIT(5)
#define SDXC_IDMACNormalIntSum	BIT(8)
#define SDXC_IDMACAbnormalIntSum BIT(9)
#define SDXC_IDMACHostAbtInTx	BIT(10)
#define SDXC_IDMACHostAbtInRx	BIT(10)
#define SDXC_IDMACIdle		(0U << 13)
#define SDXC_IDMACSuspend	(1U << 13)
#define SDXC_IDMACDESCRd	(2U << 13)
#define SDXC_IDMACDESCCheck	(3U << 13)
#define SDXC_IDMACRdReqWait	(4U << 13)
#define SDXC_IDMACWrReqWait	(5U << 13)
#define SDXC_IDMACRd		(6U << 13)
#define SDXC_IDMACWr		(7U << 13)
#define SDXC_IDMACDESCClose	(8U << 13)


#define MMC_CLK_400K					0
#define MMC_CLK_25M						1
#define MMC_CLK_50M						2
#define MMC_CLK_50MDDR				3
#define MMC_CLK_50MDDR_8BIT		4
#define MMC_CLK_100M					5
#define MMC_CLK_200M					6
#define MMC_CLK_MOD_NUM				7

struct sunxi_mmc_clk_dly {
	u32 mode;
	u32 oclk_dly;
	u32 sclk_dly;
};


#define MMC_CLK_SAMPLY_POINIT	8
struct sunxi_mmc_sdly_rty {
	u32 mode;
	u32 sclk_dly[MMC_CLK_SAMPLY_POINIT];
	u32 rty_cnt;
};

struct sunxi_mmc_des {
	u32			:1,
		dic		:1, /* disable interrupt on completion */
		last_des	:1, /* 1-this data buffer is the last buffer */
		first_des	:1, /* 1-data buffer is the first buffer,
						   0-data buffer contained in the next descriptor is 1st buffer */
		des_chain	:1, /* 1-the 2nd address in the descriptor is the next descriptor address */
		end_of_ring	:1, /* 1-last descriptor flag when using dual data buffer in descriptor */
				:24,
		card_err_sum	:1, /* transfer error flag */
		own		:1; /* des owner:1-idma owns it, 0-host owns it */
#if defined CONFIG_SUN4I
#define SDXC_DES_NUM_SHIFT 12
#define SDXC_DES_BUFFER_MAX_LEN	(1 << SDXC_DES_NUM_SHIFT)
	u32	data_buf1_sz	:13,
		data_buf2_sz	:13,
    				:6;
#elif defined CONFIG_ARCH_SUN7I
#define SDXC_DES_NUM_SHIFT 15
#define SDXC_DES_BUFFER_MAX_LEN	(1 << SDXC_DES_NUM_SHIFT)
	u32	data_buf1_sz	:16,
		data_buf2_sz	:16;
#else
#define SDXC_DES_NUM_SHIFT 15
#define SDXC_DES_BUFFER_MAX_LEN	(1 << SDXC_DES_NUM_SHIFT)
	u32	data_buf1_sz	:16,
		data_buf2_sz	:16;
#endif
	u32	buf_addr_ptr1;
	u32	buf_addr_ptr2;
};

struct sunxi_mmc_host {
	unsigned mmc_no;
	unsigned hclkbase;
#ifndef CONFIG_ARCH_SUN7I
	unsigned hclkrst;
#endif
	unsigned mclkbase;
	unsigned database;
#if defined(CONFIG_ARCH_SUN9IW1P1)
	unsigned commreg;
#endif
	unsigned fatal_err;
	unsigned mod_clk;
	struct sunxi_mmc *reg;
	struct sunxi_mmc_des* pdes;
	/*sample delay and output deley setting*/
	struct sunxi_mmc_clk_dly mmc_clk_dly[MMC_CLK_MOD_NUM];
	struct sunxi_mmc_sdly_rty mmc_clk_sdly_rty_tbl[MMC_CLK_MOD_NUM];
	u32 raw_int_bak ;
	u32 samply_mode;

};

/* support 4 mmc hosts */
struct mmc mmc_dev[4];
struct sunxi_mmc_host mmc_host[4];


void mmc_dump_errinfo(struct sunxi_mmc_host* smc_host, struct mmc_cmd *cmd)
{
	MMCINFO("smc %d err, cmd %d, %s%s%s%s%s%s%s%s%s%s\n",
		smc_host->mmc_no, cmd? cmd->cmdidx: -1,
		smc_host->raw_int_bak & SDXC_RespErr     ? " RE"     : "",
		smc_host->raw_int_bak & SDXC_RespCRCErr  ? " RCE"    : "",
		smc_host->raw_int_bak & SDXC_DataCRCErr  ? " DCE"    : "",
		smc_host->raw_int_bak & SDXC_RespTimeout ? " RTO"    : "",
		smc_host->raw_int_bak & SDXC_DataTimeout ? " DTO"    : "",
		smc_host->raw_int_bak & SDXC_DataStarve  ? " DS"     : "",
		smc_host->raw_int_bak & SDXC_FIFORunErr  ? " FE"     : "",
		smc_host->raw_int_bak & SDXC_HardWLocked ? " HL"     : "",
		smc_host->raw_int_bak & SDXC_StartBitErr ? " SBE"    : "",
		smc_host->raw_int_bak & SDXC_EndBitErr   ? " EBE"    : "",
		smc_host->raw_int_bak ==0  ? " STO"    : ""
		);
}




static int mmc_resource_init(int sdc_no)
{
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
	MMCDBG("init mmc %d resource\n", sdc_no);
	switch (sdc_no) {
		case 0:
			mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC0_BASE;
			mmchost->mclkbase = CCM_SDC0_SCLK_CTRL;
			break;
		case 1:
			mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC1_BASE;
			mmchost->mclkbase = CCM_SDC1_SCLK_CTRL;
			break;
		case 2:
			mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC2_BASE;
			mmchost->mclkbase = CCM_SDC2_SCLK_CTRL;
			break;
		case 3:
			mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC3_BASE;
			mmchost->mclkbase = CCM_SDC3_SCLK_CTRL;
			break;
		default:
			MMCINFO("Wrong mmc number %d\n", sdc_no);
			break;
	}
#ifdef CONFIG_ARCH_SUN9IW1P1
	mmchost->hclkbase = CCM_AHB0_GATE0_CTRL;
	mmchost->hclkrst  = CCM_AHB0_RST_REG0;
	mmchost->commreg  = SUNXI_MMC_COMMON_BASE + sdc_no*4;
#elif defined CONFIG_ARCH_SUN7I
	mmchost->hclkbase = CCM_AHB_GATE0_CTRL;
#else
	mmchost->hclkbase = CCM_AHB1_GATE0_CTRL;
	mmchost->hclkrst  = CCM_AHB1_RST_REG0;
#endif
	mmchost->database = (unsigned int)mmchost->reg + MMC_REG_FIFO_OS;
	mmchost->mmc_no = sdc_no;

#ifdef CONFIG_ARCH_SUN9IW1P1
	mmchost->mmc_clk_dly[MMC_CLK_25M].mode 			= MMC_CLK_25M;
	mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly 	= 0;
	mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly 	= 5;

	mmchost->mmc_clk_dly[MMC_CLK_50M].mode 			= MMC_CLK_50M;
	mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly 	= 5;
	mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly 	= 4;
#else
	/**init retry table**/
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].mode = MMC_CLK_400K;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[0]= 0;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[1]= 7;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[2]= 6;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[3]= 5;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[4]= 4;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[5]= 3;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[6]= 2;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[7]= 1;

	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].mode = MMC_CLK_25M;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[0]= 5;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[1]= 6;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[2]= 4;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[3]= 7;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[4]= 3;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[5]= 2;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[6]= 1;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[7]= 0;

	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].mode = MMC_CLK_50M;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[0]= 4;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[1]= 5;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[2]= 3;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[3]= 6;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[4]= 2;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[5]= 7;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[6]= 1;
	mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[7]= 0;



	mmchost->mmc_clk_dly[MMC_CLK_400K].mode 	= MMC_CLK_400K;
	mmchost->mmc_clk_dly[MMC_CLK_400K].oclk_dly 	= 0;
	mmchost->mmc_clk_dly[MMC_CLK_400K].sclk_dly 	= mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[0];

	mmchost->mmc_clk_dly[MMC_CLK_25M].mode 	= MMC_CLK_25M;
	mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly 	= 0;
	mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly 	= mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[0];

	mmchost->mmc_clk_dly[MMC_CLK_50M].mode 			= MMC_CLK_50M;
	mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly 	= 3;
	mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly 	= mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[0];


	mmchost->mmc_clk_dly[MMC_CLK_100M].mode 	= MMC_CLK_100M;
	mmchost->mmc_clk_dly[MMC_CLK_100M].oclk_dly 	= 1;
	mmchost->mmc_clk_dly[MMC_CLK_100M].sclk_dly 	= 4;

	mmchost->mmc_clk_dly[MMC_CLK_200M].mode 	= MMC_CLK_200M;
	mmchost->mmc_clk_dly[MMC_CLK_200M].oclk_dly 	= 1;
	mmchost->mmc_clk_dly[MMC_CLK_200M].sclk_dly 	= 4;
#endif

	return 0;
}

static void get_fex_para(int sdc_no)
{
	int rval;
	int ret = 0;
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];

	if(sdc_no == 0)
	{
		gpio_request_simple("card0_boot_para", NULL);

		ret = script_parser_fetch("card0_boot_para","sdc_wipe", &rval, 1);
		if(ret < 0)
        	MMCINFO("get sdc_phy_wipe fail.\n");
		else {
			if (rval & DRV_PARA_DISABLE_SECURE_WIPE) {
				MMCINFO("disable driver secure wipe operation.\n");
				mmc_dev[sdc_no].drv_wipe_feature |= DRV_PARA_DISABLE_SECURE_WIPE;
			} else if (rval & DRV_PARA_DISABLE_EMMC_SANITIZE) {
				MMCINFO("disable emmc sanitize feature.\n");
				mmc_dev[sdc_no].drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_SANITIZE;
			} else if (rval & DRV_PARA_DISABLE_EMMC_SECURE_PURGE) {
				MMCINFO("disable emmc secure purge feature.\n");
				mmc_dev[sdc_no].drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_SECURE_PURGE;
			} else if (rval & DRV_PARA_DISABLE_EMMC_TRIM) {
				MMCINFO("disable emmc trim feature.\n");
				mmc_dev[sdc_no].drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_TRIM;
			}
		}

		ret = script_parser_fetch("card0_boot_para", "sdc_erase", &rval, 1);
		if(ret < 0)
			MMCINFO("get sdc0 sdc_erase fail.\n");
		else {
			if (rval & DRV_PARA_DISABLE_EMMC_ERASE) {
				MMCINFO("disable emmc erase.\n");
				mmc_dev[sdc_no].drv_erase_feature |= DRV_PARA_DISABLE_EMMC_ERASE;
			} else if (rval & DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE) {
				MMCINFO("enable emmc sanitize when erase.\n");
				mmc_dev[sdc_no].drv_erase_feature |= DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE;
			}
		}

		ret = script_parser_fetch("card0_boot_para","sdc_f_max", &rval, 1);
		if(ret < 0)
        MMCINFO("get sdc_f_max fail,use default %dHz\n",mmc_dev[sdc_no].f_max);
		else{
				if((rval>mmc_dev[sdc_no].f_max)||(rval<mmc_dev[sdc_no].f_min)){
					MMCINFO("input sdc_f_max wrong ,use default sdc_f_max %d (min %d)\n",
						mmc_dev[sdc_no].f_max, mmc_dev[sdc_no].f_min);
				}else{
					mmc_dev[sdc_no].f_max = rval;
					MMCINFO("get sdc_f_max ok, sdc_f_max = %d\n", mmc_dev[sdc_no].f_max);
				}
		}


		ret = script_parser_fetch("card0_boot_para","sdc_ex_dly_used", &rval, 1);
		if(ret < 0){
        		MMCINFO("get sdc_ex_dly_used fail,use default dly\n");
			return;
		}else{
				if(rval == 1){  //maual sample point from fex
					MMCINFO("get sdc_ex_dly_used ok\n");
					MMCINFO("use manual sample point  in fex\n");
				}else{
					MMCINFO("undefined value %d,use default dly\n",rval);
					return;
				}
		}
		/*************************25M dly*************************************/
		ret = script_parser_fetch("card0_boot_para","sdc_odly_25M", &rval, 1);
		if(ret < 0)
        MMCINFO("get sdc_odly_25M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
		else{
				if((rval>7)||(rval<0)){
					MMCINFO("input sdc_odly_25M wrong ,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
				}else{
					mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly = rval;
					MMCINFO("get sdc_odly_25M ok, odly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
				}
		}

		ret = script_parser_fetch("card0_boot_para","sdc_sdly_25M", &rval, 1);
		if(ret < 0)
        MMCINFO("get sdc_sdly_25M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
		else{
				if((rval>7)||(rval<0)){
					MMCINFO("input sdc_sdly_25M wrong ,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
				}else{
					mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly = rval;
					MMCINFO("get sdc_sdly_25M ok, sdly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
				}
		}

			/*************************50M dly*************************************/
		ret = script_parser_fetch("card0_boot_para","sdc_odly_50M", &rval, 1);
		if(ret < 0)
        MMCINFO("get sdc_odly_50M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
		else{
			  if((rval>7)||(rval<0)){
			  	MMCINFO("input sdc_odly_50M wrong, use default dly %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
			  }else{
					mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly = rval;
					MMCINFO("get sdc_odly_50M ok, odly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
				}
		}

		ret = script_parser_fetch("card0_boot_para","sdc_sdly_50M", &rval, 1);
		if(ret < 0)
        MMCINFO("get sdc_sdly_50M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
		else{
				if((rval>7)||(rval<0)){
					MMCINFO("input sdc_sdly_50M wrong, use default dly %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
				}else{
					mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly = rval;
					MMCINFO("get sdc_sdly_50M ok, sdly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
				}
		}



	}else {// if(sdc_no == 2)
		gpio_request_simple("card2_boot_para", NULL);

		ret = script_parser_fetch("card2_boot_para","sdc_wipe", &rval, 1);
		if(ret < 0)
        	MMCINFO("get sdc_phy_wipe fail.\n");
		else {
			if (rval & DRV_PARA_DISABLE_SECURE_WIPE) {
				MMCINFO("disable driver secure wipe operation.\n");
				mmc_dev[sdc_no].drv_wipe_feature |= DRV_PARA_DISABLE_SECURE_WIPE;
			} else if (rval & DRV_PARA_DISABLE_EMMC_SANITIZE) {
				MMCINFO("disable emmc sanitize feature.\n");
				mmc_dev[sdc_no].drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_SANITIZE;
			} else if (rval & DRV_PARA_DISABLE_EMMC_SECURE_PURGE) {
				MMCINFO("disable emmc secure purge feature.\n");
				mmc_dev[sdc_no].drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_SECURE_PURGE;
			} else if (rval & DRV_PARA_DISABLE_EMMC_TRIM) {
				MMCINFO("disable emmc trim feature.\n");
				mmc_dev[sdc_no].drv_wipe_feature |= DRV_PARA_DISABLE_EMMC_TRIM;
			}
		}

		ret = script_parser_fetch("card2_boot_para", "sdc_erase", &rval, 1);
		if(ret < 0)
			MMCINFO("get sdc0 sdc_erase fail.\n");
		else {
			if (rval & DRV_PARA_DISABLE_EMMC_ERASE) {
				MMCINFO("disable emmc erase.\n");
				mmc_dev[sdc_no].drv_erase_feature |= DRV_PARA_DISABLE_EMMC_ERASE;
			} else if (rval & DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE) {
				MMCINFO("enable emmc sanitize.\n");
				mmc_dev[sdc_no].drv_erase_feature |= DRV_PARA_ENABLE_EMMC_SANITIZE_WHEN_ERASE;
			}
		}

#if defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1) || defined(CONFIG_ARCH_SUN8IW8P1)||(defined CONFIG_ARCH_SUN8IW7P1)
		/*************************sdc_2xmode*************************************/
		ret = script_parser_fetch("card2_boot_para","sdc_2xmode", &rval, 1);
		if(ret < 0)
		{
			mmc_dev[sdc_no].host_func = MMC_NO_FUNC;
			MMCINFO("get sdc_2xmode fail  used =  %d\n",mmc_dev[sdc_no].host_func);
		}
		else
		{
			if(rval == 1)
			{
				mmc_dev[sdc_no].host_func = MMC_HOST_2XMODE_FUNC;
			}
			else
			{
				mmc_dev[sdc_no].host_func = MMC_NO_FUNC;
			}
			MMCINFO("get sdc_2xmode ok, val = %d\n", mmc_dev[sdc_no].host_func);
		}
		/*************************sdc_ddrmode*************************************/
		ret = script_parser_fetch("card2_boot_para","sdc_ddrmode", &rval, 1);
		if(ret < 0)
		{
			mmc_dev[sdc_no].mmc_func_en.ddr_func_en = 0;
			MMCINFO("get sdc_ddrmode fail  used =  %d\n",mmc_dev[sdc_no].mmc_func_en.ddr_func_en );
		}
		else
		{
			if(rval == 1)
			{
				mmc_dev[sdc_no].mmc_func_en.ddr_func_en = 1;
			}
			else
			{
				mmc_dev[sdc_no].mmc_func_en.ddr_func_en = 0;
			}
			MMCINFO("get sdc_ddrmode ok, val = %d\n", mmc_dev[sdc_no].mmc_func_en.ddr_func_en );
		}
#endif
		ret = script_parser_fetch("card2_boot_para","sdc_f_max", &rval, 1);
		if(ret < 0)
            MMCINFO("get sdc_f_max fail,use default  %dHz\n", mmc_dev[sdc_no].f_max);
		else {
				if((rval>mmc_dev[sdc_no].f_max)||(rval<mmc_dev[sdc_no].f_min)){
					MMCINFO("input sdc_f_max wrong %d,use default sdc_f_max %d (min %d)\n",
						rval, mmc_dev[sdc_no].f_max, mmc_dev[sdc_no].f_min);
				}else{
					mmc_dev[sdc_no].f_max = rval;
					MMCINFO("get sdc_f_max ok, sdc_f_max = %d\n", mmc_dev[sdc_no].f_max);
				}
		}

		ret = script_parser_fetch("card2_boot_para","card_line", &rval, 1);
		if(ret < 0)
        MMCINFO("get card_line fail,use default card_line %d\n",4);
		else{
#ifndef CONFIG_ARCH_SUN7I
				if((rval!=8)&&(rval!=1)&&(rval!=4)){
					MMCINFO("input card_line wrong ,use default card_line %d\n",4);
				}else{
					if(rval == 8 ){
						mmc_dev[sdc_no].host_caps |= MMC_MODE_8BIT;
					}else if(rval == 4){
						mmc_dev[sdc_no].host_caps &= ~MMC_MODE_8BIT;
						mmc_dev[sdc_no].host_caps |= MMC_MODE_4BIT;
					}else{
						mmc_dev[sdc_no].host_caps &= ~MMC_MODE_8BIT;
						mmc_dev[sdc_no].host_caps &= ~MMC_MODE_4BIT;
					}
					MMCINFO("get card_line ok, card_line = %d\n", rval);
				}
#else
				if((rval!=1)&&(rval!=4)){
					MMCINFO("input card_line wrong ,use default card_line %d\n",4);
				}else{
					if(rval == 4){
						mmc_dev[sdc_no].host_caps &= ~MMC_MODE_8BIT;
						mmc_dev[sdc_no].host_caps |= MMC_MODE_4BIT;
					}else{
						mmc_dev[sdc_no].host_caps &= ~MMC_MODE_8BIT;
						mmc_dev[sdc_no].host_caps &= ~MMC_MODE_4BIT;
					}
					MMCINFO("get card_line ok, card_line = %d\n", rval);
				}
#endif
		}


		ret = script_parser_fetch("card2_boot_para","sdc_ex_dly_used", &rval, 1);
		if(ret < 0){
        		MMCINFO("get sdc_ex_dly_used fail,use default\n");
			return;
		}else{
				int rval_ker =0;
				ret = script_parser_fetch("mmc2_para","sdc_ex_dly_used", &rval_ker, 1);
				int work_mode = uboot_spare_head.boot_data.work_mode;
				//MMCINFO("ret%d,rval_ker%d,rval%d",ret,rval_ker,rval);
				if((ret>=0)&&(rval_ker == 2)&&(rval == 2)){//only when kernal use auto sample,uboot will use auto sample.
					struct tuning_sdly  *sdly= (struct tuning_sdly  *)uboot_spare_head.boot_data.sdcard_spare_data;
					mmc_dev[sdc_no].sample_mode = AUTO_SAMPLE_MODE;
					MMCINFO("get sdc_ex_dly_used ok\n");
					MMCINFO("use auto sdly \n");
					if(work_mode != WORK_MODE_BOOT){
						//usb product will auto get sample point,so no need to get auto sdly,so first used default value
						MMCINFO("Product will auto sample,not need to get auto sdly\n");
						return;
					}
					MMCDBG("get auto sdly %d in 25M\n",sdly->sdly_25M);
					mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly = sdly->sdly_25M;
					MMCDBG("get auto sdly %d in 50M\n",sdly->sdly_50M);
					mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly = sdly->sdly_50M;
					MMCINFO("auto sdly %d in 25M \n", mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
					MMCINFO("auto sdly %d in 50M \n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
					return;
				}else if(rval == 1){  //maual sample point from fex
					MMCINFO("get sdc_ex_dly_used ok\n");
					MMCINFO("use manual sample point  in fex\n");
				}else{
					MMCINFO("undefined value %d or kernel not use auto sample,use default dly\n",rval);
					return;
				}
		}


		/*************************25M dly*************************************/
		ret = script_parser_fetch("card2_boot_para","sdc_odly_25M", &rval, 1);
		if(ret < 0){
        		MMCINFO("get sdc_odly_25M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
		}else{
				if((rval>7)||(rval<0)){
					MMCINFO("input sdc_odly_25M wrong ,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
				}else{
					mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly = rval;
					MMCINFO("get sdc_odly_25M ok, odly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
				}
		}

		ret = script_parser_fetch("card2_boot_para","sdc_sdly_25M", &rval, 1);
		if(ret < 0)
        MMCINFO("get sdc_sdly_25M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
		else{
				if((rval>7)||(rval<0)){
					MMCINFO("input sdc_sdly_25M wrong ,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
				}else{
					mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly = rval;
					MMCINFO("get sdc_sdly_25M ok, sdly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
				}
		}

			/*************************50M dly*************************************/
		ret = script_parser_fetch("card2_boot_para","sdc_odly_50M", &rval, 1);
		if(ret < 0)
        MMCINFO("get sdc_odly_50M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
		else{
			  if((rval>7)||(rval<0)){
			  	MMCINFO("input sdc_odly_50M wrong, use default dly %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
			  }else{
					mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly = rval;
					MMCINFO("get sdc_odly_50M ok, odly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
				}
		}

		ret = script_parser_fetch("card2_boot_para","sdc_sdly_50M", &rval, 1);
		if(ret < 0)
        MMCINFO("get sdc_sdly_50M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
		else{
				if((rval>7)||(rval<0)){
					MMCINFO("input sdc_sdly_50M wrong, use default dly %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
				}else{
					mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly = rval;
					MMCINFO("get sdc_sdly_50M ok, sdly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
				}
		}


	}
}


static int mmc_clk_io_onoff(int sdc_no,int onoff)
{
	int rval;
	struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
#if defined(CONFIG_ARCH_SUN9IW1P1)
	if(gd->securemode == SUNXI_SECURE_MODE_NO_SECUREOS)  //burn mode
	{
		if(sdc_no == 0)
		{
		    gpio_request_simple("card0_boot_para", NULL);
		}
		else // if(sdc_no == 2)
		{
		    gpio_request_simple("card2_boot_para", NULL);
		}
		return 0;
	}
#endif

	get_fex_para(sdc_no);

#if defined(CONFIG_ARCH_SUN8IW1P1) || defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1) ||(defined CONFIG_ARCH_SUN8IW7P1) || defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN8IW9P1)
	/* config ahb clock */
	if(onoff)
	{
		rval = readl(mmchost->hclkrst);
		rval |= (1 << (8 + sdc_no));
		writel(rval, mmchost->hclkrst);
		rval = readl(mmchost->hclkbase);
		rval |= (1 << (8 + sdc_no));
		writel(rval, mmchost->hclkbase);
	}
	else
	{
		rval = readl(mmchost->hclkbase);
		rval &= ~(1 << (8 + sdc_no));
		writel(rval, mmchost->hclkbase);

		rval = readl(mmchost->hclkrst);
		rval &= ~ (1 << (8 + sdc_no));
		writel(rval, mmchost->hclkrst);
	}
#elif defined(CONFIG_ARCH_SUN7I) || defined(CONFIG_ARCH_SUN5I)
	/* config ahb clock */
	rval = readl(mmchost->hclkbase);
	rval |= (1 << (8 + sdc_no));
	writel(rval, mmchost->hclkbase);
#elif defined(CONFIG_ARCH_SUN9IW1P1)
	/* config ahb clock */
	if (onoff)
	{
		rval = smc_readl(mmchost->hclkbase);
		rval |= (1 << 8);
		smc_writel(rval, mmchost->hclkbase);
	
		rval = smc_readl(mmchost->hclkrst);
		rval |= (1 << 8);
		smc_writel(rval, mmchost->hclkrst);
	
		rval = smc_readl(mmchost->commreg);
		rval |= (1<<16)|(1<<18);
		smc_writel(rval, mmchost->commreg);
	} 
	else 
	{
		rval = smc_readl(mmchost->hclkbase);
		rval &= (~(1 << 8));
		smc_writel(rval, mmchost->hclkbase);
	
		rval = smc_readl(mmchost->hclkrst);
		rval &= (~(1 << 8));
		smc_writel(rval, mmchost->hclkrst);
	
		rval = smc_readl(mmchost->commreg);
		rval &= (~((1<<16)|(1<<18)));
		smc_writel(rval, mmchost->commreg);
	}
#else
	#error The platform is not seleted
#endif
	/* config mod clock */
	smc_writel(0x80000000, mmchost->mclkbase);
	mmchost->mod_clk = 24000000;
	dumphex32("ccmu", (char*)SUNXI_CCM_BASE, 0x100);
	dumphex32("gpio", (char*)SUNXI_PIO_BASE, 0x100);
	dumphex32("mmc", (char*)mmchost->reg, 0x100);

	return 0;
}

static int mmc_update_clk(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned int cmd;
	unsigned timeout = 1000;

	cmd = (1U << 31) | (1 << 21) | (1 << 13);
  	writel(cmd, &mmchost->reg->cmd);
	while((readl(&mmchost->reg->cmd)&0x80000000) && --timeout){
		__msdelay(1);
	}
	if (!timeout){
		MMCINFO("mmc %d,update clk failed\n",mmchost->mmc_no);
		dumphex32("mmc", (char*)mmchost->reg, 0x100);
		return -1;
	}

	writel(readl(&mmchost->reg->rint), &mmchost->reg->rint);
	return 0;
}

#if defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1)||defined(CONFIG_ARCH_SUN8IW8P1) ||(defined CONFIG_ARCH_SUN8IW7P1)
static int mmc_2xmode_config_clock(struct mmc *mmc, unsigned clk)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned rval = readl(&mmchost->reg->clkcr);
	unsigned int clkdiv = 0;
	unsigned int rntsr = readl(&mmchost->reg->ntsr);
	unsigned int rgctrl = readl(&mmchost->reg->gctrl);

	/* Disable Clock */
	rval &= ~(1 << 16);
	writel(rval, &mmchost->reg->clkcr);
	if(mmc_update_clk(mmc))
		return -1;

	//disable mclk first
	writel(0x4000000,mmchost->mclkbase);
	MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no, readl(mmchost->mclkbase));

	/*NTSR*/
	rntsr |= (1<<31);
	MMCDBG("mmc %d rntsr 0x%x\n",mmchost->mmc_no,rntsr);
	writel(rntsr, &mmchost->reg->ntsr);


	/*set ddr mode*/
	if(mmc->io_mode == MMC_MODE_DDR_52MHz){
		MMCDBG("first %d rgctrl 0x%x\n",mmchost->mmc_no,rgctrl);
		rgctrl |= 1 << 10;
		writel(rgctrl, &mmchost->reg->gctrl);
		MMCDBG("after %d rgctrl 0x%x\n",mmchost->mmc_no,readl(&mmchost->reg->gctrl));
	}
	else{
		MMCDBG("mmc not set ddr mmc->io_mode = %x\n",mmc->io_mode);
	}

	if (clk <=400000) {
		mmchost->mod_clk = 400000;
		writel(0x4001000e, mmchost->mclkbase);
		MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no, readl(mmchost->mclkbase));
	} else {
		u32 pllclk;
		u32 n,m;

		pllclk = sunxi_clock_get_pll6() * 1000000;
		/*set ddr mode clock*/
		if(mmc->io_mode == MMC_MODE_DDR_52MHz){
			clkdiv = pllclk /( clk *4 ) - 1;
		}else{
			clkdiv = pllclk /( clk *2 ) - 1;
		}

		if (clkdiv < 16) {
			n = 0;
			m = clkdiv;
		} else if (clkdiv < 32) {
			n = 1;
			m = clkdiv>>1;
		} else {
			n = 2;
			m = clkdiv>>2;
		}
		mmchost->mod_clk = clk;

		if (clk <= 26000000){
			writel(0x41000000| (n << 16) | m, mmchost->mclkbase);
		}
		else{
			writel(0x41000000 | (n << 16) | m, mmchost->mclkbase);
		}
		MMCDBG("init mmc %d pllclk %d, clk %d, mclkbase %x\n",mmchost->mmc_no,
				pllclk, mmchost->mod_clk, readl(mmchost->mclkbase));
		MMCDBG("Get round clk %d\n",pllclk/(1<<n)/(m+1));
		if (mmc->io_mode == MMC_MODE_DDR_52MHz)
			mmc->clock = pllclk/(1<<n)/(m+1)/2/2;
		else
			mmc->clock = pllclk/(1<<n)/(m+1)/2;
	}
	//re-enable mclk
	writel(readl(mmchost->mclkbase)|(1<<31),mmchost->mclkbase);
	MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no, readl(mmchost->mclkbase));
	/*
	 * CLKCREG[7:0]: divider
	 * CLKCREG[16]:  on/off
	 * CLKCREG[17]:  power save
	 */
	/* Change Divider Factor */
	rval &= ~(0xFF);
	if (mmc->io_mode == MMC_MODE_DDR_52MHz)
		rval |= 0x1;
	writel(rval, &mmchost->reg->clkcr);
	if(mmc_update_clk(mmc)){
		MMCINFO("mmc %d disable clock failed\n",mmchost->mmc_no);
		return -1;
	}
	/* Re-enable Clock */
	rval |= (3 << 16);
	writel(rval, &mmchost->reg->clkcr);
	if(mmc_update_clk(mmc)){
		MMCINFO("mmc %d re-enable clock failed\n",mmchost->mmc_no);
		return -1;
	}

	dumphex32("ccmu", (char*)SUNXI_CCM_BASE, 0x100);
	dumphex32("gpio", (char*)SUNXI_PIO_BASE, 0x100);
	dumphex32("mmc", (char*)mmchost->reg, 0x100);

	MMCDBG("mmc %d ntsr 0x%x\n",mmchost->mmc_no,readl(&mmchost->reg->ntsr));
	return 0;

}
#endif

static int mmc_config_clock(struct mmc *mmc, unsigned clk)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned rval = readl(&mmchost->reg->clkcr);
	unsigned int clkdiv = 0;
	u32 sdly = 0;
	u32 odly = 0;

#if defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1)||defined(CONFIG_ARCH_SUN8IW8P1)||(defined CONFIG_ARCH_SUN8IW7P1)
	if( (mmchost->mmc_no== 2) && (mmc->host_func & MMC_HOST_2XMODE_FUNC) )
	{
		MMCDBG("mmc %d 2xmode config clk\n",mmchost->mmc_no);
		return ( mmc_2xmode_config_clock(mmc,clk) );
	}
	else
#endif
	{
		MMCDBG("mmc %d old config clk %d \n",mmchost->mmc_no,mmc->host_func);
		/* Disable Clock */
		rval &= ~(1 << 16);
		writel(rval, &mmchost->reg->clkcr);
		if(mmc_update_clk(mmc))
			return -1;

		//disable mclk first
		smc_writel(0,mmchost->mclkbase);
		MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no,smc_readl(mmchost->mclkbase));
		if (clk <=400000) {
		sdly = mmchost->mmc_clk_dly[MMC_CLK_400K].sclk_dly;
		odly = mmchost->mmc_clk_dly[MMC_CLK_400K].oclk_dly;
		mmchost->mod_clk = 400000;
	    smc_writel(0x0002000f|(sdly << 20)|(odly << 8), mmchost->mclkbase);
			MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no,smc_readl(mmchost->mclkbase));
			MMCDBG("Get round clk %d\n",400000);
			mmc->clock = 400000;
			mmc->pll_clock = 24000000;
		} else {
			u32 pllclk;
			u32 n,m;
#if (defined(CONFIG_ARCH_SUN7I)|| defined(CONFIG_ARCH_SUN8IW1P1) || defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1) ||(defined CONFIG_ARCH_SUN8IW7P1) ||  defined(CONFIG_ARCH_SUN8IW8P1) )||(defined(CONFIG_ARCH_SUN8IW9P1))
			pllclk = sunxi_clock_get_pll6() * 1000000;
#elif defined(CONFIG_ARCH_SUN9IW1P1)
			pllclk = sunxi_clock_get_pll4_periph1() * 1000000;
#elif defined(CONFIG_ARCH_SUN5I)
			pllclk = sunxi_clock_get_pll5() * 1000000;

#else
#error the platform is not config
#endif
			clkdiv = pllclk / clk - 1;
			if (clkdiv < 16) {
				n = 0;
				m = clkdiv;
			} else if (clkdiv < 32) {
				n = 1;
				m = clkdiv>>1;
			} else {
				n = 2;
				m = clkdiv>>2;
			}
			mmchost->mod_clk = clk;
#if defined(CONFIG_ARCH_SUN5I)
			if (clk <= 26000000)
				writel(0x02500000 | (n << 16) | m, mmchost->mclkbase);
			else
				writel(0x02500300 | (n << 16) | m, mmchost->mclkbase);
#else
			if (clk <= 26000000){
				sdly = mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly;
				odly = mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly;
				smc_writel(0x01000000 |(sdly << 20)|(odly << 8)| (n << 16) | m, mmchost->mclkbase);
			}
			else{
				sdly = mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly;
				odly = mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly;
				smc_writel(0x01000000 | (sdly << 20) | (odly << 8) | (n << 16) | m, mmchost->mclkbase);
			}
#endif
			MMCDBG("init mmc %d pllclk %d, clk %d, mclkbase %x\n",mmchost->mmc_no,
					pllclk, mmchost->mod_clk, smc_readl(mmchost->mclkbase));
			MMCDBG("Get round clk %d\n",pllclk/(1<<n)/(m+1));
			mmc->clock = pllclk/(1<<n)/(m+1);
			mmc->pll_clock = pllclk;
		}
		//re-enable mclk
		smc_writel(smc_readl(mmchost->mclkbase)|(1<<31),mmchost->mclkbase);
		MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no,smc_readl(mmchost->mclkbase));
		/*
		 * CLKCREG[7:0]: divider
		 * CLKCREG[16]:  on/off
		 * CLKCREG[17]:  power save
		 */
		/* Change Divider Factor */
		rval &= ~(0xFF);
		writel(rval, &mmchost->reg->clkcr);
		if(mmc_update_clk(mmc)){
			MMCINFO("mmc %d disable clock failed\n",mmchost->mmc_no);
			return -1;
		}
		/* Re-enable Clock */
		rval |= (3 << 16);
		writel(rval, &mmchost->reg->clkcr);
		if(mmc_update_clk(mmc)){
			MMCINFO("mmc %d re-enable clock failed\n",mmchost->mmc_no);
			return -1;
		}
	}
	return 0;
}


static void mmc_set_ios(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;


	MMCDBG("mmc %d ios: bus: %d, clock: %d\n", mmchost->mmc_no,mmc->bus_width, mmc->clock);

	if (mmc->clock && mmc_config_clock(mmc, mmc->clock)) {
		MMCINFO("[mmc]: mmc %d  update clock failed\n",mmchost->mmc_no);
		mmchost->fatal_err = 1;
		return;
	}
	/* Change bus width */
	if (mmc->bus_width == 8)
		writel(2, &mmchost->reg->width);
	else if (mmc->bus_width == 4)
		writel(1, &mmchost->reg->width);
	else
		writel(0, &mmchost->reg->width);
	MMCDBG("host bus width %x\n",readl(&mmchost->reg->width));
}

static int mmc_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
#ifndef CONFIG_SUN7I
	/* Reset controller */
	writel(0x40000007, &mmchost->reg->gctrl);
	while(readl(&mmchost->reg->gctrl)&0x7);
	/* release eMMC reset signal */
	writel(1, &mmchost->reg->hwrst);
	writel(0, &mmchost->reg->hwrst);
	udelay(1000);
	writel(1, &mmchost->reg->hwrst);
#else
	writel(0x7, &mmchost->reg->gctrl);
	while(readl(&mmchost->reg->gctrl)&0x7);
#endif
	return 0;
}

static int mmc_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned i;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned *buff;
	unsigned timeout = 1000;

	if (data->flags & MMC_DATA_READ) {
		buff = (unsigned int *)data->dest;
		for (i=0; i<(byte_cnt>>2); i++) {
			while(--timeout && (readl(&mmchost->reg->status)&(1 << 2))){
				__msdelay(1);
			}
			if (timeout <= 0)
				goto out;
			buff[i] = readl(mmchost->database);
			timeout = 1000;
		}
	} else {
		buff = (unsigned int *)data->src;
		for (i=0; i<(byte_cnt>>2); i++) {
			while(--timeout && (readl(&mmchost->reg->status)&(1 << 3))){
				__msdelay(1);
			}
			if (timeout <= 0)
				goto out;
			writel(buff[i], mmchost->database);
			timeout = 1000;
		}
	}

out:
	if (timeout <= 0){
		MMCINFO("transfer by cpu failed\n");
		return -1;
	}

	return 0;
}

static int mmc_trans_data_by_dma(struct mmc *mmc, struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	struct sunxi_mmc_des *pdes = mmchost->pdes;
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned char *buff;
	unsigned des_idx = 0;
	unsigned buff_frag_num = 0;
	unsigned remain;
	unsigned i, rval;

	buff = data->flags & MMC_DATA_READ ?
			(unsigned char *)data->dest : (unsigned char *)data->src;
	buff_frag_num = byte_cnt >> SDXC_DES_NUM_SHIFT;
	remain = byte_cnt & (SDXC_DES_BUFFER_MAX_LEN-1);
	if (remain)
		buff_frag_num ++;
	else
		remain = SDXC_DES_BUFFER_MAX_LEN;

	flush_cache((unsigned long)buff, (unsigned long)byte_cnt);
	for (i=0; i < buff_frag_num; i++, des_idx++) {
		memset((void*)&pdes[des_idx], 0, sizeof(struct sunxi_mmc_des));
		pdes[des_idx].des_chain = 1;
		pdes[des_idx].own = 1;
		pdes[des_idx].dic = 1;
		if (buff_frag_num > 1 && i != buff_frag_num-1)
			pdes[des_idx].data_buf1_sz = SDXC_DES_BUFFER_MAX_LEN;
		else
			pdes[des_idx].data_buf1_sz = remain;

		pdes[des_idx].buf_addr_ptr1 = (u32)buff + i * SDXC_DES_BUFFER_MAX_LEN;
		if (i==0)
			pdes[des_idx].first_des = 1;

		if (i == buff_frag_num-1) {
			pdes[des_idx].dic = 0;
			pdes[des_idx].last_des = 1;
			pdes[des_idx].end_of_ring = 1;
			pdes[des_idx].buf_addr_ptr2 = 0;
		} else {
			pdes[des_idx].buf_addr_ptr2 = (u32)&pdes[des_idx+1];
		}
//		MMCDBG("frag %d, remain %d, des[%d](%08x): "
//			"[0] = %08x, [1] = %08x, [2] = %08x, [3] = %08x\n",
//			i, remain, des_idx, (u32)&pdes[des_idx],
//			(u32)((u32*)&pdes[des_idx])[0], (u32)((u32*)&pdes[des_idx])[1],
//			(u32)((u32*)&pdes[des_idx])[2], (u32)((u32*)&pdes[des_idx])[3]);
	}
	flush_cache((unsigned long)pdes, sizeof(struct sunxi_mmc_des) * (des_idx+1));

	/*
	 * GCTRLREG
	 * GCTRL[2]	: DMA reset
	 * GCTRL[5]	: DMA enable
	 *
	 * IDMACREG
	 * IDMAC[0]	: IDMA soft reset
	 * IDMAC[1]	: IDMA fix burst flag
	 * IDMAC[7]	: IDMA on
	 *
	 * IDIECREG
	 * IDIE[0]	: IDMA transmit interrupt flag
	 * IDIE[1]	: IDMA receive interrupt flag
	 */
	rval = readl(&mmchost->reg->gctrl);
	writel(rval|(1 << 5)|(1 << 2), &mmchost->reg->gctrl);	/* dma enable */
	writel((1 << 0), &mmchost->reg->dmac); /* idma reset */
	writel((1 << 1) | (1 << 7), &mmchost->reg->dmac); /* idma on */
	rval = readl(&mmchost->reg->idie) & (~3);
	if (data->flags & MMC_DATA_WRITE)
		rval |= (1 << 0);
	else
		rval |= (1 << 1);
	writel(rval, &mmchost->reg->idie);
	writel((unsigned long)pdes, &mmchost->reg->dlba);
	writel((2U<<28)|(7<<16)|8, &mmchost->reg->ftrglevel);

	return 0;
}

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	unsigned int cmdval = 0x80000000;
	signed int timeout = 0;
	int error = 0;
	unsigned int status = 0;
	unsigned int usedma = 0;
	unsigned int bytecnt = 0;

	if (mmchost->fatal_err){
		MMCINFO("mmc %d Found fatal err,so no send cmd\n",mmchost->mmc_no);
		return -1;
	}
	if (cmd->resp_type & MMC_RSP_BUSY)
		MMCDBG("mmc %d mmc cmd %d check rsp busy\n", mmchost->mmc_no,cmd->cmdidx);
	if ((cmd->cmdidx == 12)&&!(cmd->flags&MMC_CMD_MANUAL)){
    	MMCDBG("note we don't send stop cmd,only check busy here");
    	timeout = 500*1000;
		do {
				status = readl(&mmchost->reg->status);
				if (!timeout--) {
                	error = -1;
                    MMCINFO("mmc %d cmd12 busy timeout\n",mmchost->mmc_no);
                    goto out;
                    }
                    __usdelay(1);
        } while (status & (1 << 9));
        return 0;
	}
	/*
	 * CMDREG
	 * CMD[5:0]	: Command index
	 * CMD[6]	: Has response
	 * CMD[7]	: Long response
	 * CMD[8]	: Check response CRC
	 * CMD[9]	: Has data
	 * CMD[10]	: Write
	 * CMD[11]	: Steam mode
	 * CMD[12]	: Auto stop
	 * CMD[13]	: Wait previous over
	 * CMD[14]	: About cmd
	 * CMD[15]	: Send initialization
	 * CMD[21]	: Update clock
	 * CMD[31]	: Load cmd
	 */
	if (!cmd->cmdidx)
		cmdval |= (1 << 15);
	if (cmd->resp_type & MMC_RSP_PRESENT)
		cmdval |= (1 << 6);
	if (cmd->resp_type & MMC_RSP_136)
		cmdval |= (1 << 7);
	if (cmd->resp_type & MMC_RSP_CRC)
		cmdval |= (1 << 8);
	if (data) {
		if ((u32)data->dest & 0x3) {
			MMCINFO("mmc %d dest is not 4 byte align\n",mmchost->mmc_no);
			error = -1;
			goto out;
		}

		cmdval |= (1 << 9) | (1 << 13);
		if (data->flags & MMC_DATA_WRITE)
			cmdval |= (1 << 10);
		if (data->blocks > 1&&!(cmd->flags&MMC_CMD_MANUAL))
			cmdval |= (1 << 12);
		writel(data->blocksize, &mmchost->reg->blksz);
		writel(data->blocks * data->blocksize, &mmchost->reg->bytecnt);
	}else{
		if ((cmd->cmdidx == 12)&&(cmd->flags&MMC_CMD_MANUAL)){
			cmdval|=1<<14;//stop current data transferin progress.
			cmdval &=~ (1 << 13);//Send command at once, even if previous data transfer has notcompleted
		}
	}

	MMCDBG("mmc %d, cmd %d(0x%08x), arg 0x%08x\n", mmchost->mmc_no, cmd->cmdidx, cmdval|cmd->cmdidx, cmd->cmdarg);
	writel(cmd->cmdarg, &mmchost->reg->arg);
	if (!data)
		writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);

	/*
	 * transfer data and check status
	 * STATREG[2] : FIFO empty
	 * STATREG[3] : FIFO full
	 */
	if (data) {
		int ret = 0;

		bytecnt = data->blocksize * data->blocks;
		MMCDBG("mmc %d trans data %d bytes\n",mmchost->mmc_no, bytecnt);
#ifdef CONFIG_MMC_SUNXI_USE_DMA
		if (bytecnt > 64) {
#else
		if (0) {
#endif
			usedma = 1;
			writel(readl(&mmchost->reg->gctrl)&(~0x80000000), &mmchost->reg->gctrl);
			ret = mmc_trans_data_by_dma(mmc, data);
			writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);
		} else {
			writel(readl(&mmchost->reg->gctrl)|0x80000000, &mmchost->reg->gctrl);
			writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);
			ret = mmc_trans_data_by_cpu(mmc, data);
		}
		if (ret) {
			MMCINFO("mmc %d Transfer failed\n",mmchost->mmc_no);
			error = readl(&mmchost->reg->rint) & 0xbfc2;
			if(!error)
				error = 0xffffffff;
			goto out;
		}
	}

	timeout = 1000;
	do {
		status = readl(&mmchost->reg->rint);
		if (!timeout-- || (status & 0xbfc2)) {
			error = status & 0xbfc2;
			if(!error)
				error = 0xffffffff;//represet software timeout
			MMCINFO("mmc %d cmd %d err %x\n",mmchost->mmc_no, cmd->cmdidx, error);
			goto out;
		}
		__usdelay(1);
	} while (!(status&0x4));

	if (data) {
		unsigned done = 0;
		timeout = usedma ? (50*bytecnt/25) : 0xffffff;//0.04us(25M)*2(4bit width)*25()
		if(timeout < 0xffffff){
			timeout = 0xffffff;
		}
		MMCDBG("mmc %d cacl timeout %x\n",mmchost->mmc_no, timeout);
		do {
			status = readl(&mmchost->reg->rint);
			if (!timeout-- || (status & 0xbfc2)) {
				error = status & 0xbfc2;
				if(!error)
					error = 0xffffffff;//represet software timeout
				MMCINFO("mmc %d data err %x\n",mmchost->mmc_no, error);
				goto out;
			}
			if ((data->blocks > 1)&&!(cmd->flags&MMC_CMD_MANUAL))//not wait auto stop when MMC_CMD_MANUAL is set
				done = status & (1 << 14);
			else
				done = status & (1 << 3);
			__usdelay(1);
		} while (!done);

	    if((data->flags & MMC_DATA_READ)&& usedma){
            timeout = 0xffffff;
            done = 0;
            status = 0;
		    MMCDBG("mmc %d cacl rd dma timeout %x\n",mmchost->mmc_no, timeout);
		    do {
			    status = readl(&mmchost->reg->idst);
			    if (!timeout-- || (status & 0x234)) {
                    error = status & 0x1E34;
				    if(!error)
					    error = 0xffffffff;//represet software timeout
				    MMCINFO("mmc %d wait dma over err %x\n",mmchost->mmc_no, error);
				    goto out;
			    }
				done = status & (1 << 1);
			    __usdelay(1);
		    } while (!done);
            MMCDBG("idst *****0x%d******\n",readl(&mmchost->reg->idst));
        }

	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		if ((cmd->cmdidx == MMC_CMD_ERASE)
			|| ((cmd->cmdidx == MMC_CMD_SWITCH)
				&&(((cmd->cmdarg>>16)&0xFF) == EXT_CSD_SANITIZE_START)))
			timeout = 0x1fffffff;
		else
			timeout = 500*1000;

		do {
			status = readl(&mmchost->reg->status);
			if (!timeout--) {
				error = -1;
				MMCINFO("mmc %d busy timeout\n",mmchost->mmc_no);
				goto out;
			}
			__usdelay(1);
		} while (status & (1 << 9));

		if ((cmd->cmdidx == MMC_CMD_ERASE)
			|| ((cmd->cmdidx == MMC_CMD_SWITCH)
				&&(((cmd->cmdarg>>16)&0xFF) == EXT_CSD_SANITIZE_START)))
			MMCINFO("%s: cmd %d wait rsp busy 0x%x us \n",__FUNCTION__,
				cmd->cmdidx, 0x1fffffff-timeout);
	}


	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&mmchost->reg->resp3);
		cmd->response[1] = readl(&mmchost->reg->resp2);
		cmd->response[2] = readl(&mmchost->reg->resp1);
		cmd->response[3] = readl(&mmchost->reg->resp0);
		MMCDBG("mmc %d mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
			mmchost->mmc_no,
			cmd->response[3], cmd->response[2],
			cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = readl(&mmchost->reg->resp0);
		MMCDBG("mmc %d mmc resp 0x%08x\n",mmchost->mmc_no, cmd->response[0]);
	}
out:
	if(error){
		mmchost->raw_int_bak = readl(&mmchost->reg->rint )& 0xbfc2;
		//mmc_dump_errinfo(mmchost,cmd);
	}
	if (data && usedma) {
		/* IDMASTAREG
		 * IDST[0] : idma tx int
		 * IDST[1] : idma rx int
		 * IDST[2] : idma fatal bus error
		 * IDST[4] : idma descriptor invalid
		 * IDST[5] : idma error summary
		 * IDST[8] : idma normal interrupt sumary
		 * IDST[9] : idma abnormal interrupt sumary
		 */
		status = readl(&mmchost->reg->idst);
		writel(status, &mmchost->reg->idst);
		writel(0, &mmchost->reg->idie);
		writel(0, &mmchost->reg->dmac);
		writel(readl(&mmchost->reg->gctrl)&(~(1 << 5)), &mmchost->reg->gctrl);
    }
	if (error) {
		if(data && (data->flags&MMC_DATA_READ)&&(bytecnt==512)){
			writel(readl(&mmchost->reg->gctrl)|0x80000000, &mmchost->reg->gctrl);
			writel(0xdeb, &mmchost->reg->dbgc);
			timeout = 1000;
			int tmp;
			MMCINFO("Read remain data\n");
			while(readl(&mmchost->reg->bbcr)<512){
				tmp = readl(mmchost->database);
				 tmp=tmp;
				MMCDBG("Read data %x,bbcr %x\n",tmp,readl(&mmchost->reg->bbcr));
				__usdelay(1);
				if(!(timeout--)){
					MMCINFO("Read remain data timeout\n");
					break;
				}
			}


		}


		writel(0x7, &mmchost->reg->gctrl);
		while(readl(&mmchost->reg->gctrl)&0x7){
		}

		mmc_update_clk(mmc);
		//MMCINFO("mmc %d mmc cmd %d err 0x%08x\n",mmchost->mmc_no, cmd->cmdidx, error);
	}
	writel(0xffffffff, &mmchost->reg->rint);
	//writel(readl(&mmchost->reg->gctrl)|(1 << 1), &mmchost->reg->gctrl);

    if(data&&(data->flags&MMC_DATA_READ)){
        unsigned char *buff = (unsigned char *)data->dest;
        unsigned byte_cnt = data->blocksize * data->blocks;
        flush_cache((unsigned long)buff, (unsigned long)byte_cnt);
        MMCDBG("invald cache after read complete\n");
    }



	if (error)
		return -1;
	else
		return 0;
}




int sunxi_decide_rty(struct mmc *mmc,int err_no,uint rst_cnt)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	int cnt= 0;
	if(rst_cnt){
		mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].rty_cnt = 0;
		mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].rty_cnt = 0;
		mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].rty_cnt = 0;
		return 0;
	}

	if(err_no && (!(err_no & SDXC_RespTimeout)||(err_no==0xffffffff))){
		if(mmc->clock <= (400*1000)){
			mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].rty_cnt++;
			cnt = mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].rty_cnt;
			if(cnt>7){
				MMCINFO("Beyond the retry times\n");
				return -1;
			}
			mmchost->mmc_clk_dly[MMC_CLK_400K].sclk_dly= mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_400K].sclk_dly[cnt];
			MMCINFO("Get next samply point %d in clk %d\n",mmchost->mmc_clk_dly[MMC_CLK_400K].sclk_dly,mmc->clock);
		}else if(mmc->clock <= (26*1000*1000)){
			mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].rty_cnt++;
			cnt = mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].rty_cnt;
			if(cnt>7){
				MMCINFO("Beyond the retry times\n");
				return -1;
			}
			mmchost->mmc_clk_dly[MMC_CLK_25M] .sclk_dly= mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_25M].sclk_dly[cnt];
			MMCINFO("Get next samply point %d in clk %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly,mmc->clock);
		}else if(mmc->clock <= (52*1000*1000)){
			mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].rty_cnt++;
			cnt = mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].rty_cnt;
			if(cnt>7){
				MMCINFO("Beyond the retry times\n");
				return -1;
			}
			mmchost->mmc_clk_dly[MMC_CLK_50M] .sclk_dly= mmchost->mmc_clk_sdly_rty_tbl[MMC_CLK_50M].sclk_dly[cnt];
			MMCINFO("Get next samply point %d in clk %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly,mmc->clock);
		}else{
			MMCINFO("clk is over the retry clk\n");
			return -1;
		}
		mmchost->raw_int_bak = 0;
		return 0;
	}
	MMCDBG("rto or no error or software timeout,no need retry\n");
	return -1;

}


int sunxi_update_sdly(struct mmc *mmc,uint sdly)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;

	if(mmc->clock <= (400*1000)){
		mmchost->mmc_clk_dly[MMC_CLK_400K].sclk_dly= sdly;
	}else if(mmc->clock <= (26*1000*1000)){
		mmchost->mmc_clk_dly[MMC_CLK_25M] .sclk_dly= sdly;
	}else if(mmc->clock <= (52*1000*1000)){
		mmchost->mmc_clk_dly[MMC_CLK_50M] .sclk_dly= sdly;
	}else  if(mmc->clock <= (100*1000*1000)){
		mmchost->mmc_clk_dly[MMC_CLK_100M] .sclk_dly= sdly;
	}else if(mmc->clock <= (200*1000*1000)){
		mmchost->mmc_clk_dly[MMC_CLK_200M] .sclk_dly= sdly;
	}else {
		MMCINFO("clk %d is over the tuning clk %d\n",mmc->clock,200*1000*1000);
		return -1;
	}
	return 0;
}


int sunxi_detail_errno(struct mmc *mmc)
{
	struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
	u32 err_no = mmchost->raw_int_bak;
	mmchost->raw_int_bak = 0;
	return err_no;

}


/*
void secure_test(struct mmc *mmc)
{
	const char w_buf[512] = {
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,

		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,

		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,

		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,

		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,

		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,

		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,

		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,

	};

	mmc_init(mmc);
	char *buf = (char *)malloc((SDMMC_ITEM_SIZE*2*MAX_SECURE_STORAGE_MAX_ITEM+20)*512);
	mmc->block_dev.block_read(2,SDMMC_SECURE_STORAGE_START_ADD,10,buf);
	mmc->block_dev.block_write(2,SDMMC_SECURE_STORAGE_START_ADD,10,buf);
	mmc->block_dev.block_read_mass_pro(2,SDMMC_SECURE_STORAGE_START_ADD,10,buf);
	mmc->block_dev.block_write_mass_pro(2,SDMMC_SECURE_STORAGE_START_ADD,10,buf);

	MMCINFO("*******************\n");
	mmc->block_dev.block_read(2,SDMMC_SECURE_STORAGE_START_ADD+2,10,buf);
	mmc->block_dev.block_write(2,SDMMC_SECURE_STORAGE_START_ADD-3,10,buf);
	mmc->block_dev.block_read_mass_pro(2,SDMMC_SECURE_STORAGE_START_ADD+8,12,buf);
	mmc->block_dev.block_write_mass_pro(2,SDMMC_SECURE_STORAGE_START_ADD-7,13,buf);

	MMCINFO("*******************\n");
	mmc->block_dev.block_read(2,SDMMC_SECURE_STORAGE_START_ADD+2,(SDMMC_ITEM_SIZE*2*MAX_SECURE_STORAGE_MAX_ITEM)+3,buf);
	mmc->block_dev.block_write(2,SDMMC_SECURE_STORAGE_START_ADD-9,(SDMMC_ITEM_SIZE*2*MAX_SECURE_STORAGE_MAX_ITEM)+4,buf);
	mmc->block_dev.block_read_mass_pro(2,SDMMC_SECURE_STORAGE_START_ADD+11,(SDMMC_ITEM_SIZE*2*MAX_SECURE_STORAGE_MAX_ITEM)+9,buf);
	mmc->block_dev.block_write_mass_pro(2,SDMMC_SECURE_STORAGE_START_ADD-20,(SDMMC_ITEM_SIZE*2*MAX_SECURE_STORAGE_MAX_ITEM)+10,buf);



	mmc->block_dev.block_write_secure(2,0,(void *)w_buf,1);
	mmc->block_dev.block_read_secure(2,0,(u8 *)buf,1);
	if(memcmp((void *)w_buf,(void *)buf,512)){
		MMCINFO("cmp failed\n");
	}else{
		MMCINFO("cmp ok\n");
	}
}
*/


int sunxi_mmc_init(int sdc_no)
{
	struct mmc *mmc;
	int work_mode = uboot_spare_head.boot_data.work_mode;

	MMCINFO("mmc driver ver %s\n", DRIVER_VER);

	memset(&mmc_dev[sdc_no], 0, sizeof(struct mmc));
	memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));
	mmc = &mmc_dev[sdc_no];

	sprintf(mmc->name, "SUNXI SD/MMC");
	mmc->priv = &mmc_host[sdc_no];
	mmc->send_cmd = mmc_send_cmd;
	mmc->set_ios = mmc_set_ios;
	mmc->init = mmc_core_init;
	mmc->control_num = sdc_no;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34
		| MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30
		| MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_34_35
		| MMC_VDD_35_36;
	//bus width will be change in mmc_clk_io_onoff according to sys_config.fex
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT;
	mmc->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS| MMC_MODE_HC | MMC_MODE_DDR_52MHz;

	if(sdc_no == 0){
       mmc->f_min = 400000;
#if defined(CONFIG_ARCH_SUN9IW1P1)
       mmc->f_max = 48000000;
#else
       mmc->f_max = 50000000;
#endif
	}else if (sdc_no == 2){
       mmc->f_min = 400000;
#if defined(CONFIG_ARCH_SUN9IW1P1)
       mmc->f_max = 48000000;
#else
       mmc->f_max = 50000000;
#endif
	}

	MMCDBG("mmc->host_caps %x\n",mmc->host_caps);
	mmc_host[sdc_no].pdes = malloc(64 * 1024);
	if(mmc_host[sdc_no].pdes == NULL){
		MMCINFO("get mem for descriptor failed\n");
		return -1;
	}
	mmc_resource_init(sdc_no);
	mmc_clk_io_onoff(sdc_no,1);

#if defined CONFIG_ARCH_SUN9IW1P1
	MMCINFO("PC Bias: 0x%08x 0x%08x\n", (0x6000800+0x308), *(volatile unsigned int *)(0x6000800+0x308));
#endif
	if((mmc->sample_mode == AUTO_SAMPLE_MODE)
		&&(work_mode != WORK_MODE_BOOT)){
		mmc->decide_retry = sunxi_decide_rty;
		mmc->update_sdly =sunxi_update_sdly;
		mmc->get_detail_errno = sunxi_detail_errno;
	}

	/****************mmc function enable*********************/
	if(mmc->mmc_func_en.ddr_func_en){
		mmc->host_caps |= MMC_MODE_DDR_52MHz;
	}
	else{
		mmc->host_caps &= ~(MMC_MODE_DDR_52MHz);
	}
	//while((*(volatile unsigned int *)0) != 1);
	mmc_register(mmc);
	//secure_test(mmc);

	return 0;
}

int sunxi_mmc_exit(int sdc_no)
{
	mmc_clk_io_onoff(sdc_no, 0);
	mmc_unregister(sdc_no);
	//memset(&mmc_dev[sdc_no], 0, sizeof(struct mmc));
	//memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));
	MMCDBG("sunxi mmc%d exit\n",sdc_no);
	return 0;
}
