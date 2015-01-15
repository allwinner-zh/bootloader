#include "lcd_b079xan01.h"
#include "panels.h"

static void LCD_power_on(__u32 sel);
static void LCD_power_off(__u32 sel);
static void LCD_bl_open(__u32 sel);
static void LCD_bl_close(__u32 sel);

static void LCD_panel_init(__u32 sel);
static void LCD_panel_exit(__u32 sel);

void spi_24bit_3wire(__u32 tx)
{
	__u8 i;

	spi_csx_set(0);

	for(i=0;i<24;i++) {
		sunxi_lcd_delay_us(1);
		spi_sck_set(0);
		sunxi_lcd_delay_us(1);
		if(tx & 0x800000)
		spi_sdi_set(1);
		else
		spi_sdi_set(0);
		sunxi_lcd_delay_us(1);
		spi_sck_set(1);
		sunxi_lcd_delay_us(1);
		tx <<= 1;
	}
	spi_sdi_set(1);
	sunxi_lcd_delay_us(1);
	spi_csx_set(1);
	sunxi_lcd_delay_us(3);
}
void lp079x01_init(__panel_para_t * info)
{
        ssd2828_rst(0);
        panel_rst(0);
        lcd_2828_pd(0);
        sunxi_lcd_delay_ms(20);
        ssd2828_rst(1);
        panel_rst(1);
        sunxi_lcd_delay_ms(50);

	spi_24bit_3wire(0x7000B1);  //VSA=50, HAS=64
	spi_24bit_3wire(0x723240);

	spi_24bit_3wire(0x7000B2); //VBP=30+50, HBP=56+64
	spi_24bit_3wire(0x725078);

	spi_24bit_3wire(0x7000B3); //VFP=36, HFP=60
	spi_24bit_3wire(0x72243C);

	spi_24bit_3wire(0x7000B4); //HACT=768
	spi_24bit_3wire(0x720300);

	spi_24bit_3wire(0x7000B5); //VACT=1240
	spi_24bit_3wire(0x720400);

	spi_24bit_3wire(0x7000B6);
	//todo, cfg to 18bpp packed
	spi_24bit_3wire(0x72000B); //0x720009:burst mode, 18bpp packed
														 //0x72000A:burst mode, 18bpp loosely packed
							   						 //0x72000B:burst mode, 24bpp 
							   						 
	spi_24bit_3wire(0x7000DE); //no of lane=4
	spi_24bit_3wire(0x720003);

	spi_24bit_3wire(0x7000D6); //RGB order and packet number in blanking period
	spi_24bit_3wire(0x720005);

	spi_24bit_3wire(0x7000B9); //disable PLL
	spi_24bit_3wire(0x720000);

	spi_24bit_3wire(0x7000BA); //lane speed=560
	spi_24bit_3wire(0x72C015); //may modify according to requirement, 500Mbps to  560Mbps, (n+1)*12M
	
	spi_24bit_3wire(0x7000BB); //LP clock
	spi_24bit_3wire(0x720008);

	spi_24bit_3wire(0x7000B9); //enable PPL
	spi_24bit_3wire(0x720001);

	spi_24bit_3wire(0x7000c4); //enable BTA
	spi_24bit_3wire(0x720001);

	spi_24bit_3wire(0x7000B7); //enter LP mode
	spi_24bit_3wire(0x720342);

	spi_24bit_3wire(0x7000B8); //VC
	spi_24bit_3wire(0x720000);

	spi_24bit_3wire(0x7000BC); //set packet size
	spi_24bit_3wire(0x720000);

	spi_24bit_3wire(0x700011); //sleep out cmd

	sunxi_lcd_delay_ms(200);
	spi_24bit_3wire(0x700029); //display on

	sunxi_lcd_delay_ms(200);
	spi_24bit_3wire(0x7000B7); //video mode on
	spi_24bit_3wire(0x72030b);

}


void lp079x01_exit(__panel_para_t * info)
{
	spi_24bit_3wire(0x7000B7); //enter LP mode
	spi_24bit_3wire(0x720342);

	sunxi_lcd_delay_ms(50);
	spi_24bit_3wire(0x700028); //display off
	sunxi_lcd_delay_ms(10);
	spi_24bit_3wire(0x700010); //sleep in cmd
	sunxi_lcd_delay_ms(20);
	ssd2828_rst(0);
}

