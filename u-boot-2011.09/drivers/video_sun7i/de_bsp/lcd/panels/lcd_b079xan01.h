#ifndef __LCD_B079XAN01_H__
#define __LCD_B079XAN01_H__
#include "panels.h"

extern __lcd_panel_t b079xan01_panel;

void lp079x01_init(__panel_para_t * info);
void lp079x01_exit(__panel_para_t * info);

#define spi_csx_set(v)	(LCD_GPIO_write(0, 3, v))
#define spi_sck_set(v)  (LCD_GPIO_write(0, 0, v))
#define spi_sdi_set(v)  (LCD_GPIO_write(0, 1, v))

#define ssd2828_rst(v)  (LCD_GPIO_write(0, 4, v))
#define panel_rst(v)    (LCD_GPIO_write(0, 2, v))
#define lcd_2828_pd(v)  (LCD_GPIO_write(0, 5, v))	//PH22

#endif
