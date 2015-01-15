#ifndef __EBSP_DISPLAY_H__
#define __EBSP_DISPLAY_H__

#if defined(CONFIG_FPGA_V4_PLATFORM) || defined(CONFIG_FPGA_V7_PLATFORM) || defined(CONFIG_A67_FPGA)
#define __FPGA_DEBUG__
#endif

//#define __LINUX_PLAT__
//#define __MELIS_OSAL__
//#define __WINCE_OSAL__
#define __BOOT_OSAL__
#define __UBOOT_OSAL__

typedef unsigned int __hdle;

/*basic data information definition*/
#define false 0
#define true 1

#if defined(__LINUX_PLAT__)
#include <linux/module.h>
#include "linux/kernel.h"
#include "linux/mm.h"
#include <asm/uaccess.h>
#include <asm/memory.h>
#include <asm/unistd.h>
#include "linux/semaphore.h"
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/fb.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()¡¢kthread_run()
#include <linux/err.h> //IS_ERR()¡¢PTR_ERR()
#include <linux/delay.h>
#include <linux/platform_device.h>
#include "asm-generic/int-ll64.h"
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/pwm.h>
#include <mach/sys_config.h>
#include <mach/irqs.h>
#include <mach/platform.h>
#if defined(CONFIG_ARCH_SUN8IW1P1) || defined(CONFIG_ARCH_SUN9IW1P1)
#include <linux/ion_sunxi.h>
#else
#include <linux/sunxi_physmem.h>
#include <mach/clock.h>
#endif

#include "../OSAL/OSAL.h"

#define DEFAULT_PRINT_LEVLE 0
#if 1
#define OSAL_PRINTF(msg...) do{printk(KERN_WARNING "[DISP] ");printk(msg);}while(0)
#define __inf(msg...)       do{if(bsp_disp_get_print_level()){printk(KERN_WARNING "[DISP] ");printk(msg);}}while(0)
#define __msg(msg...)       do{if(bsp_disp_get_print_level()){printk(KERN_WARNING "[DISP] file:%s,line:%d:    ",__FILE__,__LINE__);printk(msg);}}while(0)
#define __wrn(msg...)       do{{printk(KERN_WARNING "[DISP] file:%s,line:%d:    ",__FILE__,__LINE__);printk(msg);}}while(0)
#define __here__            do{if(bsp_disp_get_print_level()==2){printk(KERN_WARNING "[DISP] file:%s,line:%d\n",__FILE__,__LINE__);}}while(0)
#define __debug(msg...)     do{if(bsp_disp_get_print_level()==2){printk(KERN_WARNING "[DISP] ");printk(msg);}}while(0)
#else
#define OSAL_PRINTF(msg...) printk(msg)
#define __inf(msg...)
#define __msg(msg...)
#define __wrn(msg...)       do{{printk(KERN_WARNING "[DISP] file:%s,line:%d:    ",__FILE__,__LINE__);printk(msg);}}while(0)
#define __here__
#define __debug(msg...)
#endif

#endif//end of define __LINUX_PLAT__

#ifdef __BOOT_OSAL__
#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/drv_display.h>
#include <sys_config.h>
#include <asm/arch/intc.h>
#include <asm/arch/cpu.h>
#include <pmu.h>
#include <pwm.h>
#include <asm/arch/timer.h>
#include <asm/arch/platform.h>

#include "../../OSAL/OSAL.h"

#include <linux/list.h>

#include <asm/memory.h>

#define OSAL_PRINTF
#define __inf(msg...)
#define __msg(msg...)
#define __wrn(msg...) printf(msg)
#define __here
#define __debug

#endif

typedef struct
{
	u32                   reg_base[DISP_MOD_NUM];
	u32                   reg_size[DISP_MOD_NUM];
	u32                   irq_no[DISP_MOD_NUM];

	s32 (*disp_int_process)(u32 sel);
	s32 (*vsync_event)(u32 sel);
	s32 (*start_process)(void);
	s32 (*capture_event)(u32 sel);
	s32 (*shadow_protect)(u32 sel, bool protect);
}__disp_bsp_init_para;


