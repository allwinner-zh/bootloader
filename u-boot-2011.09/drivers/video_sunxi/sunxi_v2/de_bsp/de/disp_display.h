
#ifndef __DISP_DISPLAY_H__
#define __DISP_DISPLAY_H__

#include "disp_private.h"

typedef struct
{
	bool                  have_cfg_reg;
	u32                   cache_flag;
	u32                   cfg_cnt;
#ifdef __LINUX_PLAT__
	spinlock_t              flag_lock;
#endif
	bool                  vsync_event_en;
	bool                  dvi_enable;
}__disp_screen_t;

typedef struct
{
	__disp_bsp_init_para    init_para;//para from driver
	__disp_screen_t         screen[3];
	u32                   print_level;
	u32                   lcd_registered[3];
	u32                   hdmi_registered;
	u32                   edp_registered;
}__disp_dev_t;

extern __disp_dev_t gdisp;
s32 disp_init_connections(void);
extern s32 scaler_close(u32 scaler_id);
extern s32 scaler_exit(void);
extern s32 disp_lcd_event_proc(void *parg);
extern s32 manager_event_proc(void *parg);

void LCD_OPEN_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);
void LCD_CLOSE_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);

#endif
