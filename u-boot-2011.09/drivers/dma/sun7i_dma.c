/*
 * (C) Copyright 2007-2012
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
#include <asm/arch/cpu.h>
#include <asm/arch/dma_i.h>
#include <asm/arch/dma.h>
#include <asm/arch/intc.h>


#define SUNXI_DMA_MAX     (CFG_SW_DMA_NORMAL_MAX + CFG_SW_DMA_DEDICATE_MAX)
#define SUNXI_DMA_CHANANL_SIZE    (0x20)
#define  DMA_PKG_HALF_INT   (1<<0)
#define  DMA_PKG_END_INT    (1<<1)
#define  DMA_QUEUE_END_INT  (1<<2)

typedef struct
{
	unsigned int irq_en;
	unsigned int irq_pend;
	unsigned int status;
}
sunxi_dma_int_set;

typedef struct sunxi_dma_channal_set_t
{
	volatile unsigned int enable;
	volatile unsigned int pause;
	volatile unsigned int start_addr;		//起始地址
	volatile unsigned int config;
	volatile unsigned int cur_src_addr;		//当前传输地址
	volatile unsigned int cur_dst_addr;
	volatile unsigned int left_bytes;		//剩余未传字节数
	volatile unsigned int parameters;		//参数
}
sunxi_dma_channal_set;



typedef struct sunxi_dma_source_t
{
	unsigned int      		used;
	unsigned int            channal_count;
	sunxi_dma_channal_set	*channal;
	unsigned int			reserved;
	volatile sunxi_dma_start_t  config;
	struct dma_irq_handler  dma_func;
}
sunxi_dma_source;

static int    dma_int_count = 0;

sw_dma_channal_set_t  dma_channal[CFG_SW_DMA_NORMAL_MAX + CFG_SW_DMA_DEDICATE_MAX] =
{
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL0  , 0                                           },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL1  , 0                                           },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL2  , 0                                           },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL3  , 0                                           },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL4  , 0                                           },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL5  , 0                                           },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL6  , 0                                           },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL7  , 0                                           },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE0, (sw_dma_other_t)CFG_SW_DMA_DEDICATE0_OTHER  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE1, (sw_dma_other_t)CFG_SW_DMA_DEDICATE1_OTHER  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE2, (sw_dma_other_t)CFG_SW_DMA_DEDICATE2_OTHER  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE3, (sw_dma_other_t)CFG_SW_DMA_DEDICATE3_OTHER  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE4, (sw_dma_other_t)CFG_SW_DMA_DEDICATE4_OTHER  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE5, (sw_dma_other_t)CFG_SW_DMA_DEDICATE5_OTHER  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE6, (sw_dma_other_t)CFG_SW_DMA_DEDICATE6_OTHER  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE7, (sw_dma_other_t)CFG_SW_DMA_DEDICATE7_OTHER  }
};


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
static void sunxi_dma_int_func(void *p)
{
    int i;
	uint pending;
	sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	for(i=0; i<SUNXI_DMA_MAX; i++)
	{
		if(dma_channal[i].dma_func.m_func)
		{
			pending = (DMA_PKG_END_INT << (i * 2));
			if(dma_int->irq_pend & pending)
			{
				dma_int->irq_pend = pending;

				dma_channal[i].dma_func.m_func(dma_channal[i].dma_func.m_data);
			}
		}
	}
    
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
void sunxi_dma_init(void)
{
	int i;
    sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	dma_int->irq_en = 0;

	dma_int->irq_pend = 0xffffffff;
	for(i=0;i<SUNXI_DMA_MAX;i++){
		memset((void*)(&dma_channal[i].dma_func), 0, sizeof(struct dma_irq_handler));
	}
	dma_int_count = 0;
	irq_install_handler(AW_IRQ_DMA, sunxi_dma_int_func, 0);

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
void sunxi_dma_exit(void)
{
    sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	dma_int->irq_en = 0;

	dma_int->irq_pend = 0xffffffff;

	irq_free_handler(AW_IRQ_DMA);
}

/*
****************************************************************************************************
*
*             DMAC_RequestDma
*
*  Description:
*       request dma
*
*  Parameters:
*		type	0: normal timer
*				1: special timer
*  Return value:
*		dma handler
*		if 0, fail
****************************************************************************************************
*/
uint sunxi_dma_request(uint dmatype)
{
    uint   i;

    if(dmatype == DMAC_DMATYPE_NORMAL)
    {
        for(i=0;i<CFG_SW_DMA_NORMAL_MAX;i++)
        {
            if(!dma_channal[i].used)
            {
                dma_channal[i].used = 1;
                dma_channal[i].channalNo = i;
				
                return (uint)&dma_channal[i];
            }
        }
    }
    else if(dmatype == DMAC_DMATYPE_DEDICATED)
    {
        for(i=CFG_SW_DMA_NORMAL_MAX;i<CFG_SW_DMA_NORMAL_MAX + CFG_SW_DMA_DEDICATE_MAX;i++)
        {
            if(!dma_channal[i].used)
            {
                dma_channal[i].used = 1;
                dma_channal[i].channalNo = i;

                return (uint)&dma_channal[i];
            }
        }
    }

    return 0;
}
/*
****************************************************************************************************
*
*             DMAC_ReleaseDma
*
*  Description:
*       release dma
*
*  Parameters:
*       hDma	dma handler
*
*  Return value:
*		EPDK_OK/FAIL
****************************************************************************************************
*/
int sunxi_dma_release(uint hDma)
{
	 sw_dma_channal_set_t  *dma_channal = (sw_dma_channal_set_t *)hDma;
	sunxi_dma_disable_int(hDma);
	sunxi_dma_free_int(hDma);
    /* stop dma */    
	dma_channal->channal->config &= 0x7fffffff; /* stop dma */    
	/* free dma handle */    
	dma_channal->used = 0;    
	dma_channal->channalNo = -1;
    return 0;
}
/*
****************************************************************************************************
*
*             TMRC_SettingDMA
*
*  Description:
*       start interrupt
*
*  Parameters:
*       hTmr	timer handler
*		pArg    *(pArg + 0)         ctrl
*               *(pArg + 1)         page size
*               *(pArg + 2)         page step
*               *(pArg + 3)         comity & block count
*
*  Return value:
*		EPDK_OK/FAIL
*
**********************************************************************************************************************
*/
int sunxi_dma_setting(uint hDMA, sunxi_dma_setting_t *cfg)
{
	sw_dma_channal_set_t    * pDma = (sw_dma_channal_set_t *)hDMA;
	sunxi_dma_setting_t     * arg  = (sunxi_dma_setting_t  *)cfg;
	uint                   value;
	uint *tmp = NULL;
    if(pDma->other)
    {
        __ddma_config_t    ddma_arg = {0};
		tmp = (uint *)&ddma_arg;
        ddma_arg.src_drq_type     = arg->cfg.src_drq_type;
        ddma_arg.src_addr_type    = arg->cfg.src_addr_type;
        ddma_arg.src_burst_length = arg->cfg.src_burst_length;
        ddma_arg.src_data_width   = arg->cfg.src_data_width;

        ddma_arg.dst_drq_type     = arg->cfg.dst_drq_type;
        ddma_arg.dst_addr_type    = arg->cfg.dst_addr_type;
        ddma_arg.dst_burst_length = arg->cfg.dst_burst_length;
        ddma_arg.dst_data_width   = arg->cfg.dst_data_width;

        ddma_arg.continuous_mode  = arg->cfg.continuous_mode;
		value = *tmp;
        pDma->channal->config       = value & (~0x80000000);
        pDma->other->src_wait_cyc         = arg->wait_cyc;
		pDma->other->src_data_block_size  = arg->data_block_size;
        pDma->other->dst_wait_cyc         = arg->wait_cyc;
        pDma->other->dst_data_block_size  = arg->data_block_size;
    }
    else
    {
        __ndma_config_t    ndma_arg = {0};
		tmp = (uint *)&ndma_arg;
        ndma_arg.src_drq_type     = arg->cfg.src_drq_type;
        ndma_arg.src_addr_type    = arg->cfg.src_addr_type;
        ndma_arg.src_burst_length = arg->cfg.src_burst_length;
        ndma_arg.src_data_width   = arg->cfg.src_data_width;

        ndma_arg.dst_drq_type     = arg->cfg.dst_drq_type;
        ndma_arg.dst_addr_type    = arg->cfg.dst_addr_type;
        ndma_arg.dst_burst_length = arg->cfg.dst_burst_length;
        ndma_arg.dst_data_width   = arg->cfg.dst_data_width;

        ndma_arg.continuous_mode  = arg->cfg.continuous_mode;
        ndma_arg.wait_state       = arg->cfg.wait_state;

        value = *tmp;
        pDma->channal->config       = value & (~0x80000000);
    }

    return 0;
}