static void LCD_cfg_panel_info(__panel_extend_para_t * info)
{
	__u32 i = 0, j=0;
	__u32 items;
	__u8 lcd_gamma_tbl[][2] =
	{
		//{input value, corrected value}
		{0, 0},
		{15, 15},
		{30, 30},
		{45, 45},
		{60, 60},
		{75, 75},
		{90, 90},
		{105, 105},
		{120, 120},
		{135, 135},
		{150, 150},
		{165, 165},
		{180, 180},
		{195, 195},
		{210, 210},
		{225, 225},
		{240, 240},
		{255, 255},
	};

	__u8 lcd_bright_curve_tbl[][2] =
	{
		//{input value, corrected value}
		{0    ,0  },//0
		{15   ,3  },//0
		{30   ,6  },//0
		{45   ,9  },// 1
		{60   ,12  },// 2
		{75   ,16  },// 5
		{90   ,22  },//9
		{105   ,28 }, //15
		{120  ,36 },//23
		{135  ,44 },//33
		{150  ,54 },
		{165  ,67 },
		{180  ,84 },
		{195  ,108},
		{210  ,137},
		{225 ,171},
		{240 ,210},
		{255 ,255},
	};

	__u32 lcd_cmap_tbl[2][3][4] = {
	{
		{LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
		{LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
		{LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
		{LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
		{LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
		},
	};

	memset(info,0,sizeof(__panel_extend_para_t));

	items = sizeof(lcd_gamma_tbl)/2;
	for(i=0; i<items-1; i++) {
		__u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

		for(j=0; j<num; j++) {
			__u32 value = 0;

			value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

	items = sizeof(lcd_bright_curve_tbl)/2;
	for(i=0; i<items-1; i++) {
		__u32 num = lcd_bright_curve_tbl[i+1][0] - lcd_bright_curve_tbl[i][0];

		for(j=0; j<num; j++) {
			__u32 value = 0;

			value = lcd_bright_curve_tbl[i][1] + ((lcd_bright_curve_tbl[i+1][1] - lcd_bright_curve_tbl[i][1]) * j)/num;
			info->lcd_bright_curve_tbl[lcd_bright_curve_tbl[i][0] + j] = value;
		}
	}
	info->lcd_bright_curve_tbl[255] = lcd_bright_curve_tbl[items-1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static __s32 LCD_open_flow(__u32 sel)
{
	LCD_OPEN_FUNC(sel, LCD_power_on, 0);   //open lcd power, and delay 0ms
	LCD_OPEN_FUNC(sel, LCD_panel_init, 10);   //open lcd power, than delay 10ms
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 50);     //open lcd controller, and delay 50ms
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

	return 0;
}

static __s32 LCD_close_flow(__u32 sel)
{
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);       //close lcd backlight, and delay 0ms
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
	LCD_CLOSE_FUNC(sel, LCD_panel_exit,	100);   //open lcd power, than delay 200ms
	LCD_CLOSE_FUNC(sel, LCD_power_off, 100);   //close lcd power, and delay 500ms

	return 0;
}

static void LCD_power_on(__u32 sel)
{
	sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power0
	sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(__u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power0
}

static void LCD_bl_open(__u32 sel)
{
	sunxi_lcd_pwm_enable(sel);//open pwm module
	sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
}

static void LCD_bl_close(__u32 sel)
{
	sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
	sunxi_lcd_pwm_disable(sel);//close pwm module
}

static void LCD_panel_init(__u32 sel)
{
	__panel_para_t *info = OSAL_malloc(sizeof(__panel_para_t));

	sunxi_lcd_get_panel_para(sel, info);
	lp079x01_init(info);
	OSAL_free(info);
	return;
}

static void LCD_panel_exit(__u32 sel)
{
	__panel_para_t *info = OSAL_malloc(sizeof(__panel_para_t));

	sunxi_lcd_get_panel_para(sel, info);
	lp079x01_exit(info);
	OSAL_free(info);
	return;
}

__lcd_panel_t b079xan01_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "b079xan01",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		/*.lcd_user_defined_func = LCD_user_defined_func,*/
	},
};


