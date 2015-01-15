#include "dev_disp.h"
#include <asm/arch/timer.h>


fb_info_t g_fbi;
__disp_drv_t g_disp_drv;

#define __user
#define HZ 100
#define EFAULT 1

static __u32    lcd_flow_cnt[2] = {0};
static __bool   lcd_op_finished[2] = {0};
struct timer_list lcd_timer[2];
static __bool   lcd_op_start[2] = {0};
static unsigned int gbuffer[4096];
static __u32 suspend_output_type[2] = {0,0};
static __u32 suspend_status = 0;//0:normal; suspend_status&1 != 0:in early_suspend; suspend_status&2 != 0:in suspend;

static uint display_opens;

__u32 output_type = DISP_OUTPUT_TYPE_LCD;

void DRV_lcd_close_callback(void *parg);

__s32 copy_from_user(void *dest, void* src, __u32 size)
{
    memcpy(dest, src, size);
	return 0;
}

__s32 copy_to_user(void *dest, void* src, __u32 size)
{
    memcpy(dest, src, size);
	return 0;
}


__s32 disp_delay_ms(__u32 ms)
{
    /* todo */
    udelay(ms*1000);
    return 0;
}
// [before][step_0][delay_0][step_1][delay_1]......[step_n-2][delay_n-2][step_n-1][delay_n-1][after]
void DRV_lcd_open_callback(void *parg)
{
    __lcd_flow_t *flow;
    __u32 sel = (__u32)parg;
    __s32 i = lcd_flow_cnt[sel]++;

    flow = bsp_disp_lcd_get_open_flow(sel);

	if(i < flow->func_num)
    {
    	flow->func[i].func(sel);
        if(flow->func[i].delay == 0)
        {
        	DRV_lcd_open_callback((void*)sel);
        }
        else
        {
        	lcd_timer[sel].data = sel;
			lcd_timer[sel].expires = flow->func[i].delay;
			lcd_timer[sel].function = DRV_lcd_open_callback;
			add_timer(&lcd_timer[sel]);
    	}
    }
    else if(i == flow->func_num)
    {
    	bsp_disp_lcd_open_after(sel);
        lcd_op_finished[sel] = 1;
    }
}


__s32 DRV_lcd_open(__u32 sel)
{
	if(bsp_disp_lcd_used(sel))
    {
        lcd_flow_cnt[sel] = 0;
        lcd_op_finished[sel] = 0;
        lcd_op_start[sel] = 1;

        init_timer(&lcd_timer[sel]);

        bsp_disp_lcd_open_before(sel);
        DRV_lcd_open_callback((void*)sel);
    }
    return 0;
}

__bool DRV_lcd_check_open_finished(__u32 sel)
{
	if(bsp_disp_lcd_used(sel) && (lcd_op_start[sel] == 1))
	{
	    if(lcd_op_finished[sel])
	    {
	        del_timer(&lcd_timer[sel]);
            lcd_op_start[sel] = 0;
	    }
		return lcd_op_finished[sel];
	}

	return 1;
}

__s32 DRV_lcd_close(__u32 sel)
{
    if(bsp_disp_lcd_used(sel))
    {
        lcd_flow_cnt[sel] = 0;
        lcd_op_finished[sel] = 0;
        lcd_op_start[sel] = 1;

        init_timer(&lcd_timer[sel]);

        bsp_disp_lcd_close_befor(sel);
        DRV_lcd_close_callback((void*)sel);
    }

    return 0;
}


void DRV_lcd_close_callback(void *parg)
{
    __lcd_flow_t *flow;
    __u32 sel = (__u32)parg;
    __s32 i = lcd_flow_cnt[sel]++;

    flow = bsp_disp_lcd_get_close_flow(sel);

    if(i < flow->func_num)
    {
    	flow->func[i].func(sel);
        if(flow->func[i].delay == 0)
        {
            DRV_lcd_close_callback((void*)sel);
        }
        else
        {
            lcd_timer[sel].data = sel;
			lcd_timer[sel].expires = flow->func[i].delay;
			lcd_timer[sel].function = DRV_lcd_close_callback;
			add_timer(&lcd_timer[sel]);
        }
    }
    else if(i == flow->func_num)
    {
        bsp_disp_lcd_close_after(sel);
        lcd_op_finished[sel] = 1;
    }
}


__bool DRV_lcd_check_close_finished(__u32 sel)
{
    if(bsp_disp_lcd_used(sel) && (lcd_op_start[sel] == 1))
    {
        if(lcd_op_finished[sel])
        {
            del_timer(&lcd_timer[sel]);
            lcd_op_start[sel] = 0;
        }
        return lcd_op_finished[sel];
    }

    return 1;
}

__s32 DRV_hdmi_open(void)
{
    Hdmi_hal_video_enable_sync(1);
    return 0;
}

__s32 DRV_hdmi_close(void)
{
    Hdmi_hal_video_enable_sync(0);

    return 0;
}

__s32 DRV_hdmi_set_display_mode(__disp_tv_mode_t mode)
{
    __u32 hdmi_mode;

    switch(mode) {
    case DISP_TV_MOD_480I:
	hdmi_mode = HDMI1440_480I;
	break;

    case DISP_TV_MOD_576I:
	hdmi_mode = HDMI1440_576I;
	break;

    case DISP_TV_MOD_480P:
	hdmi_mode = HDMI480P;
	break;

    case DISP_TV_MOD_576P:
	hdmi_mode = HDMI576P;
	break;

    case DISP_TV_MOD_720P_50HZ:
	hdmi_mode = HDMI720P_50;
	break;

    case DISP_TV_MOD_720P_60HZ:
	hdmi_mode = HDMI720P_60;
	break;

    case DISP_TV_MOD_1080I_50HZ:
	hdmi_mode = HDMI1080I_50;
	break;

    case DISP_TV_MOD_1080I_60HZ:
	hdmi_mode = HDMI1080I_60;
	break;

    case DISP_TV_MOD_1080P_24HZ:
	hdmi_mode = HDMI1080P_24;
	break;

    case DISP_TV_MOD_1080P_50HZ:
	hdmi_mode = HDMI1080P_50;
	break;

    case DISP_TV_MOD_1080P_60HZ:
	hdmi_mode = HDMI1080P_60;
	break;

    case DISP_TV_MOD_1080P_24HZ_3D_FP:
	hdmi_mode = HDMI1080P_24_3D_FP;
	break;

    case DISP_TV_MOD_720P_50HZ_3D_FP:
        hdmi_mode = HDMI720P_50_3D_FP;
        break;

    case DISP_TV_MOD_720P_60HZ_3D_FP:
        hdmi_mode = HDMI720P_60_3D_FP;
        break;

    default:
        __wrn("unsupported video mode %d when set display mode\n", mode);
        return -1;
        }

    return Hdmi_hal_set_display_mode(hdmi_mode);
}

