#include "disp_display.h"
#include "smc.h"
__disp_dev_t gdisp;

s32 bsp_disp_init(__disp_bsp_init_para * para)
{
	u32 num_screens, screen_id;

#if defined(CONFIG_ARCH_SUN9IW1P1)
	u32 temp;
#endif

	memset(&gdisp,0x00,sizeof(__disp_dev_t));
	memcpy(&gdisp.init_para,para,sizeof(__disp_bsp_init_para));
	para->shadow_protect = bsp_disp_shadow_protect;
	disp_init_feat();

	num_screens = bsp_disp_feat_get_num_screens();
	for(screen_id = 0; screen_id < num_screens; screen_id++) {
#if defined(__LINUX_PLAT__)
		spin_lock_init(&gdisp.screen[screen_id].flag_lock);
#endif
	}

#if defined(CONFIG_ARCH_SUN9IW1P1)
	//for_test
//	temp = *((volatile u32*)(0x060005a8));
//	temp = temp | 0x4;
//	*((volatile u32*)(0x060005a8)) = temp;
	temp = smc_readl(0x060005a8);
	temp |= 4;
	smc_writel(temp, 0x060005a8);
#endif

	//bsp_disp_set_print_level(DEFAULT_PRINT_LEVLE);
	disp_notifier_init();
	disp_init_al(para);
	disp_init_lcd(para);
#if defined(CONFIG_ARCH_SUN9IW1P1)
	disp_init_hdmi(para);
	disp_init_cursor(para);
#endif
	disp_init_mgr(para);
	disp_init_lyr(para);
	disp_init_smcl(para);
	disp_init_smbl(para);

	disp_init_connections();

	return DIS_SUCCESS;
}

s32 bsp_disp_exit(u32 mode)
{
	u32 num_screens;
	u32 screen_id;
	u32 num_scalers;
	u32 scaler_id;
	struct disp_lcd *lcd;
	struct disp_manager *manager;

	num_screens = bsp_disp_feat_get_num_screens();
	num_scalers = bsp_disp_feat_get_num_scalers();

	if(mode == DISP_EXIT_MODE_CLEAN_ALL) {
		/* close all mod and unregister all irq */
		for(scaler_id = 0; scaler_id < num_scalers; scaler_id++) {
			scaler_close(scaler_id);
		}
		scaler_exit();

		for(screen_id = 0; screen_id < num_screens; screen_id++) {
			lcd = disp_get_lcd(screen_id);
			manager = disp_get_layer_manager(screen_id);

			manager->exit(manager);
			lcd->exit(lcd);
		}
	}	else if(mode == DISP_EXIT_MODE_CLEAN_PARTLY) {
		/* unregister all irq */
//		OSAL_InterruptDisable(AW_IRQ_DEBE0);
//		OSAL_UnRegISR(AW_IRQ_DEBE0, manager_event_proc, (void*)0);
		OSAL_InterruptDisable(AW_IRQ_LCD0);
		OSAL_UnRegISR(AW_IRQ_LCD0, disp_lcd_event_proc, (void*)0);

//		OSAL_InterruptDisable(AW_IRQ_DEBE1);
//		OSAL_UnRegISR(AW_IRQ_DEBE1, manager_event_proc, (void*)0);
		OSAL_InterruptDisable(AW_IRQ_LCD1);
		OSAL_UnRegISR(AW_IRQ_LCD1, disp_lcd_event_proc, (void*)0);
	}

	return DIS_SUCCESS;
}

s32 bsp_disp_open(void)
{
	return DIS_SUCCESS;
}

s32 bsp_disp_close(void)
{
	return DIS_SUCCESS;
}

s32 disp_init_connections(void)
{
	u32 screen_id = 0;
	u32 num_screens = 0;

	DE_INF("disp_init_connections\n");

	num_screens = bsp_disp_feat_get_num_screens();
	for(screen_id=0; screen_id<num_screens; screen_id++) {
		struct disp_manager *mgr;
		struct disp_smcl *smcl;
		struct disp_smbl *smbl;
#if defined(CONFIG_ARCH_SUN9IW1P1)
		struct disp_cursor *cursor;
#endif
		u32 num_layers, layer_id;

		num_layers = bsp_disp_feat_get_num_layers(screen_id);
		mgr = disp_get_layer_manager(screen_id);
		if(!mgr)
			continue;
		for(layer_id=0; layer_id<num_layers; layer_id++) {
			struct disp_layer *lyr;

			lyr = disp_get_layer(screen_id, layer_id);
			if(!lyr)
				continue;

			if(mgr->add_layer)
				mgr->add_layer(mgr, lyr);
			else
				DE_WRN("mgr's add_layer is NULL\n");

			if(lyr->set_manager)
				lyr->set_manager(lyr, mgr);
			else
				DE_WRN("lyr's set_manager is NULL\n");
		}
		smcl = disp_get_smcl(screen_id);
		if(smcl && smcl->set_manager)
			smcl->set_manager(smcl, mgr);
		else
			DE_WRN("smcl's set manager is NULL\n");

		smbl = disp_get_smbl(screen_id);
		if(smbl && smbl->set_manager)
			smbl->set_manager(smbl, mgr);
		else
			DE_WRN("smbl's set manager is NULL\n");

#if defined(CONFIG_ARCH_SUN9IW1P1)
		cursor = disp_get_cursor(screen_id);
		if(cursor && cursor->set_manager)
			cursor->set_manager(cursor, mgr);
		else
			DE_WRN("cursor's set manager is NULL\n");
#endif
	}

	return 0;
}
/***********************************************************
 *
 * interrupt proc
 *
 ***********************************************************/
