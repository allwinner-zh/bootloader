
#ifndef __LCD_PANNEL_CFG_H__
#define __LCD_PANNEL_CFG_H__

//#include "dev_lcd.h"
#include "panels/panels.h"
#include "lcd_source_interface.h"

void LCD_set_panel_funs(void);

extern void LCD_OPEN_FUNC(u32 sel, LCD_FUNC func, u32 delay/*ms*/);
extern void LCD_CLOSE_FUNC(u32 sel, LCD_FUNC func, u32 delay/*ms*/);
extern void LCD_delay_ms(u32 ms) ;
extern void LCD_delay_us(u32 ns);
extern void TCON_open(u32 sel);
extern void TCON_close(u32 sel);
extern s32 LCD_PWM_EN(u32 sel, bool b_en);
extern s32 LCD_BL_EN(u32 sel, bool b_en);
extern s32 LCD_POWER_EN(u32 sel, bool b_en);
extern void LCD_CPU_register_irq(u32 sel, void (*Lcd_cpuisr_proc) (void));
extern void LCD_CPU_WR(u32 sel, u32 index, u32 data);
extern void LCD_CPU_WR_INDEX(u32 sel,u32 index);
extern void LCD_CPU_WR_DATA(u32 sel, u32 data);
extern void LCD_CPU_AUTO_FLUSH(u32 sel, bool en);
extern void pwm_clock_enable(u32 sel);
extern void pwm_clock_disable(u32 sel);
extern s32 LCD_POWER_ELDO3_EN(u32 sel, bool b_en, u32 voltage);
extern s32 LCD_POWER_DLDO1_EN(u32 sel, bool b_en, u32 voltage);

extern s32 lcd_iic_write(u8 slave_addr, u8 sub_addr, u8 value);
extern s32 lcd_iic_read(u8 slave_addr, u8 sub_addr, u8* value);

extern s32 lcd_get_panel_para(u32 sel,disp_panel_para * info);

//extern s32 dsi_dcs_wr(u32 sel,u8 cmd,u8* para_p,u32 para_num);
extern s32 dsi_dcs_wr_0para(u32 sel,u8 cmd);
extern s32 dsi_dcs_wr_1para(u32 sel,u8 cmd,u8 para);
extern s32 dsi_dcs_wr_2para(u32 sel,u8 cmd,u8 para1,u8 para2);
extern s32 dsi_dcs_wr_3para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3);
extern s32 dsi_dcs_wr_4para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4);
extern s32 dsi_dcs_wr_5para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4,u8 para5);
extern __s32 dsi_dcs_rd(__u32 sel,__u8	cmd,__u8* para_p,__u32*	num_p);


extern s32 LCD_GPIO_request(u32 sel, u32 io_index);
extern s32 LCD_GPIO_release(u32 sel,u32 io_index);
extern s32 LCD_GPIO_set_attr(u32 sel,u32 io_index, bool b_output);
extern s32 LCD_GPIO_read(u32 sel,u32 io_index);
extern s32 LCD_GPIO_write(u32 sel,u32 io_index, u32 data);

#define BIT0		  0x00000001
#define BIT1		  0x00000002
#define BIT2		  0x00000004
#define BIT3		  0x00000008
#define BIT4		  0x00000010
#define BIT5		  0x00000020
#define BIT6		  0x00000040
#define BIT7		  0x00000080
#define BIT8		  0x00000100
#define BIT9		  0x00000200
#define BIT10		  0x00000400
#define BIT11		  0x00000800
#define BIT12		  0x00001000
#define BIT13		  0x00002000
#define BIT14		  0x00004000
#define BIT15		  0x00008000
#define BIT16		  0x00010000
#define BIT17		  0x00020000
#define BIT18		  0x00040000
#define BIT19		  0x00080000
#define BIT20		  0x00100000
#define BIT21		  0x00200000
#define BIT22		  0x00400000
#define BIT23		  0x00800000
#define BIT24		  0x01000000
#define BIT25		  0x02000000
#define BIT26		  0x04000000
#define BIT27		  0x08000000
#define BIT28		  0x10000000
#define BIT29		  0x20000000
#define BIT30		  0x40000000
#define BIT31		  0x80000000

#define sys_get_wvalue(n)   (*((volatile u32 *)(n)))          /* word input */
#define sys_put_wvalue(n,c) (*((volatile u32 *)(n))  = (c))   /* word output */

#endif

