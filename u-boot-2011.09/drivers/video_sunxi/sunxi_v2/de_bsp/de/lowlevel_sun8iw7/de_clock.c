#include "de_clock.h"

#define SETMASK(width, shift)   ((width?((-1U) >> (32-width)):0)  << (shift))
#define CLRMASK(width, shift)   (~(SETMASK(width, shift)))
#define GET_BITS(shift, width, reg)     \
            (((reg) & SETMASK(width, shift)) >> (shift))
#define SET_BITS(shift, width, reg, val) \
            (((reg) & CLRMASK(width, shift)) | (val << (shift)))

#define __inf(msg...)
#define __wrn(msg...)		printf(msg)

#define readl(addr) (*(volatile u32 *) (addr))
#define writel(val, addr) ((*(volatile u32 *) (addr)) = (val))

clk_pll_para disp_clk_pll_tbl[] ={
	PLL_CFG(SYS_CLK_PLL3,		CLK_NONE,		297000000,	0x01c20010,	31,	0,	4,	8,	7,	32,	0, 32, 0)
	PLL_CFG(SYS_CLK_PLL10,		CLK_NONE,		297000000,	0x01c20048,	31,	0,	4,	8,	7,	32, 0, 32, 0)
	PLL_CFG(SYS_CLK_MIPIPLL,	SYS_CLK_PLL3,	0,			0x01c20040,	31, 0,	4,	8,	4,	4,	2, 22, 2)
};

clk_mod_para disp_clk_mod_tbl[] ={
	MOD_CFG(MOD_CLK_DEBE0,		SYS_CLK_PLL10,		145000000,	0x01c20064,	12,	0x01c202c4,	12,	0x01c20100,	26,	0x01c20104,	31,	24,	3,	0,	4)
	MOD_CFG(MOD_CLK_DEFE0,		SYS_CLK_PLL10,		145000000,	0x01c20064,	14,	0x01c202c4,	14,	0x01c20100,	24,	0x01c2010c,	31,	24,	3,	0,	4)
	MOD_CFG(MOD_CLK_LCD0CH0,	SYS_CLK_MIPIPLL,	0,			0x01c20064,	4,	0x01c202c4,	4,	0x0,		32,	0x01c20118,	31,	24,	3,	32,	0)
	MOD_CFG(MOD_CLK_MIPIDSIS,	SYS_CLK_PLL10,		0,	        0x01c20060,	1,	0x01c202c0,	1,	0x0,		32,	0x01c20168,	31,	24,	2,	16,	4)
	MOD_CFG(MOD_CLK_MIPIDSIP,	SYS_CLK_PLL10,		150000000,	0x01c20060,	1,	0x01c202c0,	1,	0x0,		32,	0x01c20168,	15,	8,	2,	0,	4)
	MOD_CFG(MOD_CLK_IEPDRC0,	SYS_CLK_PLL10,		145000000,	0x01c20064,	25,	0x01c202c4,	25,	0x01c20100,	16,	0x01c20180,	31,	24,	3,	0,	4)
	MOD_CFG(MOD_CLK_LVDS,		CLK_NONE,			0,			0x0,		32,	0x01c202c8,	0,	0x0,		32,	0x0,		32,	32,	0,	32,	0)
	MOD_CFG(MOD_CLK_SAT0,		CLK_NONE,			0,			0x01c20064,	26,	0x01c202c4,	26,	0x0,		32,	0x0,		32,	32,	0,	32,	0)
};
u32 pll_enable_count[] = {0, 0, 0};
u32 mod_enable_count[] = {0, 0, 0, 0, 0, 0, 0, 0};

u32 dif(u32 value0, u32 value1)
{
	if(value0 > value1)
		return value0 - value1;
	else
		return value1 - value0;
}

void calc_src_coef3(u32 *div_M, u32 *fac_K, u32 *fac_N, u32 src_freq, u32 freq)
{
	u32 m, k, n;
	u64 temp;
	u32 temp_m = 0;
	u32 temp_k = 0;
	u32 temp_n = 0;
	u64 cur_val = 0;

	freq = freq > 1440000000 ? 1440000000 : freq;

	for(n = 0; n < 16; n++)
		for(k = 0; k < 4; k++)
			for(m = 0; m < 16; m++)
			{
				temp = src_freq;
				temp = temp * (n + 1) * (k + 1) / (m + 1);
				if(dif(temp, freq) < dif(cur_val, freq))
				{
					temp_m = m;
					temp_k = k;
					temp_n = n;
					cur_val = temp;
				}
			}

	*div_M = temp_m;
	*fac_K = temp_k;
	*fac_N = temp_n;
}