s32 bsp_disp_cfg_get(u32 screen_id)
{
	return gdisp.screen[screen_id].cfg_cnt;
}

s32 bsp_disp_shadow_protect(u32 screen_id, bool protect)
{
	s32 ret = -1;
	u32 cnt = 0;
	u32 max_cnt = 100;//todo?  count according to output device para
	u32 delay = 10;//us
	disp_video_timing tt;
#ifdef __LINUX_PLAT__
	unsigned long flags;
#endif

	if(DISP_OUTPUT_TYPE_LCD == bsp_disp_get_output_type(screen_id)) {
		struct disp_lcd* lcd;

		lcd = disp_get_lcd(screen_id);
		if(!lcd) {
			DE_WRN("get lcd%d fail\n", screen_id);
		}

		if(lcd->get_timing) {
			u32 load_reg_time;
			u32 start_delay;
			u32 fps;

			lcd->get_timing(lcd, &tt);
			start_delay = disp_al_lcd_get_start_delay(screen_id);
			start_delay = (0 == start_delay)? 32:start_delay;
			fps = tt.pixel_clk*1000/(tt.ver_total_time*tt.hor_total_time);
			fps = (0 == fps)? 60:fps;
			load_reg_time = (1*1000*1000)/(fps*tt.ver_total_time)*start_delay;
			max_cnt = load_reg_time/delay + 1;
			max_cnt = max_cnt * 2;
		}
	} else if(DISP_OUTPUT_TYPE_HDMI == bsp_disp_get_output_type(screen_id)) {
		//todo?
		//for_test
		max_cnt = 10;
		delay = 10;
	}

	if(protect) {
		while((0 != ret) && (cnt < max_cnt)) {
#ifdef __LINUX_PLAT__
			spin_lock_irqsave(&gdisp.screen[screen_id].flag_lock, flags);
#endif
			cnt ++;
			if(gdisp.screen[screen_id].have_cfg_reg == false) {
				gdisp.screen[screen_id].cfg_cnt++;
				ret = 0;
			}
#ifdef __LINUX_PLAT__
			spin_unlock_irqrestore(&gdisp.screen[screen_id].flag_lock, flags);
#endif
			if(0 != ret)
				bsp_disp_delay_us(delay);
		}

		if(0 != ret) {
			DE_INF("wait for reg load finish time out\n");
#if defined(__LINUX_PLAT__)
			spin_lock_irqsave(&gdisp.screen[screen_id].flag_lock, flags);
#endif
			gdisp.screen[screen_id].cfg_cnt++;
#if defined(__LINUX_PLAT__)
			spin_unlock_irqrestore(&gdisp.screen[screen_id].flag_lock, flags);
#endif
		}
	} else {
#if defined(__LINUX_PLAT__)
			spin_lock_irqsave(&gdisp.screen[screen_id].flag_lock, flags);
#endif
			gdisp.screen[screen_id].cfg_cnt--;
#if defined(__LINUX_PLAT__)
			spin_unlock_irqrestore(&gdisp.screen[screen_id].flag_lock, flags);
#endif
	}
	return DIS_SUCCESS;
}

s32 bsp_disp_vsync_event_enable(u32 screen_id, bool enable)
{
	gdisp.screen[screen_id].vsync_event_en = enable;

	return DIS_SUCCESS;
}

s32 disp_sync_all(u32 screen_id)
{
	struct disp_manager *mgr;
	struct disp_smcl *smcl;
	struct disp_smbl *smbl;
#if defined(CONFIG_ARCH_SUN9IW1P1)
	struct disp_cursor *cursor;
#endif

	mgr = disp_get_layer_manager(screen_id);
	if(!mgr) {
		DE_WRN("get mgr%d fail\n", screen_id);
	}

	if(mgr->sync)
		mgr->sync(mgr);

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		//DE_WRN("get smcl %d fail\n", screen_id);
	} else {
		if(smcl->sync)
			smcl->sync(smcl);
		}

	smbl = disp_get_smbl(screen_id);
	if(!smbl) {
		//DE_WRN("get smbl%d fail\n", screen_id);
	} else {
		if(smbl->sync)
			smbl->sync(smbl);
		}

#if defined(CONFIG_ARCH_SUN9IW1P1)
	cursor = disp_get_cursor(screen_id);
	if(!cursor) {
		//DE_WRN("get cursor%d fail\n", screen_id);
	} else {
		if(cursor->sync)
			cursor->sync(cursor);
		}

#endif
	return 0;
}