__s32 DRV_hdmi_mode_support(__disp_tv_mode_t mode)
{
    __u32 hdmi_mode;

    switch(mode) {
    case DISP_TV_MOD_480I:
	hdmi_mode = HDMI1440_480I;
	break;

    case DISP_TV_MOD_576I:
	hdmi_mode = HDMI1440_576I;
	break;

    case DISP_TV_MOD_480P:
	hdmi_mode = HDMI480P;
	break;

    case DISP_TV_MOD_576P:
	hdmi_mode = HDMI576P;
	break;

    case DISP_TV_MOD_720P_50HZ:
	hdmi_mode = HDMI720P_50;
	break;

    case DISP_TV_MOD_720P_60HZ:
	hdmi_mode = HDMI720P_60;
	break;

    case DISP_TV_MOD_1080I_50HZ:
	hdmi_mode = HDMI1080I_50;
	break;

    case DISP_TV_MOD_1080I_60HZ:
	hdmi_mode = HDMI1080I_60;
	break;

    case DISP_TV_MOD_1080P_24HZ:
	hdmi_mode = HDMI1080P_24;
	break;

    case DISP_TV_MOD_1080P_50HZ:
	hdmi_mode = HDMI1080P_50;
	break;

    case DISP_TV_MOD_1080P_60HZ:
	hdmi_mode = HDMI1080P_60;
	break;

    case DISP_TV_MOD_1080P_24HZ_3D_FP:
        hdmi_mode = HDMI1080P_24_3D_FP;
        break;

    case DISP_TV_MOD_720P_50HZ_3D_FP:
        hdmi_mode = HDMI720P_50_3D_FP;
        break;

    case DISP_TV_MOD_720P_60HZ_3D_FP:
        hdmi_mode = HDMI720P_60_3D_FP;
        break;

    default:
	hdmi_mode = HDMI720P_50;
	break;
        }

    return Hdmi_hal_mode_support(hdmi_mode);
}

__s32 DRV_hdmi_get_HPD_status(void)
{
    return Hdmi_hal_get_HPD();
}

__s32 DRV_hdmi_set_pll(__u32 pll, __u32 clk)
{
    Hdmi_hal_set_pll(pll, clk);
    return 0;
}

__s32 disp_int_process(__u32 sel)
{
    /* to do */
    return 0;
}

int sunxi_disp_get_source_ops(struct sunxi_disp_source_ops *src_ops)
{
	src_ops->sunxi_lcd_delay_ms = bsp_disp_lcd_delay_ms;
	src_ops->sunxi_lcd_delay_us = bsp_disp_lcd_delay_us;
	src_ops->sunxi_lcd_tcon_enable = bsp_disp_lcd_tcon_open;
	src_ops->sunxi_lcd_tcon_disable = bsp_disp_lcd_tcon_close;
	src_ops->sunxi_lcd_pwm_enable = bsp_disp_lcd_pwm_enable;
	src_ops->sunxi_lcd_pwm_disable = bsp_disp_lcd_pwm_disable;
	src_ops->sunxi_lcd_backlight_enable = bsp_disp_lcd_backlight_enable;
	src_ops->sunxi_lcd_backlight_disable = bsp_disp_lcd_backlight_disable;
	src_ops->sunxi_lcd_power_enable = bsp_disp_lcd_power_enable;
	src_ops->sunxi_lcd_power_disable = bsp_disp_lcd_power_disable;
	src_ops->sunxi_lcd_pin_cfg = bsp_disp_lcd_pin_cfg;
	src_ops->sunxi_lcd_dsi_clk_enable = dsi_clk_enable;
	src_ops->sunxi_lcd_get_driver_name = bsp_disp_lcd_get_driver_name;
	src_ops->sunxi_lcd_set_panel_funs = bsp_disp_lcd_set_panel_funs;
	src_ops->sunxi_lcd_get_panel_para = bsp_disp_lcd_get_panel_para;
	src_ops->sunxi_lcd_iic_read = bsp_disp_lcd_iic_read;
	src_ops->sunxi_lcd_iic_write = bsp_disp_lcd_iic_write;

	return 0;
}

