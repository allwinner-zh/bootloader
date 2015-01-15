/*
 * (C) Copyright 2007-2013
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

#ifndef __SUNXI_INTC_H__
#define __SUNXI_INTC_H__

/* mask */
#define  AW_IRQ_NMI              0
#define  AW_IRQ_UART0            1
#define  AW_IRQ_UART1            2
#define  AW_IRQ_UART2            3
#define  AW_IRQ_UART3            4
#define  AW_IRQ_IR0              5
#define  AW_IRQ_IR1              6
#define  AW_IRQ_TWI0             7
#define  AW_IRQ_TWI1             8
#define  AW_IRQ_TWI2             9

#define  AW_IRQ_SPI0             10
#define  AW_IRQ_SPI1             11
#define  AW_IRQ_SPI2             12
#define  AW_IRQ_SPDIF            13
#define  AW_IRQ_AC97             14
#define  AW_IRQ_TS               15
#define  AW_IRQ_IIS              16

#define  AW_IRQ_UART4            17
#define  AW_IRQ_UART5            18
#define  AW_IRQ_UART6            19
#define  AW_IRQ_UART7            20

#define  AW_IRQ_KEYPAD           21
#define  AW_IRQ_TIMER0           22
#define  AW_IRQ_TIMER1           23
#define  AW_IRQ_TIMER245         24
#define  AW_IRQ_TIMER3           25
#define  AW_IRQ_CAN              26
#define  AW_IRQ_DMA              27
#define  AW_IRQ_PIO              28
#define  AW_IRQ_TP               29

#define  AW_IRQ_ADDA             30
#define  AW_IRQ_LRADC            31
#define  AW_IRQ_SDMMC0           32
#define  AW_IRQ_SDMMC1           33
#define  AW_IRQ_SDMMC2           34
#define  AW_IRQ_SDMMC3           35
#define  AW_IRQ_MS               36
#define  AW_IRQ_NAND             37
#define  AW_IRQ_USB_OTG          38
#define  AW_IRQ_USB1             39
#define  AW_IRQ_USB2             40

#define  AW_IRQ_SCR              41

#define  AW_IRQ_CSI0             42
#define  AW_IRQ_CSI1             43
#define  AW_IRQ_LCD0             44
#define  AW_IRQ_LCD1             45
#define  AW_IRQ_MP               46
#define  AW_IRQ_DEBE0			47
#define  AW_IRQ_DEFE0           47
#define  AW_IRQ_DEBE1			48
#define  AW_IRQ_DEFE1           48
#define  AW_IRQ_PMU              49

#define  AW_IRQ_SPI3             50
#define  AW_IRQ_TZASC            51
#define  AW_IRQ_PATA             52
#define  AW_IRQ_VE               53

#define  AW_IRQ_SS               54
#define  AW_IRQ_EMAC             55
#define  AW_IRQ_SATA             56
#define  AW_IRQ_GPS              57
#define  AW_IRQ_HDMI             58
#define  AW_IRQ_TVE01            59

#define  AW_IRQ_ACE              60
#define  AW_IRQ_TVD              61
#define  AW_IRQ_PS0              62
#define  AW_IRQ_PS1              63
#define  AW_IRQ_USB3             64
#define  AW_IRQ_USB4             65
#define  AW_IRQ_PLE              66
#define  AW_IRQ_TIMER4           67
#define  AW_IRQ_TIMER5           68
#define  AW_IRQ_GPU_GP           69

#define  AW_IRQ_GPU_MP           70
#define  AW_IRQ_GPU_PP0          71
#define  AW_IRQ_GPU_PPMMU0       72
#define  AW_IRQ_GPU_PMU          73
#define  AW_IRQ_GPU_RSV0         74
#define  AW_IRQ_GPU_RSV1         75
#define  AW_IRQ_GPU_RSV2         76
#define  AW_IRQ_GPU_RSV3         77
#define  AW_IRQ_GPU_RSV4         78
#define  AW_IRQ_GPU_RSV5         79
#define  AW_IRQ_GPU_RSV6         80

#define  INT_IRQ_NUM             (AW_IRQ_GPU_RSV6+1)

#define  INTC_REG_VCTR             ( SUNXI_INTC_BASE + 0x00)
#define  INTC_REG_VTBLBADDR        ( SUNXI_INTC_BASE + 0x04)

#define  INTC_REG_NMI_CTRL         ( SUNXI_INTC_BASE + 0x0C)

#define  INTC_REG_IRQ_PENDCLR0     ( SUNXI_INTC_BASE + 0x10)
#define  INTC_REG_IRQ_PENDCLR1     ( SUNXI_INTC_BASE + 0x14)
#define  INTC_REG_IRQ_PENDCLR2     ( SUNXI_INTC_BASE + 0x18)

#define  INTC_REG_FIQ_PENDCLR0     ( SUNXI_INTC_BASE + 0x20)
#define  INTC_REG_FIQ_PENDCLR1     ( SUNXI_INTC_BASE + 0x24)
#define  INTC_REG_FIQ_PENDCLR2     ( SUNXI_INTC_BASE + 0x28)

#define  INTC_REG_IRQ_SEL0         ( SUNXI_INTC_BASE + 0x30)
#define  INTC_REG_IRQ_SEL1         ( SUNXI_INTC_BASE + 0x34)
#define  INTC_REG_IRQ_SEL2         ( SUNXI_INTC_BASE + 0x38)

#define  INTC_REG_ENABLE0          ( SUNXI_INTC_BASE + 0x40)
#define  INTC_REG_ENABLE1          ( SUNXI_INTC_BASE + 0x44)
#define  INTC_REG_ENABLE2          ( SUNXI_INTC_BASE + 0x48)

#define  INTC_REG_MASK0            ( SUNXI_INTC_BASE + 0x50)
#define  INTC_REG_MASK1            ( SUNXI_INTC_BASE + 0x54)
#define  INTC_REG_MASK2            ( SUNXI_INTC_BASE + 0x58)

#define  INTC_REG_RESP0            ( SUNXI_INTC_BASE + 0x60)
#define  INTC_REG_RSEP1            ( SUNXI_INTC_BASE + 0x64)
#define  INTC_REG_RESP2            ( SUNXI_INTC_BASE + 0x68)

#define  INTC_REG_FF0              ( SUNXI_INTC_BASE + 0x70)
#define  INTC_REG_FF1              ( SUNXI_INTC_BASE + 0x74)
#define  INTC_REG_FF2              ( SUNXI_INTC_BASE + 0x78)

#define  INTC_REG_PRIO0            ( SUNXI_INTC_BASE + 0x80)
#define  INTC_REG_PRIO1            ( SUNXI_INTC_BASE + 0x84)
#define  INTC_REG_PRIO2            ( SUNXI_INTC_BASE + 0x88)
#define  INTC_REG_PRIO3            ( SUNXI_INTC_BASE + 0x8C)
#define  INTC_REG_PRIO4            ( SUNXI_INTC_BASE + 0x90)


extern void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data);
extern void irq_free_handler(int irq);
extern int irq_enable(int irq_no);
extern int irq_disable(int irq_no);

int arch_interrupt_init (void);

int arch_interrupt_exit (void);


#endif
