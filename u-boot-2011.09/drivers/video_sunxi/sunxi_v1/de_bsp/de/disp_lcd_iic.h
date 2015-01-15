#include "disp_display_i.h"
#include "disp_lcd.h"
#include "disp_display.h"

//---------------------------------------------------------
__s32 bsp_disp_lcd_iic_write(__u8 slave_addr, __u8 sub_addr, __u8 value);
__s32 bsp_disp_lcd_iic_read(__u8 slave_addr, __u8 sub_addr, __u8* value);