extern int lcd_init(void);
__s32 drv_disp_init(void)
{
    __disp_bsp_init_para para;

    sunxi_pwm_init();

    debug("====display init =====\n");

    memset(&para, 0, sizeof(__disp_bsp_init_para));

#if (!defined (CONFIG_ARCH_SUN7I)) && (!defined (CONFIG_ARCH_SUN5I))
    para.reg_base[DISP_MOD_BE0]    = SUNXI_DE_BE0_BASE;
    para.reg_size[DISP_MOD_BE0]    = 0x9fc;
    para.reg_base[DISP_MOD_BE1]    = SUNXI_DE_BE1_BASE;
    para.reg_size[DISP_MOD_BE1]    = 0x9fc;
    para.reg_base[DISP_MOD_FE0]    = SUNXI_DE_FE0_BASE;
    para.reg_size[DISP_MOD_FE0]    = 0x22c;
    para.reg_base[DISP_MOD_FE1]    = SUNXI_DE_FE1_BASE;
    para.reg_size[DISP_MOD_FE1]    = 0x22c;
    para.reg_base[DISP_MOD_LCD0]   = SUNXI_LCD0_BASE;
    para.reg_size[DISP_MOD_LCD0]   = 0x3fc;
    para.reg_base[DISP_MOD_LCD1]   = SUNXI_LCD1_BASE;
    para.reg_size[DISP_MOD_LCD1]   = 0x3fc;
    para.reg_base[DISP_MOD_CCMU]   = SUNXI_CCM_BASE;
    para.reg_size[DISP_MOD_CCMU]   = 0x2dc;
    para.reg_base[DISP_MOD_PIOC]   = SUNXI_PIO_BASE;
    para.reg_size[DISP_MOD_PIOC]   = 0x27c;
    para.reg_base[DISP_MOD_PWM]    = SUNXI_PWM_BASE;
    para.reg_size[DISP_MOD_PWM]    = 0x3c;
    para.reg_base[DISP_MOD_DEU0]   = SUNXI_DE_DEU0_BASE;
    para.reg_size[DISP_MOD_DEU0]   = 0x60;
    para.reg_base[DISP_MOD_DEU1]   = SUNXI_DE_DEU1_BASE;
    para.reg_size[DISP_MOD_DEU1]   = 0x60;
    para.reg_base[DISP_MOD_CMU0]   = SUNXI_DE_BE0_BASE;
    para.reg_size[DISP_MOD_CMU0]   = 0xfc;
    para.reg_base[DISP_MOD_CMU1]   = SUNXI_DE_BE1_BASE;
    para.reg_size[DISP_MOD_CMU1]   = 0xfc;
    para.reg_base[DISP_MOD_DRC0]   = SUNXI_DE_DRC0_BASE;
    para.reg_size[DISP_MOD_DRC0]   = 0xfc;
    para.reg_base[DISP_MOD_DRC1]   = SUNXI_DE_DRC1_BASE;
    para.reg_size[DISP_MOD_DRC1]   = 0xfc;
    para.reg_base[DISP_MOD_DSI0]   = SUNXI_MIPI_DSI0_BASE;
    para.reg_size[DISP_MOD_DSI0]   = 0x2fc;
    para.reg_base[DISP_MOD_DSI0_DPHY]   = SUNXI_MIPI_DSI0_DPHY_BASE;
    para.reg_size[DISP_MOD_DSI0_DPHY]   = 0xfc;
    para.reg_base[DISP_MOD_HDMI]   = SUNXI_HDMI_BASE;
    para.reg_size[DISP_MOD_HDMI]   = 0x58c;

    para.irq[DISP_MOD_BE0]         = AW_IRQ_DEBE0;
    para.irq[DISP_MOD_BE1]         = AW_IRQ_DEBE1;
    para.irq[DISP_MOD_FE0]         = AW_IRQ_DEFE0;
    para.irq[DISP_MOD_FE1]         = AW_IRQ_DEFE1;
    para.irq[DISP_MOD_DRC0]        = AW_IRQ_DRC01;
    para.irq[DISP_MOD_DRC1]        = AW_IRQ_DEU01;
    para.irq[DISP_MOD_LCD0]        = AW_IRQ_LCD0;
    para.irq[DISP_MOD_LCD1]        = AW_IRQ_LCD1;
    para.irq[DISP_MOD_DSI0]        = AW_IRQ_MIPIDSI;
#else
para.reg_base[DISP_MOD_BE0]    = SUNXI_DE_BE0_BASE;
    para.reg_size[DISP_MOD_BE0]    = 0x5ff;
    para.reg_base[DISP_MOD_BE1]    = SUNXI_DE_BE1_BASE;
    para.reg_size[DISP_MOD_BE1]    = 0x5ff;
    para.reg_base[DISP_MOD_FE0]    = SUNXI_DE_FE0_BASE;
    para.reg_size[DISP_MOD_FE0]    = 0xa18;
    para.reg_base[DISP_MOD_FE1]    = SUNXI_DE_FE1_BASE;
    para.reg_size[DISP_MOD_FE1]    = 0xa18;
    para.reg_base[DISP_MOD_LCD0]   = SUNXI_LCD0_BASE;
    para.reg_size[DISP_MOD_LCD0]   = 0x800;
    para.reg_base[DISP_MOD_LCD1]   = SUNXI_LCD1_BASE;
    para.reg_size[DISP_MOD_LCD1]   = 0x800;
    para.reg_base[DISP_MOD_CCMU]   = SUNXI_CCM_BASE;
    para.reg_size[DISP_MOD_CCMU]   = 0x2dc;
    para.reg_base[DISP_MOD_PIOC]   = SUNXI_PIO_BASE;
    para.reg_size[DISP_MOD_PIOC]   = 0x27c;
    para.reg_base[DISP_MOD_PWM]    = SUNXI_PWM_BASE;
    para.reg_size[DISP_MOD_PWM]    = 0x3c;
    para.reg_base[DISP_MOD_HDMI]   = SUNXI_HDMI_BASE;
    para.reg_size[DISP_MOD_HDMI]   = 0x580;
    para.reg_base[DISP_MOD_TVE0]   = SUNXI_TVE0_BASE;
    para.reg_size[DISP_MOD_TVE0]   = 0x20c;

    para.irq[DISP_MOD_BE0]         = AW_IRQ_DEBE0;
    para.irq[DISP_MOD_BE1]         = AW_IRQ_DEBE1;
    para.irq[DISP_MOD_FE0]         = AW_IRQ_DEFE0;
    para.irq[DISP_MOD_FE1]         = AW_IRQ_DEFE1;
    para.irq[DISP_MOD_LCD0]        = AW_IRQ_LCD0;
    para.irq[DISP_MOD_LCD1]        = AW_IRQ_LCD1;
#endif
    para.disp_int_process    = disp_int_process;
#if ((defined CONFIG_SUN6I) || (defined CONFIG_ARCH_SUN8IW1P1)  || (defined CONFIG_ARCH_SUN7I) || (defined CONFIG_ARCH_SUN5I))
    para.hdmi_open = DRV_hdmi_open;
    para.hdmi_close = DRV_hdmi_close;
    para.hdmi_set_mode = DRV_hdmi_set_display_mode;
    para.hdmi_mode_support = DRV_hdmi_mode_support;
    para.hdmi_get_HPD_status = DRV_hdmi_get_HPD_status;
    para.hdmi_set_pll = DRV_hdmi_set_pll;
    #endif
    //para.hdmi_get_disp_func     = disp_get_hdmi_func;

    memset(&g_disp_drv, 0, sizeof(__disp_drv_t));

    bsp_disp_init(&para);

#if ((defined CONFIG_SUN6I) || (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN7I) || (defined CONFIG_ARCH_SUN5I))
    Hdmi_set_reg_base(0x01c16000);
    Hdmi_hal_init();
    #endif

    bsp_disp_open();
    lcd_init();
#if 0
	if(0)
   	{
        __disp_color_t bk_color;

        printf("====DRV_lcd_open before ====\n");
				DRV_lcd_open(0);
        printf("====DRV_lcd_open after ====\n");
				bsp_disp_print_reg(1, DISP_MOD_PIOC);
				bsp_disp_print_reg(1, DISP_MOD_PWM);
				bsp_disp_print_reg(1, DISP_MOD_CCMU);
				bsp_disp_print_reg(1, DISP_MOD_LCD0);
        bsp_disp_lcd_set_src(0,DISP_LCDC_SRC_WHITE);
        printf("====lcd_white_src ====\n");
        __msdelay(1000);
				*(__u32*)0x1c0c0f4 = 0xffff;
				bsp_disp_print_reg(1, DISP_MOD_LCD0);
        //bsp_disp_lcd_set_src(0,DISP_LCDC_SRC_BLACK);
        //printf("====lcd_black_src ====\n");
        //__msdelay(1000);
				bsp_disp_print_reg(1, DISP_MOD_LCD0);
        bsp_disp_lcd_set_src(0,DISP_LCDC_SRC_DE_CH1);
        printf("====lcd_ch1_src ====\n");
        __msdelay(1000);

        bk_color.red = 0xff;
        bk_color.green = 0x00;
        bk_color.blue = 0x00;
        bsp_disp_set_bk_color(0,&bk_color);
        printf("==== red bk color ====\n");
        __msdelay(1000);
				return 0;

        bk_color.red = 0x00;
        bk_color.green = 0xff;
        bk_color.blue = 0x00;
        bsp_disp_set_bk_color(0,&bk_color);
        printf("==== red green color ====\n");
        __msdelay(1000);

        bk_color.red = 0x00;
        bk_color.green = 0x00;
        bk_color.blue = 0xff;
        bsp_disp_set_bk_color(0,&bk_color);
        printf("==== red bk color ====\n");
        __msdelay(1000);
    }
    printf("====display init end ====\n");
#endif
	printf("DRV_DISP_Init: opened\n");
	display_opens = 1;

    return 0;
}

__s32 drv_disp_exit(void)
{
	if(!display_opens)
	{
		printf("DRV_DISP_Exit: not open\n");
		return 0;
	}
	display_opens = 0;
    //bsp_disp_close();
	bsp_disp_exit(g_disp_drv.exit_mode);
	printf("DRV_DISP_Exit: closed\n");

    return 0;
}

