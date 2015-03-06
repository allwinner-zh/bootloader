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
#include "boot0_i.h"
#include "spi_hal.h"

//#define DMAC_REGS_BASE        0x1c02000
#define DMA_OFFSET   		  0x20
#define DMA_N_OFFSET 		  0x100

#define DMAC_REG_o_CFG        0x00
#define DMAC_REG_o_SRC_ADDR   0x04
#define DMAC_REG_o_DST_ADDR   0x08
#define DMAC_REG_o_BYTE_CNT   0x0c

#define DMAC_REG_N0_CFG      (DMAC_REGS_BASE + 1 * DMA_OFFSET + DMA_N_OFFSET + DMAC_REG_o_CFG)
#define DMAC_REG_N0_SRC_ADDR (DMAC_REGS_BASE + 1 * DMA_OFFSET + DMA_N_OFFSET + DMAC_REG_o_SRC_ADDR)
#define DMAC_REG_N0_DST_ADDR (DMAC_REGS_BASE + 1 * DMA_OFFSET + DMA_N_OFFSET + DMAC_REG_o_DST_ADDR)
#define DMAC_REG_N0_BYTE_CNT (DMAC_REGS_BASE + 1 * DMA_OFFSET + DMA_N_OFFSET + DMAC_REG_o_BYTE_CNT)
#define DMAC_REG_N1_CFG      (DMAC_REGS_BASE + 2 * DMA_OFFSET + DMA_N_OFFSET + DMAC_REG_o_CFG)
#define DMAC_REG_N1_SRC_ADDR (DMAC_REGS_BASE + 2 * DMA_OFFSET + DMA_N_OFFSET + DMAC_REG_o_SRC_ADDR)
#define DMAC_REG_N1_DST_ADDR (DMAC_REGS_BASE + 2 * DMA_OFFSET + DMA_N_OFFSET + DMAC_REG_o_DST_ADDR)
#define DMAC_REG_N1_BYTE_CNT (DMAC_REGS_BASE + 2 * DMA_OFFSET + DMA_N_OFFSET + DMAC_REG_o_BYTE_CNT)

enum DMADWidth
{
	WID8 = 0,
	WID16 = 1,
	WID32 = 2
};

enum NDMAAddrType
{
	INCR = 0,
	NOCHANGE = 1
};

enum DDMAAddrType
{
	LINEAR = 0,
	IO,
	PAGE_HMOD,
	PAGE_VMOD
};

enum DMABurstLen
{
	SINGLE = 0,
	BLEN4 = 1
};

#define NDMA_SPI0       24