void sync_event_proc(u32 screen_id)
{
	u32 cur_line = 0, start_delay = 0;
	//u32 num_scalers;

#if defined(__LINUX_PLAT__)
	unsigned long flags;
#endif

	if(screen_id < 2) {
		//num_scalers = bsp_disp_feat_get_num_scalers();

		cur_line = disp_al_lcd_get_cur_line(screen_id);
		start_delay = disp_al_lcd_get_start_delay(screen_id);
		if(cur_line > start_delay-4) {
			return ;
		}
	}

#if defined(__LINUX_PLAT__)
	spin_lock_irqsave(&gdisp.screen[screen_id].flag_lock, flags);
#endif
	if(0 == bsp_disp_cfg_get(screen_id)) {
		gdisp.screen[screen_id].have_cfg_reg = true;
#if defined(__LINUX_PLAT__)
	spin_unlock_irqrestore(&gdisp.screen[screen_id].flag_lock, flags);
#endif
		/* set ready of all mod, todo? */
		disp_sync_all(screen_id);
	} else {
#if defined(__LINUX_PLAT__)
	spin_unlock_irqrestore(&gdisp.screen[screen_id].flag_lock, flags);
#endif
	}

	return ;
}

s32 bsp_disp_get_output_type(u32 screen_id)
{
	struct disp_manager *mgr;
	u32 output_type = DISP_OUTPUT_TYPE_NONE;

	mgr = disp_get_layer_manager(screen_id);
	if(mgr && (mgr->get_output_type)) {
		mgr->get_output_type(mgr, &output_type);
	} else {
		DE_WRN("get mgr%d fail\n", screen_id);
	}

	return output_type;
}

s32 bsp_disp_get_screen_width(u32 screen_id)
{
	struct disp_manager *mgr;
	u32 width = 0, height = 0;

	mgr = disp_get_layer_manager(screen_id);
	if(mgr && (mgr->get_screen_size)) {
		mgr->get_screen_size(mgr, &width, &height);
		return width;
	}

	return DIS_FAIL;

}

s32 bsp_disp_get_screen_height(u32 screen_id)
{
	struct disp_manager *mgr;
	u32 width = 0, height = 0;

	mgr = disp_get_layer_manager(screen_id);
	if(mgr && (mgr->get_screen_size)) {
		mgr->get_screen_size(mgr, &width, &height);
		return height;
	}

	return DIS_FAIL;

}

s32 bsp_disp_get_screen_physical_width(u32 screen_id)
{
	u32 width = 0, height = 0;
	struct disp_lcd *lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->get_physical_size) {
		lcd->get_physical_size(lcd, &width, &height);
	}

	return width;
}

s32 bsp_disp_get_screen_physical_height(u32 screen_id)
{
	u32 width = 0, height = 0;
	struct disp_lcd *lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->get_physical_size) {
		lcd->get_physical_size(lcd, &width, &height);
	}

	return height;
}

s32 bsp_disp_get_screen_width_from_output_type(u32 screen_id, u32 output_type, u32 output_mode)
{
	u32 width, height;

	if(DISP_OUTPUT_TYPE_LCD == output_type) {
		struct disp_lcd *lcd;

		lcd = disp_get_lcd(screen_id);
		if(lcd && lcd->get_resolution) {
			lcd->get_resolution(lcd, &width, &height);
		}
	} else if(DISP_OUTPUT_TYPE_HDMI == output_type) {
		switch(output_mode) {
		case DISP_TV_MOD_576P:
			width = 720;
			height = 576;
			break;
		case DISP_TV_MOD_480P:
			width = 720;
			height = 480;
			break;
		case DISP_TV_MOD_720P_50HZ:
		case DISP_TV_MOD_720P_60HZ:
			width = 1280;
			height = 720;
			break;
		case DISP_TV_MOD_1080P_50HZ:
		case DISP_TV_MOD_1080P_60HZ:
		case DISP_TV_MOD_1080P_30HZ:
		case DISP_TV_MOD_1080P_24HZ:
			width = 1920;
			height = 1080;
			break;
		case DISP_TV_MOD_3840_2160P_30HZ:
		case DISP_TV_MOD_3840_2160P_24HZ:
		case DISP_TV_MOD_3840_2160P_25HZ:
			width = 3840;
			height = 2160;
			break;
		}
	}

	return width;
}

s32 bsp_disp_get_screen_height_from_output_type(u32 screen_id, u32 output_type, u32 output_mode)
{
	u32 width, height;

	if(DISP_OUTPUT_TYPE_LCD == output_type) {
		struct disp_lcd *lcd;

		lcd = disp_get_lcd(screen_id);
		if(lcd && lcd->get_resolution) {
			lcd->get_resolution(lcd, &width, &height);
		}
	} else if(DISP_OUTPUT_TYPE_HDMI == output_type) {
		switch(output_mode) {
		case DISP_TV_MOD_576P:
			width = 720;
			height = 576;
			break;
		case DISP_TV_MOD_480P:
			width = 720;
			height = 480;
			break;
		case DISP_TV_MOD_720P_50HZ:
		case DISP_TV_MOD_720P_60HZ:
			width = 1280;
			height = 720;
			break;
		case DISP_TV_MOD_1080P_50HZ:
		case DISP_TV_MOD_1080P_60HZ:
		case DISP_TV_MOD_1080P_30HZ:
		case DISP_TV_MOD_1080P_24HZ:
			width = 1920;
			height = 1080;
			break;
		case DISP_TV_MOD_3840_2160P_30HZ:
		case DISP_TV_MOD_3840_2160P_24HZ:
		case DISP_TV_MOD_3840_2160P_25HZ:
			width = 3840;
			height = 2160;
			break;
		}
	}

	return height;
}

