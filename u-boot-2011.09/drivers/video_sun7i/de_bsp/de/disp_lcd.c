#include "disp_lcd.h"
#include "disp_display.h"
#include "disp_event.h"
#include "disp_de.h"
#include "disp_clk.h"

static __lcd_flow_t         open_flow[2];
static __lcd_flow_t         close_flow[2];
__panel_para_t              gpanel_info[2];
static __lcd_panel_fun_t    lcd_panel_fun[2];

void LCD_get_reg_bases(__reg_bases_t *para)
{
	para->base_lcdc0 = gdisp.init_para.reg_base[DISP_MOD_LCD0];
	para->base_lcdc1 = gdisp.init_para.reg_base[DISP_MOD_LCD1];
	para->base_pioc = gdisp.init_para.reg_base[DISP_MOD_PIOC];
	para->base_ccmu = gdisp.init_para.reg_base[DISP_MOD_CCMU];
	para->base_pwm  = gdisp.init_para.reg_base[DISP_MOD_PWM];
}

void Lcd_Panel_Parameter_Check(__u32 screen_id)
{
#if 0
	__panel_para_t* info;
	__u32 cycle_num = 1;
	__u32 Lcd_Panel_Err_Flag = 0;
	__u32 Lcd_Panel_Wrn_Flag = 0;
	__u32 Disp_Driver_Bug_Flag = 0;

	__u32 lcd_fclk_frq;
	__u32 lcd_clk_div;

	info = &(gpanel_info[screen_id]);

	if(info->lcd_if==0 && info->lcd_hv_if==1 && info->lcd_hv_smode==0)
		cycle_num = 3;
	else if(info->lcd_if==0 && info->lcd_hv_if==1 && info->lcd_hv_smode==1)
		cycle_num = 2;
	else if(info->lcd_if==1 && info->lcd_cpu_if==1)
		cycle_num = 3;
	else if(info->lcd_if==1 && info->lcd_cpu_if==2)
		cycle_num = 2;
	else if(info->lcd_if==1 && info->lcd_cpu_if==3)
		cycle_num = 2;
	else if(info->lcd_if==1 && info->lcd_cpu_if==5)
		cycle_num = 2;
	else if(info->lcd_if==1 && info->lcd_cpu_if==6)
		cycle_num = 3;
	else if(info->lcd_if==1 && info->lcd_cpu_if==7)
		cycle_num = 2;
	else
		cycle_num = 1;

	if(info->lcd_hbp > info->lcd_hv_hspw)
	{
		;
	}
	else
	{
		Lcd_Panel_Err_Flag |= BIT0;
	}

	if(info->lcd_vbp > info->lcd_hv_vspw)
	{
		;
	}
	else
	{
		Lcd_Panel_Err_Flag |= BIT1;
	}

	if(info->lcd_ht >= (info->lcd_hbp+info->lcd_x*cycle_num+4))
	{
		;
	}
	else
	{
		Lcd_Panel_Err_Flag |= BIT2;
	}

	if((info->lcd_vt/2) >= (info->lcd_vbp+info->lcd_y+2))
	{
		;
	}
	else
	{
		Lcd_Panel_Err_Flag |= BIT3;
	}

	lcd_clk_div = tcon0_get_dclk_div(screen_id);
	if(lcd_clk_div >= 6)
	{
		;
	}
	else if((lcd_clk_div ==5) || (lcd_clk_div ==4) || (lcd_clk_div ==2))
	{
		if((info->lcd_io_cfg0 != 0x00000000) && (info->lcd_io_cfg0 != 0x04000000))
		{
			Lcd_Panel_Err_Flag |= BIT10;
		}
	}
	else
	{
		Disp_Driver_Bug_Flag |= 1;
	}


	if((info->lcd_if==1 && info->lcd_cpu_if==0)
	 ||(info->lcd_if==3 && info->lcd_lvds_bitwidth==1))
	{
		if(info->lcd_frm != 1)
			Lcd_Panel_Wrn_Flag |= BIT0;
	}
	else if(info->lcd_if==1 && info->lcd_cpu_if==4)
	{
		if(info->lcd_frm != 2)
			Lcd_Panel_Wrn_Flag |= BIT1;
	}

	lcd_fclk_frq = (info->lcd_dclk_freq * 1000*1000)/((info->lcd_vt/2) * info->lcd_ht);
	if(lcd_fclk_frq<50 || lcd_fclk_frq>70)
	{
		Lcd_Panel_Wrn_Flag |= BIT2;
	}

	if(Lcd_Panel_Err_Flag != 0 || Lcd_Panel_Wrn_Flag != 0)
	{
		if(Lcd_Panel_Err_Flag != 0)
		{
			__u32 i;
			for(i=0;i<200;i++)
			{
				OSAL_PRINTF("*** Lcd in danger...\n");
			}
		}

		OSAL_PRINTF("*****************************************************************\n");
		OSAL_PRINTF("***\n");
		OSAL_PRINTF("*** LCD Panel Parameter Check\n");
		OSAL_PRINTF("***\n");
		OSAL_PRINTF("***             by dulianping\n");
		OSAL_PRINTF("***\n");
		OSAL_PRINTF("*****************************************************************\n");

		OSAL_PRINTF("*** \n");
		OSAL_PRINTF("*** Interface:");
		if(info->lcd_if==0 && info->lcd_hv_if==0)
			{OSAL_PRINTF("*** Parallel HV Panel\n");}
		else if(info->lcd_if==0 && info->lcd_hv_if==1)
			{OSAL_PRINTF("*** Serial HV Panel\n");}
		else if(info->lcd_if==0 && info->lcd_hv_if==2)
			{OSAL_PRINTF("*** Serial YUV Panel\n");}
		else if(info->lcd_if==3 && info->lcd_lvds_bitwidth==0)
			{OSAL_PRINTF("*** 24Bit LVDS Panel\n");}
		else if(info->lcd_if==3 && info->lcd_lvds_bitwidth==1)
			{OSAL_PRINTF("*** 18Bit LVDS Panel\n");}
		else if(info->lcd_if==1 && info->lcd_cpu_if==0)
			{OSAL_PRINTF("*** 18Bit CPU Panel\n");}
		else if(info->lcd_if==1 && info->lcd_cpu_if==4)
			{OSAL_PRINTF("*** 16Bit CPU Panel\n");}
		else
		{
			OSAL_PRINTF("\n");
			OSAL_PRINTF("*** lcd_if:     %d\n",info->lcd_if);
			OSAL_PRINTF("*** lcd_hv_if:  %d\n",info->lcd_hv_if);
			OSAL_PRINTF("*** lcd_cpu_if: %d\n",info->lcd_cpu_if);
		}

		if(info->lcd_frm==0)
			{OSAL_PRINTF("*** Lcd Frm Disable\n");}
		else if(info->lcd_frm==1)
			{OSAL_PRINTF("*** Lcd Frm to RGB666\n");}
		else if(info->lcd_frm==2)
			{OSAL_PRINTF("*** Lcd Frm to RGB565\n");}

		OSAL_PRINTF("*** \n");
		OSAL_PRINTF("*** Timing:\n");
		OSAL_PRINTF("*** lcd_x:      %d\n",info->lcd_x);
		OSAL_PRINTF("*** lcd_y:      %d\n",info->lcd_y);
		OSAL_PRINTF("*** lcd_ht:     %d\n",info->lcd_ht);
		OSAL_PRINTF("*** lcd_hbp:    %d\n",info->lcd_hbp);
		OSAL_PRINTF("*** lcd_vt:     %d\n",info->lcd_vt);
		OSAL_PRINTF("*** lcd_vbp:    %d\n",info->lcd_vbp);
		OSAL_PRINTF("*** lcd_hspw:   %d\n",info->lcd_hv_hspw);
		OSAL_PRINTF("*** lcd_vspw:   %d\n",info->lcd_hv_vspw);
		OSAL_PRINTF("*** lcd_frame_frq:  %dHz\n",lcd_fclk_frq);

		//´òÓ¡´íÎóÌáÊ¾
		OSAL_PRINTF("*** \n");
		if(Lcd_Panel_Err_Flag & BIT0)
			{OSAL_PRINTF("*** Err01: Violate \"lcd_hbp > lcd_hspw\"\n");}
		if(Lcd_Panel_Err_Flag & BIT1)
			{OSAL_PRINTF("*** Err02: Violate \"lcd_vbp > lcd_vspw\"\n");}
		if(Lcd_Panel_Err_Flag & BIT2)
			{OSAL_PRINTF("*** Err03: Violate \"lcd_ht >= (lcd_hbp+lcd_x*%d+4)\"\n", cycle_num);}
		if(Lcd_Panel_Err_Flag & BIT3)
			{OSAL_PRINTF("*** Err04: Violate \"(lcd_vt/2) >= (lcd_vbp+lcd_y+2)\"\n");}
		if(Lcd_Panel_Err_Flag & BIT10)
			{OSAL_PRINTF("*** Err10: Violate \"lcd_io_cfg0\",use \"0x00000000\" or \"0x04000000\"");}
		if(Lcd_Panel_Wrn_Flag & BIT0)
			{OSAL_PRINTF("*** WRN01: Recommend \"lcd_frm = 1\"\n");}
		if(Lcd_Panel_Wrn_Flag & BIT1)
			{OSAL_PRINTF("*** WRN02: Recommend \"lcd_frm = 2\"\n");}
		if(Lcd_Panel_Wrn_Flag & BIT2)
			{OSAL_PRINTF("*** WRN03: Recommend \"lcd_dclk_frq = %d\"\n",((info->lcd_vt/2) * info->lcd_ht)*60/(1000*1000));}
		OSAL_PRINTF("*** \n");

    	if(Lcd_Panel_Err_Flag != 0)
    	{
            __u32 image_base_addr;
            __u32 reg_value = 0;

            image_base_addr = DE_Get_Reg_Base(screen_id);

            sys_put_wvalue(image_base_addr+0x804,0xffff00ff);//set background color

            reg_value = sys_get_wvalue(image_base_addr+0x800);
            sys_put_wvalue(image_base_addr+0x800,reg_value & 0xfffff0ff);//close all layer

#ifdef __LINUX_OSAL__
            LCD_delay_ms(2000);
            sys_put_wvalue(image_base_addr+0x804,0x00000000);//set background color
            sys_put_wvalue(image_base_addr+0x800,reg_value);//open layer
#endif
            OSAL_PRINTF("*** Try new parameters,you can make it pass!\n");
    	}
        OSAL_PRINTF("*** LCD Panel Parameter Check End\n");
        OSAL_PRINTF("*****************************************************************\n");
	}
#endif
}