void calc_src_coef2(u32 *div_M, u32 *fac_N, u32 freq)
{
	u32 m, n;
	u64 temp;
	u32 temp_m = 0;
	u32 temp_n = 0;
	u64 cur_val = 0;

	freq = freq > 1440000000 ? 1440000000 : freq;

	for(n = 0; n < 128; n++)
		for(m = 0; m < 16; m++)
		{
			temp = 24000000 * (n + 1) / (m + 1);
			if(dif(temp, freq) < dif(cur_val, freq))
			{
				temp_m = m;
				temp_n = n;
				cur_val = temp;
			}
		}

		*div_M = temp_m;
		*fac_N = temp_n;
}

void de_clk_init_pll(void)
{
	u32 i;
	u32 count;

	count = sizeof(disp_clk_pll_tbl) / sizeof(clk_pll_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_pll_tbl[i].freq)
			de_set_pll_rate(disp_clk_pll_tbl[i].clk_id, disp_clk_pll_tbl[i].freq);
	}
}

u32 de_get_pll_rate(__disp_clk_id_t clk_id)
{
	u32 pll3_freq;
	u32 fac_N, fac_M, ext_fac;
	u32 reg_val;

	if(clk_id == SYS_CLK_PLL10)
	{
		reg_val = readl(disp_clk_pll_tbl[1].pll_adr);
		if(GET_BITS(24, 2, reg_val) == 0)
			return 270000000;
		else if(GET_BITS(24, 2, reg_val) == 2)
			return 297000000;
		else
		{
			fac_N = GET_BITS(disp_clk_pll_tbl[1].fac_shift, disp_clk_pll_tbl[1].fac_bit_num, reg_val);
			fac_M = GET_BITS(disp_clk_pll_tbl[1].div_shift, disp_clk_pll_tbl[1].div_bit_num, reg_val);
			return 24000000 * (fac_N + 1) / (fac_M + 1);
		}
	}
	else
	{
		reg_val = readl(disp_clk_pll_tbl[0].pll_adr);
		if(GET_BITS(24, 2, reg_val) == 0)
			pll3_freq = 270000000;
		else if(GET_BITS(24, 2, reg_val) == 2)
			pll3_freq = 297000000;
		else
		{
			fac_N = GET_BITS(disp_clk_pll_tbl[0].fac_shift, disp_clk_pll_tbl[0].fac_bit_num, reg_val);
			fac_M = GET_BITS(disp_clk_pll_tbl[0].div_shift, disp_clk_pll_tbl[0].div_bit_num, reg_val);
			pll3_freq = 24000000 * (fac_N + 1) / (fac_M + 1);
		}

		if(clk_id == SYS_CLK_PLL3)
		{
			return pll3_freq;
		}
		else if(clk_id == SYS_CLK_MIPIPLL)
		{
			reg_val = readl(disp_clk_pll_tbl[2].pll_adr);
			fac_N = GET_BITS(disp_clk_pll_tbl[2].fac_shift, disp_clk_pll_tbl[2].fac_bit_num, reg_val);
			fac_M = GET_BITS(disp_clk_pll_tbl[2].div_shift, disp_clk_pll_tbl[2].div_bit_num, reg_val);
			ext_fac = GET_BITS(disp_clk_pll_tbl[2].ext_fac_shift, disp_clk_pll_tbl[2].ext_fac_bit_num, reg_val);
			return pll3_freq * (fac_N + 1) * (ext_fac + 1) / (fac_M + 1);
		}
		else
		{
		}
	}

	return 0;
}

