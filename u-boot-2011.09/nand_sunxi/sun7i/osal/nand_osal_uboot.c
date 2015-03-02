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
#include  <common.h>
#include  <malloc.h>
#include  <stdarg.h>
#include  <asm/arch/dma.h>
#include  <sys_config.h>

//#define NAND_DEBUG
#if defined   CONFIG_ARCH_SUN7I
#define SUN7I
#elif defined CONFIG_ARCH_SUN8IW1P1
#define  SUN8IW1P1
#elif defined CONFIG_ARCH_SUN8IW3P1
#define  SUN8IW3P1
#elif defined CONFIG_ARCH_SUN9IW1P1
#define  SUN9IW1P1
#else
#error "please select a platform\n"
#endif

#define   CCMU_REGS_BASE    0x01c20000

unsigned int hDMA;


int malloc_size = 0;

static __u32 boot_mode;

int NAND_set_boot_mode(__u32 boot)
{
	boot_mode = boot;
	return 0;
}

int NAND_Print(const char * str, ...)
{
	if(boot_mode)
		return 0;
	else
	{
	    static char _buf[1024];
	    va_list args;

	    va_start(args, str);
	    vsprintf(_buf, str, args);

	    printf(_buf);
		return 0;
	}
    
}

__s32 NAND_CleanFlushDCacheRegion(__u32 buff_addr, __u32 len)
{
	flush_cache(buff_addr, len);

	return 0;
}

__u32 NAND_DMASingleMap(__u32 rw, __u32 buff_addr, __u32 len)
{
	return buff_addr;
}

__u32 NAND_DMASingleUnmap(__u32 rw, __u32 buff_addr, __u32 len)
{
	return buff_addr;
}


int NAND_WaitDmaFinish(void)
{
    return 0;
}

unsigned int NAND_RequestDMA(void)
{

	hDMA = sunxi_dma_request(1);
	if(!hDMA)
	{
		printf("nand : request dma failed\n");

		return 0;
	}
	return hDMA;
}
int NAND_ReleaseDMA(void)
{
	/* stop dma                 */
//    DMA_Stop(hDMA);

 //   return DMA_Release(hDMA);
}

__s32 NAND_DMAConfigStart(int rw, unsigned int buff_addr, int len)
{

    sunxi_dma_setting_t dma_param;
    __u32 saddr, daddr;

	if(rw)  //write
	{

		dma_param.cfg.src_drq_type = DMAC_CFG_SRC_TYPE_D_SDRAM;
		dma_param.cfg.src_addr_type = DMAC_CFG_SRC_ADDR_TYPE_LINEAR_MODE;  //linemode
		dma_param.cfg.src_burst_length = DMAC_CFG_SRC_4_BURST;  //burst mode
		dma_param.cfg.src_data_width = DMAC_CFG_SRC_DATA_WIDTH_32BIT;  //32bit

		dma_param.cfg.dst_drq_type = DMAC_CFG_DEST_TYPE_NFC;
		dma_param.cfg.dst_addr_type = DMAC_CFG_DEST_ADDR_TYPE_IO_MODE; //IO mode
		dma_param.cfg.dst_burst_length = DMAC_CFG_DEST_4_BURST; // burst mode
		dma_param.cfg.dst_data_width = DMAC_CFG_DEST_DATA_WIDTH_32BIT; //32 bit

		dma_param.wait_cyc = 0x07; // invalid value
		dma_param.loop_mode = DMAC_CFG_CONTINUOUS_DISABLE; //no continous

		dma_param.data_block_size =  0x7f; //commit register

	}
	else //read
	{
		dma_param.cfg.src_drq_type = DMAC_CFG_SRC_TYPE_NFC;
		dma_param.cfg.src_addr_type = DMAC_CFG_SRC_ADDR_TYPE_IO_MODE;  //IO mode
		dma_param.cfg.src_burst_length = DMAC_CFG_SRC_4_BURST;  //burst mode
		dma_param.cfg.src_data_width = DMAC_CFG_SRC_DATA_WIDTH_32BIT;  //32bit

		dma_param.cfg.dst_drq_type = DMAC_CFG_DEST_TYPE_D_SDRAM;
		dma_param.cfg.dst_addr_type = DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE; //line mode
		dma_param.cfg.dst_burst_length = DMAC_CFG_DEST_4_BURST; // burst mode
		dma_param.cfg.dst_data_width = DMAC_CFG_DEST_DATA_WIDTH_32BIT; //32 bit

		dma_param.wait_cyc = 0x07; // invalid value
		dma_param.loop_mode = DMAC_CFG_CONTINUOUS_DISABLE; //no continous

		dma_param.data_block_size =  0x7f; //commit register
	}

	sunxi_dma_setting(hDMA, &dma_param);
	
	if(rw) //write
	{
	    saddr = buff_addr;
	    daddr = 0x01c03030;
	}
	else
	{
	    daddr = buff_addr;
	    saddr = 0x01c03030;
	}

    flush_cache(buff_addr, len);
    sunxi_dma_start(hDMA, saddr, daddr, len);

	return 0;
}