__s32 LCD_parse_panel_para(__u32 screen_id, __panel_para_t * info)
{
    __s32 ret = 0;
    char primary_key[25];
    __s32 value = 0;

    if(!bsp_disp_lcd_used(screen_id))//no need to get panel para if lcd_used eq 0
        return 0;

    sprintf(primary_key, "lcd%d_para", screen_id);

    memset(info, 0, sizeof(__panel_para_t));

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_x", &value, 1);
    if(ret == 0)
    {
        info->lcd_x = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_y", &value, 1);
    if(ret == 0)
    {
        info->lcd_y = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_width", &value, 1);
    if(ret == 0)
    {
        info->lcd_width = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_height", &value, 1);
    if(ret == 0)
    {
        info->lcd_height = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_dclk_freq", &value, 1);
    if(ret == 0)
    {
        info->lcd_dclk_freq = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_pwm_freq", &value, 1);
    if(ret == 0)
    {
        info->lcd_pwm_freq = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_pwm_pol", &value, 1);
    if(ret == 0)
    {
        info->lcd_pwm_pol = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_if", &value, 1);
    if(ret == 0)
    {
        info->lcd_if = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_hbp", &value, 1);
    if(ret == 0)
    {
        info->lcd_hbp = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_ht", &value, 1);
    if(ret == 0)
    {
        info->lcd_ht = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_vbp", &value, 1);
    if(ret == 0)
    {
        info->lcd_vbp = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_vt", &value, 1);
    if(ret == 0)
    {
        info->lcd_vt = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_hv_if", &value, 1);
    if(ret == 0)
    {
        info->lcd_hv_if = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_vspw", &value, 1);
    if(ret == 0)
    {
        info->lcd_vspw = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_hspw", &value, 1);
    if(ret == 0)
    {
        info->lcd_hspw = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_lvds_if", &value, 1);
    if(ret == 0)
    {
        info->lcd_lvds_if = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_lvds_mode", &value, 1);
    if(ret == 0)
    {
        info->lcd_lvds_mode = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_lvds_colordepth", &value, 1);
    if(ret == 0)
    {
        info->lcd_lvds_colordepth= value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_lvds_io_polarity", &value, 1);
    if(ret == 0)
    {
        info->lcd_lvds_io_polarity = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_cpu_if", &value, 1);
    if(ret == 0)
    {
        info->lcd_cpu_if = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_cpu_te", &value, 1);
    if(ret == 0)
    {
        info->lcd_cpu_te = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_frm", &value, 1);
    if(ret == 0)
    {
        info->lcd_frm = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_dsi_if", &value, 1);
    if(ret == 0)
    {
        info->lcd_dsi_if = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_dsi_lane", &value, 1);
    if(ret == 0)
    {
        info->lcd_dsi_lane = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_dsi_format", &value, 1);
    if(ret == 0)
    {
        info->lcd_dsi_format = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_dsi_eotp", &value, 1);
    if(ret == 0)
    {
        info->lcd_dsi_eotp = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_dsi_te", &value, 1);
    if(ret == 0)
    {
        info->lcd_dsi_te = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_edp_tx_ic", &value, 1);
    if(ret == 0)
    {
        info->lcd_edp_tx_ic= value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_edp_tx_rate", &value, 1);
    if(ret == 0)
    {
        info->lcd_edp_tx_rate = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_edp_tx_lane", &value, 1);
    if(ret == 0)
    {
        info->lcd_edp_tx_lane= value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_edp_colordepth", &value, 1);
    if(ret == 0)
    {
        info->lcd_edp_colordepth = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_hv_clk_phase", &value, 1);
    if(ret == 0)
    {
        info->lcd_hv_clk_phase = value;
    }

	ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_hv_sync_polarity", &value, 1);
    if(ret == 0)
    {
        info->lcd_hv_sync_polarity = value;
    }
    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_gamma_en", &value, 1);
    if(ret == 0)
    {
        info->lcd_gamma_en = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_cmap_en", &value, 1);
    if(ret == 0)
    {
        info->lcd_cmap_en = value;
    }

    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_xtal_freq", &value, 1);
    if(ret == 0)
    {
        info->lcd_xtal_freq = value;
    }


    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_size", (int*)info->lcd_size, 2);
    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_model_name", (int*)info->lcd_model_name, 2);

    return 0;
}

void LCD_get_sys_config(__u32 screen_id, __disp_lcd_cfg_t *lcd_cfg)
{
    static char io_name[28][20] = {"lcdd0", "lcdd1", "lcdd2", "lcdd3", "lcdd4", "lcdd5", "lcdd6", "lcdd7", "lcdd8", "lcdd9", "lcdd10", "lcdd11",
                         "lcdd12", "lcdd13", "lcdd14", "lcdd15", "lcdd16", "lcdd17", "lcdd18", "lcdd19", "lcdd20", "lcdd21", "lcdd22",
                         "lcdd23", "lcdclk", "lcdde", "lcdhsync", "lcdvsync"};
    disp_gpio_set_t  *gpio_info;
    int  value = 1;
    char primary_key[20], sub_name[25];
    int i = 0;
    int  ret;

    sprintf(primary_key, "lcd%d_para", screen_id);

//lcd_used
    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_used", &value, 1);
    if(ret == 0)
    {
        lcd_cfg->lcd_used = value;
    }

    if(lcd_cfg->lcd_used == 0) //no need to get lcd config if lcd_used eq 0
        return ;

//lcd_bl_en
    lcd_cfg->lcd_bl_en_used = 0;
    gpio_info = &(lcd_cfg->lcd_bl_en);
    ret = OSAL_Script_FetchParser_Data(primary_key,"lcd_bl_en", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
    if(ret == 0)
    {
        lcd_cfg->lcd_bl_en_used = 1;
    }

//lcd_power0
	for(i=0; i<3; i++)
	{
		if(i==0)
			sprintf(sub_name, "lcd_power");
		else
			sprintf(sub_name, "lcd_power%d", i);
		lcd_cfg->lcd_power_used[i] = 0;
		gpio_info = &(lcd_cfg->lcd_power[i]);
		ret = OSAL_Script_FetchParser_Data(primary_key,sub_name, (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
		if(ret == 0) {
		  lcd_cfg->lcd_power_used[i] = 1;
		}
	}

//lcd_pwm
    lcd_cfg->lcd_pwm_used= 0;
    #ifdef CONFIG_SUN6I_RESERVE

    gpio_info = &(lcd_cfg->lcd_pwm);
    ret = OSAL_Script_FetchParser_Data(primary_key,"lcd_pwm", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
    if(ret == 0)
    {
        lcd_cfg->lcd_pwm_used = 1;
        lcd_cfg->lcd_pwm_ch = 0;

        __inf("lcd_pwm_used=%d,lcd_pwm_ch=%d\n", lcd_cfg->lcd_pwm_used, lcd_cfg->lcd_pwm_ch);
    }

    #else
    value = 0;
    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_pwm_used", &value, 1);
    if(ret == 0)
        lcd_cfg->lcd_pwm_used = value;

    value = 0;
    ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_pwm_ch", &value, 1);
    if(ret == 0)
        lcd_cfg->lcd_pwm_ch = value;

    #endif
#if 0
//lcd_gpio
    for(i=0; i<4; i++)
    {
        sprintf(sub_name, "lcd_gpio_%d", i);

        gpio_info = &(lcd_cfg->lcd_gpio[i]);
        ret = OSAL_Script_FetchParser_Data(primary_key,sub_name, (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
        if(ret == 0)
        {
            lcd_cfg->lcd_gpio_used[i]= 1;
        }
    }

//lcd_gpio_scl,lcd_gpio_sda
    gpio_info = &(lcd_cfg->lcd_gpio[LCD_GPIO_SCL]);
    ret = OSAL_Script_FetchParser_Data(primary_key,"lcd_gpio_scl", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
    if(ret == 0)
    {
        lcd_cfg->lcd_gpio_used[LCD_GPIO_SCL]= 1;
    }
    gpio_info = &(lcd_cfg->lcd_gpio[LCD_GPIO_SDA]);
    ret = OSAL_Script_FetchParser_Data(primary_key,"lcd_gpio_sda", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
    if(ret == 0)
    {
        lcd_cfg->lcd_gpio_used[LCD_GPIO_SDA]= 1;
    }
#else
//lcd_gpio
    for(i=0; i<6; i++)
    {
        sprintf(sub_name, "lcd_gpio_%d", i);

        gpio_info = &(lcd_cfg->lcd_gpio[i]);
        ret = OSAL_Script_FetchParser_Data(primary_key,sub_name, (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
        if(ret == 0)
        {
            lcd_cfg->lcd_gpio_used[i]= 1;
        }
    }
    
#endif
//lcd io
    for(i=0; i<28; i++)
    {
        gpio_info = &(lcd_cfg->lcd_io[i]);
        ret = OSAL_Script_FetchParser_Data(primary_key,io_name[i], (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
        if(ret == 0)
        {
            lcd_cfg->lcd_io_used[i]= 1;
        }
    }

//backlight adjust
	for(i = 0; i < 101; i++) {
		sprintf(sub_name, "lcd_bl_%d_percent", i);
		lcd_cfg->backlight_curve_adjust[i] = 0;

		if(i == 100)
		lcd_cfg->backlight_curve_adjust[i] = 255;

		ret = OSAL_Script_FetchParser_Data(primary_key, sub_name, &value, 1);
		if(ret == 0) {
			value = (value > 100)? 100:value;
			value = value * 255 / 100;
			lcd_cfg->backlight_curve_adjust[i] = value;
		}
	}


//init_bright
    sprintf(primary_key, "disp_init");
    sprintf(sub_name, "lcd%d_backlight", screen_id);

    ret = OSAL_Script_FetchParser_Data(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->backlight_bright = 197;
    }
    else
    {
        if(value > 256)
        {
            value = 256;
        }
        lcd_cfg->backlight_bright = value;
    }

//bright,constraction,saturation,hue
    sprintf(primary_key, "disp_init");
    sprintf(sub_name, "lcd%d_bright", screen_id);
    ret = OSAL_Script_FetchParser_Data(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->lcd_bright = 50;
    }
    else
    {
        if(value > 100)
        {
            value = 100;
        }
        lcd_cfg->lcd_bright = value;
    }

    sprintf(sub_name, "lcd%d_contrast", screen_id);
    ret = OSAL_Script_FetchParser_Data(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->lcd_contrast = 50;
    }
    else
    {
        if(value > 100)
        {
            value = 100;
        }
        lcd_cfg->lcd_contrast = value;
    }

    sprintf(sub_name, "lcd%d_saturation", screen_id);
    ret = OSAL_Script_FetchParser_Data(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->lcd_saturation = 50;
    }
    else
    {
        if(value > 100)
        {
            value = 100;
        }
        lcd_cfg->lcd_saturation = value;
    }

    sprintf(sub_name, "lcd%d_hue", screen_id);
    ret = OSAL_Script_FetchParser_Data(primary_key, sub_name, &value, 1);
    if(ret < 0)
    {
        lcd_cfg->lcd_hue = 50;
    }
    else
    {
        if(value > 100)
        {
            value = 100;
        }
        lcd_cfg->lcd_hue = value;
    }
}


__s32 bsp_disp_lcd_delay_ms(__u32 ms)
{
#ifdef __LINUX_OSAL__
	__u32 timeout = ms*HZ/1000;

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(timeout);
#endif
#ifdef __BOOT_OSAL__
	wBoot_timer_delay(ms);//assume cpu runs at 1000Mhz,10 clock one cycle
#endif
#ifdef __UBOOT_OSAL__
    __msdelay(ms);
#endif
	return 0;
}


__s32 bsp_disp_lcd_delay_us(__u32 us)
{
#ifdef __LINUX_OSAL__
	udelay(us);
#endif
#ifdef __BOOT_OSAL__
	volatile __u32 time;

	for(time = 0; time < (us*700/10);time++);//assume cpu runs at 700Mhz,10 clock one cycle
#endif
#ifdef __UBOOT_OSAL__
    __usdelay(us);
#endif
	return 0;
}

void LCD_OPEN_FUNC(__u32 screen_id, LCD_FUNC func, __u32 delay)
{
	open_flow[screen_id].func[open_flow[screen_id].func_num].func = func;
	open_flow[screen_id].func[open_flow[screen_id].func_num].delay = delay;
	open_flow[screen_id].func_num++;
}


void LCD_CLOSE_FUNC(__u32 screen_id, LCD_FUNC func, __u32 delay)
{
	close_flow[screen_id].func[close_flow[screen_id].func_num].func = func;
	close_flow[screen_id].func[close_flow[screen_id].func_num].delay = delay;
	close_flow[screen_id].func_num++;
}


__s32 bsp_disp_lcd_tcon_open(__u32 screen_id)
{
	if(gpanel_info[screen_id].tcon_index == 0) {
		tcon0_open(screen_id,gpanel_info+screen_id);
		gdisp.screen[screen_id].lcdc_status |= LCDC_TCON0_USED;
	}	else {
		tcon1_open(screen_id);
		gdisp.screen[screen_id].lcdc_status |= LCDC_TCON1_USED;
	}

	if(gpanel_info[screen_id].lcd_if == LCD_IF_LVDS) {
		lvds_open(screen_id,gpanel_info+screen_id);
	} else if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
		dsi_open(screen_id, &gpanel_info[screen_id]);
	}

	return 0;
}

__s32 bsp_disp_lcd_tcon_close(__u32 screen_id)
{
	if(gpanel_info[screen_id].lcd_if == LCD_IF_LVDS) {
		lvds_close(screen_id);
	}else if (gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
		dsi_close(screen_id);
	}

	if(gpanel_info[screen_id].tcon_index == 0) {
		tcon0_close(screen_id);
		gdisp.screen[screen_id].lcdc_status &= LCDC_TCON0_USED_MASK;
	}	else {
		tcon1_close(screen_id);
		gdisp.screen[screen_id].lcdc_status &= LCDC_TCON1_USED_MASK;
	}

	return 0;
}

int TCON_get_open_status(__u32 screen_id)
{
	if(gdisp.screen[screen_id].lcdc_status & (LCDC_TCON0_USED | LCDC_TCON1_USED)) {
		return 1;
	} else {
		return 0;
	}
}

__s32 TCON_get_cur_line(__u32 screen_id, __u32 tcon_index)
{
	if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
		return dsi_get_cur_line(screen_id);
	} else {
		return tcon_get_cur_line(screen_id, tcon_index);
	}

	return 0;
}

__s32 TCON_get_start_delay(__u32 screen_id, __u32 tcon_index)
{
	if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
		return dsi_get_start_delay(screen_id);
	} else	{
		return tcon_get_start_delay(screen_id, tcon_index);
	}

	return 0;
}

#ifdef CONFIG_SUN6I_RESERVE

static __u32 pwm_read_reg(__u32 offset)
{
	__u32 value = 0;

	value = sys_get_wvalue(gdisp.init_para.reg_base[DISP_MOD_PWM]+offset);

	return value;
}

static __s32 pwm_write_reg(__u32 offset, __u32 value)
{
	sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PWM]+offset, value);

	return 0;
}

__s32 pwm_enable(__u32 channel)
{
	__u32 tmp = 0;

	tmp = pwm_read_reg(channel*0x10);
	tmp |= (1<<4);
	pwm_write_reg(channel*0x10,tmp);

	gdisp.pwm[channel].enable = 1;

	return 0;
}

__s32 pwm_disable(__u32 channel)
{
	__u32 tmp = 0;

	tmp = pwm_read_reg(channel*0x10);

	tmp &= (~(1<<4));
	pwm_write_reg(channel*0x10,tmp);

	gdisp.pwm[channel].enable = 0;

	return 0;
}




//channel: pwm channel,0/1
//pwm_info->freq:  pwm freq, in hz
//pwm_info->active_state: 0:low level; 1:high level
__s32 pwm_set_para(__u32 channel, __pwm_info_t * pwm_info)
{
	__u32 pre_scal[7] = {1, 2, 4, 8, 16, 32, 64};
	__u32 pre_scal_id = 0, entire_cycle = 256, active_cycle = 192;
	__u32 i=0, tmp=0;
	__u32 entire_cycle_max = 0x10000;//65536;
	__u32 freq;

	//    __inf("pwm_set_para, chn:%d, en:%d, active_state:%d, duty_ns:%d, period_ns:%d, mode:%d \n",
	//        channel, pwm_info->enable, pwm_info->active_state, pwm_info->duty_ns, pwm_info->period_ns,pwm_info->mode);

	if(pwm_info->period_ns != 0) {
		freq = 1000000000 / pwm_info->period_ns;
	} else {
		DE_WRN("pwm%d period_ns is ZERO\n", channel);
		freq = 1000;
	}

	if(freq > 24000000) {
		DE_WRN("pwm preq is large then 24mhz, fix to 24mhz\n");
		freq = 24000000;
	}
	pre_scal_id = (freq > 400)? 0:6;
	entire_cycle = 24000000 / freq / pre_scal[pre_scal_id];

	/* entire_cycle must smaller than 65536 */
	for(i=1; entire_cycle > entire_cycle_max; i++) {
		entire_cycle >>= 1;
		pre_scal_id = i;
	}

	if(pre_scal_id > 6)	{
		pre_scal_id = 6;
		DE_WRN("pwm preq is too small, may be imprecise!\n");
	}

	if(pwm_info->period_ns < 5*100*1000) {
		active_cycle = (pwm_info->duty_ns * entire_cycle + (pwm_info->period_ns/2)) / pwm_info->period_ns;
	}	else {
		active_cycle = ((pwm_info->duty_ns / 100) * entire_cycle + (pwm_info->period_ns/2)) / (pwm_info->period_ns/100);
	}

	gdisp.pwm[channel].enable = pwm_info->enable;
	gdisp.pwm[channel].freq = freq;
	gdisp.pwm[channel].pre_scal = pre_scal[pre_scal_id];
	gdisp.pwm[channel].active_state = pwm_info->active_state;
	gdisp.pwm[channel].duty_ns = pwm_info->duty_ns;
	gdisp.pwm[channel].period_ns = pwm_info->period_ns;
	gdisp.pwm[channel].entire_cycle = entire_cycle;
	gdisp.pwm[channel].active_cycle = active_cycle;
#if 0
	DE_INF("[PWM]freq = %d, pre_scal=%d, active_state=%d, duty_ns=%d,period_ns=%d, entire_cycle=%d, active_cycle=%d, mode=%d \n",
	    gdisp.pwm[channel].freq,  gdisp.pwm[channel].pre_scal,
	    gdisp.pwm[channel].active_state,gdisp.pwm[channel].duty_ns,gdisp.pwm[channel].period_ns,gdisp.pwm[channel].entire_cycle,
	    gdisp.pwm[channel].active_cycle, gdisp.pwm[channel].mode);
#endif
	pwm_write_reg(channel*0x10+0x04, ((entire_cycle - 1)<< 16) | active_cycle);

	tmp = ((0<<7) | (1<<6) | (pwm_info->active_state<<5) | pre_scal_id);//bit7: channel mode(0cycle,1pulse; bit6:gatting the special clock for pwm0; bit5:pwm0  active state is high level
	pwm_write_reg(channel*0x10,tmp);

	if(pwm_info->enable) {
		pwm_enable(channel);
	} else {
		pwm_disable(channel);
	}

	return 0;
}

__s32 pwm_get_para(__u32 channel, __pwm_info_t * pwm_info)
{
	pwm_info->enable = gdisp.pwm[channel].enable;
	pwm_info->active_state = gdisp.pwm[channel].active_state;
	pwm_info->duty_ns = gdisp.pwm[channel].duty_ns;
	pwm_info->period_ns = gdisp.pwm[channel].period_ns;

	return 0;
}

__s32 pwm_set_duty_ns(__u32 channel, __u32 duty_ns)
{
	__u32 active_cycle = 0;

	if(gdisp.pwm[channel].period_ns < 5*100*1000) {
		active_cycle = (duty_ns * gdisp.pwm[channel].entire_cycle + (gdisp.pwm[channel].period_ns/2)) / gdisp.pwm[channel].period_ns;
	}	else {
		active_cycle = ((duty_ns / 100) * gdisp.pwm[channel].entire_cycle + (gdisp.pwm[channel].period_ns/2)) / (gdisp.pwm[channel].period_ns/100);
	}

	active_cycle = (active_cycle > gdisp.pwm[channel].entire_cycle)?  gdisp.pwm[channel].entire_cycle:active_cycle;

	//tmp = pwm_read_reg(channel*0x10+0x04);
	pwm_write_reg(channel*0x10+0x04,(gdisp.pwm[channel].entire_cycle<<16) | active_cycle);

	gdisp.pwm[channel].duty_ns = duty_ns;

	DE_INF("[PWM]: duty_ns=%d,period_ns=%d,active_cycle=%d,entire_cycle=%d\n", duty_ns, gdisp.pwm[channel].period_ns, active_cycle, gdisp.pwm[channel].entire_cycle);
	return 0;
}

#endif

__s32 LCD_PWM_EN(__u32 screen_id, __bool b_en)
{
    #ifdef CONFIG_SUN6I_RESERVE
	if(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_used) {
		disp_gpio_set_t  gpio_info[1];
		__hdle hdl;

		memcpy(gpio_info, &(gdisp.screen[screen_id].lcd_cfg.lcd_pwm), sizeof(disp_gpio_set_t));

		//printf("LCD_PWM_EN, sel=%d, b_en=%d, pwm_ch=%d\n", screen_id, b_en, gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch);
		if(b_en) {
			pwm_enable(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch);
			hdl = OSAL_GPIO_Request(gpio_info, 1);
			OSAL_GPIO_Release(hdl, 2);
		}	else {
			gpio_info->mul_sel = 7;
			hdl = OSAL_GPIO_Request(gpio_info, 1);
			OSAL_GPIO_Release(hdl, 2);
			pwm_disable(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch);
		}
	}

    #else
    if(b_en)
        sunxi_pwm_enable(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch);
    else
        sunxi_pwm_disable(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch);

    #endif

	return 0;
}

__s32 bsp_disp_lcd_pwm_enable(__u32 screen_id)
{
	return LCD_PWM_EN(screen_id, 1);
}

__s32 bsp_disp_lcd_pwm_disable(__u32 screen_id)
{
	return LCD_PWM_EN(screen_id, 0);
}

__s32 LCD_BL_EN(__u32 screen_id, __bool b_en)
{
	disp_gpio_set_t  gpio_info[1];
	__hdle hdl;

	if(gdisp.screen[screen_id].lcd_cfg.lcd_bl_en_used) {
		memcpy(gpio_info, &(gdisp.screen[screen_id].lcd_cfg.lcd_bl_en), sizeof(disp_gpio_set_t));

		if(!b_en)	{
			gpio_info->data = (gpio_info->data==0)?1:0;
			gpio_info->mul_sel = 7;
		}

		hdl = OSAL_GPIO_Request(gpio_info, 1);
		OSAL_GPIO_Release(hdl, 2);
	}

	return 0;
}

__s32 bsp_disp_lcd_backlight_enable(__u32 screen_id)
{
	return LCD_BL_EN(screen_id, 1);
}

__s32 bsp_disp_lcd_backlight_disable(__u32 screen_id)
{
	return LCD_BL_EN(screen_id, 0);
}

__s32 LCD_POWER_EN_EXT(__u32 sel, __bool b_en)
{
	disp_gpio_set_t  gpio_info[1];
	__hdle hdl;
	//printf("LCD_POWER_EN_EXT, sel=%d, b_en=%d\n", sel, b_en);

	//if(gdisp.screen[sel].lcd_cfg.lcd_power_used) {
	if(1) {
		if(gpanel_info[sel].lcd_if == LCD_IF_EXT_DSI) {
			if(b_en) {
				//axp_set_eldo3(1); //eldo3 1.2v
				axp_set_supply_status(0, PMU_SUPPLY_ELDO3, 0, 1);
				bsp_disp_lcd_delay_ms(10);
			} else {
				//axp_set_eldo3(0); //eldo3 1.2v
				axp_set_supply_status(0, PMU_SUPPLY_ELDO3, 0, 0);
			}
		}
		memcpy(gpio_info, &(gdisp.screen[sel].lcd_cfg.lcd_power[0]), sizeof(disp_gpio_set_t));

		if(!b_en) {
			gpio_info->data = (gpio_info->data==0)?1:0;
		}

		hdl = OSAL_GPIO_Request(gpio_info, 1);
		OSAL_GPIO_Release(hdl, 2);

		/* anx9804 */
		if((gpanel_info[sel].lcd_if == LCD_IF_EDP) && (gpanel_info[sel].lcd_edp_tx_ic == 0)) {
			if(b_en) {
				//printf("axp_set_eldo3, on\n");
				//axp_set_eldo3(1); //eldo3 1.8v
				axp_set_supply_status(0, PMU_SUPPLY_ELDO3, 0, 1);
			}	else {
				//axp_set_eldo3(0);
				axp_set_supply_status(0, PMU_SUPPLY_ELDO3, 0, 0);
			}
		}
		/* anx6345 */
		else if((gpanel_info[sel].lcd_if == LCD_IF_EDP) && (gpanel_info[sel].lcd_edp_tx_ic == 1))
		{
			if(b_en) {
				//axp_set_dldo1(1);//dldo1 2.5v
				axp_set_supply_status(0, PMU_SUPPLY_DLDO1, 0, 1);
				bsp_disp_lcd_delay_ms(5);
				//axp_set_eldo3(1);//eldo3 1.2v
				axp_set_supply_status(0, PMU_SUPPLY_ELDO3, 0, 1);
			} else {
				//axp_set_eldo3(0);
				axp_set_supply_status(0, PMU_SUPPLY_ELDO3, 0, 0);
				//axp_set_dldo1(0);
				axp_set_supply_status(0, PMU_SUPPLY_DLDO1, 0, 0);
			}
		}
	}
	bsp_disp_lcd_delay_ms(30);
	return 0;
}

__s32 LCD_POWER_EN(__u32 screen_id, __u32 pwr_id, __bool b_en)
{
#if defined CONFIG_ARCH_SUN8IW1P1
	LCD_POWER_EN_EXT(screen_id, b_en);
#else
	disp_gpio_set_t  gpio_info[1];
	__hdle hdl;
	if(gdisp.screen[screen_id].lcd_cfg.lcd_power_used[pwr_id]) {
		memcpy(gpio_info, &(gdisp.screen[screen_id].lcd_cfg.lcd_power[pwr_id]), sizeof(disp_gpio_set_t));
		if(!b_en)	{
			gpio_info->data = (gpio_info->data==0)?1:0;
		}

		hdl = OSAL_GPIO_Request(gpio_info, 1);
		OSAL_GPIO_Release(hdl, 2);
	}
#endif
	return 0;
}

__s32 bsp_disp_lcd_power_enable(__u32 screen_id, __u32 pwr_id)
{
	return LCD_POWER_EN(screen_id, pwr_id, 1);
}

__s32 bsp_disp_lcd_power_disable(__u32 screen_id, __u32 pwr_id)
{
	return LCD_POWER_EN(screen_id, pwr_id, 0);
}

__s32 bsp_disp_lcd_power_disable_all(__u32 screen_id)
{
	__u32 i;
	for(i=0; i<3; i++) {
		LCD_POWER_EN(screen_id, i, 1);
		LCD_POWER_EN(screen_id, i, 0);
	}
	return 0;
}

__s32 LCD_GPIO_request(__u32 screen_id, __u32 io_index)
{
	return 0;
}

__s32 LCD_GPIO_release(__u32 screen_id,__u32 io_index)
{
	return 0;
}

__s32 LCD_GPIO_set_attr(__u32 screen_id,__u32 io_index, __bool b_output)
{
	char gpio_name[20];

	sprintf(gpio_name, "lcd_gpio_%d", io_index);
	return  OSAL_GPIO_DevSetONEPIN_IO_STATUS(gdisp.screen[screen_id].gpio_hdl[io_index], b_output, gpio_name);
}

__s32 LCD_GPIO_read(__u32 screen_id,__u32 io_index)
{
	char gpio_name[20];

	sprintf(gpio_name, "lcd_gpio_%d", io_index);
	return OSAL_GPIO_DevREAD_ONEPIN_DATA(gdisp.screen[screen_id].gpio_hdl[io_index], gpio_name);
}

__s32 LCD_GPIO_write(__u32 screen_id,__u32 io_index, __u32 data)
{
	char gpio_name[20];

	sprintf(gpio_name, "lcd_gpio_%d", io_index);
	return OSAL_GPIO_DevWRITE_ONEPIN_DATA(gdisp.screen[screen_id].gpio_hdl[io_index], data, gpio_name);
}

__s32 LCD_GPIO_init(__u32 screen_id)
{
	__u32 i = 0;

	for(i=0; i<6; i++) {
		gdisp.screen[screen_id].gpio_hdl[i] = 0;

		if(gdisp.screen[screen_id].lcd_cfg.lcd_gpio_used[i]) {
			disp_gpio_set_t  gpio_info[1];

			memcpy(gpio_info, &(gdisp.screen[screen_id].lcd_cfg.lcd_gpio[i]), sizeof(disp_gpio_set_t));
			gdisp.screen[screen_id].gpio_hdl[i] = OSAL_GPIO_Request(gpio_info, 1);
		}
	}

	return 0;
}

__s32 LCD_GPIO_exit(__u32 screen_id)
{
	__u32 i = 0;

	for(i=0; i<6; i++) {
		if(gdisp.screen[screen_id].gpio_hdl[i]) {
			disp_gpio_set_t  gpio_info[1];

			OSAL_GPIO_Release(gdisp.screen[screen_id].gpio_hdl[i], 2);

			memcpy(gpio_info, &(gdisp.screen[screen_id].lcd_cfg.lcd_gpio[i]), sizeof(disp_gpio_set_t));
			gpio_info->mul_sel = 7;
			gdisp.screen[screen_id].gpio_hdl[i] = OSAL_GPIO_Request(gpio_info, 1);
			OSAL_GPIO_Release(gdisp.screen[screen_id].gpio_hdl[i], 2);
			gdisp.screen[screen_id].gpio_hdl[i] = 0;
		}
	}

	return 0;
}

void LCD_CPU_register_irq(__u32 screen_id, void (*Lcd_cpuisr_proc) (void))
{
	gdisp.screen[screen_id].LCD_CPUIF_ISR = Lcd_cpuisr_proc;
}

__s32 bsp_disp_lcd_pin_cfg(__u32 screen_id, __u32 bon)
{
	if(1)
	{
		__hdle lcd_pin_hdl;
		int  i;
	#if defined (CONFIG_A50_FPGA)
		if(!bon) {
			//pd28--pwm, ph0--pwr
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x6c, 0x77777777);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x70, 0x77777777);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x74, 0x77777777);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x78, 0x77777777);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0xfc, (sys_get_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0xfc) & (~0x0000000f)) | 0x00000007);
		} else {
			//pd28--pwm, ph0--pwr
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x6c, 0x22222222);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x70, 0x22222222);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x74, 0x22222222);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x78, 0x00122222);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x7c, sys_get_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x7c) | 0x30000000);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0xfc, (sys_get_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0xfc) &(0xfffffff0)) | 0x00000001);
			sys_put_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x10c, sys_get_wvalue(gdisp.init_para.reg_base[DISP_MOD_PIOC]+0x10c) | 0x00000001);
		}
	#endif
		if(bon)	{
			LCD_GPIO_init(screen_id);
		}	else {
			LCD_GPIO_exit(screen_id);
		}
		for(i=0; i<28; i++)	{
			if(gdisp.screen[screen_id].lcd_cfg.lcd_io_used[i]) {
				disp_gpio_set_t  gpio_info[1];

				memcpy(gpio_info, &(gdisp.screen[screen_id].lcd_cfg.lcd_io[i]), sizeof(disp_gpio_set_t));
				if(!bon) {
					gpio_info->mul_sel = 7;
				}	else {
					if((gpanel_info[screen_id].lcd_if == 3) && (gpio_info->mul_sel==2))	{
						gpio_info->mul_sel = 3;
					}
				}
				lcd_pin_hdl = OSAL_GPIO_Request(gpio_info, 1);
				OSAL_GPIO_Release(lcd_pin_hdl, 2);
			}
		}

		if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
			if(bon)	{
				dsi_io_open(screen_id, &gpanel_info[screen_id]);
			} else {
				dsi_io_close(screen_id);
			}
		}
	}

	return DIS_SUCCESS;
}


#ifdef __LINUX_OSAL__
__s32 disp_lcdc_event_proc(int irq, void *parg)
#else
__s32 disp_lcdc_event_proc(void *parg)
#endif
{
	__u32 screen_id = (__u32)parg;

	if(tcon_irq_query(screen_id,LCD_IRQ_TCON0_VBLK)) {
		LCD_vbi_event_proc(screen_id, 0);
	}
	if(tcon_irq_query(screen_id,LCD_IRQ_TCON1_VBLK)) {
		LCD_vbi_event_proc(screen_id, 1);
	}
	
#if (!defined CONFIG_ARCH_SUN7I) && (!defined CONFIG_ARCH_SUN5I)
	static __u32 cntr=0;
	if(tcon_irq_query(screen_id,LCD_IRQ_TCON0_CNTR)) {
		LCD_vbi_event_proc(screen_id, 0);

		if(dsi_inst_busy(screen_id) || tcon0_tri_busy(screen_id))	{
			if(cntr>=1)	{
				cntr = 0;
			}	else {
				cntr++;
			}
		}	else {
			cntr = 0;
		}

		if(cntr==0)	{
			if(gdisp.screen[screen_id].LCD_CPUIF_ISR)	{
				(*gdisp.screen[screen_id].LCD_CPUIF_ISR)();
				bsp_disp_lcd_delay_us(2);
			}
			if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
				dsi_tri_start(screen_id);
			}
			tcon0_tri_start(screen_id);
		}
	}
	if(tcon_irq_query(screen_id,LCD_IRQ_TCON0_TRIF)) {
		//  LCD_vbi_event_proc(screen_id, 0);
	}

	if(dsi_irq_query(screen_id,DSI_IRQ_VIDEO_VBLK))	{
		LCD_vbi_event_proc(screen_id, 0);
	}
#endif
	return OSAL_IRQ_RETURN;
}

__s32 __disp_lcd_bright_curve_init(__u32 screen_id);
__s32 disp_lcdc_init(__u32 screen_id)
{
	LCD_get_sys_config(screen_id, &(gdisp.screen[screen_id].lcd_cfg));
	gdisp.screen[screen_id].lcd_cfg.backlight_dimming = 256;

	if(gdisp.screen[screen_id].lcd_cfg.lcd_used) {
		LCD_parse_panel_para(screen_id, &gpanel_info[screen_id]);
		if(lcd_panel_fun[screen_id].cfg_panel_info)	{
			lcd_panel_fun[screen_id].cfg_panel_info(&gpanel_info[screen_id].lcd_extend_para);
		}
		__disp_lcd_bright_curve_init(screen_id);
		gpanel_info[screen_id].tcon_index = 0;

		bsp_disp_lcd_power_disable_all(screen_id);
	}

	lcdc_clk_init(screen_id);
	if(gpanel_info[screen_id].lcd_if == LCD_IF_LVDS) {
		lvds_clk_init();
	}	else if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
		dsi_clk_init();
	}

	lcdc_clk_on(screen_id, 0, 0);
	lcdc_clk_on(screen_id, 0, 1);
	tcon_init(screen_id);
	lcdc_clk_off(screen_id);

	if(screen_id == 0) {
		OSAL_RegISR(gdisp.init_para.irq[DISP_MOD_LCD0],0,disp_lcdc_event_proc,(void*)screen_id,0,0);
		if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI)	{
			OSAL_RegISR(gdisp.init_para.irq[DISP_MOD_DSI0],0,disp_lcdc_event_proc,(void*)screen_id,0,0);
		}
		//LCD_get_panel_funs_0(&lcd_panel_fun[screen_id]);
#ifndef __LINUX_OSAL__
		OSAL_InterruptEnable(gdisp.init_para.irq[DISP_MOD_LCD0]);
		if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
			OSAL_InterruptEnable(gdisp.init_para.irq[DISP_MOD_DSI0]);
		}
#endif
	} else {
		OSAL_RegISR(gdisp.init_para.irq[DISP_MOD_LCD1],0,disp_lcdc_event_proc,(void*)screen_id,0,0);
		//LCD_get_panel_funs_1(&lcd_panel_fun[screen_id]);
#ifndef __LINUX_OSAL__
		OSAL_InterruptEnable(gdisp.init_para.irq[DISP_MOD_LCD1]);
#endif
	}
	if(gdisp.screen[screen_id].lcd_cfg.lcd_used) {
		/* pwm config */
		if(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_used == 1)	{
			__pwm_info_t pwm_info;
			__u64 backlight_bright;
			__u64 period_ns;

			pwm_info.enable = 0;
			pwm_info.active_state = gpanel_info[screen_id].lcd_pwm_pol? 0:1;
			if(gpanel_info[screen_id].lcd_pwm_freq != 0)
			{
			pwm_info.period_ns = 1000000000 / gpanel_info[screen_id].lcd_pwm_freq;
			}else
			{
			DE_WRN("lcd%d.lcd_pwm_freq is ZERO\n", screen_id);
			pwm_info.period_ns = 1000000000 / 1000;  //default 1khz
			}

			backlight_bright = gdisp.screen[screen_id].lcd_cfg.backlight_bright;
			period_ns = pwm_info.period_ns;

			pwm_info.duty_ns = (backlight_bright * period_ns) / 256;
			DE_INF("[PWM]backlight_bright=%d,period_ns=%d,duty_ns=%d\n",(__u32)backlight_bright,(__u32)period_ns, (__u32)pwm_info.duty_ns);

                        #ifdef CONFIG_SUN6I_RESERVE
			pwm_set_para(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch, &pwm_info);

                        #else
                            sunxi_pwm_set_polarity(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch, pwm_info.active_state);
                            sunxi_pwm_config(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch, pwm_info.duty_ns, pwm_info.period_ns);

                            if(pwm_info.enable)
                                sunxi_pwm_enable(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch);
                            else
                                sunxi_pwm_disable(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch);

                        #endif
		}
	}
	return DIS_SUCCESS;
}

__s32 disp_lcdc_exit(__u32 screen_id)
{
	if(screen_id == 0) {
		OSAL_InterruptDisable(gdisp.init_para.irq[DISP_MOD_LCD0]);
		OSAL_UnRegISR(gdisp.init_para.irq[DISP_MOD_LCD0],disp_lcdc_event_proc,(void*)screen_id);
		OSAL_InterruptDisable(gdisp.init_para.irq[DISP_MOD_DSI0]);
		OSAL_UnRegISR(gdisp.init_para.irq[DISP_MOD_DSI0],disp_lcdc_event_proc,(void*)screen_id);
	}	else if(screen_id == 1)	{
		OSAL_InterruptDisable(gdisp.init_para.irq[DISP_MOD_LCD1]);
		OSAL_UnRegISR(gdisp.init_para.irq[DISP_MOD_LCD1],disp_lcdc_event_proc,(void*)screen_id);
	}

	tcon_exit(screen_id);

	lcdc_clk_exit(screen_id);

	return DIS_SUCCESS;
}

__u32 tv_mode_to_width(__disp_tv_mode_t mode)
{
	__u32 width = 0;

	switch(mode) {
	case DISP_TV_MOD_480I:
	case DISP_TV_MOD_576I:
	case DISP_TV_MOD_480P:
	case DISP_TV_MOD_576P:
	case DISP_TV_MOD_PAL:
	case DISP_TV_MOD_NTSC:
	case DISP_TV_MOD_PAL_SVIDEO:
	case DISP_TV_MOD_NTSC_SVIDEO:
	case DISP_TV_MOD_PAL_M:
	case DISP_TV_MOD_PAL_M_SVIDEO:
	case DISP_TV_MOD_PAL_NC:
	case DISP_TV_MOD_PAL_NC_SVIDEO:
		width = 720;
		break;
	case DISP_TV_MOD_720P_50HZ:
	case DISP_TV_MOD_720P_60HZ:
	case DISP_TV_MOD_720P_50HZ_3D_FP:
	case DISP_TV_MOD_720P_60HZ_3D_FP:
		width = 1280;
		break;
	case DISP_TV_MOD_1080I_50HZ:
	case DISP_TV_MOD_1080I_60HZ:
	case DISP_TV_MOD_1080P_24HZ:
	case DISP_TV_MOD_1080P_50HZ:
	case DISP_TV_MOD_1080P_60HZ:
	case DISP_TV_MOD_1080P_24HZ_3D_FP:
	case DISP_TV_MOD_1080P_25HZ:
	case DISP_TV_MOD_1080P_30HZ:
		width = 1920;
		break;
	default:
		width = 0;
		break;
	}

	return width;
}


__u32 tv_mode_to_height(__disp_tv_mode_t mode)
{
	__u32 height = 0;

	switch(mode) {
	case DISP_TV_MOD_480I:
	case DISP_TV_MOD_480P:
	case DISP_TV_MOD_NTSC:
	case DISP_TV_MOD_NTSC_SVIDEO:
	case DISP_TV_MOD_PAL_M:
	case DISP_TV_MOD_PAL_M_SVIDEO:
		height = 480;
		break;
	case DISP_TV_MOD_576I:
	case DISP_TV_MOD_576P:
	case DISP_TV_MOD_PAL:
	case DISP_TV_MOD_PAL_SVIDEO:
	case DISP_TV_MOD_PAL_NC:
	case DISP_TV_MOD_PAL_NC_SVIDEO:
		height = 576;
		break;
	case DISP_TV_MOD_720P_50HZ:
	case DISP_TV_MOD_720P_60HZ:
		height = 720;
		break;
	case DISP_TV_MOD_1080I_50HZ:
	case DISP_TV_MOD_1080I_60HZ:
	case DISP_TV_MOD_1080P_24HZ:
	case DISP_TV_MOD_1080P_50HZ:
	case DISP_TV_MOD_1080P_60HZ:
	case DISP_TV_MOD_1080P_25HZ:
	case DISP_TV_MOD_1080P_30HZ:
		height = 1080;
		break;
	case DISP_TV_MOD_1080P_24HZ_3D_FP:
		height = 1080*2;
		break;
	case DISP_TV_MOD_720P_50HZ_3D_FP:
	case DISP_TV_MOD_720P_60HZ_3D_FP:
		height = 720*2;
		break;
	default:
		height = 0;
		break;
	}

	return height;
}

__u32 vga_mode_to_width(__disp_vga_mode_t mode)
{
	__u32 width = 0;

	switch(mode) {
	case DISP_VGA_H1680_V1050:
		width = 1680;
		break;
	case DISP_VGA_H1440_V900:
		width = 1440;
		break;
	case DISP_VGA_H1360_V768:
		width = 1360;
		break;
	case DISP_VGA_H1280_V1024:
		width = 1280;
		break;
	case DISP_VGA_H1024_V768:
		width = 1024;
		break;
	case DISP_VGA_H800_V600:
		width = 800;
		break;
	case DISP_VGA_H640_V480:
		width = 640;
		break;
	case DISP_VGA_H1440_V900_RB:
		width = 1440;
		break;
	case DISP_VGA_H1680_V1050_RB:
		width = 1680;
		break;
	case DISP_VGA_H1920_V1080_RB:
	case DISP_VGA_H1920_V1080:
		width = 1920;
		break;
	case DISP_VGA_H1280_V720:
		width = 1280;
		break;
	default:
		width = 0;
		break;
	}

	return width;
}


__u32 vga_mode_to_height(__disp_vga_mode_t mode)
{
	__u32 height = 0;

	switch(mode) {
	case DISP_VGA_H1680_V1050:
		height = 1050;
		break;
	case DISP_VGA_H1440_V900:
		height = 900;
		break;
	case DISP_VGA_H1360_V768:
		height = 768;
		break;
	case DISP_VGA_H1280_V1024:
		height = 1024;
		break;
	case DISP_VGA_H1024_V768:
		height = 768;
		break;
	case DISP_VGA_H800_V600:
		height = 600;
		break;
	case DISP_VGA_H640_V480:
		height = 480;
		break;
	case DISP_VGA_H1440_V900_RB:
		height = 1440;
		break;
	case DISP_VGA_H1680_V1050_RB:
		height = 1050;
		break;
	case DISP_VGA_H1920_V1080_RB:
	case DISP_VGA_H1920_V1080:
		height = 1080;
		break;
	case DISP_VGA_H1280_V720:
		height = 720;
		break;
	default:
		height = 0;
		break;
	}

	return height;
}

// return 0: progressive scan mode; return 1: interlace scan mode
__u32 disp_get_screen_scan_mode(__disp_tv_mode_t tv_mode)
{
	__u32 ret = 0;

	switch(tv_mode)	{
	case DISP_TV_MOD_480I:
	case DISP_TV_MOD_NTSC:
	case DISP_TV_MOD_NTSC_SVIDEO:
	case DISP_TV_MOD_PAL_M:
	case DISP_TV_MOD_PAL_M_SVIDEO:
	case DISP_TV_MOD_576I:
	case DISP_TV_MOD_PAL:
	case DISP_TV_MOD_PAL_SVIDEO:
	case DISP_TV_MOD_PAL_NC:
	case DISP_TV_MOD_PAL_NC_SVIDEO:
	case DISP_TV_MOD_1080I_50HZ:
	case DISP_TV_MOD_1080I_60HZ:
		ret = 1;
	default:
		break;
	}

	return ret;
}

__s32 bsp_disp_get_screen_width(__u32 screen_id)
{
	__u32 width = 0;

	if((gdisp.screen[screen_id].status & LCD_ON) || (gdisp.screen[screen_id].status & TV_ON)
	    || (gdisp.screen[screen_id].status & HDMI_ON) || (gdisp.screen[screen_id].status & VGA_ON))	{
		width = DE_BE_get_display_width(screen_id);
	}	else {
		width = gpanel_info[screen_id].lcd_x;
	}

	return width;
}

__s32 bsp_disp_get_screen_height(__u32 screen_id)
{
	__u32 height = 0;

	if((gdisp.screen[screen_id].status & LCD_ON) || (gdisp.screen[screen_id].status & TV_ON)
	    || (gdisp.screen[screen_id].status & HDMI_ON) || (gdisp.screen[screen_id].status & VGA_ON)) {
		height = DE_BE_get_display_height(screen_id);
	}	else {
		height = gpanel_info[screen_id].lcd_y;
	}

	return height;
}
//get width of screen in mm
__s32 bsp_disp_get_screen_physical_width(__u32 screen_id)
{
	__u32 width = 0;

	if((gdisp.screen[screen_id].status & TV_ON) || (gdisp.screen[screen_id].status & HDMI_ON)
	    || (gdisp.screen[screen_id].status & VGA_ON))	{
		width = 0;
	}	else {
		width = gpanel_info[screen_id].lcd_width;//width of lcd in mm
	}

	return width;
}

//get height of screen in mm
__s32 bsp_disp_get_screen_physical_height(__u32 screen_id)
{
	__u32 height = 0;

	if((gdisp.screen[screen_id].status & TV_ON) || (gdisp.screen[screen_id].status & HDMI_ON)
	    || (gdisp.screen[screen_id].status & VGA_ON)) {
		height = 0;
	}	else {
		height = gpanel_info[screen_id].lcd_height;//height of lcd in mm
	}

	return height;
}


__s32 bsp_disp_get_screen_width_from_output_type(__u32 screen_id, __disp_output_type_t output_type, __u32 mode)
{
	__s32 ret = 0;
	switch(output_type) {
	case DISP_OUTPUT_TYPE_TV:
	case DISP_OUTPUT_TYPE_HDMI:
		ret = tv_mode_to_width(mode);
		break;
	case DISP_OUTPUT_TYPE_VGA:
		ret = vga_mode_to_width(mode);
		break;
	case DISP_OUTPUT_TYPE_LCD:
	default:
		ret = gpanel_info[screen_id].lcd_x;
		break;
	}

	return ret;
}

__s32 bsp_disp_get_screen_height_from_output_type(__u32 screen_id, __disp_output_type_t output_type, __u32 mode)
{
	__s32 ret = 0;
	switch(output_type) {
	case DISP_OUTPUT_TYPE_TV:
	case DISP_OUTPUT_TYPE_HDMI:
		ret = tv_mode_to_height(mode);
		break;
	case DISP_OUTPUT_TYPE_VGA:
		ret = vga_mode_to_height(mode);
		break;
	case DISP_OUTPUT_TYPE_LCD:
	default:
		ret = gpanel_info[screen_id].lcd_y;
		break;
	}

	return ret;
}


__s32 bsp_disp_get_output_type(__u32 screen_id)
{
	if(gdisp.screen[screen_id].status & TV_ON) {
		return (__s32)DISP_OUTPUT_TYPE_TV;
	}

	if(gdisp.screen[screen_id].status & LCD_ON)	{
		return (__s32)DISP_OUTPUT_TYPE_LCD;
	}

	if(gdisp.screen[screen_id].status & HDMI_ON) {
		return (__s32)DISP_OUTPUT_TYPE_HDMI;
	}

	if(gdisp.screen[screen_id].status & VGA_ON)	{
		return (__s32)DISP_OUTPUT_TYPE_VGA;
	}

	return (__s32)DISP_OUTPUT_TYPE_NONE;
}


__s32 bsp_disp_get_frame_rate(__u32 screen_id)
{
	__s32 frame_rate = 60;

	if(gdisp.screen[screen_id].output_type & DISP_OUTPUT_TYPE_LCD) {
		frame_rate = (gpanel_info[screen_id].lcd_dclk_freq * 1000000) / (gpanel_info[screen_id].lcd_ht * (gpanel_info[screen_id].lcd_vt)) ;
	}	else if(gdisp.screen[screen_id].output_type & DISP_OUTPUT_TYPE_TV) {
		switch(gdisp.screen[screen_id].tv_mode)	{
		case DISP_TV_MOD_480I:
		case DISP_TV_MOD_480P:
		case DISP_TV_MOD_NTSC:
		case DISP_TV_MOD_NTSC_SVIDEO:
		case DISP_TV_MOD_PAL_M:
		case DISP_TV_MOD_PAL_M_SVIDEO:
		case DISP_TV_MOD_720P_60HZ:
		case DISP_TV_MOD_1080I_60HZ:
		case DISP_TV_MOD_1080P_60HZ:
			frame_rate = 60;
			break;
		case DISP_TV_MOD_576I:
		case DISP_TV_MOD_576P:
		case DISP_TV_MOD_PAL:
		case DISP_TV_MOD_PAL_SVIDEO:
		case DISP_TV_MOD_PAL_NC:
		case DISP_TV_MOD_PAL_NC_SVIDEO:
		case DISP_TV_MOD_720P_50HZ:
		case DISP_TV_MOD_1080I_50HZ:
		case DISP_TV_MOD_1080P_50HZ:
			frame_rate = 50;
			break;
		default:
			break;
		}
	}	else if(gdisp.screen[screen_id].output_type & DISP_OUTPUT_TYPE_HDMI) {
		switch(gdisp.screen[screen_id].hdmi_mode)	{
		case DISP_TV_MOD_480I:
		case DISP_TV_MOD_480P:
		case DISP_TV_MOD_720P_60HZ:
		case DISP_TV_MOD_1080I_60HZ:
		case DISP_TV_MOD_1080P_60HZ:
		case DISP_TV_MOD_720P_60HZ_3D_FP:
			frame_rate = 60;
			break;
		case DISP_TV_MOD_576I:
		case DISP_TV_MOD_576P:
		case DISP_TV_MOD_720P_50HZ:
		case DISP_TV_MOD_1080I_50HZ:
		case DISP_TV_MOD_1080P_50HZ:
		case DISP_TV_MOD_720P_50HZ_3D_FP:
			frame_rate = 50;
			break;
		case DISP_TV_MOD_1080P_24HZ:
		case DISP_TV_MOD_1080P_24HZ_3D_FP:
			frame_rate = 24;
			break;
		case DISP_TV_MOD_1080P_25HZ:
			frame_rate = 25;
			break;
		case DISP_TV_MOD_1080P_30HZ:
			frame_rate = 30;
			break;
		default:
			break;
		}
	}	else if(gdisp.screen[screen_id].output_type & DISP_OUTPUT_TYPE_VGA)	{
		frame_rate = 60;
	}

	return frame_rate;
}

__s32 bsp_disp_lcd_open_before(__u32 screen_id)
{
	/* todo open all clk at once? */
	lcdc_clk_on(screen_id, 0, 0);
	lcdc_clk_on(screen_id, 0, 1);
	disp_clk_cfg(screen_id, DISP_OUTPUT_TYPE_LCD, DIS_NULL);
	//lcdc_clk_on(screen_id, 0, 1);
	#if (!defined CONFIG_ARCH_SUN7I) && (!defined CONFIG_ARCH_SUN5I)
	drc_clk_open(screen_id,0);
	drc_clk_open(screen_id,1);
	#endif
	tcon_init(screen_id);
	if(gpanel_info[screen_id].lcd_if == LCD_IF_LVDS) {
		lvds_clk_on();
	}	else if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
		dsi_clk_on();
	}
	image_clk_on(screen_id, 1);
	Image_open(screen_id);//set image normal channel start bit , because every de_clk_off( )will reset this bit
	if(gpanel_info[screen_id].tcon_index == 0) {
		tcon0_cfg(screen_id,(__panel_para_t*)&gpanel_info[screen_id]);
	}	else {
	//tcon1_cfg_ex(screen_id,(__panel_para_t*)&gpanel_info[screen_id]);
	}
	if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI)	{
		dsi_cfg(screen_id, (__panel_para_t*)&gpanel_info[screen_id]);
	}
	gdisp.screen[screen_id].output_csc_type = DISP_OUT_CSC_TYPE_LCD;
	bsp_disp_set_output_csc(screen_id, gdisp.screen[screen_id].output_csc_type, bsp_disp_drc_get_input_csc(screen_id));
	DE_BE_set_display_size(screen_id, gpanel_info[screen_id].lcd_x, gpanel_info[screen_id].lcd_y);
	DE_BE_Output_Select(screen_id, screen_id);
	bsp_disp_lcd_set_src(screen_id, DISP_LCDC_SRC_DE_CH1);
	
	#if  (!defined CONFIG_ARCH_SUN7I) && (!defined CONFIG_ARCH_SUN5I)
	if(bsp_disp_cmu_get_enable(screen_id) ==1) {
		IEP_CMU_Set_Imgsize(screen_id, bsp_disp_get_screen_width(screen_id), bsp_disp_get_screen_height(screen_id));
	}
	#endif

	open_flow[screen_id].func_num = 0;
	if(lcd_panel_fun[screen_id].cfg_open_flow)	{
		lcd_panel_fun[screen_id].cfg_open_flow(screen_id);
	}	else {
		DE_WRN("lcd_panel_fun[%d].cfg_open_flow is NULL\n", screen_id);
	}

	return DIS_SUCCESS;
}

__s32 bsp_disp_lcd_open_after(__u32 screen_id)
{
	//esMEM_SwitchDramWorkMode(DRAM_WORK_MODE_LCD);
	gdisp.screen[screen_id].b_out_interlace = 0;
	gdisp.screen[screen_id].status |= LCD_ON;
	gdisp.screen[screen_id].output_type = DISP_OUTPUT_TYPE_LCD;
	Lcd_Panel_Parameter_Check(screen_id);
	//bsp_disp_drc_enable(screen_id, TRUE);
	#if (!defined CONFIG_ARCH_SUN7I) && (!defined CONFIG_ARCH_SUN5I)
	drc_enable(screen_id, 1);
	#endif
#ifdef __LINUX_OSAL__
	Display_set_fb_timming(screen_id);
#endif
	return DIS_SUCCESS;
}

__lcd_flow_t * bsp_disp_lcd_get_open_flow(__u32 screen_id)
{
	return (&open_flow[screen_id]);
}

__s32 bsp_disp_lcd_close_befor(__u32 screen_id)
{
	close_flow[screen_id].func_num = 0;
	if(lcd_panel_fun[screen_id].cfg_close_flow)	{
		lcd_panel_fun[screen_id].cfg_close_flow(screen_id);
	}	else {
		DE_WRN("lcd_panel_fun[%d].cfg_close_flow is NULL\n", screen_id);
	}
	//bsp_disp_drc_enable(screen_id, FALSE);
	drc_enable(screen_id, 2);

	gdisp.screen[screen_id].status &= LCD_OFF;
	gdisp.screen[screen_id].output_type = DISP_OUTPUT_TYPE_NONE;
	return DIS_SUCCESS;
}

__s32 bsp_disp_lcd_close_after(__u32 screen_id)
{
	Image_close(screen_id);

	image_clk_off(screen_id, 1);
	lcdc_clk_off(screen_id);
	if(gpanel_info[screen_id].lcd_if == LCD_IF_LVDS) {
		lvds_clk_off();
	}	else if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI) {
		dsi_clk_off();
	}
	drc_clk_close(screen_id,0);
	drc_clk_close(screen_id,1);

	gdisp.screen[screen_id].pll_use_status &= ((gdisp.screen[screen_id].pll_use_status == VIDEO_PLL0_USED)? VIDEO_PLL0_USED_MASK : VIDEO_PLL1_USED_MASK);

	return DIS_SUCCESS;
}

__lcd_flow_t * bsp_disp_lcd_get_close_flow(__u32 screen_id)
{
	return (&close_flow[screen_id]);
}

__s32 __disp_lcd_bright_get_adjust_value(__u32 screen_id, __u32 bright)
{
	bright = (bright > 255)? 255:bright;
	return gpanel_info[screen_id].lcd_extend_para.lcd_bright_curve_tbl[bright];

	return bright;
}

__s32 __disp_lcd_bright_curve_init(__u32 screen_id)
{
	__u32 i = 0, j=0;
	__u32 items = 0;
	__u32 lcd_bright_curve_tbl[101][2];

	for(i = 0; i < 101; i++) {
		if(gdisp.screen[screen_id].lcd_cfg.backlight_curve_adjust[i] == 0) {
			if(i == 0) {
				lcd_bright_curve_tbl[items][0] = 0;
				lcd_bright_curve_tbl[items][1] = 0;
				items++;
			}
		}	else {
			lcd_bright_curve_tbl[items][0] = 255 * i / 100;
			lcd_bright_curve_tbl[items][1] = gdisp.screen[screen_id].lcd_cfg.backlight_curve_adjust[i];
			items++;
		}
	}

	for(i=0; i<items-1; i++) {
		__u32 num = lcd_bright_curve_tbl[i+1][0] - lcd_bright_curve_tbl[i][0];

		for(j=0; j<num; j++) {
			__u32 value = 0;

			value = lcd_bright_curve_tbl[i][1] + ((lcd_bright_curve_tbl[i+1][1] - lcd_bright_curve_tbl[i][1]) * j)/num;
			gpanel_info[screen_id].lcd_extend_para.lcd_bright_curve_tbl[lcd_bright_curve_tbl[i][0] + j] = value;
		}
	}
	gpanel_info[screen_id].lcd_extend_para.lcd_bright_curve_tbl[255] = lcd_bright_curve_tbl[items-1][1];

	return 0;
}

//setting:  0,       1,      2,....  14,   15
//pol==0:  0,       2,      3,....  15,   16
//pol==1: 16,    14,    13, ...   1,   0
__s32 bsp_disp_lcd_set_bright(__u32 screen_id, __u32  bright, __u32 from_iep)
{
	__u32 duty_ns;
	__u64 backlight_bright;
	__u64 backlight_dimming;
	__u64 period_ns;

	if(!bsp_disp_lcd_used(screen_id)) {
		return DIS_NOT_SUPPORT;
	}

	bright = (bright > 255)? 255:bright;

	if(!from_iep)	{
		gdisp.screen[screen_id].lcd_cfg.backlight_bright = bright;
	}

	if((gdisp.screen[screen_id].lcd_cfg.lcd_pwm_used==1) && (gdisp.screen[screen_id].lcd_cfg.lcd_used)) {
		if(bright != 0)	{
			bright += 1;
		}

		bright = __disp_lcd_bright_get_adjust_value(screen_id, bright);

		backlight_bright = bright;
		backlight_dimming = gdisp.screen[screen_id].lcd_cfg.backlight_dimming;
		period_ns = gdisp.pwm[gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch].period_ns;
		duty_ns = (backlight_bright * backlight_dimming *  period_ns/256 + 128) / 256;
#if 0
		DE_DBG("[PWM]bright=%d,backlight_dimming=%d, period_ns=%d, duty_ns=%d\n",
		    bright,gdisp.screen[screen_id].lcd_cfg.backlight_dimming,  gdisp.pwm[gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch].period_ns,duty_ns);
#endif

#ifdef CONFIG_SUN6I_RESERVE
		pwm_set_duty_ns(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch, duty_ns);
#else
                    sunxi_pwm_config(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch, duty_ns, period_ns);

#endif
	}

	return DIS_SUCCESS;
}

__s32 bsp_disp_lcd_get_bright(__u32 screen_id)
{
	return gdisp.screen[screen_id].lcd_cfg.backlight_bright;
}

__s32 bsp_disp_set_gamma_table(__u32 screen_id, __u32 *gamtbl_addr,__u32 gamtbl_size)
{
	if((gamtbl_addr == NULL) || (gamtbl_size>1024))	{
		DE_WRN("para invalid in bsp_disp_set_gamma_table\n");
		return DIS_FAIL;
	}

	if(bsp_disp_lcd_used(screen_id)) {
		tcon_gamma(screen_id,1,gamtbl_addr);
		memcpy(gpanel_info[screen_id].lcd_extend_para.lcd_gamma_tbl, gamtbl_addr, gamtbl_size);
	}

	return DIS_SUCCESS;
}

__s32 bsp_disp_gamma_correction_enable(__u32 screen_id)
{
	//	TCON1_set_gamma_Enable(screen_id,TRUE);
	if(bsp_disp_lcd_used(screen_id)) {
		gpanel_info[screen_id].lcd_gamma_en = TRUE;
	}

	return DIS_SUCCESS;
}

__s32 bsp_disp_gamma_correction_disable(__u32 screen_id)
{
	if(bsp_disp_lcd_used(screen_id)) {
		tcon_gamma(screen_id, 0, 0);
		gpanel_info[screen_id].lcd_gamma_en = FALSE;
	}

	return DIS_SUCCESS;
}

__s32 bsp_disp_lcd_set_src(__u32 screen_id, __disp_lcdc_src_t src)
{
	switch (src) {
	case DISP_LCDC_SRC_DE_CH1:
		tcon0_src_select(screen_id, LCD_SRC_BE0);
		break;

	case DISP_LCDC_SRC_DE_CH2:
		tcon0_src_select(screen_id, LCD_SRC_BE1);
		break;

	case DISP_LCDC_SRC_DMA888:
		tcon0_src_select(screen_id, LCD_SRC_DMA888);
		break;

	case DISP_LCDC_SRC_DMA565:
		tcon0_src_select(screen_id, LCD_SRC_DMA565);
		break;

	case DISP_LCDC_SRC_WHITE:
		tcon0_src_select(screen_id, LCD_SRC_WHITE);
		break;

	case DISP_LCDC_SRC_BLACK:
		tcon0_src_select(screen_id, LCD_SRC_BLACK);
		break;

	default:
		DE_WRN("not supported lcdc src:%d in bsp_disp_lcd_set_src\n", src);
		return DIS_NOT_SUPPORT;
	}
	return DIS_SUCCESS;
}

__s32 bsp_disp_lcd_get_src(__u32 screen_id)
{
	return tcon0_src_get(screen_id);
}


__s32 bsp_disp_lcd_user_defined_func(__u32 screen_id, __u32 para1, __u32 para2, __u32 para3)
{
	return lcd_panel_fun[screen_id].lcd_user_defined_func(screen_id, para1, para2, para3);
}

__s32 bsp_disp_lcd_set_panel_funs(char *drv_name, __lcd_panel_fun_t * lcd_cfg)
{
	__u32 screen_id;
	char name[32];
	__u32 num_screens;

	num_screens = bsp_disp_feat_get_num_screens();
	for(screen_id=0; screen_id<num_screens; screen_id++) {
		memset(name, 0, 32);
		bsp_disp_lcd_get_driver_name(screen_id, name);
		if(!strcmp(drv_name, name))	{
			memset(&lcd_panel_fun[screen_id], 0, sizeof(__lcd_panel_fun_t));
			lcd_panel_fun[screen_id].cfg_panel_info= lcd_cfg->cfg_panel_info;
			lcd_panel_fun[screen_id].cfg_open_flow = lcd_cfg->cfg_open_flow;
			lcd_panel_fun[screen_id].cfg_close_flow= lcd_cfg->cfg_close_flow;
			lcd_panel_fun[screen_id].lcd_user_defined_func = lcd_cfg->lcd_user_defined_func;
			gdisp.lcd_registered = 1;
			if(gdisp.init_para.start_process) {
				gdisp.init_para.start_process();
			}

			return 0;
		}
	}
	return -1;
}

__s32 bsp_disp_get_timming(__u32 screen_id, __disp_tcon_timing_t * tt)
{
	memset(tt, 0, sizeof(__disp_tcon_timing_t));

	if(gdisp.screen[screen_id].status & LCD_ON)	{
		tcon_get_timing(screen_id, 0, tt);
		tt->pixel_clk = gpanel_info[screen_id].lcd_dclk_freq * 1000;
		if(gpanel_info[screen_id].lcd_if == LCD_IF_DSI)	{
			tt->hor_pixels = gpanel_info[screen_id].lcd_x;
			tt->ver_pixels = gpanel_info[screen_id].lcd_y;
			tt->hor_total_time= gpanel_info[screen_id].lcd_ht;
			tt->hor_sync_time= gpanel_info[screen_id].lcd_hspw;
			tt->hor_back_porch= gpanel_info[screen_id].lcd_hbp-gpanel_info[screen_id].lcd_hspw;
			tt->hor_front_porch= gpanel_info[screen_id].lcd_ht-gpanel_info[screen_id].lcd_hbp - gpanel_info[screen_id].lcd_x;
			tt->ver_total_time= gpanel_info[screen_id].lcd_vt;
			tt->ver_sync_time= gpanel_info[screen_id].lcd_vspw;
			tt->ver_back_porch= gpanel_info[screen_id].lcd_vbp-gpanel_info[screen_id].lcd_vspw;
			tt->ver_front_porch= gpanel_info[screen_id].lcd_vt-gpanel_info[screen_id].lcd_vbp -gpanel_info[screen_id].lcd_y;
		}
	}	else if((gdisp.screen[screen_id].status & TV_ON )|| (gdisp.screen[screen_id].status & HDMI_ON))	{
		__disp_tv_mode_t mode = gdisp.screen[screen_id].tv_mode;;

		tcon_get_timing(screen_id, 1, tt);
		tt->pixel_clk = (clk_tab.tv_clk_tab[mode].tve_clk / clk_tab.tv_clk_tab[mode].pre_scale) / 1000;
	}	else if(gdisp.screen[screen_id].status & VGA_ON )	{
		__disp_tv_mode_t mode = gdisp.screen[screen_id].vga_mode;

		tcon_get_timing(screen_id, 1, tt);
		tt->pixel_clk = (clk_tab.tv_clk_tab[mode].tve_clk / clk_tab.vga_clk_tab[mode].pre_scale) / 1000;
	}	else {
		DE_INF("get timming fail because device is not output !\n");
		return -1;
	}

	return 0;
}
/*
__u32 bsp_disp_get_cur_line(__u32 screen_id)
{
    __u32 line = 0;

    if(gdisp.screen[screen_id].status & LCD_ON)
    {
        line = LCDC_get_cur_line(screen_id, 0);
    }
    else if((gdisp.screen[screen_id].status & TV_ON )|| (gdisp.screen[screen_id].status & HDMI_ON) || (gdisp.screen[screen_id].status & VGA_ON ))
    {
        line = LCDC_get_cur_line(screen_id, 1);
    }

    return line;
}
*/
__s32 bsp_disp_close_lcd_backlight(__u32 screen_id)
{
	disp_gpio_set_t  gpio_info[1];
	__hdle hdl;
	int ret;
	char primary_key[20];
	__u32 lcd_used = 0;
	int value;

	sprintf(primary_key, "lcd%d_para", screen_id);

	ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_used", &value, 1);
	if(ret != 0) {
		lcd_used = value;
	}

	if(lcd_used == 0)	{
		return 0;
	}

	ret = OSAL_Script_FetchParser_Data(primary_key,"lcd_bl_en", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
	if(ret == 0) {
		gpio_info->data = (gpio_info->data==0)?1:0;
		hdl = OSAL_GPIO_Request(gpio_info, 1);
		OSAL_GPIO_Release(hdl, 2);
	}

	ret = OSAL_Script_FetchParser_Data(primary_key,"lcd_pwm", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
	if(ret == 0) {
		gpio_info->mul_sel = 0;
		hdl = OSAL_GPIO_Request(gpio_info, 1);
		OSAL_GPIO_Release(hdl, 2);
	}

	return 0;
}

__s32 bsp_disp_open_lcd_backlight(__u32 screen_id)
{
	disp_gpio_set_t  gpio_info[1];
	__hdle hdl;
	int ret;
	char primary_key[20];
	__u32 lcd_used = 0;
	int value;

	sprintf(primary_key, "lcd%d_para", screen_id);

	ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_used", &value, 1);
	if(ret != 0) {
		lcd_used = value;
	}

	if(lcd_used == 0)	{
		return 0;
	}

	ret = OSAL_Script_FetchParser_Data(primary_key,"lcd_bl_en", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
	if(ret == 0) {
		hdl = OSAL_GPIO_Request(gpio_info, 1);
		OSAL_GPIO_Release(hdl, 2);
	}

	ret = OSAL_Script_FetchParser_Data(primary_key,"lcd_pwm", (int *)gpio_info, sizeof(disp_gpio_set_t)/sizeof(int));
	if(ret == 0)	{
		hdl = OSAL_GPIO_Request(gpio_info, 1);
		OSAL_GPIO_Release(hdl, 2);
	}

	return 0;
}

__s32 bsp_disp_lcd_set_bright_dimming(__u32 screen_id, __u32 backlight_dimming)
{
	gdisp.screen[screen_id].lcd_cfg.backlight_dimming = backlight_dimming;

	return DIS_SUCCESS;
}

__s32 bsp_disp_lcd_used(__u32 screen_id)
{
	return gdisp.screen[screen_id].lcd_cfg.lcd_used;
}

__s32 bsp_disp_lcd_get_panel_para(__u32 screen_id, __panel_para_t * info)
{
	memcpy(info, &gpanel_info[screen_id], sizeof(__panel_para_t));

	return DIS_SUCCESS;
}


__s32 LCD_CPU_WR(__u32 screen_id,__u32 index,__u32 data)
{
	tcon0_cpu_wr_16b(screen_id,index,data);
	return 0;
}

__s32 LCD_CPU_WR_INDEX(__u32 screen_id,__u32 index)
{
	tcon0_cpu_wr_16b_index(screen_id,index);
	return 0;
}

__s32 LCD_CPU_WR_DATA(__u32 screen_id,__u32 data)
{
	tcon0_cpu_wr_16b_data(screen_id,data);
	return 0;
}

#ifdef CONFIG_SUN6I_RESERVE
__s32 bsp_disp_restore_lcdc_reg(__u32 screen_id)
{
	tcon_init(screen_id);

	if(bsp_disp_lcd_used(screen_id)) {
		__pwm_info_t pwm_info;

		if(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_used) {
			pwm_get_para(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch, &pwm_info);

			pwm_info.enable = 0;
			    pwm_set_para(gdisp.screen[screen_id].lcd_cfg.lcd_pwm_ch, &pwm_info);
		}
	}

	return 0;
}

#endif
//fps: 1~75
__s32 bsp_disp_lcd_set_fps(__u32 screen_id, __u32 fps)
{
	if(bsp_disp_lcd_used(screen_id)) {
		fps = (fps>75)? 75:fps;
		fps = (fps == 0)? 60:fps;
		gdisp.screen[screen_id].lcd_fps_cfg = fps;
		gdisp.screen[screen_id].lcd_fps_cfg_request = 1;
	}

	return 0;
}

__s32 disp_lcd_set_fps(__u32 screen_id)
{
	__u32 lcd_dclk_freq_cfg;

	if((bsp_disp_get_output_type(screen_id) == DISP_OUTPUT_TYPE_LCD) && (gdisp.screen[screen_id].lcd_fps_cfg_request == 1))	{
		lcd_dclk_freq_cfg = gdisp.screen[screen_id].lcd_fps_cfg * gpanel_info[screen_id].lcd_ht * gpanel_info[screen_id].lcd_vt / 1000000;

		if(lcd_dclk_freq_cfg != gpanel_info[screen_id].lcd_dclk_freq)	{
			gpanel_info[screen_id].lcd_dclk_freq = (lcd_dclk_freq_cfg>gpanel_info[screen_id].lcd_dclk_freq)? (gpanel_info[screen_id].lcd_dclk_freq+1):(gpanel_info[screen_id].lcd_dclk_freq-1);
			if(gpanel_info[screen_id].lcd_dclk_freq < 18)	{
				DE_WRN("__disp_lcd_set_fps, dclk freq(%d Mhz) is too small\n", gpanel_info[screen_id].lcd_dclk_freq);
				gpanel_info[screen_id].lcd_dclk_freq = 18;
			}
			disp_clk_cfg(screen_id, DISP_OUTPUT_TYPE_LCD, DIS_NULL);
		}
	}
	return 0;
}

__s32 bsp_disp_lcd_get_size(__u32 screen_id, char *lcd_size)
{
	if(bsp_disp_lcd_used(screen_id)) {
		memcpy((void*)lcd_size, (void*)gpanel_info[screen_id].lcd_size, 8);
	}

	return 0;
}

__s32 bsp_disp_lcd_get_model_name(__u32 screen_id, char *model_name)
{
	if(bsp_disp_lcd_used(screen_id)) {
		memcpy((void*)model_name, (void*)gpanel_info[screen_id].lcd_model_name, 32);
	}

	return 0;
}

__s32 bsp_disp_lcd_get_driver_name(__u32 screen_id, char* name)
{
	char primary_key[20];
	__s32 ret;

	sprintf(primary_key, "lcd%d_para", screen_id);

	ret = OSAL_Script_FetchParser_Data(primary_key, "lcd_driver_name",  (int*)name, 16);
	return ret;
}

__s32 bsp_disp_lcd_dsi_io_enable(__u32 screen_id)
{
	return dsi_io_open(screen_id, &gpanel_info[screen_id]);
}

__s32 disp_lcd_get_support(__u32 screen_id)
{
	return (bsp_disp_feat_get_supported_output_types(screen_id) & DISP_OUTPUT_TYPE_LCD);
}

__s32 bsp_disp_lcd_get_registered()
{
	return gdisp.lcd_registered;
}

#ifdef __LINUX_OSAL__
EXPORT_SYMBOL(LCD_OPEN_FUNC);
EXPORT_SYMBOL(LCD_CLOSE_FUNC);
EXPORT_SYMBOL(LCD_get_reg_bases);
EXPORT_SYMBOL(pwm_set_para);
EXPORT_SYMBOL(pwm_get_para);
EXPORT_SYMBOL(pwm_set_duty_ns);
EXPORT_SYMBOL(pwm_enable);
EXPORT_SYMBOL(pwm_disable);
EXPORT_SYMBOL(lcd_get_panel_para);
#endif