extern s32 bsp_disp_shadow_protect(u32 screen_id, bool protect);
//extern s32 bsp_disp_set_print_level(u32 print_level);
extern s32 bsp_disp_get_print_level(void);
extern s32 bsp_disp_vsync_event_enable(u32 screen_id, bool enable);
extern s32 bsp_disp_get_lcd_registered(u32 screen_id);
extern s32 bsp_disp_get_hdmi_registered(void);
extern s32 bsp_disp_get_output_type(u32 screen_id);
extern s32 bsp_disp_get_screen_width(u32 screen_id);
extern s32 bsp_disp_get_screen_height(u32 screen_id);
extern s32 bsp_disp_get_screen_physical_width(u32 screen_id);
extern s32 bsp_disp_get_screen_physical_height(u32 screen_id);
extern s32 bsp_disp_get_screen_width_from_output_type(u32 screen_id, u32 output_type, u32 output_mode);
extern s32 bsp_disp_get_screen_height_from_output_type(u32 screen_id, u32 output_type, u32 output_mode);
extern s32 bsp_disp_open(void);
extern s32 bsp_disp_close(void);
extern s32 bsp_disp_init(__disp_bsp_init_para * para);
extern s32 bsp_disp_exit(u32 mode);
extern s32 bsp_disp_get_timming(u32 screen_id, disp_video_timing * tt);

extern s32 bsp_disp_hdmi_enable(u32 screen_id);
extern s32 bsp_disp_hdmi_disable(u32 screen_id);
extern s32 bsp_disp_hdmi_set_mode(u32 screen_id,  disp_tv_mode mode);
extern s32 bsp_disp_hdmi_get_mode(u32 screen_id);
extern s32 bsp_disp_hdmi_check_support_mode(u32 screen_id,  u8 mode);
extern s32 bsp_disp_hdmi_get_input_csc(u32 screen_id);
extern s32 bsp_disp_set_hdmi_func(u32 screen_id, disp_hdmi_func * func);

extern s32 bsp_disp_lcd_pre_enable(u32 sel);
extern s32 bsp_disp_lcd_post_enable(u32 sel);
extern disp_lcd_flow * bsp_disp_lcd_get_open_flow(u32 sel);
extern s32 bsp_disp_lcd_pre_disable(u32 sel);
extern s32 bsp_disp_lcd_post_disable(u32 sel);
extern disp_lcd_flow * bsp_disp_lcd_get_close_flow(u32 sel);
extern s32 bsp_disp_lcd_set_bright(u32 sel, u32  bright);
extern s32 bsp_disp_lcd_get_bright(u32 sel);
extern s32 bsp_disp_lcd_power_enable(u32 screen_id, u32 pwr_id);
extern s32 bsp_disp_lcd_power_disable(u32 screen_id, u32 pwr_id);
extern s32 bsp_disp_lcd_backlight_enable(u32 screen_id);
extern s32 bsp_disp_lcd_backlight_disable(u32 screen_id);
extern s32 bsp_disp_lcd_pwm_enable(u32 screen_id);
extern s32 bsp_disp_lcd_pwm_disable(u32 screen_id);
extern s32 bsp_disp_get_timming(u32 sel, disp_video_timing * tt);
extern s32 bsp_disp_lcd_is_used(u32 sel);
extern s32 bsp_disp_lcd_tcon_enable(u32 sel);
extern s32 bsp_disp_lcd_tcon_disable(u32 sel);
extern s32 bsp_disp_lcd_get_registered(void);
extern s32 bsp_disp_lcd_delay_ms(u32 ms);
extern s32 bsp_disp_lcd_delay_us(u32 us);
extern s32 bsp_disp_lcd_set_panel_funs(char *name, disp_lcd_panel_fun * lcd_cfg);
extern s32 bsp_disp_lcd_pin_cfg(u32 screen_id, u32 bon);
extern s32 bsp_disp_lcd_gpio_set_value(u32 screen_id, u32 io_index, u32 value);
extern s32 bsp_disp_lcd_gpio_set_direction(u32 screen_id, u32 io_index, u32 direction);