#define NDMA_SDRAM      22

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
static void spi_onoff(__u32 spi_no, __u32 onoff)
{
	__u32 reg_val;

	if(onoff)
	{
		if (spi_no == 0)
	    {
	    	writel(0x3333, (0x1c20800 + 0x48));

	    	reg_val = readl(0x1c20800 + 0x64);
	    	reg_val &= ~(0xff << 0);
			reg_val |= (0x55 << 0);
	        writel(reg_val, (0x1c20800 + 0x64));

	        reg_val = readl(CCMU_REG_AHB1_GATING0);
	        reg_val |= 1<<20;
	        writel(reg_val, CCMU_REG_AHB1_GATING0);

	        reg_val = readl(CCMU_REG_SPI0);
	        reg_val |= (1U<<31);
	        writel(reg_val, CCMU_REG_SPI0);
		}
	}
	else
    {
    	reg_val = readl(CCMU_REG_AHB1_GATING0);
	    reg_val &= ~(1<<20);
	    writel(reg_val, CCMU_REG_AHB1_GATING0);

        reg_val = readl(CCMU_REG_SPI0);
        reg_val &= ~(1U<<31);
        writel(reg_val, CCMU_REG_SPI0);
    }

	return;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int sunxi_clock_get_corepll(void)
{
	unsigned int 	reg_val;
	int 			div_p, factor_n;
	int 			factor_k, factor_m;
	int 			clock;

	reg_val  = readl(CCMU_REG_PLL1_CTRL);
	div_p    = 1 << ((reg_val >> 16) & 0x3);
	factor_n = (reg_val >> 8) & 0x1f;
	factor_k = ((reg_val >> 4) & 0x3) + 1;
	factor_m = ((reg_val >> 0) & 0x3) + 1;

	clock = 24 * factor_n * factor_k/div_p/factor_m;

	return clock;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
static int sunxi_clock_get_ahb(void)
{
	unsigned int reg_val;
	int          clock;
	int 		 factor_axi, factor_ahb;

	reg_val    = readl(CCMU_REG_CPUAHBAPB0_RATIO);
	factor_axi = ((reg_val >> 0) & 0x03) + 1;
	factor_ahb =  (reg_val >> 4) & 0x03;

	clock =  sunxi_clock_get_corepll();

	clock /= factor_axi;
	clock >>= factor_ahb;

	return clock;
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
__u32 sunxi_clock_get_pll5(void)
{
	__u32 reg_val;
	__u32 factor_n, factor_k, div_p;

	reg_val = readl(CCMU_REG_PLL5_CTRL);
	factor_n = (reg_val >> 8) & 0x1f;
	factor_k = (reg_val >> 4) & 0x03;
	div_p    = (reg_val >>16) & 0x03;

	return 24 * factor_n * (factor_k + 1)/(1<<div_p);
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
__u32 spi_set_src_clk(__u32 spi_no, int source, __u32 sclk)
{
    __u32 rval;
    __u32 div;
    __u32 src_pll, spi_run_clk;
    __u32 m, n;

	src_pll = sunxi_clock_get_pll5();
	msg("spi src pll = %d\n", src_pll);

    div = (2 * src_pll + sclk)/(2 * sclk);
    div = div==0 ? 1 : div;
    if (div > 128)
	{
	    m = 1;
	    n = 0;

	    msg("Source clock is too high\n");
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

    writel(rval, CCMU_REG_SPI0);
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
int spic_init(__u32 spi_no)
{
	__u32 reg_val, div;

	//设置DMA初始配置

	//设置SPI时钟
	spi_set_src_clk(0, 2, SPI_MCLK);

    spi_onoff(spi_no, 0);
    spi_onoff(spi_no, 1);

	reg_val = (1 << SPI_CTL_TPE_OFFSET) | (1 << SPI_CTL_SS_LEVEL_OFFSET)
            | (1 << SPI_CTL_DHB_OFFSET) | (1 << SPI_CTL_SMC_OFFSET) | (1 << SPI_CTL_RFRST_OFFSET)
            | (1 << SPI_CTL_TFRST_OFFSET) | (1 << SPI_CTL_SSPOL_OFFSET) | (1 << SPI_CTL_POL_OFFSET) | (1 << SPI_CTL_PHA_OFFSET) | (1 << SPI_CTL_MODE_OFFSET) | (1 << SPI_CTL_EN_OFFSET);

    writel(reg_val, SPI_CONTROL);
    writel(0, SPI_DMACTRL);

    /* set spi clock */
    msg("ahb clock=%d\n", div = sunxi_clock_get_ahb());
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
		msg("SPI Status Register Initialized Fail: \n", readl(SPI_INTSTAT));

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
int spic_exit(__u32 spi_no)
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
int spic_rw(__u32 scount, void *saddr, __u32 rcount, void *raddr)
{
    int  time, ret = -1;
    u8  *tx_addr, *rx_addr;
    int  rdma = 0;
    __u32 config;

    if ((scount + rcount) > 64 * 1024)
    {
        msg("too much data to transfer at once\n");

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

		writel(SPI_RX_DATA, DMAC_REG_N0_SRC_ADDR);
		writel((__u32)rx_addr, DMAC_REG_N0_DST_ADDR);
		writel(rcount, DMAC_REG_N0_BYTE_CNT);
		config = 0x80000000 | (0x3   << 27)
               | WID8 << 25 | SINGLE << 23 | INCR << 21
               | NDMA_SDRAM << 16
               | WID8 << 9  | SINGLE << 7  | NOCHANGE << 5
               | NDMA_SPI0;

        writel(config, DMAC_REG_N0_CFG);

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

            /* config DMA */
			writel(SPI_TX_DATA, DMAC_REG_N1_DST_ADDR);
			writel((__u32)tx_addr, DMAC_REG_N1_SRC_ADDR);
			writel(scount, DMAC_REG_N1_BYTE_CNT);
			config = 0x80000000 | (0x3   << 27)
				   | WID8 << 25 | SINGLE << 23 | NOCHANGE << 21
				   | NDMA_SPI0 <<16
				   | WID8 << 9  | SINGLE << 7  | INCR     << 5
				   | NDMA_SDRAM;

	        writel(config, DMAC_REG_N1_CFG);
            /* wait DMA finish */
            while ((time-- > 0) && (readl(DMAC_REG_N1_CFG) & 0x80000000));
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
        		    msg("LINE: %d\n", __LINE__);

        			return ret;
        		}
        	}
        }

        if (time <= 0)
        {
            msg("LINE: %d\n", __LINE__);

            return ret;
        }
    }

	time = 0xffff;
    while (rcount && (time > 0))
    {
    	if ((readl(SPI_INTSTAT) >> 16) & 0x0f)
        {
            *rx_addr++ = *(volatile u8 *)(SPI_RX_DATA);
            --rcount;
            time = 0xffff;
        }
        --time;
    }

    if (time <= 0)
    {
        msg("LINE: %d\n", __LINE__);

    	return ret;
    }

    if (rdma)
    {
    	time = 0xffffff;
        while ((time-- > 0)&& (readl(DMAC_REG_N0_CFG) & 0x80000000));
        if (time <= 0)
        {
            msg("LINE: %d\n", __LINE__);

            return ret;
        }
    }

    if (time > 0)
    {
        __u32 tmp;

        time = 0xfffff;

        tmp = (readl(SPI_INTSTAT) >> 16) & 0x01;

        do
        {
        	tmp = (readl(SPI_INTSTAT) >> 16) & 0x01;
        	if((time--) <= 0)
        	{
        		msg("LINE: %d\n", __LINE__);

            	return ret;
        	}
        }
        while(!tmp);
    }

    return 0;
}

/////////////end test status using interrupt/////////////////////






