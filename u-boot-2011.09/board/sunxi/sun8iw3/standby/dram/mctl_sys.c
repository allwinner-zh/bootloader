/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * CPL <cplanxy@allwinnertech.com>
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
#include "mctl_reg.h"
#include "mctl_sys.h"

static unsigned int ccm_dram_gating_reserved;

int dram_power_save_process(void)
{
//	unsigned int reg_val;
//	unsigned int time = 0xffffff;

	mctl_self_refresh_entry();

//	standby_serial_putc('A');

//  	//turn off sclk
//	reg_val = mctl_read_w(MC_CCR);
//	reg_val &= ~(0x1<<19);
//	mctl_write_w(MC_CCR, reg_val);

//	standby_serial_putc('B');

	//gate off DRAMC AHB clk
//	reg_val = mctl_read_w(CCM_AHB1_GATE0_REG);
//	reg_val &= ~(0x1<<14);
//	mctl_write_w(CCM_AHB1_GATE0_REG, reg_val);

//	//turn off PLL5
//	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
//	reg_val &= ~(0x1U<<31);
//	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);
//
//	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
//	reg_val |= (0x1U<<20);
//	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);
//
//	while(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<20)){}

//	standby_serial_putc('C');

//	//gating off dram cfg clock
//	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
//	reg_val &= ~(0x1U<<31);
//	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);
//
//	//configuration update
//	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
//	reg_val |= (0x1u<<16);
//	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);
//
//	//wait for configuration update
//	while(mctl_read_w(CCM_DRAM_CFG_REG) & (0x1<<16)){}

	return 0;
}

int dram_power_up_process(void)
{
//	unsigned int reg_val;

//	standby_serial_putc('D');

	//turn on PLL5
//	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
//	reg_val |= 0x1<<20;
//	reg_val |= 0x1u<<31;
//	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);
//
//	standby_serial_putc('F');
//
//	while(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<20)){}
//
//standby_serial_putc('G');
//	//wait for stable ONLY FOR IC
//	while(!(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<28))){}
//
//standby_serial_putc('H');
//	standby_timer_delay(10);
	//SYS: 1.6mA  DRAM 9.8mA

//	standby_serial_putc('E');

//	//turn on sclk
//	reg_val = mctl_read_w(MC_CCR);
//   	reg_val |= (0x1<<19);
//	mctl_write_w(MC_CCR, reg_val);


//	//sdram controller reset de-assert
//	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
//	reg_val |= 0x1u<<31;
//	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);
//
//	//configuration update
//	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
//	reg_val |= (0x1u<<16);
//	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);
//
//	//wait for configuration update
//	while(mctl_read_w(CCM_DRAM_CFG_REG) & (0x1<<16)){}

	mctl_self_refresh_exit();

	return 0;
}

void mctl_self_refresh_entry(void)
{
	unsigned int reg_val;
//	unsigned int ch_id;

  	//gating off the host access interface
  	ccm_dram_gating_reserved = mctl_read_w(CCM_DRAM_GATE_REG);
  	mctl_write_w(CCM_DRAM_GATE_REG, 0);

  	//master access disable
  	mctl_write_w(MC_MAER, 0);

	//set selfrefresh command
  	reg_val = mctl_read_w(MX_PWRCTL);
  	reg_val |= 0x1<<0;
  	mctl_write_w(MX_PWRCTL, reg_val);

  	//confirm dram controller has enter selfrefresh
  	while(((mctl_read_w(MX_STATR)&0x7) != 0x3)){}
  	//SYS: 28.3mA  DRAM 39.6mA
 	//ck output to level 0
  	reg_val = mctl_read_w(MP_PGCR(0));
  	reg_val &= ~(0x3<<26);
  	mctl_write_w(MP_PGCR(0), reg_val);
  	//SYS: 28.3mA  DRAM 39.6mA
  	//put PAD into power down state
  	reg_val = mctl_read_w(MP_ACIOCR);
  	reg_val |= (0x1<<3)|(0x1<<8)|(0x1<<18);
  	mctl_write_w(MP_ACIOCR, reg_val);

  	reg_val = mctl_read_w(MP_DXCCR);
  	reg_val |= (0x1<<3)|(0x1<<4);
  	mctl_write_w(MP_DXCCR, reg_val);

  	reg_val = mctl_read_w(MP_DSGCR);
  	reg_val &= ~(0x1<<28);
  	mctl_write_w(MP_DSGCR, reg_val);

	//pad hold
	reg_val = mctl_read_w(VDD_SYS_PWROFF_GATING);
  	reg_val |= (0x1<<0);
  	mctl_write_w(VDD_SYS_PWROFF_GATING, reg_val);
  	//SYS: 1.2mA  DRAM 7.1mA
}