u32 de_set_pll_rate(__disp_clk_id_t clk_id, u32 freq)
{
	u32 fac_N, fac_M, ext_fac;
	u32 reg_val;
	if(clk_id == SYS_CLK_PLL10)
	{
		reg_val = readl(disp_clk_pll_tbl[1].pll_adr);
		if(freq == 270000000)
		{
			reg_val = SET_BITS(24, 2, reg_val, 0);
			reg_val = SET_BITS(0, 4, reg_val, 0);
		}
		else if(freq == 297000000)
		{
			reg_val = SET_BITS(24, 2, reg_val, 2);
			reg_val = SET_BITS(0, 4, reg_val, 0);
		}
		else
		{
			calc_src_coef2(&fac_M, &fac_N, freq);
			reg_val= SET_BITS(24, 2, reg_val, 1);
			reg_val = SET_BITS(disp_clk_pll_tbl[1].fac_shift, disp_clk_pll_tbl[1].fac_bit_num, reg_val, fac_N);
			reg_val = SET_BITS(disp_clk_pll_tbl[1].div_shift, disp_clk_pll_tbl[1].div_bit_num, reg_val, fac_M);
		}
		writel(reg_val, disp_clk_pll_tbl[1].pll_adr);
		return 1;
	}
	else if(clk_id == SYS_CLK_PLL3)
	{
		reg_val = readl(disp_clk_pll_tbl[0].pll_adr);
		if(freq == 270000000)
		{
			reg_val = SET_BITS(24, 2, reg_val, 0);
			reg_val = SET_BITS(0, 4, reg_val, 0);
		}
		else if(freq == 297000000)
		{
			reg_val = SET_BITS(24, 2, reg_val, 2);
			reg_val = SET_BITS(0, 4, reg_val, 0);
		}
		else
		{
			calc_src_coef2(&fac_M, &fac_N, freq);
			reg_val = SET_BITS(24, 2, reg_val, 1);
			reg_val = SET_BITS(disp_clk_pll_tbl[0].fac_shift, disp_clk_pll_tbl[0].fac_bit_num, reg_val, fac_N);
			reg_val = SET_BITS(disp_clk_pll_tbl[0].div_shift, disp_clk_pll_tbl[0].div_bit_num, reg_val, fac_M);
		}
		writel(reg_val, disp_clk_pll_tbl[0].pll_adr);
		return 1;
	}
	else if(clk_id == SYS_CLK_MIPIPLL)
	{
		reg_val = readl(disp_clk_pll_tbl[2].pll_adr);
		u32 pll3_freq = de_get_pll_rate(SYS_CLK_PLL3);
		if(pll3_freq < 0) {
			__wrn("get pll3 freq failed!\n");
			return -1;
		}
		calc_src_coef3(&fac_M, &ext_fac, &fac_N, pll3_freq, freq);
		reg_val = SET_BITS(disp_clk_pll_tbl[2].fac_shift, disp_clk_pll_tbl[2].fac_bit_num, reg_val, fac_N);
		reg_val = SET_BITS(disp_clk_pll_tbl[2].div_shift, disp_clk_pll_tbl[2].div_bit_num, reg_val, fac_M);
		reg_val = SET_BITS(disp_clk_pll_tbl[2].ext_fac_shift, disp_clk_pll_tbl[2].ext_fac_bit_num, reg_val, ext_fac);
		reg_val = SET_BITS(disp_clk_pll_tbl[2].ext_enable_shift, disp_clk_pll_tbl[2].ext_enable_shift_num, reg_val, 3);

		writel(reg_val, disp_clk_pll_tbl[2].pll_adr);
		return 1;
	}
	else
	{
	}
	__wrn("set pll freq fail!\n");
	return 0;
}

u32 de_set_mclk_src(__hdle clk_hdl)
{
	u32 i;
	u32 count;
	u32 reg_val;

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;

	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			reg_val = readl(disp_clk_mod_tbl[i].mod_adr);
			if(disp_clk_mod_tbl[i].src_id == SYS_CLK_PLL10)
			{
				reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_src_shift, disp_clk_mod_tbl[i].mod_src_bit_num, reg_val, 5);
				writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
				return 1;
			}
			else if(disp_clk_mod_tbl[i].src_id == SYS_CLK_PLL3)
			{
				reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_src_shift, disp_clk_mod_tbl[i].mod_src_bit_num, reg_val, 0);
				writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
				return 1;
			}
			else if(disp_clk_mod_tbl[i].src_id == SYS_CLK_MIPIPLL)
			{
				reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_src_shift, disp_clk_mod_tbl[i].mod_src_bit_num, reg_val, 4);
				writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
				return 1;
			}
			else
			{
				break;
			}
		}
	}
	__wrn("set mod clock src fail!\n");
	return 0;
}