extern s32 bsp_disp_layer_set_info(u32 screen_id, u32 layer_id,disp_layer_info *player);
extern s32 bsp_disp_layer_get_info(u32 screen_id, u32 layer_id,disp_layer_info *player);
extern s32 bsp_disp_layer_enable(u32 screen_id, u32 layer_id);
extern s32 bsp_disp_layer_disable(u32 screen_id, u32 layer_id);
extern s32 bsp_disp_layer_is_enabled(u32 screen_id, u32 layer_id);
extern s32 bsp_disp_layer_get_frame_id(u32 screen_id, u32 layer_id);

extern s32 bsp_disp_set_back_color(u32 screen_id, disp_color_info *bk_color);

extern s32 bsp_disp_smcl_enable(u32 screen_id);
extern s32 bsp_disp_smcl_disable(u32 screen_id);
extern s32 bsp_disp_smcl_is_enabled(u32 screen_id);
extern s32 bsp_disp_smcl_set_bright(u32 screen_id, u32 val);
extern s32 bsp_disp_smcl_set_saturation(u32 screen_id, u32 val);
extern s32 bsp_disp_smcl_set_contrast(u32 screen_id, u32 val);
extern s32 bsp_disp_smcl_set_hue(u32 screen_id, u32 val);
extern s32 bsp_disp_smcl_set_mode(u32 screen_id, u32 val);
extern s32 bsp_disp_smcl_set_window(u32 screen_id, disp_window *window);
extern s32 bsp_disp_smcl_get_bright(u32 screen_id);
extern s32 bsp_disp_smcl_get_saturation(u32 screen_id);
extern s32 bsp_disp_smcl_get_contrast(u32 screen_id);
extern s32 bsp_disp_smcl_get_hue(u32 screen_id);
extern s32 bsp_disp_smcl_get_mode(u32 screen_id);
extern s32 bsp_disp_smcl_get_window(u32 screen_id, disp_window *window);

extern s32 bsp_disp_smbl_enable(u32 screen_id);
extern s32 bsp_disp_smbl_disable(u32 screen_id);
extern s32 bsp_disp_smbl_is_enabled(u32 screen_id);
extern s32 bsp_disp_smbl_set_window(u32 screen_id, disp_window *window);
extern s32 bsp_disp_smbl_get_window(u32 screen_id, disp_window *window);

extern s32 bsp_disp_cursor_enable(u32 screen_id);
extern s32 bsp_disp_cursor_disable(u32 screen_id);
extern s32 bsp_disp_cursor_is_enabled(u32 screen_id);
extern s32 bsp_disp_cursor_set_pos(u32 screen_id, disp_position *pos);
extern s32 bsp_disp_cursor_get_pos(u32 screen_id, disp_position *pos);
extern s32 bsp_disp_cursor_set_fb(u32 screen_id, disp_cursor_fb *fb);
extern s32 bsp_disp_cursor_set_palette(u32 screen_id, void *palette, u32 offset, u32 palette_size);

int bsp_disp_feat_get_num_screens(void);
int bsp_disp_feat_get_num_layers(u32 screen_id);
int bsp_disp_feat_get_num_scalers(void);
disp_output_type bsp_disp_feat_get_supported_output_types(u32 screen_id);
enum __disp_layer_feat bsp_disp_feat_get_layer_feats(u32 screen_id,
    disp_layer_mode mode, u32 scaler_index);
enum __disp_enhance_feat bsp_disp_feat_get_enhance_feats(u32 screen_id);
int bsp_disp_feat_get_layer_horizontal_resolution_max(u32 screen_id,
    disp_layer_mode mode, u32 scaler_index);
int bsp_disp_feat_get_layer_vertical_resolution_max(u32 screen_id,
    disp_layer_mode mode, u32 scaler_index);
int bsp_disp_feat_get_layer_horizontal_resolution_min(u32 screen_id,
    disp_layer_mode mode, u32 scaler_index);
int bsp_disp_feat_get_layer_vertical_resolution_min(u32 screen_id,
    disp_layer_mode mode, u32 scaler_index);
int bsp_disp_feat_get_de_flicker_support(u32 screen_id);
int bsp_disp_feat_get_smart_backlight_support(u32 screen_id);
int bsp_disp_feat_get_image_detail_enhance_support(u32 screen_id);

#if defined(__LINUX_PLAT__)
s32 Display_set_fb_timming(u32 sel);
#endif

#endif