void mctl_self_refresh_exit(void)
{
	unsigned int reg_val;

	//ck output to level 0
  	reg_val = mctl_read_w(MP_PGCR(0));
  	reg_val &= ~(0x3<<26);
	reg_val |= 0x2<<26;
  	mctl_write_w(MP_PGCR(0), reg_val);

	reg_val = mctl_read_w(MP_DSGCR);
  	reg_val |= (0x1<<28);
  	mctl_write_w(MP_DSGCR, reg_val);

  	reg_val = mctl_read_w(MP_DXCCR);
  	reg_val &= ~((0x1<<3)|(0x1<<4));
  	mctl_write_w(MP_DXCCR, reg_val);

  	reg_val = mctl_read_w(MP_ACIOCR);
  	reg_val &= ~((0x1<<3)|(0x1<<8)|(0x1<<18));
  	mctl_write_w(MP_ACIOCR, reg_val);

	//pad hold
	reg_val = mctl_read_w(VDD_SYS_PWROFF_GATING);
  	reg_val &= ~(0x1<<0);
  	mctl_write_w(VDD_SYS_PWROFF_GATING, reg_val);
  	//SYS: 31.6mA  DRAM 36.8mA
	//set selfrefresh command
  	reg_val = mctl_read_w(MX_PWRCTL);
  	reg_val &= ~(0x1<<0);
  	mctl_write_w(MX_PWRCTL, reg_val);

  	//confirm dram controller has exit from selfrefresh
  	while(((mctl_read_w(MX_STATR)&0x7) != 0x1)){}


	//Enable the DRAM master access
	mctl_write_w(MC_MAER, 0xFFFFFFFF);

	//ccmu dram gating bit return
  	mctl_write_w(CCM_DRAM_GATE_REG, ccm_dram_gating_reserved);
}
#if 0

int soft_switch_freq(__dram_para_t *dram_para)
{
	mctl_deep_sleep_entry(dram_para);

    return 0;
}