u32 de_get_mclk_src(__hdle clk_hdl)
{
	u32 i;
	u32 count;
	u32 src_sel;
	u32 reg_val;

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;
	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);

	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			reg_val = readl(disp_clk_mod_tbl[i].mod_adr);
			src_sel = GET_BITS(disp_clk_mod_tbl[i].mod_src_shift, disp_clk_mod_tbl[i].mod_src_bit_num, reg_val);
			if(src_sel == 0)
			{
				return SYS_CLK_PLL3;
			}
			else if(src_sel == 4)
			{
				return SYS_CLK_MIPIPLL;
			}
			else if(src_sel == 5)
			{
				return SYS_CLK_PLL10;
			}
			else
			{
				break;
			}
		}
	}
	__wrn("get mod clock src fail!\n");
	return 0;
}

u32 de_set_mclk_freq(__hdle clk_hdl, u32 nFreq)
{
	u32 i;
	u32 count;
	u32 src_freq;
	u32 div;
	__disp_clk_id_t src_id;
	u32 reg_val;

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;
	if(hdl->clk_id == MOD_CLK_MIPIDSIS)
		return 0;

	src_id = de_get_mclk_src(clk_hdl);
	if(!src_id) {
		__wrn("get mode clk src fail!\n");
		return 0;
	}

	src_freq = de_get_pll_rate(src_id);
	if(!src_freq) {
		__wrn("get src freq fail!\n");
		return 0;
	}

	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			if(nFreq == 0) {
				nFreq = disp_clk_mod_tbl[i].freq;
			}

			if(nFreq == 0) {
				__wrn("get mode clk freq from table fail\n");
				return 0;
			}

			div = src_freq / nFreq;
			if((div > 16) || (div < 1)) {
				__wrn("div is overflow\n");
				return 0;
			}

			reg_val = readl(disp_clk_mod_tbl[i].mod_adr);
			reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_div_shift, disp_clk_mod_tbl[i].mod_div_bit_num, reg_val, (div - 1));
			writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
			return 1;
		}
	}

	__wrn("not get clk hdl\n");
	return 0;
}

u32 de_get_mclk_freq(__hdle clk_hdl)
{
	u32 i;
	u32 count;
	u32 src_freq;
	u32 div;
	__disp_clk_id_t src_id;
	u32 reg_val;

	src_id = de_get_mclk_src(clk_hdl);
	if(!src_id) {
		__wrn("get mode clk src fail!\n");
		return 0;
	}

	src_freq = de_get_pll_rate(src_id);

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;
	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			reg_val = readl(disp_clk_mod_tbl[i].mod_adr);
			div = GET_BITS(disp_clk_mod_tbl[i].mod_div_shift, disp_clk_mod_tbl[i].mod_div_bit_num, reg_val);
			return src_freq / (div + 1);
		}
	}
	return 0;
}

u32 de_pll_enable(__disp_clk_id_t clk_id, s32 bOnOff)
{
	u32 i;
	u32 count;
	u32 reg_val;

	count = sizeof(disp_clk_pll_tbl) / sizeof(clk_pll_para);

	for(i = 0; i < count; i++)
	{
		if(disp_clk_pll_tbl[i].clk_id == clk_id)
		{
			reg_val = readl(disp_clk_pll_tbl[i].pll_adr);
			if(bOnOff)
			{
				pll_enable_count[i]++;
				if(pll_enable_count[i] == 1)
				{
					reg_val = SET_BITS(disp_clk_pll_tbl[i].enable_shift, 1, reg_val, 1);
					writel(reg_val, disp_clk_pll_tbl[i].pll_adr);
					if(disp_clk_pll_tbl[i].src_id)
					{
						de_pll_enable(disp_clk_pll_tbl[i].src_id, bOnOff);
					}
				}
				return 1;
			}
			else
			{
				if(pll_enable_count[i] == 0)
				{
					__wrn("pll %d is already disable\n", clk_id);
				}

				pll_enable_count[i]--;
				if(pll_enable_count[i] == 0)
				{
					reg_val = SET_BITS(disp_clk_pll_tbl[i].enable_shift, 1, reg_val, 0);
					writel(reg_val, disp_clk_pll_tbl[i].pll_adr);
					if(disp_clk_pll_tbl[i].src_id)
					{
						de_pll_enable(disp_clk_pll_tbl[i].src_id, bOnOff);
					}
				}
				return 1;
			}
		}
	}

	return 0;
}

