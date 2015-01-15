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
#include <asm/arch/dma.h>
#include <asm/arch/intc.h>
#include <asm/arch/cpu.h>

sw_dma_channal_set_t  dma_channal_source[CFG_SW_DMA_NORMAL_MAX + CFG_SW_DMA_DEDICATE_MAX] =
{
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL0  , 0                                         , {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL1  , 0                                         , {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL2  , 0                                         , {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL3  , 0                                         , {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL4  , 0                                         , {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL5  , 0                                         , {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL6  , 0                                         , {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFS_SW_DMA_NORMAL7  , 0                                         , {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE0, (sw_dma_other_t)CFG_SW_DMA_DEDICATE0_OTHER, {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE1, (sw_dma_other_t)CFG_SW_DMA_DEDICATE1_OTHER, {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE2, (sw_dma_other_t)CFG_SW_DMA_DEDICATE2_OTHER, {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE3, (sw_dma_other_t)CFG_SW_DMA_DEDICATE3_OTHER, {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE4, (sw_dma_other_t)CFG_SW_DMA_DEDICATE4_OTHER, {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE5, (sw_dma_other_t)CFG_SW_DMA_DEDICATE5_OTHER, {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE6, (sw_dma_other_t)CFG_SW_DMA_DEDICATE6_OTHER, {NULL, NULL}  },
    {0, -1, (sw_dma_t)CFG_SW_DMA_DEDICATE7, (sw_dma_other_t)CFG_SW_DMA_DEDICATE7_OTHER, {NULL, NULL}  }
};

typedef struct
{
	unsigned int irq_en0;
	unsigned int irq_pending0;
}
sunxi_dma_int_set;

#define  DMA_PKG_HALF_INT   (1<<0)
#define  DMA_PKG_END_INT    (1<<1)
#define  DMA_QUEUE_END_INT  (1<<2)

static int dma_int_count;
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

	for(i=0; i<=31; i+=2)
	{
		if(dma_channal_source[i/2].dma_func.m_func)
		{
			pending = (DMA_PKG_END_INT << i);
			if(dma_int->irq_pending0 & pending)
			{
				dma_int->irq_pending0 = pending;

				dma_channal_source[i/2].dma_func.m_func(dma_channal_source[i].dma_func.m_data);
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
    sunxi_dma_int_set *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	dma_int->irq_en0 = 0;
	dma_int->irq_pending0 = 0xffffffff;

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

	dma_int->irq_en0 = 0;
	dma_int->irq_pending0 = 0xffffffff;

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
    __u32   i;

    if(dmatype == DMAC_DMATYPE_NORMAL)
    {
        for(i=0;i<CFG_SW_DMA_NORMAL_MAX;i++)
        {
            if(!dma_channal_source[i].used)
            {
                dma_channal_source[i].used = 1;
                dma_channal_source[i].channalNo = i;

                return (__u32)&dma_channal_source[i];
            }
        }
    }
    else if(dmatype == DMAC_DMATYPE_DEDICATED)
    {
        for(i=CFG_SW_DMA_NORMAL_MAX;i<CFG_SW_DMA_NORMAL_MAX + CFG_SW_DMA_DEDICATE_MAX;i++)
        {
            if(!dma_channal_source[i].used)
            {
                dma_channal_source[i].used = 1;
                dma_channal_source[i].channalNo = i;

                return (__u32)&dma_channal_source[i];
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
int sunxi_dma_release(uint hdma)
{
    sw_dma_channal_set_t * pdma = (sw_dma_channal_set_t *)hdma;

	sunxi_dma_disable_int(hdma);
	sunxi_dma_free_int(hdma);
    /* stop dma                 */
    pdma->channal->config &= 0x7fffffff; /* stop dma */
    /* free dma handle          */
    pdma->used = 0;
    pdma->channalNo = -1;

    return 0;
}
/*
****************************************************************************************************
*
*             sunxi_dma_setting
*
*  Description:
*       start interrupt
*
*  Parameters:
*
*
*
*
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int sunxi_dma_setting(uint hdma, void *cfg)
{
	sw_dma_channal_set_t    * pdma = (sw_dma_channal_set_t *)hdma;
	sunxi_dma_setting_t     * arg  = (sunxi_dma_setting_t  *)cfg;
	__u32                   value;
	uint *tmp;

	if(pdma->other)
    {
        __ddma_config_t    ddma_arg = {0};

        ddma_arg.src_drq_type     = arg->cfg.src_drq_type;
        ddma_arg.src_addr_type    = arg->cfg.src_addr_mode;
        ddma_arg.src_burst_length = arg->cfg.src_burst_length;
        ddma_arg.src_data_width   = arg->cfg.src_data_width;

        ddma_arg.dst_drq_type     = arg->cfg.dst_drq_type;
        ddma_arg.dst_addr_type    = arg->cfg.dst_addr_mode;
        ddma_arg.dst_burst_length = arg->cfg.dst_burst_length;
        ddma_arg.dst_data_width   = arg->cfg.dst_data_width;

        ddma_arg.continuous_mode  = arg->cfg.continuous_mode;

		tmp = (uint *)&ddma_arg;
        value = *tmp;
        pdma->channal->config       = value & (~0x80000000);
        pdma->other->page_size      = arg->pgsz;
        pdma->other->page_step      = arg->pgstp;
        pdma->other->comity_counter = arg->cmt_blk_cnt;
    }
    else
    {
        __ndma_config_t    ndma_arg = {0};

        ndma_arg.src_drq_type     = arg->cfg.src_drq_type;
        ndma_arg.src_addr_type    = arg->cfg.src_addr_mode;
        ndma_arg.src_burst_length = arg->cfg.src_burst_length;
        ndma_arg.src_data_width   = arg->cfg.src_data_width;

        ndma_arg.dst_drq_type     = arg->cfg.dst_drq_type;
        ndma_arg.dst_addr_type    = arg->cfg.dst_addr_mode;
        ndma_arg.dst_burst_length = arg->cfg.dst_burst_length;
        ndma_arg.dst_data_width   = arg->cfg.dst_data_width;

        ndma_arg.continuous_mode  = arg->cfg.continuous_mode;
        ndma_arg.wait_state       = arg->cfg.wait_state;

        tmp = (uint *)&ndma_arg;
        value = *tmp;

        pdma->channal->config       = value & (~0x80000000);
    }

    return 0;
}

/*
**********************************************************************************************************************
*
*             sunxi_dma_start
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
****************************************************************************************************
*/
int sunxi_dma_start(uint hdma, uint saddr, uint daddr, uint bytes)
{
	sw_dma_channal_set_t  * pdma = (sw_dma_channal_set_t *)hdma;

    pdma->channal->src_addr  = saddr;
    pdma->channal->dst_addr  = daddr;
    pdma->channal->bytes     = bytes;
    pdma->channal->config   |= 0x80000000;   /* start dma */

    return 0;
}
/*
**********************************************************************************************************************
*
*             sunxi_dma_stop
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
int sunxi_dma_stop(uint hdma)
{
	sw_dma_channal_set_t    * pdma = (sw_dma_channal_set_t *)hdma;

    pdma->channal->config &= 0x7fffffff; /* stop dma */

    return 0;
}
/*
**********************************************************************************************************************
*
*             sunxi_dma_querystatus
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
int sunxi_dma_querystatus(uint hdma)
{
    sw_dma_channal_set_t    * pdma = (sw_dma_channal_set_t *)hdma;

    return (pdma->channal->config >> 31) & 0x01;
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
	sw_dma_channal_set_t   *pdma = (sw_dma_channal_set_t *)hdma;
	sunxi_dma_int_set      *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	int                     dma_no;

	dma_no = pdma->channalNo;
	dma_int->irq_pending0 = (3 << (dma_no<<1));     //clear int status

	if(!dma_channal_source[dma_no].dma_func.m_func)
	{
		dma_channal_source[dma_no].dma_func.m_func = dma_int_func;
		dma_channal_source[dma_no].dma_func.m_data = p;
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
int sunxi_dma_free_int(uint hdma)
{
	sunxi_dma_int_set      *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;
	sw_dma_channal_set_t   *pdma = (sw_dma_channal_set_t *)hdma;
	int                     dma_no;

	dma_no = pdma->channalNo;
	dma_int->irq_pending0 = (3 << (dma_no<<1));     //clear int status

	if(dma_channal_source[dma_no].dma_func.m_func)
	{
		dma_channal_source[dma_no].dma_func.m_func = 0;
		dma_channal_source[dma_no].dma_func.m_data = 0;
	}
	else
	{
		printf("this dma channel int is not used\n");

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
	int dma_no;
	sw_dma_channal_set_t   *pdma = (sw_dma_channal_set_t *)hdma;
	sunxi_dma_int_set      *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	dma_no = pdma->channalNo;
	if(dma_int->irq_en0 & (2 << (dma_no<<1)))
	{
		printf("this dma int is avaible already\n");

		return 0;
	}
	//enable dma int
	dma_int->irq_en0 |= (2 << (dma_no<<1));
	//enable golbal int
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
	int dma_no;
	sw_dma_channal_set_t   *pdma    = (sw_dma_channal_set_t *)hdma;
	sunxi_dma_int_set      *dma_int = (sunxi_dma_int_set *)SUNXI_DMA_BASE;

	dma_no = pdma->channalNo;
	if(!(dma_int->irq_en0 & (2 << (dma_no<<1))))
	{
		printf("this dma int is unavaible\n");

		return 0;
	}
	//disable dma int
	dma_int->irq_en0 &= ~(2 << (dma_no<<1));
	//enable golbal int
	dma_int_count --;
	if(!dma_int_count)
	{
		irq_disable(AW_IRQ_DMA);
	}

	return 0;
}


