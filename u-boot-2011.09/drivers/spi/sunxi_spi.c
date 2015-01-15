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
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/spi.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/dma.h>

//spic_info spic_info[SPIC_NUM];

//__vu32 spi_busy;
//__vu32 spi_txptr;
//__vu32 spi_rxptr;
//__vu32 spi_txnum;
//__vu8* spi_txbuf;
//__vu32 spi_rxnum;
//__vu8* spi_rxbuf;

static  uint  spi_tx_dma_hd;
static  uint  spi_rx_dma_hd;

static uint spi_test_counter;
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
*    note          :  support spi2 only
*
*
************************************************************************************************************
*/
void spi_onoff(uint spi_no, uint onoff)
{
	uint reg_val;

	if(onoff)
	{
		if (spi_no == 0)
	    {
	    	writel(0x3333, (0x1c20800 + 0x48));

	    	reg_val = readl(0x1c20800 + 0x64);
	    	reg_val &= ~(0xff << 0);
			reg_val |= (0x55 << 0);
	        writel(reg_val, (0x1c20800 + 0x64));

	        reg_val = readl(CCM_AHB1_GATE0_CTRL);
	        reg_val |= 1<<20;
	        writel(reg_val, CCM_AHB1_GATE0_CTRL);

	        reg_val = readl(CCM_SPI0_SCLK_CTRL);
	        reg_val |= (1U<<31);
	        writel(reg_val, CCM_SPI0_SCLK_CTRL);
		}
	}
	else
    {
    	reg_val = readl(CCM_AHB1_GATE0_CTRL);
	    reg_val &= ~(1<<20);
	    writel(reg_val, CCM_AHB1_GATE0_CTRL);

        reg_val = readl(CCM_SPI0_SCLK_CTRL);
        reg_val &= ~(1U<<31);
        writel(reg_val, CCM_SPI0_SCLK_CTRL);
    }

	return;
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
*    note          :  support spi2 only
*
*
************************************************************************************************************
*/
uint spi_set_src_clk(uint spi_no, int source, uint sclk)
{
    uint rval;
    uint div;
    uint src_pll, spi_run_clk;
    uint m, n;

	src_pll = sunxi_clock_get_pll5();
	printf("spi src pll = %d\n", src_pll);

    div = (2 * src_pll + sclk)/(2 * sclk);
    div = div==0 ? 1 : div;
    if (div > 128)
	{
	    m = 1;
	    n = 0;

	    printf("Source clock is too high\n");
	}
	else if (div > 64)
	{
	    n = 3;
	    m = div >> 3;
	}
	else if (div > 32)
	{
	    n = 2;
	    m = div >> 2;
	}
	else if (div > 16)
	{
	    n = 1;
	    m = div >> 1;
	}
	else
	{
	    n = 0;
	    m = div;
	}

	source = 2;
    rval = (source << 24) | (n << 16) | (m - 1);

    writel(rval, CCM_SPI0_SCLK_CTRL);
	spi_run_clk = src_pll / (1 << n) / (m - 1);

    return spi_run_clk;
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
static int spi_dma_recv_start(uint spi_no, uchar* pbuf, uint byte_cnt)
{
	flush_cache((uint)pbuf, byte_cnt);

	sunxi_dma_start(spi_rx_dma_hd, SPI_RX_DATA, (uint)pbuf, byte_cnt);

	return 0;
}

static int spi_wait_dma_recv_over(uint spi_no)
{
	return sunxi_dma_querystatus(spi_rx_dma_hd);
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
static int spi_dma_send_start(uint spi_no, uchar* pbuf, uint byte_cnt)
{
	flush_cache((uint)pbuf, byte_cnt);

	sunxi_dma_start(spi_tx_dma_hd, (uint)pbuf, SPI_TX_DATA, byte_cnt);

	return 0;
}

static int spi_wait_dma_send_over(uint spi_no)
{
	return sunxi_dma_querystatus(spi_tx_dma_hd);
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
int spic_init(uint spi_no)
{
	uint reg_val, div;
	sunxi_dma_setting_t  spi_tx_dma;
    sunxi_dma_setting_t  spi_rx_dma;

	spi_tx_dma_hd = sunxi_dma_request(DMAC_DMATYPE_NORMAL);
	spi_rx_dma_hd = sunxi_dma_request(DMAC_DMATYPE_NORMAL);

	if((spi_tx_dma_hd == 0) || (spi_rx_dma_hd == 0))
	{
		printf("spi request dma failed\n");

		return -1;
	}
	//配置spi rx dma资源
	spi_rx_dma.pgsz   = 0;
	spi_rx_dma.pgstp  = 0;
	spi_rx_dma.cmt_blk_cnt = 0;
	//config recv(from spi fifo to dram)
	spi_rx_dma.cfg.src_drq_type     = DMAC_CFG_TYPE_SPI0;  //SPI0
	spi_rx_dma.cfg.src_addr_mode    = DMAC_CFG_SRC_ADDR_TYPE_IO_MODE;
	spi_rx_dma.cfg.src_burst_length = DMAC_CFG_SRC_1_BURST;
	spi_rx_dma.cfg.src_data_width   = DMAC_CFG_SRC_DATA_WIDTH_32BIT;

	spi_rx_dma.cfg.dst_drq_type     = DMAC_CFG_TYPE_DRAM;  //DRAM
	spi_rx_dma.cfg.dst_addr_mode    = DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE;
	spi_rx_dma.cfg.dst_burst_length = DMAC_CFG_DEST_1_BURST;
	spi_rx_dma.cfg.dst_data_width   = DMAC_CFG_DEST_DATA_WIDTH_32BIT;

	spi_rx_dma.cfg.wait_state       = 4;
	spi_rx_dma.cfg.continuous_mode  = 0;
	//配置spi tx dma资源
	spi_tx_dma.pgsz   = 0;
	spi_tx_dma.pgstp  = 0;
	spi_tx_dma.cmt_blk_cnt = 0;
	//spi_tx_dma.
	//config send(from dram to spi fifo)
	spi_tx_dma.cfg.src_drq_type     = DMAC_CFG_TYPE_DRAM;  //
	spi_tx_dma.cfg.src_addr_mode    = DMAC_CFG_SRC_ADDR_TYPE_LINEAR_MODE;
	spi_tx_dma.cfg.src_burst_length = DMAC_CFG_SRC_1_BURST;
	spi_tx_dma.cfg.src_data_width   = DMAC_CFG_SRC_DATA_WIDTH_32BIT;

	spi_tx_dma.cfg.dst_drq_type     = DMAC_CFG_TYPE_SPI0;  //SPI0
	spi_tx_dma.cfg.dst_addr_mode    = DMAC_CFG_DEST_ADDR_TYPE_IO_MODE;
	spi_tx_dma.cfg.dst_burst_length = DMAC_CFG_DEST_1_BURST;
	spi_tx_dma.cfg.dst_data_width   = DMAC_CFG_DEST_DATA_WIDTH_32BIT;

	spi_tx_dma.cfg.wait_state       = 4;
	spi_tx_dma.cfg.continuous_mode  = 0;

	sunxi_dma_setting(spi_rx_dma_hd, (void *)&spi_rx_dma);
	sunxi_dma_setting(spi_tx_dma_hd, (void *)&spi_tx_dma);

	spi_set_src_clk(0, 2, SPI_MCLK);

    spi_onoff(spi_no, 0);
    spi_onoff(spi_no, 1);

	reg_val = (1 << SPI_CTL_TPE_OFFSET) | (1 << SPI_CTL_SS_LEVEL_OFFSET)
            | (1 << SPI_CTL_DHB_OFFSET) | (1 << SPI_CTL_SMC_OFFSET) | (1 << SPI_CTL_RFRST_OFFSET)
            | (1 << SPI_CTL_TFRST_OFFSET) | (1 << SPI_CTL_SSPOL_OFFSET) | (1 << SPI_CTL_POL_OFFSET) | (1 << SPI_CTL_PHA_OFFSET) | (1 << SPI_CTL_MODE_OFFSET) | (1 << SPI_CTL_EN_OFFSET);

    writel(reg_val, SPI_CONTROL);
    writel(0, SPI_DMACTRL);

    /* set spi clock */
    printf("ahb clock=%d\n", div = sunxi_clock_get_ahb());
    //div = sunxi_clock_get_ahb() / 20 - 1;
    div = div/20-1;
    reg_val  = 1 << 12;
    reg_val |= div;
    writel(reg_val, SPI_CLCKRATE);
	writel(0, SPI_WATCLCK);
	//Clear Status Register
	writel(0xfffff, SPI_INTSTAT);
	//wait clear busy, add in aw1623, 2013-12-22 11:25:03
	while (readl(SPI_INTSTAT) & (1U << 31));
	if(readl(SPI_INTSTAT) != 0x1b00)
	{
		printf("SPI Status Register Initialized Fail: \n", readl(SPI_INTSTAT));

		return -1;
	}

	//Set Interrput Control Register
    writel(0x0000, SPI_INTCTRL);        //Close All Interrupt

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
int spic_exit(uint spi_no)
{
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
int spic_rw(uint scount, void *saddr, uint rcount, void *raddr)
{
    int  time, ret = -1;
    u8  *tx_addr, *rx_addr;
    int  rdma;

    if ((scount + rcount) > 64 * 1024)
    {
        printf("too much data to transfer at once\n");

        return -1;
    }

    tx_addr = (u8 *)saddr;
    rx_addr = (u8 *)raddr;

    writel(readl(SPI_INTSTAT) | 0xffff, SPI_INTSTAT);
    writel(0, SPI_DMACTRL);
    writel(scount + rcount, SPI_BURSTCNT);
    writel(scount,          SPI_TRANSCNT);

    if (rcount > 7)
    {
        /* RXFIFO half full dma request enable */
        writel(0x02, SPI_DMACTRL);

		flush_cache((uint)raddr, rcount);
		spi_dma_recv_start(0, raddr, rcount);

        rdma   = 1;
        rcount = 0;
    }
	writel(readl(SPI_CONTROL) | (1 << 0x0a), SPI_CONTROL);

    if (scount)
    {
        time = 0xffffff;
        if(scount > 7)
        {
            /* RXFIFO half empty dma request enable */
            writel(readl(SPI_DMACTRL) | 0x0200, SPI_DMACTRL);

			spi_dma_send_start(0, saddr, scount);
            /* wait DMA finish */
			while ((time-- > 0)&& spi_wait_dma_send_over(0));
        }
        else
        {
            for (; scount > 0; scount--)
            {
                *(volatile u8 *)(SPI_TX_DATA) = *tx_addr;
                tx_addr += 1;
            }

            time = 0xffffff;
            while((readl(SPI_INTSTAT) >> 20) & 0x0f)
        	{
        		time--;
        		if (time <= 0)
        		{
        		    printf("LINE: %d\n", __LINE__);

        			return ret;
        		}
        	}
        }

        if (time <= 0)
        {
            printf("LINE: %d\n", __LINE__);

            return ret;
        }
    }

	time = 0xffff;
    while (rcount && (time > 0))
    {
    	//spi_test_counter = 0;
    	if ((readl(SPI_INTSTAT) >> 16) & 0x0f)
        {
            *rx_addr++ = *(volatile u8 *)(SPI_RX_DATA);
//            spi_test_counter ++;
//            if(spi_test_counter = 7)
//            {
//            	puts("\n");
//            }
//            printf("0x%02x  ", *(rx_addr-1));
            --rcount;
            time = 0xffff;
        }
        --time;
    }

    if (time <= 0)
    {
        printf("LINE: %d\n", __LINE__);

    	return ret;
    }

    if (rdma)
    {
        time = 0xffffff;
        while ((time-- > 0)&& spi_wait_dma_recv_over(0));
        if (time <= 0)
        {
            printf("LINE: %d\n", __LINE__);

            return ret;
        }
    }

    if (time > 0)
    {
        uint tmp;

        time = 0xfffff;

        tmp = (readl(SPI_INTSTAT) >> 16) & 0x01;

        do
        {
        	tmp = (readl(SPI_INTSTAT) >> 16) & 0x01;
        	if((time--) <= 0)
        	{
        		printf("LINE: %d\n", __LINE__);

            	return ret;
        	}
        }
        while(!tmp);
    }

    return 0;
}

/////////////end test status using interrupt/////////////////////