long disp_ioctl(void *hd, unsigned int cmd, void *arg)
{
	unsigned long karg[4];
	unsigned long ubuffer[4] = {0};
	__s32 ret = 0;

	if(!display_opens)
	{
		printf("de not open\n");
		return -1;
	}
	if (copy_from_user((void*)karg,(void __user*)arg,4*sizeof(unsigned long)))
	{
		__wrn("copy_from_user fail %d \n",__LINE__);
		return -EFAULT;
	}

	ubuffer[0] = *(unsigned long*)karg;
	ubuffer[1] = (*(unsigned long*)(karg+1));
	ubuffer[2] = (*(unsigned long*)(karg+2));
	ubuffer[3] = (*(unsigned long*)(karg+3));

    if(cmd < DISP_CMD_FB_REQUEST)
    {
        if((ubuffer[0] != 0) && (ubuffer[0] != 1))
        {
            __wrn("para err in disp_ioctl, cmd = 0x%x,screen id = %d\n", cmd, (int)ubuffer[0]);
            return -1;
        }
    }
    if(suspend_status & 2)
    {
        __wrn("ioctl:%x fail when in suspend!\n", cmd);
        return -1;
    }

#if 0
    if(cmd!=DISP_CMD_TV_GET_INTERFACE && cmd!=DISP_CMD_HDMI_GET_HPD_STATUS && cmd!=DISP_CMD_GET_OUTPUT_TYPE
    	&& cmd!=DISP_CMD_SCN_GET_WIDTH && cmd!=DISP_CMD_SCN_GET_HEIGHT
    	&& cmd!=DISP_CMD_VIDEO_SET_FB && cmd!=DISP_CMD_VIDEO_GET_FRAME_ID)
    {
        OSAL_PRINTF("cmd:0x%x,%ld,%ld\n",cmd, ubuffer[0], ubuffer[1]);
    }
#endif

    switch(cmd)
    {
    //----disp global----
    	case DISP_CMD_SET_BKCOLOR:
	    {
	        __disp_color_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[1],sizeof(__disp_color_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
		    ret = bsp_disp_set_bk_color(ubuffer[0], &para);
		    break;
	    }

    	case DISP_CMD_SET_COLORKEY:
    	{
    	    __disp_colorkey_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[1],sizeof(__disp_colorkey_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_set_color_key(ubuffer[0], &para);
		    break;
		}

    	case DISP_CMD_SET_PALETTE_TBL:
    	    if((ubuffer[1] == 0) || ((int)ubuffer[3] <= 0))
    	    {
    	        __wrn("para invalid in disp ioctrl DISP_CMD_SET_PALETTE_TBL,buffer:0x%x, size:0x%x\n", (unsigned int)ubuffer[1], (unsigned int)ubuffer[3]);
    	        return -1;
    	    }
    		if(copy_from_user(gbuffer, (void __user *)ubuffer[1],ubuffer[3]))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_set_palette_table(ubuffer[0], (__u32 *)gbuffer, ubuffer[2], ubuffer[3]);
    		break;

    	case DISP_CMD_GET_PALETTE_TBL:
    	    if((ubuffer[1] == 0) || ((int)ubuffer[3] <= 0))
    	    {
    	        __wrn("para invalid in disp ioctrl DISP_CMD_GET_PALETTE_TBL,buffer:0x%x, size:0x%x\n", (unsigned int)ubuffer[1], (unsigned int)ubuffer[3]);
    	        return -1;
    	    }
    		ret = bsp_disp_get_palette_table(ubuffer[0], (__u32 *)gbuffer, ubuffer[2], ubuffer[3]);
    		if(copy_to_user((void __user *)ubuffer[1], gbuffer,ubuffer[3]))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;

    	case DISP_CMD_START_CMD_CACHE:
    		ret = bsp_disp_cmd_cache(ubuffer[0]);
    		break;

    	case DISP_CMD_EXECUTE_CMD_AND_STOP_CACHE:
    		ret = bsp_disp_cmd_submit(ubuffer[0]);
    		break;

    	case DISP_CMD_GET_OUTPUT_TYPE:
    		ret =  bsp_disp_get_output_type(ubuffer[0]);
    		break;

    	case DISP_CMD_SCN_GET_WIDTH:
    		ret = bsp_disp_get_screen_width(ubuffer[0]);
    		break;

    	case DISP_CMD_SCN_GET_HEIGHT:
    		ret = bsp_disp_get_screen_height(ubuffer[0]);
    		break;

    	case DISP_CMD_SET_GAMMA_TABLE:
    	    if((ubuffer[1] == 0) || ((int)ubuffer[2] <= 0))
    	    {
    	        __wrn("para invalid in disp ioctrl DISP_CMD_SET_GAMMA_TABLE,buffer:0x%x, size:0x%x\n", (unsigned int)ubuffer[1], (unsigned int)ubuffer[2]);
    	        return -1;
    	    }
    		if(copy_from_user(gbuffer, (void __user *)ubuffer[1],ubuffer[2]))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_set_gamma_table(ubuffer[0], (__u32 *)gbuffer, ubuffer[2]);
    		break;

    	case DISP_CMD_GAMMA_CORRECTION_ON:
    		ret = bsp_disp_gamma_correction_enable(ubuffer[0]);
    		break;

    	case DISP_CMD_GAMMA_CORRECTION_OFF:
    		ret = bsp_disp_gamma_correction_disable(ubuffer[0]);
    		break;

        case DISP_CMD_SET_BRIGHT:
            ret = bsp_disp_cmu_set_bright(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_GET_BRIGHT:
            ret = bsp_disp_cmu_get_bright(ubuffer[0]);
    		break;

        case DISP_CMD_SET_CONTRAST:
            ret = bsp_disp_cmu_set_contrast(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_GET_CONTRAST:
            ret = bsp_disp_cmu_get_contrast(ubuffer[0]);
    		break;

        case DISP_CMD_SET_SATURATION:
            ret = bsp_disp_cmu_set_saturation(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_GET_SATURATION:
            ret = bsp_disp_cmu_get_saturation(ubuffer[0]);
    		break;

        case DISP_CMD_SET_HUE:
            ret = bsp_disp_cmu_set_hue(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_GET_HUE:
            ret = bsp_disp_cmu_get_hue(ubuffer[0]);
    		break;

        case DISP_CMD_ENHANCE_ON:
            ret = bsp_disp_cmu_enable(ubuffer[0], 1);
    		break;

        case DISP_CMD_ENHANCE_OFF:
            ret = bsp_disp_cmu_enable(ubuffer[0], 0);
    		break;

        case DISP_CMD_GET_ENHANCE_EN:
            ret = bsp_disp_cmu_get_enable(ubuffer[0]);
    		break;

        case DISP_CMD_SET_ENHANCE_MODE:
            ret = bsp_disp_cmu_set_mode(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_GET_ENHANCE_MODE:
            ret = bsp_disp_cmu_get_mode(ubuffer[0]);
    		break;

        case DISP_CMD_SET_ENHANCE_WINDOW:
        {
            __disp_rect_t para;

            if(copy_from_user(&para, (void __user *)ubuffer[1],sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_from_user fail\n");
    			return  -EFAULT;
    		}
            ret = bsp_disp_cmu_set_window(ubuffer[0], &para);
    		break;
        }

        case DISP_CMD_GET_ENHANCE_WINDOW:
        {
            __disp_rect_t para;

            ret = bsp_disp_cmu_get_window(ubuffer[0], &para);
            if(copy_to_user((void __user *)ubuffer[1],&para, sizeof(__disp_layer_info_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_CAPTURE_SCREEN:
    	    ret = bsp_disp_capture_screen(ubuffer[0], (__disp_capture_screen_para_t *)ubuffer[1]);
    	    break;

        case DISP_CMD_SET_SCREEN_SIZE:
            ret = bsp_disp_set_screen_size(ubuffer[0], (__disp_rectsz_t*)ubuffer[1]);
            break;

        case DISP_CMD_DE_FLICKER_ON:
            ret = bsp_disp_de_flicker_enable(ubuffer[0], 1);
            break;

        case DISP_CMD_DE_FLICKER_OFF:
            ret = bsp_disp_de_flicker_enable(ubuffer[0], 0);
            break;

        case DISP_CMD_DRC_ON:
            ret = bsp_disp_drc_enable(ubuffer[0], 1);
            break;

        case DISP_CMD_DRC_OFF:
            ret = bsp_disp_drc_enable(ubuffer[0], 0);
            break;

        case DISP_CMD_GET_DRC_EN:
            ret = bsp_disp_drc_get_enable(ubuffer[0]);
            break;

        case DISP_CMD_DRC_SET_WINDOW:
        {
            __disp_rect_t para;

            if(copy_from_user(&para, (void __user *)ubuffer[1],sizeof(__disp_rect_t)))
            {
                __wrn("copy_from_user fail\n");
                return  -EFAULT;
            }

            ret = bsp_disp_drc_set_window(ubuffer[0], &para);
            break;
       }

       case DISP_CMD_DRC_GET_WINDOW:
        {
            __disp_rect_t para;

            ret = bsp_disp_drc_get_window(ubuffer[0], &para);
            if(copy_to_user((void __user *)ubuffer[1], &para,sizeof(__disp_rect_t)))
            {
                __wrn("copy_to_user fail\n");
                return  -EFAULT;
            }

            return ret;
            break;
       }
    //----layer----
    	case DISP_CMD_LAYER_REQUEST:
    		ret = bsp_disp_layer_request(ubuffer[0], (__disp_layer_work_mode_t)ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_RELEASE:
    		ret = bsp_disp_layer_release(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_OPEN:
    		ret = bsp_disp_layer_open(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_CLOSE:
    		ret = bsp_disp_layer_close(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_SET_FB:
    	{
    	    __disp_fb_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_fb_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_layer_set_framebuffer(ubuffer[0], ubuffer[1], &para);
    		//DRV_disp_wait_cmd_finish(ubuffer[0]);
    		break;
    	}

    	case DISP_CMD_LAYER_GET_FB:
    	{
    	    __disp_fb_t para;

    		ret = bsp_disp_layer_get_framebuffer(ubuffer[0], ubuffer[1], &para);
    		if(copy_to_user((void __user *)ubuffer[2], &para,sizeof(__disp_fb_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_LAYER_SET_SRC_WINDOW:
    	{
    	    __disp_rect_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_layer_set_src_window(ubuffer[0],ubuffer[1], &para);
    		//DRV_disp_wait_cmd_finish(ubuffer[0]);
    		break;
        }

    	case DISP_CMD_LAYER_GET_SRC_WINDOW:
    	{
    	    __disp_rect_t para;

    		ret = bsp_disp_layer_get_src_window(ubuffer[0],ubuffer[1], &para);
    		if(copy_to_user((void __user *)ubuffer[2], &para, sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_LAYER_SET_SCN_WINDOW:
    	{
    	    __disp_rect_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_layer_set_screen_window(ubuffer[0],ubuffer[1], &para);
    		//DRV_disp_wait_cmd_finish(ubuffer[0]);
    		break;
        }

    	case DISP_CMD_LAYER_GET_SCN_WINDOW:
    	{
    	    __disp_rect_t para;

    		ret = bsp_disp_layer_get_screen_window(ubuffer[0],ubuffer[1], &para);
    		if(copy_to_user((void __user *)ubuffer[2], &para, sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_LAYER_SET_PARA:
    	{
    	    __disp_layer_info_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_layer_info_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_layer_set_para(ubuffer[0], ubuffer[1], &para);
    		//DRV_disp_wait_cmd_finish(ubuffer[0]);
    		break;
        }

    	case DISP_CMD_LAYER_GET_PARA:
    	{
    	    __disp_layer_info_t para;

    		ret = bsp_disp_layer_get_para(ubuffer[0], ubuffer[1], &para);
    		if(copy_to_user((void __user *)ubuffer[2],&para, sizeof(__disp_layer_info_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_LAYER_TOP:
    		ret = bsp_disp_layer_set_top(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_BOTTOM:
    		ret = bsp_disp_layer_set_bottom(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_ALPHA_ON:
    		ret = bsp_disp_layer_alpha_enable(ubuffer[0], ubuffer[1], 1);
    		break;

    	case DISP_CMD_LAYER_ALPHA_OFF:
    		ret = bsp_disp_layer_alpha_enable(ubuffer[0], ubuffer[1], 0);
    		break;

    	case DISP_CMD_LAYER_SET_ALPHA_VALUE:
    		ret = bsp_disp_layer_set_alpha_value(ubuffer[0], ubuffer[1], ubuffer[2]);
    		//DRV_disp_wait_cmd_finish(ubuffer[0]);
    		break;

    	case DISP_CMD_LAYER_CK_ON:
    		ret = bsp_disp_layer_colorkey_enable(ubuffer[0], ubuffer[1], 1);
    		break;

    	case DISP_CMD_LAYER_CK_OFF:
    		ret = bsp_disp_layer_colorkey_enable(ubuffer[0], ubuffer[1], 0);
    		break;

    	case DISP_CMD_LAYER_SET_PIPE:
    		ret = bsp_disp_layer_set_pipe(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

    	case DISP_CMD_LAYER_GET_ALPHA_VALUE:
    		ret = bsp_disp_layer_get_alpha_value(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_GET_ALPHA_EN:
    		ret = bsp_disp_layer_get_alpha_enable(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_GET_CK_EN:
    		ret = bsp_disp_layer_get_colorkey_enable(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_GET_PRIO:
    		ret = bsp_disp_layer_get_piro(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_LAYER_GET_PIPE:
    		ret = bsp_disp_layer_get_pipe(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_SMOOTH:
            ret = bsp_disp_layer_set_smooth(ubuffer[0], ubuffer[1],(__disp_video_smooth_t) ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_SMOOTH:
            ret = bsp_disp_layer_get_smooth(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_BRIGHT:
            ret = bsp_disp_cmu_layer_set_bright(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_BRIGHT:
            ret = bsp_disp_cmu_layer_get_bright(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_CONTRAST:
            ret = bsp_disp_cmu_layer_set_contrast(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_CONTRAST:
            ret = bsp_disp_cmu_layer_get_contrast(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_SATURATION:
            ret = bsp_disp_cmu_layer_set_saturation(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_SATURATION:
            ret = bsp_disp_cmu_layer_get_saturation(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_HUE:
            ret = bsp_disp_cmu_layer_set_hue(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_HUE:
            ret = bsp_disp_cmu_layer_get_hue(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_ENHANCE_ON:
            ret = bsp_disp_cmu_layer_enable(ubuffer[0], ubuffer[1], 1);
    		break;

        case DISP_CMD_LAYER_ENHANCE_OFF:
            ret = bsp_disp_cmu_layer_enable(ubuffer[0], ubuffer[1], 0);
    		break;

        case DISP_CMD_LAYER_GET_ENHANCE_EN:
            ret = bsp_disp_cmu_layer_get_enable(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_ENHANCE_MODE:
            ret = bsp_disp_cmu_layer_set_mode(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_ENHANCE_MODE:
            ret = bsp_disp_cmu_layer_get_mode(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_ENHANCE_WINDOW:
        {
            __disp_rect_t para;
            if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_from_user fail\n");
    			return  -EFAULT;
    		}
            ret = bsp_disp_cmu_layer_set_window(ubuffer[0], ubuffer[1], &para);
    		break;
       }

        case DISP_CMD_LAYER_GET_ENHANCE_WINDOW:
        {
            __disp_rect_t para;
            ret = bsp_disp_cmu_layer_get_window(ubuffer[0], ubuffer[1], &para);
            if(copy_to_user((void __user *)ubuffer[2],&para, sizeof(__disp_layer_info_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
       }
        case DISP_CMD_LAYER_VPP_ON:
            ret = bsp_disp_deu_enable(ubuffer[0], ubuffer[1], 1);
    		break;

        case DISP_CMD_LAYER_VPP_OFF:
            ret = bsp_disp_deu_enable(ubuffer[0], ubuffer[1], 0);
    		break;

        case DISP_CMD_LAYER_GET_VPP_EN:
            ret = bsp_disp_deu_get_enable(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_LUMA_SHARP_LEVEL:
            ret = bsp_disp_deu_set_luma_sharp_level(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_LUMA_SHARP_LEVEL:
            ret = bsp_disp_deu_get_luma_sharp_level(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_CHROMA_SHARP_LEVEL:
            ret = bsp_disp_deu_set_chroma_sharp_level(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_CHROMA_SHARP_LEVEL:
            ret = bsp_disp_deu_get_chroma_sharp_level(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_WHITE_EXTEN_LEVEL:
            ret = bsp_disp_deu_set_white_exten_level(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_WHITE_EXTEN_LEVEL:
            ret = bsp_disp_deu_get_white_exten_level(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_LAYER_SET_BLACK_EXTEN_LEVEL:
            ret = bsp_disp_deu_set_black_exten_level(ubuffer[0], ubuffer[1], ubuffer[2]);
    		break;

        case DISP_CMD_LAYER_GET_BLACK_EXTEN_LEVEL:
            ret = bsp_disp_deu_get_black_exten_level(ubuffer[0], ubuffer[1]);
    		break;
        case DISP_CMD_LAYER_VPP_SET_WINDOW:
        {
            __disp_rect_t para;

            if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_rect_t)))
            {
                __wrn("copy_from_user fail\n");
                return  -EFAULT;
            }

            ret = bsp_disp_deu_set_window(ubuffer[0], ubuffer[1], &para);
            break;
       }

       case DISP_CMD_LAYER_VPP_GET_WINDOW:
        {
            __disp_rect_t para;

            ret = bsp_disp_deu_get_window(ubuffer[0], ubuffer[1], &para);
            if(copy_to_user((void __user *)ubuffer[2], &para,sizeof(__disp_rect_t)))
            {
                __wrn("copy_to_user fail\n");
                return  -EFAULT;
            }

            return ret;
            break;
       }

    //----scaler----
    	case DISP_CMD_SCALER_REQUEST:
    		ret = bsp_disp_scaler_request();
    		break;

    	case DISP_CMD_SCALER_RELEASE:
    		ret = bsp_disp_scaler_release(ubuffer[1]);
    		break;

    	case DISP_CMD_SCALER_EXECUTE:
    	{
    	    __disp_scaler_para_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_scaler_para_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_scaler_start(ubuffer[1],&para);
    		break;
        }

        case DISP_CMD_SCALER_EXECUTE_EX:
    	{
    	    __disp_scaler_para_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_scaler_para_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_scaler_start_ex(ubuffer[1],&para);
    		break;
        }

    //----hwc----
    	case DISP_CMD_HWC_OPEN:
    		ret =  bsp_disp_hwc_enable(ubuffer[0], 1);
    		break;

    	case DISP_CMD_HWC_CLOSE:
    		ret =  bsp_disp_hwc_enable(ubuffer[0], 0);
    		break;

    	case DISP_CMD_HWC_SET_POS:
    	{
    	    __disp_pos_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[1],sizeof(__disp_pos_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_hwc_set_pos(ubuffer[0], &para);
    		break;
        }

    	case DISP_CMD_HWC_GET_POS:
    	{
    	    __disp_pos_t para;

    		ret = bsp_disp_hwc_get_pos(ubuffer[0], &para);
    		if(copy_to_user((void __user *)ubuffer[1],&para, sizeof(__disp_pos_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_HWC_SET_FB:
    	{
    	    __disp_hwc_pattern_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[1],sizeof(__disp_hwc_pattern_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_hwc_set_framebuffer(ubuffer[0], &para);
    		break;
        }

    	case DISP_CMD_HWC_SET_PALETTE_TABLE:
			if((ubuffer[1] == 0) || ((int)ubuffer[3] <= 0))
            {
                __wrn("para invalid in display ioctrl DISP_CMD_HWC_SET_PALETTE_TABLE,buffer:0x%x, size:0x%x\n", (unsigned int)ubuffer[1], (unsigned int)ubuffer[3]);
                return -1;
            }
    		if(copy_from_user(gbuffer, (void __user *)ubuffer[1],ubuffer[3]))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_hwc_set_palette(ubuffer[0], (void*)gbuffer, ubuffer[2], ubuffer[3]);
    		break;


    //----video----
    	case DISP_CMD_VIDEO_START:
    		ret = bsp_disp_video_start(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_VIDEO_STOP:
    		ret = bsp_disp_video_stop(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_VIDEO_SET_FB:
    	{
    	    __disp_video_fb_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_video_fb_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
    		ret = bsp_disp_video_set_fb(ubuffer[0], ubuffer[1], &para);
    		break;
        }

        case DISP_CMD_VIDEO_GET_FRAME_ID:
            ret = bsp_disp_video_get_frame_id(ubuffer[0], ubuffer[1]);
    		break;

        case DISP_CMD_VIDEO_GET_DIT_INFO:
        {
            __disp_dit_info_t para;

            ret = bsp_disp_video_get_dit_info(ubuffer[0], ubuffer[1],&para);
    		if(copy_to_user((void __user *)ubuffer[2],&para, sizeof(__disp_dit_info_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    //----lcd----
    	case DISP_CMD_LCD_ON:
    		ret = DRV_lcd_open(ubuffer[0]);
    		output_type = DISP_OUTPUT_TYPE_LCD;
            if(suspend_status != 0)
            {
                suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_LCD;
            }
    		break;

    	case DISP_CMD_LCD_OFF:
    		ret = DRV_lcd_close(ubuffer[0]);
            if(suspend_status != 0)
            {
                suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_NONE;
            }
    		break;

    	case DISP_CMD_LCD_SET_BRIGHTNESS:
    		ret = bsp_disp_lcd_set_bright(ubuffer[0], ubuffer[1], 0);
    		break;

    	case DISP_CMD_LCD_GET_BRIGHTNESS:
    		ret = bsp_disp_lcd_get_bright(ubuffer[0]);
    		break;

//    	case DISP_CMD_LCD_CPUIF_XY_SWITCH:
// 		ret = bsp_disp_lcd_xy_switch(ubuffer[0], ubuffer[1]);
  //  		break;

    	case DISP_CMD_LCD_SET_SRC:
    		ret = bsp_disp_lcd_set_src(ubuffer[0], (__disp_lcdc_src_t)ubuffer[1]);
    		break;

        case DISP_CMD_LCD_USER_DEFINED_FUNC:
            ret = bsp_disp_lcd_user_defined_func(ubuffer[0], ubuffer[1], ubuffer[2], ubuffer[3]);
            break;

    //----tv----
    	case DISP_CMD_TV_ON:
    		ret = bsp_disp_tv_open(ubuffer[0]);
            if(suspend_status != 0)
            {
                suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_TV;
            }
    		break;

    	case DISP_CMD_TV_OFF:
    		ret = bsp_disp_tv_close(ubuffer[0]);
            if(suspend_status != 0)
            {
                suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_NONE;
            }
    		break;

    	case DISP_CMD_TV_SET_MODE:
    		ret = bsp_disp_tv_set_mode(ubuffer[0], (__disp_tv_mode_t)ubuffer[1]);
    		break;

    	case DISP_CMD_TV_GET_MODE:
    		ret = bsp_disp_tv_get_mode(ubuffer[0]);
    		break;

    	case DISP_CMD_TV_AUTOCHECK_ON:
    		ret = bsp_disp_tv_auto_check_enable(ubuffer[0]);
    		break;

    	case DISP_CMD_TV_AUTOCHECK_OFF:
    		ret = bsp_disp_tv_auto_check_disable(ubuffer[0]);
    		break;

    	case DISP_CMD_TV_GET_INTERFACE:
    	    if(suspend_status != 0)
    	    {
    	        ret = DISP_TV_NONE;
    	    }
    	    else
    	    {
    		    ret = bsp_disp_tv_get_interface(ubuffer[0]);
            }
    		break;

    	case DISP_CMD_TV_SET_SRC:
    		ret = bsp_disp_tv_set_src(ubuffer[0], (__disp_lcdc_src_t)ubuffer[1]);
    		break;

        case DISP_CMD_TV_GET_DAC_STATUS:
            if(suspend_status != 0)
            {
                ret = 0;
            }
            else
            {
                ret =  bsp_disp_tv_get_dac_status(ubuffer[0], ubuffer[1]);
            }
            break;

        case DISP_CMD_TV_SET_DAC_SOURCE:
            ret =  bsp_disp_tv_set_dac_source(ubuffer[0], ubuffer[1], (__disp_tv_dac_source)ubuffer[2]);
            break;

        case DISP_CMD_TV_GET_DAC_SOURCE:
            ret =  bsp_disp_tv_get_dac_source(ubuffer[0], ubuffer[1]);
            break;

    //----hdmi----
    	case DISP_CMD_HDMI_ON:
    		ret = bsp_disp_hdmi_open(ubuffer[0]);
    		output_type = DISP_OUTPUT_TYPE_HDMI;
            if(suspend_status != 0)
            {
                suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_HDMI;
            }
    		break;

    	case DISP_CMD_HDMI_OFF:
    		ret = bsp_disp_hdmi_close(ubuffer[0]);
            if(suspend_status != 0)
            {
                suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_NONE;
            }
    		break;

    	case DISP_CMD_HDMI_SET_MODE:
    		ret = bsp_disp_hdmi_set_mode(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_HDMI_GET_MODE:
    		ret = bsp_disp_hdmi_get_mode(ubuffer[0]);
    		break;

    	case DISP_CMD_HDMI_GET_HPD_STATUS:
    	    if(suspend_status != 0)
    	    {
    	        ret = 0;
    	    }
    	    else
    	    {
    	        ret = bsp_disp_hdmi_get_hpd_status(ubuffer[0]);
    	    }
    		break;

    	case DISP_CMD_HDMI_SUPPORT_MODE:
    		ret = bsp_disp_hdmi_check_support_mode(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_HDMI_SET_SRC:
    		ret = bsp_disp_hdmi_set_src(ubuffer[0], (__disp_lcdc_src_t)ubuffer[1]);
    		break;

    //----vga----
    	case DISP_CMD_VGA_ON:
    		ret = bsp_disp_vga_open(ubuffer[0]);
            if(suspend_status != 0)
            {
                suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_VGA;
            }
    		break;

    	case DISP_CMD_VGA_OFF:
    		ret = bsp_disp_vga_close(ubuffer[0]);
            if(suspend_status != 0)
            {
                suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_NONE;
            }
    		break;

    	case DISP_CMD_VGA_SET_MODE:
    		ret = bsp_disp_vga_set_mode(ubuffer[0], (__disp_vga_mode_t)ubuffer[1]);
    		break;

    	case DISP_CMD_VGA_GET_MODE:
    		ret = bsp_disp_vga_get_mode(ubuffer[0]);
    		break;

    	case DISP_CMD_VGA_SET_SRC:
    		ret = bsp_disp_vga_set_src(ubuffer[0], (__disp_lcdc_src_t)ubuffer[1]);
    		break;

    //----sprite----
    /*
    	case DISP_CMD_SPRITE_OPEN:
    		ret = bsp_disp_sprite_open(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_CLOSE:
    		ret = bsp_disp_sprite_close(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_SET_FORMAT:
    		ret = bsp_disp_sprite_set_format(ubuffer[0], (__disp_pixel_fmt_t)ubuffer[1], (__disp_pixel_seq_t)ubuffer[2]);
    		break;

    	case DISP_CMD_SPRITE_GLOBAL_ALPHA_ENABLE:
    		ret = bsp_disp_sprite_alpha_enable(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_GLOBAL_ALPHA_DISABLE:
    		ret = bsp_disp_sprite_alpha_disable(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_GET_GLOBAL_ALPHA_ENABLE:
    		ret = bsp_disp_sprite_get_alpha_enable(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_SET_GLOBAL_ALPHA_VALUE:
    		ret = bsp_disp_sprite_set_alpha_vale(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_GET_GLOBAL_ALPHA_VALUE:
    		ret = bsp_disp_sprite_get_alpha_value(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_SET_ORDER:
    		ret = bsp_disp_sprite_set_order(ubuffer[0], ubuffer[1],ubuffer[2]);
    		break;

    	case DISP_CMD_SPRITE_GET_TOP_BLOCK:
    		ret = bsp_disp_sprite_get_top_block(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_GET_BOTTOM_BLOCK:
    		ret = bsp_disp_sprite_get_bottom_block(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_SET_PALETTE_TBL:
            if((ubuffer[1] == 0) || ((int)ubuffer[3] <= 0))
            {
                __wrn("para invalid in display ioctrl DISP_CMD_SPRITE_SET_PALETTE_TBL,buffer:0x%x, size:0x%x\n", (unsigned int)ubuffer[1], (unsigned int)ubuffer[3]);
                return -1;
            }
    		if(copy_from_user(gbuffer, (void __user *)ubuffer[1],ubuffer[3]))
    		{
    		    __wrn("copy_from_user fail\n");
    			return  -EFAULT;
    		}
    		ret =  bsp_disp_sprite_set_palette_table(ubuffer[0], (__u32 * )gbuffer,ubuffer[2],ubuffer[3]);
    		break;

    	case DISP_CMD_SPRITE_GET_BLOCK_NUM:
    		ret = bsp_disp_sprite_get_block_number(ubuffer[0]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_REQUEST:
    	{
    	    __disp_sprite_block_para_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[1],sizeof(__disp_sprite_block_para_t)))
    		{
    		    __wrn("copy_from_user fail\n");
    			return  -EFAULT;
    		}
    		ret = bsp_disp_sprite_block_request(ubuffer[0], &para);
    		break;
        }

    	case DISP_CMD_SPRITE_BLOCK_RELEASE:
    		ret = bsp_disp_sprite_block_release(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_SET_SCREEN_WINDOW:
    	{
    	    __disp_rect_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_from_user fail\n");
    			return  -EFAULT;
    		}
    		ret = bsp_disp_sprite_block_set_screen_win(ubuffer[0], ubuffer[1],&para);
    		break;
        }

    	case DISP_CMD_SPRITE_BLOCK_GET_SCREEN_WINDOW:
    	{
    	    __disp_rect_t para;

    		ret = bsp_disp_sprite_block_get_srceen_win(ubuffer[0], ubuffer[1],&para);
    		if(copy_to_user((void __user *)ubuffer[2],&para, sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_SPRITE_BLOCK_SET_SOURCE_WINDOW:
    	{
    	    __disp_rect_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_from_user fail\n");
    			return  -EFAULT;
    		}
    		ret = bsp_disp_sprite_block_set_src_win(ubuffer[0], ubuffer[1],&para);
    		break;
        }

    	case DISP_CMD_SPRITE_BLOCK_GET_SOURCE_WINDOW:
    	{
    	    __disp_rect_t para;

    		ret = bsp_disp_sprite_block_get_src_win(ubuffer[0], ubuffer[1],&para);
    		if(copy_to_user((void __user *)ubuffer[2],&para, sizeof(__disp_rect_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_SPRITE_BLOCK_SET_FB:
    	{
    	    __disp_fb_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_fb_t)))
    		{
    		    __wrn("copy_from_user fail\n");
    			return  -EFAULT;
    		}
    		ret = bsp_disp_sprite_block_set_framebuffer(ubuffer[0], ubuffer[1],&para);
    		break;
        }

    	case DISP_CMD_SPRITE_BLOCK_GET_FB:
    	{
    	    __disp_fb_t para;

    		ret = bsp_disp_sprite_block_get_framebufer(ubuffer[0], ubuffer[1],&para);
    		if(copy_to_user((void __user *)ubuffer[2],&para, sizeof(__disp_fb_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }

    	case DISP_CMD_SPRITE_BLOCK_SET_TOP:
    		ret = bsp_disp_sprite_block_set_top(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_SET_BOTTOM:
    		ret = bsp_disp_sprite_block_set_bottom(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_GET_PREV_BLOCK:
    		ret = bsp_disp_sprite_block_get_pre_block(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_GET_NEXT_BLOCK:
    		ret = bsp_disp_sprite_block_get_next_block(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_GET_PRIO:
    		ret = bsp_disp_sprite_block_get_prio(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_OPEN:
    		ret = bsp_disp_sprite_block_open(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_CLOSE:
    		ret = bsp_disp_sprite_block_close(ubuffer[0], ubuffer[1]);
    		break;

    	case DISP_CMD_SPRITE_BLOCK_SET_PARA:
    	{
    	    __disp_sprite_block_para_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[2],sizeof(__disp_sprite_block_para_t)))
    		{
    		    __wrn("copy_from_user fail\n");
    			return  -EFAULT;
    		}
    		ret = bsp_disp_sprite_block_set_para(ubuffer[0], ubuffer[1],&para);
    		break;
        }

    	case DISP_CMD_SPRITE_BLOCK_GET_PARA:
    	{
    	    __disp_sprite_block_para_t para;

    		ret = bsp_disp_sprite_block_get_para(ubuffer[0], ubuffer[1],&para);
    		if(copy_to_user((void __user *)ubuffer[2],&para, sizeof(__disp_sprite_block_para_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
    		break;
        }
        */
#ifdef __LINUX_OSAL__
	//----framebuffer----
    	case DISP_CMD_FB_REQUEST:
    	{
    	    __disp_fb_create_para_t para;

    		if(copy_from_user(&para, (void __user *)ubuffer[1],sizeof(__disp_fb_create_para_t)))
    		{
    		    __wrn("copy_from_user fail %d \n",__LINE__);
    			return  -EFAULT;
    		}
			ret = Display_Fb_Request(ubuffer[0], &para);
			break;
        }

		case DISP_CMD_FB_RELEASE:
			ret = Display_Fb_Release(ubuffer[0]);
			break;

	    case DISP_CMD_FB_GET_PARA:
	    {
    	    __disp_fb_create_para_t para;

			ret = Display_Fb_get_para(ubuffer[0], &para);
    		if(copy_to_user((void __user *)ubuffer[1],&para, sizeof(__disp_fb_create_para_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
			break;
        }

	    case DISP_CMD_GET_DISP_INIT_PARA:
	    {
    	    __disp_init_t para;

			ret = Display_get_disp_init_para(&para);
    		if(copy_to_user((void __user *)ubuffer[0],&para, sizeof(__disp_init_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
			break;
        }
//----for test----
		case DISP_CMD_MEM_REQUEST:
			ret =  disp_mem_request(ubuffer[0]);
			break;


		case DISP_CMD_MEM_RELASE:
			ret =  disp_mem_release(ubuffer[0]);
			break;

		case DISP_CMD_MEM_SELIDX:
			g_disp_mm_sel = ubuffer[0];
			break;

		case DISP_CMD_MEM_GETADR:
			ret = g_disp_mm[ubuffer[0]].mem_start;
			break;

		case DISP_CMD_MEM_GET_INFO:
		{
    	    __disp_mm_t para;

			ret = disp_mem_get_info(ubuffer[0], &para);
    		if(copy_to_user((void __user *)ubuffer[1],&para, sizeof(__disp_mm_t)))
    		{
    		    __wrn("copy_to_user fail\n");
    			return  -EFAULT;
    		}
			break;
        }
		case DISP_CMD_SUSPEND:
		{
		    pm_message_t state;

			ret = disp_suspend(0, state);
			break;
        }

		case DISP_CMD_RESUME:
			ret = disp_resume(0);
			break;
#endif
		case DISP_CMD_SET_EXIT_MODE:
	    	ret =  g_disp_drv.exit_mode = ubuffer[0];
			break;

		case DISP_CMD_LCD_CHECK_OPEN_FINISH:
			ret = DRV_lcd_check_open_finished(ubuffer[0]);
			break;

		case DISP_CMD_LCD_CHECK_CLOSE_FINISH:
			ret = DRV_lcd_check_close_finished(ubuffer[0]);
			break;

        case DISP_CMD_PRINT_REG:
            ret = bsp_disp_print_reg(1, ubuffer[0]);
            break;

		default:
		    break;
    }

	return ret;
}

#define  DELAY_ONCE_TIME   (50)

__s32 drv_disp_standby(__u32 cmd, void *pArg)
{
	__s32 ret;
	__s32 timedly = 5000;
	__s32 check_time = timedly/DELAY_ONCE_TIME;

	if(cmd == BOOT_MOD_ENTER_STANDBY)
	{
	    if(output_type == DISP_OUTPUT_TYPE_HDMI)
	    {
	    	DRV_hdmi_close();
		}
		else
        {
        	DRV_lcd_close(0);
		}
		do
		{
			ret = DRV_lcd_check_close_finished(0);
			if(ret == 1)
			{
				break;
			}
			else if(ret == -1)
			{
				return -1;
			}
			__msdelay(DELAY_ONCE_TIME);
			check_time --;
			if(check_time <= 0)
			{
				return -1;
			}
		}
		while(1);
		bsp_disp_clk_off(3);

		return 0;
	}
	else if(cmd == BOOT_MOD_EXIT_STANDBY)
	{
		bsp_disp_clk_on(3);
		if(output_type == DISP_OUTPUT_TYPE_HDMI)
		{
			DRV_hdmi_open();
		}
		else
		{
			DRV_lcd_open(0);
        }

		do
		{
			ret = DRV_lcd_check_open_finished(0);
			if(ret == 1)
			{
				break;
			}
			else if(ret == -1)
			{
				return -1;
			}
			__msdelay(DELAY_ONCE_TIME);
			check_time --;
			if(check_time <= 0)
			{
				return -1;
			}
		}
		while(1);

		return 0;
	}

	return -1;
}


