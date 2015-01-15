/*
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON arm boot sub-system
*
*						  Copyright(C), 2006-2010, SoftWinners Microelectronic Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      : javen
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include "OSAL_Clock.h"

__hdle osal_ccmu_get_clk_by_name(__disp_clk_id_t clk_id)
{
	u32 i;
	u32 count;

	count = sizeof(disp_clk_pll_tbl) / sizeof(clk_pll_para);
	for(i = 0; i < count; i++) {
		if(disp_clk_pll_tbl[i].clk_id == clk_id) {
			return (__hdle)&disp_clk_pll_tbl[i];
		}
	}

	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++) {
		if(disp_clk_mod_tbl[i].clk_id == clk_id) {
			return (__hdle)&disp_clk_mod_tbl[i];
		}
	}
	__wrn("get clock handle by id %d fail!\n", clk_id);
	return 0;
}

void osal_init_clk_pll(void)
{
	return de_clk_init_pll();
}

u32 OSAL_CCMU_SetSrcFreq(u32 nSclkNo, u32 nFreq)
{
	__hdle hdl;
	clk_mod_para *clk;
	u32 ret;

	hdl = osal_ccmu_get_clk_by_name(nSclkNo);
	if(!hdl) {
		__wrn("get clock handle fail!\n");
		return 0;
	}

	clk = (clk_mod_para*)hdl;
	ret = de_set_pll_rate(clk->clk_id, nFreq);
	if(!ret) {
		__wrn("set src freq fail!\n");
		return 0;
	}

	return 1;
}

u32 OSAL_CCMU_GetSrcFreq(u32 nSclkNo)
{
	__hdle hdl;
	clk_mod_para *clk;
	u32 freq;

	hdl = osal_ccmu_get_clk_by_name(nSclkNo);
	if(!hdl) {
		__wrn("get clock handle fail!\n");
		return 0;
	}

	clk = (clk_mod_para*)hdl;
	freq = de_get_pll_rate(clk->clk_id);
	if(!freq) {
		__wrn("get src freq fail!\n");
		return 0;
	}

	return freq;
}

__hdle OSAL_CCMU_OpenMclk(__disp_clk_id_t nMclkNo)
{
	return osal_ccmu_get_clk_by_name(nMclkNo);
}

u32 OSAL_CCMU_CloseMclk(__hdle clk_hdl)
{
	return 0;
}

u32 OSAL_CCMU_SetMclkSrc(__hdle clk_hdl)
{
	return de_set_mclk_src(clk_hdl);
}

u32 OSAL_CCMU_GetMclkSrc(__hdle clk_hdl)
{
	return de_get_mclk_src(clk_hdl);
}

u32 OSAL_CCMU_SetMclkFreq(__hdle clk_hdl, u32 nFreq)
{
	return de_set_mclk_freq(clk_hdl, nFreq);
}

u32 OSAL_CCMU_GetMclkFreq(__hdle clk_hdl)
{
	return de_get_mclk_freq(clk_hdl);
}

u32 OSAL_CCMU_MclkOnOff(__hdle clk_hdl, s32 bOnOff)
{
	return de_mclk_enable(clk_hdl, bOnOff);
}