u32 de_mclk_enable(__hdle clk_hdl, s32 bOnOff)
{
	u32 i;
	u32 count;
	u32 reg_val;

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;
	__inf("mod %d clock enable", hdl->clk_id);
	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			if(bOnOff)
			{
				mod_enable_count[i]++;
				if(mod_enable_count[i] == 1)
				{
					if(disp_clk_mod_tbl[i].mod_enable_shift < 32) {
						reg_val = readl(disp_clk_mod_tbl[i].mod_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_enable_shift, 1, reg_val, 1);
						__inf("mod clock %d enable on\n", hdl->clk_id);
						writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
					}

					if(disp_clk_mod_tbl[i].ahb_gate_shift < 32) {
						reg_val = readl(disp_clk_mod_tbl[i].ahb_gate_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].ahb_gate_shift, 1, reg_val, 1);
						__inf("mod clock %d ahb gate on\n", hdl->clk_id);
						writel(reg_val, disp_clk_mod_tbl[i].ahb_gate_adr);
					}

					if(disp_clk_mod_tbl[i].ahb_reset_shift < 32) {
						reg_val = readl(disp_clk_mod_tbl[i].ahb_reset_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].ahb_reset_shift, 1, reg_val, 1);
						__inf("mod clock %d ahb reset on\n", hdl->clk_id);
						writel(reg_val, disp_clk_mod_tbl[i].ahb_reset_adr);
					}

					if(disp_clk_mod_tbl[i].dram_gate_shift < 32) {
						reg_val = readl(disp_clk_mod_tbl[i].dram_gate_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].dram_gate_shift, 1, reg_val, 1);
						__inf("mod clock %d dram gate on\n", hdl->clk_id);
						writel(reg_val, disp_clk_mod_tbl[i].dram_gate_adr);
					}

					if(disp_clk_mod_tbl[i].src_id)
					{
						de_pll_enable(disp_clk_mod_tbl[i].src_id, bOnOff);
					}
				}
				return 1;
			}
			else
			{
				if(mod_enable_count[i] == 0)
				{
					__wrn("mod %d is already disable\n", hdl->clk_id);
					break;
				}

				mod_enable_count[i]--;
				if(mod_enable_count[i] == 0)
				{
					if(disp_clk_mod_tbl[i].mod_enable_shift < 32) {
						reg_val = readl(disp_clk_mod_tbl[i].mod_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_enable_shift, 1, reg_val, 0);
						__inf("mod clock %d enable off\n", hdl->clk_id);
						writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
					}

					if(disp_clk_mod_tbl[i].ahb_gate_shift < 32) {
						reg_val = readl(disp_clk_mod_tbl[i].ahb_gate_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].ahb_gate_shift, 1, reg_val, 0);
						__inf("mod clock %d ahb gate off\n", hdl->clk_id);
						writel(reg_val, disp_clk_mod_tbl[i].ahb_gate_adr);
					}

					if(disp_clk_mod_tbl[i].ahb_reset_shift < 32) {
						reg_val = readl(disp_clk_mod_tbl[i].ahb_reset_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].ahb_gate_shift, 1, reg_val, 0);
						__inf("mod clock %d ahb reset off\n", hdl->clk_id);
						writel(reg_val, disp_clk_mod_tbl[i].ahb_reset_adr);
					}

					if(disp_clk_mod_tbl[i].dram_gate_shift < 32) {
						reg_val = readl(disp_clk_mod_tbl[i].dram_gate_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].dram_gate_shift, 1, reg_val, 0);
						__inf("mod clock %d dram gate off\n", hdl->clk_id);
						writel(reg_val, disp_clk_mod_tbl[i].dram_gate_adr);
					}

					if(disp_clk_mod_tbl[i].src_id)
					{
						de_pll_enable(disp_clk_mod_tbl[i].src_id, bOnOff);
					}
				}

				return 1;
			}
		}
	}

	return 0;
}