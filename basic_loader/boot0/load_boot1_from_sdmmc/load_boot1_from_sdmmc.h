/*
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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

#ifndef  __load_boot1_from_sdmmc_h
#define  __load_boot1_from_sdmmc_h

//SD卡相关数据结构
typedef struct _boot_sdcard_info_t
{
	__s32               card_ctrl_num;                //总共的卡的个数
	__s32				boot_offset;                  //指定卡启动之后，逻辑和物理分区的管理
	__s32 				card_no[4];                   //当前启动的卡号, 16-31:GPIO编号，0-15:实际卡控制器编号
	__s32 				speed_mode[4];                //卡的速度模式，0：低速，其它：高速
	__s32				line_sel[4];                  //卡的线制，0: 1线，其它，4线
	__s32				line_count[4];                //卡使用线的个数
}
boot_sdcard_info_t;
/*******************************************************************************
*函数名称: load_boot1_from_spinor
*函数原型：int32 load_boot1_from_spinor( void )
*函数功能: 将一份好的Boot1从spi nor flash中载入到SRAM中。
*入口参数: void
*返 回 值: OK                         载入并校验成功
*          ERROR                      载入并校验失败
*备    注:
*******************************************************************************/
extern __s32 load_boot1_from_sdmmc( char *buf);



#endif     //  ifndef __load_boot1_from_spi_nor_h

/* end of load_boot1_from_spinor.h */