/*
**********************************************************************************************************************
*
*             TMRC_StartTmr
*
*  Description:
*       start interrupt
*
*  Parameters:
*       hTmr	timer handler
*
*  Return value:
*		EPDK_OK/FAIL
*
****************************************************************************************************
*/
int sunxi_dma_start(uint hDMA, uint saddr, uint daddr, uint bytes)
{
	sw_dma_channal_set_t  * pDma = (sw_dma_channal_set_t *)hDMA;

    pDma->channal->src_addr  = saddr;
    pDma->channal->dst_addr  = daddr;
    pDma->channal->bytes     = bytes;
    pDma->channal->config   |= 0x80000000;   /* start dma */

    return 0;
}
/*
**********************************************************************************************************************
*
*             DMAC_StopDma
*
*  Description:
*       stop dma
*
*  Parameters:
*       hDma	dma handler
*
*  Return value:
*		EPDK_OK/FAIL
*
**********************************************************************************************************************
*/
int sunxi_dma_stop(uint hDma)
{
	sw_dma_channal_set_t    * pDma = (sw_dma_channal_set_t *)hDma;

    pDma->channal->config &= 0x7fffffff; /* stop dma */

    return 0;
}
/*
**********************************************************************************************************************
*
*             eGon2_RestartDMA
*
*  Description:
*       restart dma
*
*  Parameters:
*       hDma	dma handler
*
*  Return value:
*		EPDK_OK/FAIL
*
**********************************************************************************************************************
*/
int sunxi_dma_restart(uint hDma)
{
    sw_dma_channal_set_t    * pDma = (sw_dma_channal_set_t *)hDma;

    pDma->channal->config |= 0x80000000;

    return 0;
}
/*
**********************************************************************************************************************
*
*             eGon2_QueryDMAStatus
*
*  Description:
*
*
*  Parameters:
*       hDma	dma handler
*
*  Return value:
*		EPDK_OK/FAIL
*
**********************************************************************************************************************
*/
int sunxi_dma_querystatus(uint hDma)
{
    sw_dma_channal_set_t    * pDma = (sw_dma_channal_set_t *)hDma;

    return (pDma->channal->config >> 31) & 0x01;
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
int sunxi_dma_install_int(uint hdma, interrupt_handler_t dma_int_func, void *p)
{
	sw_dma_channal_set_t *dma_channal = (sw_dma_channal_set_t *)hdma;
	sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	uint  channal_count;

	if(!dma_channal->used)
	{
		return -1;
	}
	channal_count = dma_channal->channalNo;
	
	//clear half pending and end pending.
	dma_status->irq_pend = (0x3 << (channal_count*2));
    

	if(!dma_channal->dma_func.m_func)
	{
		dma_channal->dma_func.m_func = dma_int_func;
		dma_channal->dma_func.m_data = p;
	}
	else
	{
		printf("dma 0x%x int is used already, you have to free it first\n", hdma);

		return -1;
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
int sunxi_dma_enable_int(uint hdma)
{
	sw_dma_channal_set_t     *dma_channal = (sw_dma_channal_set_t *)hdma;
	sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	uint  channal_count;

	if(!dma_channal->used)
	{
		return -1;
	}

	channal_count = dma_channal->channalNo;
    //enable the ending irq.
	dma_status->irq_en = (0x2 << (channal_count*2));

	if(!dma_int_count)
	{
		irq_enable(AW_IRQ_DMA);
	}
	dma_int_count ++;

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
int sunxi_dma_disable_int(uint hdma)
{
	sw_dma_channal_set_t     *dma_channal = (sw_dma_channal_set_t *)hdma;
	sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	uint  channal_count;

	if(!dma_channal->used)
	{
		return -1;
	}

	channal_count = dma_channal->channalNo;
	dma_status->irq_en &= ~(0x2 << (channal_count*2));
	//disable golbal int
	if(dma_int_count > 0)
	{
		dma_int_count --;
	}
	if(!dma_int_count)
	{
		irq_disable(AW_IRQ_DMA);
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
int sunxi_dma_free_int(uint hdma)
{
	sw_dma_channal_set_t     *dma_channal = (sw_dma_channal_set_t *)hdma;
	sunxi_dma_int_set    *dma_status  = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	uint  channal_count;

	if(!dma_channal->used)
	{
		return -1;
	}
	channal_count = dma_channal->channalNo;
	
	//enable the ending irq.
	dma_status->irq_pend= (0x3 << (channal_count*2));
	if(dma_channal->dma_func.m_func)
	{
		dma_channal->dma_func.m_func = NULL;
		dma_channal->dma_func.m_data = NULL;
	}
	else
	{
		printf("dma 0x%x int is free, you do not need to free it again\n", hdma);

		return -1;
	}

	return 0;
}


/*
**********************************************************************************************************************
*
*             eGon2_QueryDMAChannalNo
*
*  Description:
*
*
*  Parameters:
*       hDma	dma handler
*
*  Return value:
*		dma channael no.
*
**********************************************************************************************************************
*/
int sunxi_dma_querychannalno(uint hDma)
{
    sw_dma_channal_set_t    * pDma = (sw_dma_channal_set_t *)hDma;

    return pDma->channalNo;
}




