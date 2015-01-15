/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Young <guoyingyang@allwinnertech.com>
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
#ifndef  __CPU_SWITCH_H__
#define  __CPU_SWITCH_H__

#include <asm/arch/platform.h>


#define CLUSTER0_CTRL_REG0			(CPUCFG_BASE + 0x00)
#define CLUSTER0_CTRL_REG1			(CPUCFG_BASE + 0x04)
#define CLUSTER1_CTRL_REG0			(CPUCFG_BASE + 0x10)
#define CLUSTER1_CTRL_REG1			(CPUCFG_BASE + 0x14)
#define C0_CPUX_STAT_REG			(CPUCFG_BASE + 0x30)
#define C1_CPUX_STAT_REG			(CPUCFG_BASE + 0x34)
#define C0CORE_RST_CTRL_REG			(CPUCFG_BASE + 0x80)
#define C1CORE_RST_CTRL_REG			(CPUCFG_BASE + 0x84)

#define R_C0_RST_CTRL_REG		    (RPRCM_BASE +  0x04)
#define R_C1_RST_CTRL_REG		    (RPRCM_BASE +  0x08)
#define R_C0_POWOFF_REG			    (RPRCM_BASE +  0x100)
#define R_C1_POWOFF_GATING_REG	    (RPRCM_BASE +  0x104)
#define R_C0CPU0_PWR_SWITCH		    (RPRCM_BASE +  0x140)
#define R_C0CPU1_PWR_SWITCH		    (RPRCM_BASE +  0x144)
#define R_C0CPU2_PWR_SWITCH		    (RPRCM_BASE +  0x148)
#define R_C0CPU3_PWR_SWITCH		    (RPRCM_BASE +  0x14C)
#define R_C1_CORE_PER_BASE		    (RPRCM_BASE +  0x150)
#define R_CPU_SOFT_ENTRY_REG	    (RPRCM_BASE +  0x164)


#define SUNXI_CLUSTER_CPU_STATUS(cluster)         (0x30 + (cluster)*0x4)
#define SUNXI_CPU_RST_CTRL(cluster)               (0x80 + (cluster)*0x4)
#define SUNXI_CLUSTER_CTRL0(cluster)              (0x00 + (cluster)*0x10)
#define SUNXI_CLUSTER_CTRL1(cluster)              (0x04 + (cluster)*0x10)

#define SUNXI_CPU_PWR_CLAMP(cluster, cpu)         (0x140 + (cluster*4 + cpu)*0x04)
#define SUNXI_CPU_PWR_CLAMP_STATUS(cluster, cpu)  (0x64  + (cluster*4 + cpu)*0x40)
#define SUNXI_CLUSTER_PWROFF_GATING(cluster)      (0x100 + (cluster)*0x04)
#define SUNXI_CLUSTER_PWRON_RESET(cluster)        (0x04  + (cluster)*0x04)


#define A15_CLUSTER  1
#define A7_CLUSTER   0

#define BOOT_A15_FLAG           0xA5
#define BOOT_A7_FLAG            0xA7

#define SUNXI_CHIP_REV(p, v)  (p + v)

/* sunxi platform chip version policy:
 * high 16bit value: platfrom number(Such as: sun8i/sun9i/..)
 * low  16bit value: chip version (Such as: VERSION_A,VERSION_B)
 */
#define SUNXI_CHIP_MASK  (0xFFFF0000)
#define SUNXI_CHIP_SUN9I (0x16390000)

/* sun9i chip versions define */
#define SUN9I_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN9I, 0)
#define SUN9I_REV_B SUNXI_CHIP_REV(SUNXI_CHIP_SUN9I, 1)


extern void jump_to_resume(void);
extern void save_runtime_context(u32* addr);
extern int  get_cpu_id(void);
extern void switch_to_a15(special_gpio_cfg a15_power_gpio);

extern int get_cluster_id(void);

#endif  // __SUN9IW1_CORE_H__
