/* linux/drivers/video/sunxi/lcd/dev_lcd.c
 *
 * Copyright (c) 2013 Allwinnertech Co., Ltd.
 * Author: Tyle <tyle@allwinnertech.com>
 *
 * LCD driver for sunxi platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "dev_lcd.h"

struct sunxi_lcd_drv g_lcd_drv;

int lcd_init(void)
{
	sunxi_disp_get_source_ops(&g_lcd_drv.src_ops);
	LCD_set_panel_funs();

	return 0;
}