s32 bsp_disp_get_timming(u32 screen_id, disp_video_timing * tt)
{
	memset(tt, 0, sizeof(disp_video_timing));
	if(DISP_OUTPUT_TYPE_LCD == bsp_disp_get_output_type(screen_id)) {
		struct disp_lcd *lcd;

		lcd = disp_get_lcd(screen_id);
		if(!lcd) {
			DE_WRN("get lcd%d failed\n", screen_id);
			return DIS_FAIL;
		}
		if(lcd->get_timing)
			return lcd->get_timing(lcd, tt);
	}
#if 0
	struct disp_hdmi* hdmi;
	else if(DISP_OUTPUT_TYPE_HDMI == bsp_disp_get_output_type(screen_id)) {
		//todo?
		hdmi = disp_get_hdmi(screen_id);
		if(!hdmi) {
			DE_WRN("get hdmi%d failed!\n", screen_id);
			return DIS_FAIL;
		}

	}
#endif
	return 0;
}

s32 bsp_disp_get_panel_info(u32 screen_id, disp_panel_para *info)
{
	struct disp_lcd* lcd;
	lcd = disp_get_lcd(screen_id);
	if(!lcd)
		DE_WRN("get lcd%d fail\n", screen_id);

	if(lcd->get_panel_info)
		return lcd->get_panel_info(lcd, info);

	return DIS_FAIL;
}
/***********************************************************
 *
 * lcd interface
 *
 ***********************************************************/
s32 bsp_disp_lcd_pre_enable(u32 screen_id)
{
	struct disp_lcd* lcd;
	struct disp_manager *mgr;

	u32 lcd_xres;
	u32 lcd_yres;
	mgr = disp_get_layer_manager(screen_id);
	lcd = disp_get_lcd(screen_id);
	if(!mgr || !lcd) {
		DE_WRN("get lcd%d or mgr%d fail\n", screen_id, screen_id);
	}

	if(mgr->is_enabled && mgr->is_enabled(mgr)) {
		DE_WRN("manager %d is already enable\n", screen_id);
		return DIS_FAIL;
	}

	if(mgr->enable)
		mgr->enable(mgr);
	if(mgr->set_output_type)
		mgr->set_output_type(mgr, DISP_OUTPUT_TYPE_LCD);

	if(lcd->get_resolution)
		lcd->get_resolution(lcd, &lcd_xres, &lcd_yres);

	if(mgr->set_screen_size)
		mgr->set_screen_size(mgr, lcd_xres, lcd_yres);

	if(lcd && lcd->pre_enable) {
		return lcd->pre_enable(lcd);
	}

	return DIS_FAIL;
}

extern s32 Display_set_fb_timming(u32 sel);
s32 bsp_disp_lcd_post_enable(u32 screen_id)
{
	struct disp_lcd* lcd;
	int ret = 0;

	lcd = disp_get_lcd(screen_id);
	if(!lcd) {
		DE_WRN("get lcd%d fail\n", screen_id);
	}

	if(lcd->post_enable)
		ret = lcd->post_enable(lcd);
#if defined(__LINUX_PLAT__)
	Display_set_fb_timming(screen_id);
#endif
	return ret;
}

s32 bsp_disp_lcd_pre_disable(u32 screen_id)
{
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->pre_disable) {
		return lcd->pre_disable(lcd);
	}
	return DIS_FAIL;
}

s32 bsp_disp_lcd_post_disable(u32 screen_id)
{
	struct disp_lcd* lcd;
	struct disp_manager *mgr;
	s32 ret = -1;

	mgr = disp_get_layer_manager(screen_id);
	lcd = disp_get_lcd(screen_id);
	if(!mgr || !lcd) {
		DE_WRN("get lcd%d or mgr%d fail\n", screen_id, screen_id);
	}

	if(lcd->post_disable)
		ret = lcd->post_disable(lcd);

	if(mgr->disable)
		mgr->disable(mgr);
	if(mgr->set_output_type)
		mgr->set_output_type(mgr, DISP_OUTPUT_TYPE_NONE);

	return ret;
}

s32 bsp_disp_lcd_backlight_enable(u32 screen_id)
{
	s32 ret = -1;
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->backlight_enable) {
		ret = lcd->backlight_enable(lcd);
	}

	return ret;
}

s32 bsp_disp_lcd_backlight_disable(u32 screen_id)
{
	s32 ret = -1;
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->backlight_disable) {
		ret = lcd->backlight_disable(lcd);
	}

	return ret;
}

s32 bsp_disp_lcd_pwm_enable(u32 screen_id)
{
	s32 ret = -1;
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->pwm_enable) {
		ret = lcd->pwm_enable(lcd);
	}

	return ret;
}

s32 bsp_disp_lcd_pwm_disable(u32 screen_id)
{
	s32 ret = -1;
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->pwm_disable) {
		ret = lcd->pwm_disable(lcd);
	}

	return ret;
}

s32 bsp_disp_lcd_power_enable(u32 screen_id, u32 power_id)
{
	s32 ret = -1;
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->power_enable) {
		ret = lcd->power_enable(lcd, power_id);
	}

	return ret;
}

