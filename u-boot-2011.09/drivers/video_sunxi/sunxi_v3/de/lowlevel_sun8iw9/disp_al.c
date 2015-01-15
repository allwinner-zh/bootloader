#include "disp_al.h"
#include "de_hal.h"

int disp_al_layer_apply(unsigned int disp, struct disp_layer_config_data *data, unsigned int layer_num)
{
	return de_al_lyr_apply(disp, data, layer_num);
}

int disp_al_manager_init(unsigned int disp)
{
	return de_clk_enable(DE_CLK_CORE0 + disp);
}

int disp_al_manager_exit(unsigned int disp)
{
	return de_clk_disable(DE_CLK_CORE0 + disp);
}

int disp_al_manager_apply(unsigned int disp, struct disp_manager_data *data)
{
	return de_al_mgr_apply(disp, data);
}

int disp_al_manager_sync(unsigned int disp)
{
	return de_al_mgr_sync(disp);
}

int disp_al_manager_update_regs(unsigned int disp)
{
	return de_al_mgr_update_regs(disp);
}

int disp_al_manager_query_irq(unsigned int disp)
{
	return de_al_query_irq(disp);
}

int disp_al_enhance_apply(unsigned int disp, struct disp_enhance_config *config)
{
	return de_enhance_apply(disp, config);
}

int disp_al_enhance_update_regs(unsigned int disp)
{
	return de_enhance_update_regs(disp);
}

int disp_al_enhance_sync(unsigned int disp)
{
	return de_enhance_sync(disp);
}

int disp_al_capture_init(unsigned int disp)
{
	return de_clk_enable(DE_CLK_WB);
}

int disp_al_capture_exit(unsigned int disp)
{
	return de_clk_disable(DE_CLK_WB);
}

int disp_al_capture_sync(u32 disp)
{
	static u32 count = 0;
		if(count < 10) {
			count ++;
			__inf("disp %d\n", disp);
		}
	WB_EBIOS_Update_Regs(disp);
	WB_EBIOS_Writeback_Enable(disp, 1);
	return 0;
}

int disp_al_capture_apply(unsigned int disp, struct disp_capture_config *cfg)
{
	static u32 count = 0;
		if(count < 10) {
			count ++;
			__inf("disp %d\n", disp);
		}
		__inf("disp%d, in_fmt=%d, in_stride<%d,%d,%d>, window<%d,%d,%d,%d>, out_fmt=%d, out_stride<%d,%d,%d>,crop<%d,%d,%d,%d>, addr<0x%llx,0x%llx,0x%llx>\n",
			disp, cfg->in_frame.format, cfg->in_frame.size[0].width, cfg->in_frame.size[1].width, cfg->in_frame.size[2].width,
			cfg->in_frame.crop.x, cfg->in_frame.crop.y, cfg->in_frame.crop.width, cfg->in_frame.crop.height,
			cfg->out_frame.format,  cfg->out_frame.size[0].width, cfg->out_frame.size[1].width, cfg->out_frame.size[2].width,
			cfg->out_frame.crop.x, cfg->out_frame.crop.y, cfg->out_frame.crop.width,
			cfg->out_frame.crop.height, cfg->out_frame.addr[0], cfg->out_frame.addr[1], cfg->out_frame.addr[2]);
	return WB_EBIOS_Apply(disp, cfg);
}

int disp_al_capture_get_status(unsigned int disp)
{
	return WB_EBIOS_Get_Status(disp);
}

int disp_al_smbl_apply(unsigned int disp, struct disp_smbl_info *info)
{
	static u32 count = 0;
		if(count < 10) {
			count ++;
			__inf("disp %d\n", disp);
		}
	return de_smbl_apply(disp, info);
}

int disp_al_smbl_update_regs(unsigned int disp)
{
	static u32 count = 0;
		if(count < 10) {
			count ++;
			__inf("disp %d\n", disp);
		}
	return de_smbl_update_regs(disp);
}

int disp_al_smbl_sync(unsigned int disp)
{
	static u32 count = 0;
		if(count < 10) {
			count ++;
			__inf("disp %d\n", disp);
		}
	return de_smbl_sync(disp);
}

int disp_al_smbl_get_status(unsigned int disp)
{
	static u32 count = 0;
		if(count < 10) {
			count ++;
			__inf("disp %d\n", disp);
		}

	return de_smbl_get_status(disp);
}

static struct lcd_clk_info clk_tbl[] = {
	{LCD_IF_HV,     6, 1, 1},
	{LCD_IF_CPU,   12, 1, 1},
	{LCD_IF_LVDS,   7, 1, 1},
	{LCD_IF_DSI,    4, 1, 4},
};
/* lcd */
/* lcd_dclk_freq * div -> lcd_clk_freq * div2 -> pll_freq */
/* lcd_dclk_freq * dsi_div -> lcd_dsi_freq */
int disp_al_lcd_get_clk_info(u32 screen_id, struct lcd_clk_info *info, disp_panel_para * panel)
{
	int tcon_div = 6;//tcon inner div
	int lcd_div = 1;//lcd clk div
	int dsi_div = 4;//dsi clk div
	int i;
	int find = 0;

	if(NULL == panel) {
		__wrn("panel is NULL\n");
		return 0;
	}

	for(i=0; i<sizeof(clk_tbl)/sizeof(struct lcd_clk_info); i++) {
		if(clk_tbl[i].lcd_if == panel->lcd_if) {
			tcon_div = clk_tbl[i].tcon_div;
			lcd_div = clk_tbl[i].lcd_div;
			dsi_div = clk_tbl[i].dsi_div;
			find = 1;
			break;
		}
	}

	if(0 == find)
		__wrn("cant find clk info for lcd_if %d\n", panel->lcd_if);

	info->tcon_div = tcon_div;
	info->lcd_div = lcd_div;
	info->dsi_div = dsi_div;

	return 0;
}

