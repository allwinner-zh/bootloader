/*
* (C) Copyright 2007-2013
* Allwinner Technology Co., Ltd. <www.allwinnertech.com>
* Martin zheng <zhengjiewen@allwinnertech.com>
*
* See file CREDITS for list of people who contributed to this
* project.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/
#ifndef  _STANDBY_I_H_
#define  _STANDBY_I_H_
#include <linux/types.h>
#include <asm/arch/dram.h>

/* base drivers    */
#define EGON2_EMOD_TYPE_DRV_NOP               0x00        /* 0号驱动，系统保留                                            */
#define EGON2_EMOD_TYPE_DRV_RTC               0x01        /* rtc驱动type编号                                              */
#define EGON2_EMOD_TYPE_DRV_KEY               0x02        /* 键盘驱动type编号                                             */
#define EGON2_EMOD_TYPE_DRV_DISP              0x03        /* 显示驱动type编号                                             */
#define EGON2_EMOD_TYPE_DRV_MOUSE             0x04        /* 鼠标驱动type编号                                             */
#define EGON2_EMOD_TYPE_DRV_TP                0x05        /* touch pannel驱动type编号                                     */
#define EGON2_EMOD_TYPE_DRV_FM                0x07        /* FM radio driver编号                                          */
#define EGON2_EMOD_TYPE_DRV_POWER             0x08        /* PWOER    driver编号                                          */
/* storage drivers */
#define EGON2_EMOD_TYPE_DRV_NAND              0x09
#define EGON2_EMOD_TYPE_DRV_SDMMC             0x0a
#define EGON2_EMOD_TYPE_DRV_NOR               0x0b
#define EGON2_EMOD_TYPE_DRV_HDD               0x0c
#define EGON2_EMOD_TYPE_DRV_MS                0x0d
/* bus drivers */
#define EGON2_EMOD_TYPE_DRV_IIS               0x10
#define EGON2_EMOD_TYPE_DRV_SPI               0x11
#define EGON2_EMOD_TYPE_DRV_IIC               0x12
#define EGON2_EMOD_TYPE_DRV_IR                0x13
#define EGON2_EMOD_TYPE_DRV_HDMI              0x14
#define EGON2_EMOD_TYPE_DRV_UPRL              0x15
/* usb driver (0~15)                                */
#define EGON2_EMOD_TYPE_DRV_USBD_BASE         0x16
#define EGON2_EMOD_TYPE_DRV_USBH_BASE         0x17
/* new added driver */
#define EGON2_EMOD_TYPE_DRV_MP                0x18


#define MOD_ENTER_STANDBY				  0x0
#define MOD_EXIT_STANDBY				  0x1



#define EMOD_COUNT_MAX                        64
typedef struct _boot_core_para_t
{
    __u32 user_set_clock; /*the unit of frequency is M*/
    __u32 user_set_core_vol; /*mV*/
    __u32 vol_threshold; /*on the threashold voltage*/
}boot_core_para_t;

extern  int standby_int_init(void);
extern  int standby_int_exit(void);
extern  int standby_int_query(void);
extern int standby_axp_probe_usb(void);

extern  int  standby_clock_store(void);
extern  int  standby_clock_restore(void);
extern  int  standby_clock_to_source(int clock_source);
extern  void standby_clock_plldisable(void);
extern  void standby_clock_divsetto0(void);
extern  void standby_clock_divsetback(void);
extern  void standby_clock_drampll_ouput(int op);

extern  void standby_clock_apb1_to_source(int clock);
extern  void standby_clock_24m_op(int op);

extern  void  boot_store_sp(void);
extern  void  boot_restore_sp(void);
extern  void  boot_set_sp(void);
extern  void  boot_halt(void);


extern  __s32  dram_power_save_process(boot_dram_para_t* standby_dram_para);
extern  __u32 dram_power_up_process(boot_dram_para_t* standby_dram_para);

extern  void standby_tmr_enable_watchdog(void);
extern  void standby_tmr_disable_watchdog(void);

#endif  // _STANDBY_I_H_