s32 bsp_disp_lcd_power_disable(u32 screen_id, u32 power_id)
{
	s32 ret = -1;
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->pwm_disable) {
		ret = lcd->power_disable(lcd, power_id);
	}

	return ret;
}

s32 bsp_disp_lcd_set_bright(u32 screen_id, u32 bright)
{
	s32 ret = -1;
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->set_bright) {
		ret = lcd->set_bright(lcd, bright);
	}

	return ret;
}

s32 bsp_disp_lcd_get_bright(u32 screen_id)
{
	struct disp_lcd* lcd;
	u32 bright = 0;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->get_bright)
		lcd->get_bright(lcd, &bright);

	return bright;
}

disp_lcd_flow * bsp_disp_lcd_get_open_flow(u32 screen_id)
{
	struct disp_lcd* lcd;
	disp_lcd_flow *flow = NULL;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->get_open_flow) {
		flow = lcd->get_open_flow(lcd);
	}

	return flow;
}

disp_lcd_flow * bsp_disp_lcd_get_close_flow(u32 screen_id)
{
	struct disp_lcd* lcd;
	disp_lcd_flow *flow = NULL;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->get_close_flow) {
		flow = lcd->get_close_flow(lcd);
	}

	return flow;
}

s32 bsp_disp_lcd_set_panel_funs(char *name, disp_lcd_panel_fun * lcd_cfg)
{
	struct disp_lcd* lcd;
	u32 num_screens;
	u32 screen_id;
	char drv_name[32];

	num_screens = bsp_disp_feat_get_num_screens();
	for(screen_id=0; screen_id<num_screens; screen_id++) {
		lcd = disp_get_lcd(screen_id);
		if(lcd && (lcd->get_panel_driver_name) && (lcd->set_panel_func)) {
			lcd->get_panel_driver_name(lcd, drv_name);
			if(!strcmp(drv_name, name)) {
				if(!lcd->set_panel_func(lcd, lcd_cfg)) {
					gdisp.lcd_registered[screen_id] = 1;
					DE_INF("panel driver %s register\n", name);
				}
			}
		}
	}

	if(gdisp.init_para.start_process) {
		DE_INF("gdisp.init_para.start_process\n");
		gdisp.init_para.start_process();
	}

	return 0;
}

void LCD_OPEN_FUNC(u32 screen_id, LCD_FUNC func, u32 delay)
{
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);

	if(lcd && lcd->set_open_func) {
		lcd->set_open_func(lcd, func, delay);
	}
}

void LCD_CLOSE_FUNC(u32 screen_id, LCD_FUNC func, u32 delay)
{
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);

	if(lcd && lcd->set_close_func) {
		lcd->set_close_func(lcd, func, delay);
	}
}

s32 bsp_disp_lcd_is_used(u32 screen_id)
{
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->is_used) {
		return lcd->is_used(lcd);
	}

	return 0;
}

s32 bsp_disp_lcd_tcon_enable(u32 screen_id)
{
	s32 ret = -1;
	struct disp_lcd* lcd;
	__inf("bsp_disp_lcd_tcon_enable, sel=%d\n", screen_id);

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->tcon_enable) {
		ret = lcd->tcon_enable(lcd);
	}

	return ret;
}

s32 bsp_disp_lcd_tcon_disable(u32 screen_id)
{
	s32 ret = -1;
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->tcon_disable) {
		ret = lcd->tcon_disable(lcd);
	}

	return ret;
}

s32 bsp_disp_lcd_pin_cfg(u32 screen_id, u32 bon)
{
	struct disp_lcd* lcd;
	DE_INF("bsp_disp_lcd_pin_cfg, sel=%d, %s\n", screen_id, (bon)? "on":"off");

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->pin_cfg) {
		return lcd->pin_cfg(lcd, bon);
	}

	return DIS_FAIL;
}

s32 bsp_disp_lcd_delay_ms(u32 ms)
{
	return bsp_disp_delay_ms(ms);
}

s32 bsp_disp_lcd_delay_us(u32 us)
{
	return bsp_disp_delay_us(us);
}

s32 bsp_disp_lcd_gpio_set_value(u32 screen_id, u32 io_index, u32 value)
{
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->gpio_set_value) {
		return lcd->gpio_set_value(lcd, io_index, value);
	}

	return DIS_FAIL;
}

s32 bsp_disp_lcd_gpio_set_direction(u32 screen_id, u32 io_index, u32 direction)
{
	struct disp_lcd* lcd;

	lcd = disp_get_lcd(screen_id);
	if(lcd && lcd->gpio_set_direction) {
		return lcd->gpio_set_direction(lcd, io_index, direction);
	}

	return DIS_FAIL;
}

s32 bsp_disp_get_lcd_registered(u32 screen_id)
{
	return gdisp.lcd_registered[screen_id];
}

s32 bsp_disp_get_hdmi_registered(void)
{
	return gdisp.hdmi_registered;
}

#if defined(CONFIG_ARCH_SUN9IW1P1)
/***********************************************************
 *
 * hdmi interface
 *
 ***********************************************************/