void mctl_deep_sleep_entry(__dram_para_t *para)
{
 	unsigned int reg_val;
	unsigned int time = 0xffffff;
	unsigned int factor;
	unsigned int i,j;

	mctl_self_refresh_entry();

	 //turn off sclk
	reg_val = mctl_read_w(MC_CCR);
	reg_val &= ~(0x1<<19);
	mctl_write_w(MC_CCR, reg_val);

	//gate off DRAMC AHB clk
	reg_val = mctl_read_w(CCM_AHB1_GATE0_REG);
	reg_val &= ~(0x1<<14);
	mctl_write_w(CCM_AHB1_GATE0_REG, reg_val);

	//turn off PLL5
	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
	reg_val &= ~(0x1U<<31);
	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);

	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
	reg_val |= (0x1U<<20);
	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);

	while(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<20)){}

	//gating off dram cfg clock
	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
	reg_val &= ~(0x1U<<31);
	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);

	//configuration update
	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
	reg_val |= (0x1u<<16);
	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);

	//wait for configuration update
	while(mctl_read_w(CCM_DRAM_CFG_REG) & (0x1<<16)){}

	if(para->dram_clk >= 336)	//PLL normal mode
	{
		reg_val = ((para->dram_clk/24 - 1)<<8);			//N
		reg_val |= 0<<4;								//K = 1
		reg_val |= (2-1)<<0;							//M = 2
		reg_val |= (0x1u<<31);
	}
	else						//PLL bypass mode
	{
		reg_val = (((para->dram_clk<<3)/24 - 1)<<8);	//N
		reg_val |= 0<<4;								//K = 1
		reg_val |= (2-1)<<0;							//M = 2
		reg_val |= (0x1u<<31);
	}
	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);

	reg_val = mctl_read_w(CCM_PLL_DDR_CTRL);
	reg_val |= 0x1<<20;
	mctl_write_w(CCM_PLL_DDR_CTRL, reg_val);

	mctl_delay(0x1000);

	while(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<20)){}

	//wait for stable ONLY FOR IC
	while(!(mctl_read_w(CCM_PLL_DDR_CTRL) & (0x1<<28))){}

	if(para->dram_clk < 336)	//PLL bypass mode
	{
		reg_val = mctl_read_w(MC_CCR);
		reg_val &= ~(0xffff);
		reg_val |= 0x7<<16;
		mctl_write_w(MC_CCR, reg_val);

		reg_val = mctl_read_w(MP_PIR);
		reg_val |= 0x1<<17;
		mctl_write_w(MP_PIR, reg_val);
	}

	//turn on sclk
	reg_val = mctl_read_w(MC_CCR);
   	reg_val |= (0x1<<19);
	mctl_write_w(MC_CCR, reg_val);

	//sdram controller reset de-assert
	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
	reg_val |= 0x1u<<31;
	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);

	//configuration update
	reg_val = mctl_read_w(CCM_DRAM_CFG_REG);
	reg_val |= (0x1u<<16);
	mctl_write_w(CCM_DRAM_CFG_REG, reg_val);

	//wait for configuration update
	while(mctl_read_w(CCM_DRAM_CFG_REG) & (0x1<<16)){}

	// MDFS_DBG("[DRAM]test  3==================\n");
	//change phy training value
	i = (para->dram_clk == para->dram_tpr11) ? 0 : 1;

	mctl_write_w(MP_DXBDLR0(0), mdfsvalue[i * 21 + 0]);
	mctl_write_w(MP_DXBDLR0(1), mdfsvalue[i * 21 + 1]);

	mctl_write_w(MP_DXBDLR1(0), mdfsvalue[i * 21 + 2]);
	mctl_write_w(MP_DXBDLR1(1), mdfsvalue[i * 21 + 3]);

	mctl_write_w(MP_DXBDLR2(0), mdfsvalue[i * 21 + 4]);
	mctl_write_w(MP_DXBDLR2(1), mdfsvalue[i * 21 + 5]);

	mctl_write_w(MP_DXBDLR3(0), mdfsvalue[i * 21 + 6]);
	mctl_write_w(MP_DXBDLR3(1), mdfsvalue[i * 21 + 7]);

	mctl_write_w(MP_DXBDLR4(0), mdfsvalue[i * 21 + 8]);
	mctl_write_w(MP_DXBDLR4(1), mdfsvalue[i * 21 + 9]);

	mctl_write_w(MP_DXLCDLR0(0), mdfsvalue[i * 21 + 10]);
	mctl_write_w(MP_DXLCDLR0(1), mdfsvalue[i * 21 + 11]);

	mctl_write_w(MP_DXLCDLR1(0), mdfsvalue[i * 21 + 12]);
	mctl_write_w(MP_DXLCDLR1(1), mdfsvalue[i * 21 + 13]);

	mctl_write_w(MP_DXLCDLR2(0), mdfsvalue[i * 21 + 14]);
	mctl_write_w(MP_DXLCDLR2(1), mdfsvalue[i * 21 + 15]);

	mctl_write_w(MP_DXMDLR(0), mdfsvalue[i * 21 + 16]);
	mctl_write_w(MP_DXMDLR(1), mdfsvalue[i * 21 + 17]);

	mctl_write_w(MP_DXGTR(0), mdfsvalue[i * 21 + 18]);

	//write back ZQ value & dqs training value
	reg_val = mctl_read_w(MP_ZQCR0(0));
	reg_val &= ~0xFFFFFFF;
	reg_val |= (0x1<<28) | (mdfsvalue[i * 21 + 19]);
	mctl_write_w(MP_ZQCR0(0), reg_val);

	mctl_write_w(MP_ZQCR2, mdfsvalue[i * 21 + 20]);

	mctl_self_refresh_exit();

	return;
}

#endif






