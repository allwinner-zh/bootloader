#include "lcd_source_interface.h"
#include "lcd_panel_cfg.h"

struct sunxi_lcd_drv g_lcd_drv;
extern int sunxi_disp_get_source_ops(struct sunxi_disp_source_ops *src_ops);
extern void LCD_set_panel_funs(void);

int lcd_init(void)
{
	sunxi_disp_get_source_ops(&g_lcd_drv.src_ops);
	LCD_set_panel_funs();

	return 0;
}