s32 bsp_disp_hdmi_enable(u32 screen_id)
{
	struct disp_hdmi* hdmi;
	struct disp_manager *mgr;
	hdmi = disp_get_hdmi(screen_id);
	mgr = disp_get_layer_manager(screen_id);

	if(!hdmi || !mgr) {
		DE_WRN("get hdm%dor mgr%d failed!\n", screen_id, screen_id);
		return DIS_FAIL;
	    }

	if(mgr->is_enabled && mgr->is_enabled(mgr)) {
		DE_WRN("manager %d is already enable\n", screen_id);
		return DIS_FAIL;
	}

	if(mgr->enable)
		mgr->enable(mgr);
	if(mgr->set_output_type)
		mgr->set_output_type(mgr, DISP_OUTPUT_TYPE_HDMI);

	if(hdmi->enable) {
	    return hdmi->enable(hdmi);
	    }

	return DIS_FAIL;
}

s32 bsp_disp_hdmi_disable(u32 screen_id)
{
	struct disp_hdmi* hdmi;
	struct disp_manager *mgr;
	hdmi = disp_get_hdmi(screen_id);
	mgr = disp_get_layer_manager(screen_id);
	if(!hdmi || !mgr) {
		DE_WRN("get hdm%dor mgr%d failed!\n", screen_id, screen_id);
		return DIS_FAIL;
	    }

	if(hdmi->disable)
	    hdmi->disable(hdmi);

	if(mgr->is_enabled && mgr->is_enabled(mgr))
		mgr->disable(mgr);
	if(mgr->set_output_type)
		mgr->set_output_type(mgr, DISP_OUTPUT_TYPE_NONE);

	return DIS_FAIL;
}

s32 bsp_disp_hdmi_set_mode(u32 screen_id,  disp_tv_mode mode)
{
	struct disp_hdmi *hdmi;
	hdmi = disp_get_hdmi(screen_id);
	if(!hdmi) {
	    DE_WRN("get hdmi%d failed!\n", screen_id);
	    return DIS_FAIL;
	    }

	DE_INF("hdmi %d mode <-- %d\n", screen_id, mode);

	if(hdmi->set_mode)
		return hdmi->set_mode(hdmi, mode);

	return DIS_FAIL;
}

s32 bsp_disp_hdmi_get_mode(u32 screen_id)
{
	struct disp_hdmi *hdmi;
	hdmi = disp_get_hdmi(screen_id);
	if(!hdmi) {
	    DE_WRN("get hdmi%d failed!\n", screen_id);
	    return DIS_FAIL;
	    }

	if(hdmi->get_mode)
		return hdmi->get_mode(hdmi);

	return DIS_FAIL;
}

s32 bsp_disp_hdmi_check_support_mode(u32 screen_id,  u8 mode)
{
	struct disp_hdmi *hdmi;
	hdmi = disp_get_hdmi(screen_id);
	if(!hdmi) {
	    DE_WRN("get hdmi%d failed!\n", screen_id);
	    return DIS_FAIL;
	    }

	if(hdmi->check_support_mode)
	    return hdmi->check_support_mode(hdmi, mode);

	return DIS_FAIL;
}

s32 bsp_disp_hdmi_get_input_csc(u32 screen_id)
{
	struct disp_hdmi *hdmi;
	hdmi = disp_get_hdmi(screen_id);
	if(!hdmi) {
	    DE_WRN("get hdmi%d failed!\n", screen_id);
	    return DIS_FAIL;
	    }

	if(hdmi->get_input_csc)
	    return hdmi->get_input_csc(hdmi);

	return DIS_FAIL;
}
s32 bsp_disp_set_hdmi_func(u32 screen_id, disp_hdmi_func * func)
{
	struct disp_hdmi* hdmi;
	s32 ret = -1;
	hdmi = disp_get_hdmi(screen_id);
	if(!hdmi) {
	    DE_WRN("get hdmi%d failed!\n", screen_id);
	    return DIS_FAIL;
	    }

	gdisp.hdmi_registered = 1;
	if(hdmi->set_func)
	    ret = hdmi->set_func(hdmi, func);

	if(gdisp.init_para.start_process) {
		DE_INF("gdisp.init_para.start_process\n");
		gdisp.init_para.start_process();
	}
	return ret;
}

#endif

/***********************************************************
 *
 * layer interface
 *
 ***********************************************************/
s32 bsp_disp_layer_set_info(u32 screen_id, u32 layer_id,disp_layer_info *player)
{
	struct disp_layer *lyr;

	lyr = disp_get_layer(screen_id, layer_id);
	if(!lyr) {
		DE_WRN("get mgr%d lyr%d fail", screen_id, layer_id);
		return DIS_FAIL;
	}

	if(lyr->set_info)
		return lyr->set_info(lyr, player);
	else
		DE_WRN("lyr set_info is NULL\n");

	return DIS_FAIL;
}

s32 bsp_disp_layer_get_info(u32 screen_id, u32 layer_id,disp_layer_info *player)
{
	struct disp_layer *lyr;

	lyr = disp_get_layer(screen_id, layer_id);
	if(!lyr) {
		DE_WRN("get mgr%d lyr%d fail", screen_id, layer_id);
		return DIS_FAIL;
	}

	if(lyr->get_info)
		return lyr->get_info(lyr, player);
	else
		DE_WRN("lyr get_info is NULL\n");

	return DIS_FAIL;
}