__u32  	NAND_QueryDmaStat(void)
{
    //return DMA_QueryStatus(hDMA);
}

int NAND_ClkRequest(void)
{
    return 0;
}

void NAND_ClkRelease(void)
{
    return ;
}

int NAND_AHBEnable(void)
{
    *(volatile __u32 *)(CCMU_REGS_BASE + 0x60) |= 1U << 13;
    return 0;
}

void NAND_AHBDisable(void)
{
    *(volatile __u32 *)(CCMU_REGS_BASE + 0x60) &= ~(1U << 13);
}

int NAND_ClkEnable(void)
{
     *(volatile __u32 *)(CCMU_REGS_BASE + 0x80) |= 1U<< 31;
     
     return 0;
}

void NAND_ClkDisable(void)
{
    *(volatile __u32 *)(CCMU_REGS_BASE + 0x80) &= ~(1U << 31);
}

/*
**********************************************************************************************************************
*
*             NAND_GetCmuClk
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/

__u32 _GetCmuClk(void)
{
	__u32 reg_val;
	__u32 factor_n;
	__u32 factor_k;
	__u32 clock;

	reg_val  = *(volatile unsigned int *)(0x01c20000 + 0x28);//PLL6
	factor_n = (reg_val >> 8) & 0x1f;
	factor_k = ((reg_val >> 4) & 0x3) + 1;

	clock = 24 * factor_n * factor_k/2;

	return clock;
}


int NAND_SetClk(unsigned int nand_clock)
{
	__u32 edo_clk, cmu_clk;
	__u32 cfg;
	__u32 nand_clk_divid_ratio;

	/*set nand clock*/
	//edo_clk = (nand_max_clock > 20)?(nand_max_clock-10):nand_max_clock;
	edo_clk = nand_clock * 2;

    cmu_clk = _GetCmuClk( );
	nand_clk_divid_ratio = cmu_clk / edo_clk;
	if (cmu_clk % edo_clk)
			nand_clk_divid_ratio++;
	if (nand_clk_divid_ratio){
		if (nand_clk_divid_ratio > 16)
			nand_clk_divid_ratio = 15;
		else
			nand_clk_divid_ratio--;
	}
	/*set nand clock gate on*/
	cfg = *(volatile __u32 *)(CCMU_REGS_BASE + 0x80);

	/*gate on nand clock*/
	cfg |= (1U << 31);
	/*take pll6 as nand src block*/
	cfg &= ~(0x3 << 24);
	cfg |=  (0x1 << 24);
	//set divn = 0
	cfg &= ~(0x03 << 16);

	/*set ratio*/
	cfg &= ~(0x0f << 0);
	cfg |= (nand_clk_divid_ratio & 0xf) << 0;

	*(volatile __u32 *)(CCMU_REGS_BASE + 0x80) = cfg;
	
	return 0;
}

int NAND_GetClk(void)
{
	__u32 cmu_clk;
	__u32 cfg;
	__u32 nand_max_clock, nand_clk_divid_ratio;

	/*set nand clock*/
    cmu_clk = _GetCmuClk( );
    
    /*set nand clock gate on*/
	cfg = *(volatile __u32 *)(CCMU_REGS_BASE + 0x80);
    nand_clk_divid_ratio = ((cfg)&0xf) +1;
    nand_max_clock = cmu_clk/(2*nand_clk_divid_ratio);
    
    return nand_max_clock;
    

}

void NAND_PIORequest(void)
{
	*(volatile uint *)(0x01c20800 + 0x48) = 0x22222222;
	*(volatile uint *)(0x01c20800 + 0x4C) = 0x22222222;
	*(volatile uint *)(0x01c20800 + 0x50) = 0x2222222;
	*(volatile uint *)(0x01c20800 + 0x54) = 0x2;

}

void NAND_PIORelease(void)
{
	return;
}

void NAND_Memset(void* pAddr, unsigned char value, unsigned int len)
{
    memset(pAddr, value, len);
}

void NAND_Memcpy(void* pAddr_dst, void* pAddr_src, unsigned int len)
{
    memcpy(pAddr_dst, pAddr_src, len);
}

#if 0
#define NAND_MEM_BASE  0x59000000

void * NAND_Malloc(unsigned int Size)
{
	__u32 mem_addr;

	mem_addr = NAND_MEM_BASE+malloc_size;

	malloc_size += Size;
	if(malloc_size%4)
		malloc_size += (4-(malloc_size%4));

	//NAND_Print("NAND_Malloc: 0x%x\n", NAND_MEM_BASE + malloc_size);

	if(malloc_size>0x4000000)
		return NULL;
	else
		return (void *)mem_addr;
}

void NAND_Free(void *pAddr, unsigned int Size)
{
	//free(pAddr);
}

