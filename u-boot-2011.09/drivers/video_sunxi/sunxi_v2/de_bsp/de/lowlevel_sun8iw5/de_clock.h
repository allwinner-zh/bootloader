#ifndef  __DE_CLOCK_H__
#define  __DE_CLOCK_H__

#include <common.h>

#define __hdle u32

typedef enum
{
	CLK_NONE = 0,

	SYS_CLK_PLL3 = 1,
	SYS_CLK_PLL7 = 2,
	SYS_CLK_PLL9 = 3,
	SYS_CLK_PLL10 = 4,
	SYS_CLK_PLL3X2 = 5,
	SYS_CLK_PLL6 = 6,
	SYS_CLK_PLL6x2 = 7,
	SYS_CLK_PLL7X2 = 8,
	SYS_CLK_MIPIPLL = 9,

	MOD_CLK_DEBE0 = 16,
	MOD_CLK_DEBE1 = 17,
	MOD_CLK_DEFE0 = 18,
	MOD_CLK_DEFE1 = 19,
	MOD_CLK_LCD0CH0 = 20,
	MOD_CLK_LCD0CH1 = 21,
	MOD_CLK_LCD1CH0 = 22,
	MOD_CLK_LCD1CH1 = 23,
	MOD_CLK_HDMI = 24,
	MOD_CLK_HDMI_DDC = 25,
	MOD_CLK_MIPIDSIS = 26,
	MOD_CLK_MIPIDSIP = 27,
	MOD_CLK_IEPDRC0 = 28,
	MOD_CLK_IEPDRC1 = 29,
	MOD_CLK_IEPDEU0 = 30,
	MOD_CLK_IEPDEU1 = 31,
	MOD_CLK_LVDS = 32,
	MOD_CLK_EDP  = 33,
	MOD_CLK_DEBE2 = 34,
	MOD_CLK_DEFE2 = 35,
	MOD_CLK_SAT0 = 36,
	MOD_CLK_SAT1 = 37,
	MOD_CLK_SAT2 = 38,
	MOD_CLK_MERGE = 39,
}__disp_clk_id_t;

#define PLL_CFG(_clk_id, _src_id, _freq, _pll_adr, _enable_shift, _div_shift,\
				_div_bit_num, _fac_shift, _fac_bit_num, _ext_fac_shift, _ext_fac_bit_num,\
				_ext_enable_shift, _ext_enable_shift_num)\
{\
	.clk_id = _clk_id,\
	.src_id = _src_id,\
	.freq = _freq,\
	.pll_adr = _pll_adr,\
	.enable_shift = _enable_shift,\
	.div_shift = _div_shift,\
	.div_bit_num = _div_bit_num,\
	.fac_shift = _fac_shift,\
	.fac_bit_num = _fac_bit_num,\
	.ext_fac_shift = _ext_fac_shift,\
	.ext_fac_bit_num = _ext_fac_bit_num,\
	.ext_enable_shift = _ext_enable_shift,\
	.ext_enable_shift_num = _ext_enable_shift_num\
},

typedef struct {
	__disp_clk_id_t clk_id;
	__disp_clk_id_t src_id;
	u32 freq;
	u32 pll_adr;
	u32 enable_shift;
	u32 div_shift;
	u32 div_bit_num;
	u32 fac_shift;
	u32 fac_bit_num;
	u32 ext_fac_shift;
	u32 ext_fac_bit_num;
	u32 ext_enable_shift;
	u32 ext_enable_shift_num;
}clk_pll_para;

#define MOD_CFG(_clk_id, _src_id, _freq, _ahb_gate_adr, _ahb_gate_shift, _ahb_reset_adr, _ahb_reset_shift,\
					_dram_gate_adr, _dram_gate_shift, _mod_adr, _mod_enable_shift, _mod_src_shift,\
					_mod_src_bit_num, _mod_div_shift, _mod_div_bit_num)\
{\
	.clk_id = _clk_id,\
	.src_id = _src_id,\
	.freq = _freq,\
	.ahb_gate_adr = _ahb_gate_adr,\
	.ahb_gate_shift = _ahb_gate_shift,\
	.ahb_reset_adr = _ahb_reset_adr,\
	.ahb_reset_shift = _ahb_reset_shift,\
	.dram_gate_adr = _dram_gate_adr,\
	.dram_gate_shift = _dram_gate_shift,\
	.mod_adr = _mod_adr,\
	.mod_enable_shift = _mod_enable_shift,\
	.mod_src_shift = _mod_src_shift,\
	.mod_src_bit_num = _mod_src_bit_num,\
	.mod_div_shift = _mod_div_shift,\
	.mod_div_bit_num = _mod_div_bit_num\
},

typedef struct {
	__disp_clk_id_t clk_id;
	__disp_clk_id_t src_id;
	u32 freq;
	u32 ahb_gate_adr;
	u32 ahb_gate_shift;
	u32 ahb_reset_adr;
	u32 ahb_reset_shift;
	u32 dram_gate_adr;
	u32 dram_gate_shift;
	u32 mod_adr;
	u32 mod_enable_shift;
	u32 mod_src_shift;
	u32 mod_src_bit_num;
	u32 mod_div_shift;
	u32 mod_div_bit_num;
}clk_mod_para;

extern clk_pll_para disp_clk_pll_tbl[3];
extern clk_mod_para disp_clk_mod_tbl[8];

extern void de_clk_init_pll(void);
extern u32 de_get_pll_rate(__disp_clk_id_t clk_id);
extern u32 de_set_pll_rate(__disp_clk_id_t clk_id, u32 freq);
extern u32 de_set_mclk_src(__hdle clk_hdl);
extern u32 de_get_mclk_src(__hdle clk_hdl);
extern u32 de_set_mclk_freq(__hdle clk_hdl, u32 nFreq);
extern u32 de_get_mclk_freq(__hdle clk_hdl);
extern u32 de_pll_enable(__disp_clk_id_t clk_id, s32 bOnOff);
extern u32 de_mclk_enable(__disp_clk_id_t clk_id, s32 bOnOff);

#endif   //__DE_CLOCK_H__