s32 bsp_disp_layer_enable(u32 screen_id, u32 layer_id)
{
	struct disp_layer *lyr;

	lyr = disp_get_layer(screen_id, layer_id);
	if(!lyr) {
		DE_WRN("get mgr%d lyr%d fail", screen_id, layer_id);
		return DIS_FAIL;
	}

	if(lyr->enable)
		return lyr->enable(lyr);
	else
		DE_WRN("lyr enable is NULL\n");

	return DIS_FAIL;
}


s32 bsp_disp_layer_disable(u32 screen_id, u32 layer_id)
{
	struct disp_layer *lyr;

	lyr = disp_get_layer(screen_id, layer_id);
	if(!lyr) {
		DE_WRN("get mgr%d lyr%d fail", screen_id, layer_id);
		return DIS_FAIL;
	}

	if(lyr->disable)
		return lyr->disable(lyr);
	else
		DE_WRN("lyr disable is NULL\n");

	return DIS_FAIL;
}

s32 bsp_disp_layer_is_enabled(u32 screen_id, u32 layer_id)
{
	struct disp_layer *lyr;

	lyr = disp_get_layer(screen_id, layer_id);
	if(!lyr) {
		DE_WRN("get mgr%d lyr%d fail", screen_id, layer_id);
		return DIS_FAIL;
	}

	if(lyr->is_enabled)
		return lyr->is_enabled(lyr);
	else
		DE_WRN("lyr is_enabled is NULL\n");

	return 0;
}

s32 bsp_disp_layer_get_frame_id(u32 screen_id, u32 layer_id)
{
	struct disp_layer *lyr;

	lyr = disp_get_layer(screen_id, layer_id);
	if(!lyr) {
		DE_WRN("get mgr%d lyr%d fail", screen_id, layer_id);
		return 0;
	}

	if(lyr->get_frame_id)
		return lyr->get_frame_id(lyr);
	else
		DE_WRN("lyr disable is NULL\n");

	return 0;
}

/***********************************************************
 *
 * manager interface
 *
 ***********************************************************/
s32 bsp_disp_set_back_color(u32 screen_id, disp_color_info *bk_color)
{
	struct disp_manager *mgr;

	mgr = disp_get_layer_manager(screen_id);
	if(!mgr) {
		DE_WRN("get mgr%d fail\n", screen_id);
		return DIS_FAIL;
	}

	if(mgr->set_back_color)
		return mgr->set_back_color(mgr, bk_color);

	return DIS_FAIL;
}

/***********************************************************
 *
 * smart color interface
 *
 ***********************************************************/
s32 bsp_disp_smcl_enable(u32 screen_id)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smcl %d enable\n", screen_id);
	if(smcl->enable)
		return smcl->enable(smcl);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_disable(u32 screen_id)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smcl %d disable\n", screen_id);
	if(smcl->disable)
		return smcl->disable(smcl);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_is_enabled(u32 screen_id)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smcl->is_enabled)
		return smcl->is_enabled(smcl);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_set_bright(u32 screen_id, u32 val)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smcl %d bright <-- %d\n", screen_id, val);
	if(smcl->set_bright)
		return smcl->set_bright(smcl, val);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_get_bright(u32 screen_id)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smcl->get_bright)
		return smcl->get_bright(smcl);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_set_saturation(u32 screen_id, u32 val)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smcl %d saturation <-- %d\n", screen_id, val);
	if(smcl->set_saturation)
		return smcl->set_saturation(smcl, val);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_get_saturation(u32 screen_id)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smcl->get_saturation)
		return smcl->get_saturation(smcl);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_set_contrast(u32 screen_id, u32 val)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smcl %d contrast <-- %d\n", screen_id, val);
	if(smcl->set_contrast)
		return smcl->set_contrast(smcl, val);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_get_contrast(u32 screen_id)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smcl->get_contrast)
		return smcl->get_contrast(smcl);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_set_hue(u32 screen_id, u32 val)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smcl %d hue <-- %d\n", screen_id, val);
	if(smcl->set_hue)
		return smcl->set_hue(smcl, val);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_get_hue(u32 screen_id)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smcl->get_hue)
		return smcl->get_hue(smcl);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_set_mode(u32 screen_id, u32 val)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smcl %d mode <-- %d\n", screen_id, val);
	if(smcl->set_mode)
		return smcl->set_mode(smcl, val);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_get_mode(u32 screen_id)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smcl->get_mode)
		return smcl->get_mode(smcl);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_set_window(u32 screen_id, disp_window *window)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smcl %d window<%d,%d,%d,%d>\n", screen_id, window->x, window->y, window->width, window->height);
	if(smcl->set_window)
		return smcl->set_window(smcl, window);

	return DIS_FAIL;
}