int disp_al_lcd_cfg(u32 screen_id, disp_panel_para * panel)
{
	struct lcd_clk_info info;

	tcon_init(screen_id);
	disp_al_lcd_get_clk_info(screen_id, &info, panel);
	DE_INF("lcd %d clk_div=%d!\n", screen_id, info.tcon_div);
	tcon0_set_dclk_div(screen_id, info.tcon_div);

	if(0 != tcon0_cfg(screen_id, panel))
		DE_WRN("lcd cfg fail!\n");
	else
		DE_INF("lcd cfg ok!\n");

	if(LCD_IF_DSI == panel->lcd_if)	{
#if defined(SUPPORT_DSI)
		if(0 != dsi_cfg(screen_id, panel)) {
			DE_WRN("dsi cfg fail!\n");
		}
#endif
	}

	return 0;
}

int disp_al_lcd_enable(u32 screen_id, disp_panel_para * panel)
{
	tcon0_open(screen_id, panel);
	if(LCD_IF_LVDS == panel->lcd_if) {
		lvds_open(screen_id, panel);
	} else if(LCD_IF_DSI == panel->lcd_if) {
#if defined(SUPPORT_DSI)
		dsi_open(screen_id, panel);
#endif
	}

	return 0;
}

int disp_al_lcd_disable(u32 screen_id, disp_panel_para * panel)
{
	if(LCD_IF_LVDS == panel->lcd_if) {
		lvds_close(screen_id);
	} else if(LCD_IF_DSI == panel->lcd_if) {
#if defined(SUPPORT_DSI)
		dsi_close(screen_id);
#endif
	}
	tcon0_close(screen_id);

	return 0;
}

/* query lcd irq, clear it when the irq queried exist
 */
int disp_al_lcd_query_irq(u32 screen_id, __lcd_irq_id_t irq_id, disp_panel_para * panel)
{
	int ret = 0;
	if(LCD_IF_DSI == panel->lcd_if) {
#if defined(SUPPORT_DSI)
		ret = dsi_irq_query(screen_id, (__dsi_irq_id_t)irq_id);
#endif
	}
	else
		ret = tcon_irq_query(screen_id, irq_id);

	return ret;
}

/* take dsi irq s32o account, todo? */
int disp_al_lcd_tri_busy(u32 screen_id, disp_panel_para * panel)
{
	int busy = 0;
	int ret = 0;

	busy |= tcon0_tri_busy(screen_id);
#if defined(SUPPORT_DSI)
	busy |= dsi_inst_busy(screen_id);
#endif
	ret = (busy == 0)? 0:1;

	return (ret);
}
/* take dsi irq s32o account, todo? */
int disp_al_lcd_tri_start(u32 screen_id, disp_panel_para * panel)
{
#if defined(SUPPORT_DSI)
	if(LCD_IF_DSI == panel->lcd_if)
		dsi_tri_start(screen_id);
#endif
	return tcon0_tri_start(screen_id);
}

int disp_al_lcd_io_cfg(u32 screen_id, u32 enable, disp_panel_para * panel)
{
#if defined(SUPPORT_DSI)
	if(LCD_IF_DSI ==  panel->lcd_if) {
		if(enable) {
			dsi_io_open(screen_id, panel);
		} else {
			dsi_io_close(screen_id);
		}
	}
#endif

	return 0;
}

int disp_al_lcd_get_cur_line(u32 screen_id, disp_panel_para * panel)
{
		if(LCD_IF_DSI == panel->lcd_if) {
#if defined(SUPPORT_DSI)
			return dsi_get_cur_line(screen_id);
#endif
		} else {
			return tcon_get_cur_line(screen_id, 0);
		}

	return 0;
}

int disp_al_lcd_get_start_delay(u32 screen_id, disp_panel_para * panel)
{
	if(LCD_IF_DSI == panel->lcd_if) {
#if defined(SUPPORT_DSI)
		return dsi_get_start_delay(screen_id);
#endif
	}
#if defined(SUPPORT_EDP)
	else if(LCD_IF_EDP == panel->lcd_if){
		return edp_get_start_delay();
	}
#endif
	else{
		return tcon_get_start_delay(screen_id, 0);
	}

	return 0;
}

/* hdmi */
int disp_al_hdmi_enable(u32 screen_id)
{
	tcon1_open(screen_id);
	return 0;
}

int disp_al_hdmi_disable(u32 screen_id)
{
	tcon1_close(screen_id);
	tcon_exit(screen_id);

	return 0;
}

int disp_al_hdmi_cfg(u32 screen_id, disp_video_timings *video_info)
{
	tcon_init(screen_id);
	tcon1_set_timming(screen_id, video_info);

	return 0;
}

int disp_init_al(disp_bsp_init_para * para)
{
	int i;
	de_al_init(para);
	de_enhance_init(para);
	de_ccsc_init(para);
	de_dcsc_init(para);
	WB_EBIOS_Init(para);
	de_clk_set_reg_base(para->reg_base[DISP_MOD_DE]);

	for(i=0; i<DEVICE_NUM; i++) {
		tcon_set_reg_base(i, para->reg_base[DISP_MOD_LCD0]);//calc lcd1 base
		de_smbl_init(i, para->reg_base[DISP_MOD_DE]);
	}

	return 0;
}