#else
void * NAND_Malloc(unsigned int Size)
{
	return malloc(Size);
}

void NAND_Free(void *pAddr, unsigned int Size)
{
	free(pAddr);
}
#endif




void  OSAL_IrqUnLock(unsigned int  p)
{
    ;
}
void  OSAL_IrqLock  (unsigned int *p)
{
    ;
}

int NAND_WaitRbReady(void)
{
    return 0;
}

void *NAND_IORemap(unsigned int base_addr, unsigned int size)
{
    return (void *)base_addr;
}

__u32 NAND_VA_TO_PA(__u32 buff_addr)
{
    return buff_addr;
}

__u32 NAND_GetIOBaseAddrCH0(void)
{
	return 0x01c03000;
}

__u32 NAND_GetIOBaseAddrCH1(void)
{
	return 0x01c05000;
}


int NAND_PhysicLockInit(void)
{
    return 0;
}

int NAND_PhysicLock(void)
{
    //down(&nand_physic_mutex);
     return 0;
}

int NAND_PhysicUnLock(void)
{
    //up(&nand_physic_mutex);
     return 0;
}

int NAND_PhysicLockExit(void)
{
     return 0;
}


__u32 NAND_GetPlatform(void)
{

	return 20;


}

//__u32 NAND_Get_nandp0(void)
//{
//	__u32 nandp0;
//	script_parser_value_type_t ret;
//
//	ret = script_parser_fetch("nand_para", "nand_p0", &nandp0, 1);	
//	if(ret!=SCRIPT_PARSER_OK)
//	{
//		tick_printf("nand : get nand_p0 fail,%x\n",nandp0);
//		return 0xffffffff;
//	}
//	else
//		return nandp0;
//}
//
//
//__u32 NAND_Getidnumberctl(void)
//{
//	__u32 id_number_ctl;
//	script_parser_value_type_t ret;
//
//	ret = script_parser_fetch("nand_para", "id_number_ctl", &id_number_ctl, 1);	
//	if(ret!=SCRIPT_PARSER_OK)
//	{
//		tick_printf("nand : get id_number_ctl fail,%x \n",id_number_ctl);
//		return 0x0;
//	}
//	tick_printf("nand : get id_number_ctl from script,%x \n",id_number_ctl);
//	return id_number_ctl;
//}


__u32 NAND_GetNandExtPara(__u32 para_num)
{
	int nand_para;
	script_parser_value_type_t ret;
	
	if (para_num == 0) {
		ret = script_parser_fetch("nand_para", "nand_p0", &nand_para, 1);	
		if(ret!=SCRIPT_PARSER_OK)
		{
			printf("NAND_GetNandExtPara: get nand_p0 fail, %x\n", nand_para);
			return 0xffffffff;
		}
		else
			return nand_para;
	} else if (para_num == 1) {
		ret = script_parser_fetch("nand_para", "nand_p1", &nand_para, 1);	
		if(ret!=SCRIPT_PARSER_OK)
		{
			printf("NAND_GetNandExtPara: get nand_p1 fail, %x\n", nand_para);
			return 0xffffffff;
		}
		else
			return nand_para;		
	} else {
		printf("NAND_GetNandExtPara: wrong para num: %d\n", para_num);
		return 0xffffffff;
	}
}

__u32 NAND_GetNandIDNumCtrl(void)
{
	int id_number_ctl;
	script_parser_value_type_t ret;

	ret = script_parser_fetch("nand_para", "id_number_ctl", &id_number_ctl, 1);	
	if(ret!=SCRIPT_PARSER_OK) {
		printf("nand : get id_number_ctl fail, %x\n",id_number_ctl);
		return 0x0;
	} else {
		printf("nand : get id_number_ctl from script, %x\n",id_number_ctl);
		return id_number_ctl;
	}	
}

__u32 NAND_GetNandCapacityLevel(void)
{
	int CapacityLevel;
	script_parser_value_type_t ret;

	ret = script_parser_fetch("nand_para", "nand_capacity_level", &CapacityLevel, 1);
	if(ret!=SCRIPT_PARSER_OK) {
		printf("nand : get CapacityLevel fail, %x\n",CapacityLevel);
		return 0x0;
	} else {
		printf("nand : get CapacityLevel from script, %x\n",CapacityLevel);
		return CapacityLevel;
	}
}

void NAND_ShowEnv(__u32 type, char *name, __u32 len, __u32 *val)
{
	int i;

    if (len && (val==NULL)) {
	printf("uboot:NAND_ShowEnv, para error!\n");
	return ;
    }

    if (type == 0)
    {
	printf("uboot:%s: ", name);
	for (i=0; i<len; i++)
	{
		if (i && (i%8==0))
			printf("\n");
		printf("%x ", val[i]);
	}
	printf("\n");
    }
    else
    {
	printf("uboot:NAND_ShowEnv, type error, %d!\n", type);
    }

    return ;
}