s32 bsp_disp_smcl_get_window(u32 screen_id, disp_window *window)
{
	struct disp_smcl *smcl;

	smcl = disp_get_smcl(screen_id);
	if(!smcl) {
		DE_WRN("get smcl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smcl->get_window)
		return smcl->get_window(smcl, window);

	return DIS_FAIL;
}

/***********************************************************
 *
 * smart backlight interface
 *
 ***********************************************************/
s32 bsp_disp_smbl_enable(u32 screen_id)
{
	struct disp_smbl *smbl;

	smbl = disp_get_smbl(screen_id);
	if(!smbl) {
		DE_WRN("get smbl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smbl %d enable\n", screen_id);
	if(smbl->enable)
		return smbl->enable(smbl);

	return DIS_FAIL;
}

s32 bsp_disp_smbl_disable(u32 screen_id)
{
	struct disp_smbl *smbl;

	smbl = disp_get_smbl(screen_id);
	if(!smbl) {
		DE_WRN("get smbl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smbl %d disable\n", screen_id);
	if(smbl->disable)
		return smbl->disable(smbl);

	return DIS_FAIL;
}

s32 bsp_disp_smbl_is_enabled(u32 screen_id)
{
	struct disp_smbl *smbl;

	smbl = disp_get_smbl(screen_id);
	if(!smbl) {
		DE_WRN("get smbl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smbl->is_enabled)
		return smbl->is_enabled(smbl);

	return DIS_FAIL;
}

s32 bsp_disp_smbl_set_window(u32 screen_id, disp_window *window)
{
	struct disp_smbl *smbl;

	smbl = disp_get_smbl(screen_id);
	if(!smbl) {
		DE_WRN("get smbl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("smbl %d window<%d,%d,%d,%d>\n", screen_id, window->x, window->y, window->width, window->height);
	if(smbl->set_window)
		return smbl->set_window(smbl, window);

	return DIS_FAIL;
}

s32 bsp_disp_smbl_get_window(u32 screen_id, disp_window *window)
{
	struct disp_smbl *smbl;

	smbl = disp_get_smbl(screen_id);
	if(!smbl) {
		DE_WRN("get smbl %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(smbl->get_window)
		return smbl->get_window(smbl, window);

	return DIS_FAIL;
}

#if defined(CONFIG_ARCH_SUN9IW1P1)
/***********************************************************
 *
 * hardware cursor interface
 *
 ***********************************************************/
s32 bsp_disp_cursor_enable(u32 screen_id)
{
	struct disp_cursor *cursor;

	cursor = disp_get_cursor(screen_id);
	if(!cursor) {
		DE_WRN("get cursor %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("cursor %d enable\n", screen_id);
	if(cursor->enable)
		return cursor->enable(cursor);

	return DIS_FAIL;
}

s32 bsp_disp_cursor_disable(u32 screen_id)
{
	struct disp_cursor *cursor;

	cursor = disp_get_cursor(screen_id);
	if(!cursor) {
		DE_WRN("get cursor %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("cursor %d disable\n", screen_id);
	if(cursor->disable)
		return cursor->disable(cursor);

	return DIS_FAIL;
}

s32 bsp_disp_cursor_is_enabled(u32 screen_id)
{
	struct disp_cursor *cursor;

	cursor = disp_get_cursor(screen_id);
	if(!cursor) {
		DE_WRN("get cursor %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(cursor->is_enabled)
		return cursor->is_enabled(cursor);

	return 0;
}

s32 bsp_disp_cursor_set_pos(u32 screen_id, disp_position *pos)
{
	struct disp_cursor *cursor;

	cursor = disp_get_cursor(screen_id);
	if(!cursor) {
		DE_WRN("get cursor %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("cursor %d set pos, [%d,%d]\n", screen_id, pos->x, pos->y);
	if(cursor->set_pos)
		return cursor->set_pos(cursor, pos);

	return 0;
}

s32 bsp_disp_cursor_get_pos(u32 screen_id, disp_position *pos)
{
	struct disp_cursor *cursor;

	cursor = disp_get_cursor(screen_id);
	if(!cursor) {
		DE_WRN("get cursor %d fail \n", screen_id);
		return DIS_FAIL;
	}

	if(cursor->get_pos)
		return cursor->get_pos(cursor, pos);

	return DIS_FAIL;
}

s32 bsp_disp_cursor_set_fb(u32 screen_id, disp_cursor_fb *fb)
{
	struct disp_cursor *cursor;

	cursor = disp_get_cursor(screen_id);
	if(!cursor) {
		DE_WRN("get cursor %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("cursor %d set fb, mod=%d, addr=0x%x\n", screen_id, fb->mode, fb->addr);
	if(cursor->set_fb)
		return cursor->set_fb(cursor, fb);

	return 0;
}

s32 bsp_disp_cursor_set_palette(u32 screen_id, void *palette, u32 offset, u32 palette_size)
{
	struct disp_cursor *cursor;

	cursor = disp_get_cursor(screen_id);
	if(!cursor) {
		DE_WRN("get cursor %d fail \n", screen_id);
		return DIS_FAIL;
	}

	DE_INF("cursor %d set palette, addr=0x%x, offset=%d, size=%d\n", screen_id, (int)palette, offset, palette_size);
	if(cursor->set_palette)
		return cursor->set_palette(cursor, palette, offset, palette_size);

	return 0;
}
#endif

#if defined(__LINUX_PLAT__)
EXPORT_SYMBOL(LCD_OPEN_FUNC);
EXPORT_SYMBOL(LCD_CLOSE_FUNC);
#endif
