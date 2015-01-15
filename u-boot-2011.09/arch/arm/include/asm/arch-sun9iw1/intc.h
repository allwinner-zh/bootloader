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

#define AW_IRQ_GIC_START    (32)

#ifndef CONFIG_A39_FPGA	//chip irq mapping

#define GIC_SRC_SPI(_n)		(32 + (_n))

#define AW_IRQ_UART0	   GIC_SRC_SPI(0)   // (32)
#define AW_IRQ_UART1	   GIC_SRC_SPI(1)   // (33)
#define AW_IRQ_UART2       GIC_SRC_SPI(2)   // (34)
#define AW_IRQ_UART3       GIC_SRC_SPI(3)   // (35)
#define AW_IRQ_UART4       GIC_SRC_SPI(4)   // (36)
#define AW_IRQ_UART5       GIC_SRC_SPI(5)   // (37)
#define AW_IRQ_TWI0        GIC_SRC_SPI(6  ) // (38)
#define AW_IRQ_TWI1        GIC_SRC_SPI(7  ) // (39)
#define AW_IRQ_TWI2        GIC_SRC_SPI(8  ) // (40)
#define AW_IRQ_TWI3        GIC_SRC_SPI(9  ) // (41)
#define AW_IRQ_TWI4        GIC_SRC_SPI(10 ) // (42)

#define AW_IRQ_PAEINT      GIC_SRC_SPI(11 ) // (43)
#define AW_IRQ_SPDIF       GIC_SRC_SPI(12 ) // (44)
#define AW_IRQ_IIS0	       GIC_SRC_SPI(13 ) // (45)
#define AW_IRQ_IIS1		   GIC_SRC_SPI(14 ) // (46)
#define AW_IRQ_PBEINT      GIC_SRC_SPI(15 ) // (47)
#define AW_IRQ_PEEINT      GIC_SRC_SPI(16 ) // (48)
#define AW_IRQ_PGEINT      GIC_SRC_SPI(17 ) // (49)

#define AW_IRQ_TIMER0      GIC_SRC_SPI(18 ) // (50)
#define AW_IRQ_TIMER1      GIC_SRC_SPI(19 ) // (51)
#define AW_IRQ_TIMER2      GIC_SRC_SPI(20 ) // (52)
#define AW_IRQ_TIMER3      GIC_SRC_SPI(21 ) // (53)
#define AW_IRQ_TIMER4      GIC_SRC_SPI(22 ) // (54)
#define AW_IRQ_TIMER5      GIC_SRC_SPI(23 ) // (55)

#define AW_IRQ_WATCHDOG    GIC_SRC_SPI(24 ) // (56)
#define AW_IRQ_LRADC       GIC_SRC_SPI(30 ) // (62)
#define AW_IRQ_NMI         GIC_SRC_SPI(32 ) // (64)
#define AW_IRQ_RTIMER0     GIC_SRC_SPI(33 ) // (65)
#define AW_IRQ_RTIMER1     GIC_SRC_SPI(34 ) // (66)
#define AW_IRQ_RDMA	       GIC_SRC_SPI(35 ) // (67)
#define AW_IRQ_RWATCHDOG   GIC_SRC_SPI(36 ) // (68)
#define AW_IRQ_RCIR        GIC_SRC_SPI(37 ) // (69)
#define AW_IRQ_RUART       GIC_SRC_SPI(38 ) // (70)
#define AW_IRQ_RRSB        GIC_SRC_SPI(39 ) // (71)
#define AW_IRQ_RPS2_0    	GIC_SRC_SPI(40 ) // (72)
#define AW_IRQ_RPS2_1     	GIC_SRC_SPI(41 ) // (73)
#define AW_IRQ_RI2S0       GIC_SRC_SPI(42 ) // (74)
#define AW_IRQ_1WIRE       GIC_SRC_SPI(43 ) // (75)
#define AW_IRQ_RTWI0       GIC_SRC_SPI(44 ) // (76)
#define AW_IRQ_RPLEINT     GIC_SRC_SPI(45 ) // (77)
#define AW_IRQ_RPMEINT     GIC_SRC_SPI(46 ) // (78)
#define AW_IRQ_RTWI1	    GIC_SRC_SPI(47 ) // (79)
#define AW_IRQ_TWD			GIC_SRC_SPI(48 ) // (80)
#define AW_IRQ_MBOX        GIC_SRC_SPI(49 ) // (81)
#define AW_IRQ_DMA         GIC_SRC_SPI(50 ) // (82)
#define AW_IRQ_HSTMR0      GIC_SRC_SPI(51 ) // (83)
#define AW_IRQ_HSTMR1      GIC_SRC_SPI(52 ) // (84)
#define AW_IRQ_HSTMR2      GIC_SRC_SPI(53 ) // (85)
#define AW_IRQ_HSTMR3      GIC_SRC_SPI(54 ) // (86)
#define AW_IRQ_HSTMR4      GIC_SRC_SPI(55 ) // (86)
#define AW_IRQ_TZASC       GIC_SRC_SPI(56 ) // (88)

#define AW_IRQ_VE          GIC_SRC_SPI(58 ) // (90)
//#definAW_IRQ_			GIC_SRC_SPI(59 ) // (91)
#define AW_IRQ_MMC0        GIC_SRC_SPI(60 ) // (92)
#define AW_IRQ_MMC1        GIC_SRC_SPI(61 ) // (93)
#define AW_IRQ_MMC2        GIC_SRC_SPI(62 ) // (94)
#define AW_IRQ_MMC3        GIC_SRC_SPI(63 ) // (95)

#define AW_IRQ_SPI0        GIC_SRC_SPI(65 ) // (97 )
#define AW_IRQ_SPI1        GIC_SRC_SPI(66 ) // (98 )
#define AW_IRQ_SPI2        GIC_SRC_SPI(67 ) // (99 )
#define AW_IRQ_SPI3        GIC_SRC_SPI(68 ) // (100)
#define AW_IRQ_NAND1       GIC_SRC_SPI(69 ) // (101)
#define AW_IRQ_NAND0       GIC_SRC_SPI(70 ) // (102)
#define AW_IRQ_USB_OTG     GIC_SRC_SPI(71 ) // (103)
#define AW_IRQ_USBEHCI0    GIC_SRC_SPI(72 ) // (104)
#define AW_IRQ_USBOHCI0    GIC_SRC_SPI(73 ) // (105)
#define AW_IRQ_USBEHCI1    GIC_SRC_SPI(74 ) // (106)
#define AW_IRQ_USBOHCI1    GIC_SRC_SPI(75 ) // (107)
#define AW_IRQ_USBEHCI2    GIC_SRC_SPI(76 ) // (108)
#define AW_IRQ_USBOHCI2    GIC_SRC_SPI(77 ) // (109)

#define AW_IRQ_SS          GIC_SRC_SPI(80 ) // (112)
#define AW_IRQ_TSC         GIC_SRC_SPI(81 ) // (113)
#define AW_IRQ_GMAC        GIC_SRC_SPI(82 ) // (114)
#define AW_IRQ_MP          GIC_SRC_SPI(83 ) // (115)
#define AW_IRQ_CSI0        GIC_SRC_SPI(84 ) // (116)
#define AW_IRQ_CSI1        GIC_SRC_SPI(85 ) // (117)
#define AW_IRQ_LCD0        GIC_SRC_SPI(86 ) // (118)
#define AW_IRQ_LCD1        GIC_SRC_SPI(87 ) // (119)
#define AW_IRQ_HDMI        GIC_SRC_SPI(88 ) // (120)
#define AW_IRQ_MIPIDSI     GIC_SRC_SPI(89 ) // (121)
#define AW_IRQ_MIPICSI     GIC_SRC_SPI(90 ) // (122)
#define AW_IRQ_DRC01       GIC_SRC_SPI(91 ) // (123)
#define AW_IRQ_DEU01       GIC_SRC_SPI(92 ) // (124)
#define AW_IRQ_DEFE0       GIC_SRC_SPI(93 ) // (125)
#define AW_IRQ_DEFE1       GIC_SRC_SPI(94 ) // (126)
#define AW_IRQ_DEBE0       GIC_SRC_SPI(95 ) // (127)
#define AW_IRQ_DEBE1       GIC_SRC_SPI(96 ) // (128)
#define AW_IRQ_GPU         GIC_SRC_SPI(97 ) // (129)
#define AW_IRQ_GPUPWR	   GIC_SRC_SPI(98 ) // (130)

#define AW_IRQ_FD          GIC_SRC_SPI(108) // (140)
#define AW_IRQ_GPADC       GIC_SRC_SPI(109) // (141)
#define AW_IRQ_SATA        GIC_SRC_SPI(110) // (142)
#define AW_IRQ_HSI         GIC_SRC_SPI(111) // (143)
#define AW_IRQ_AC97        GIC_SRC_SPI(112) // (144)
#define AW_IRQ_CIR_TX      GIC_SRC_SPI(113) // (145)
#define AW_IRQ_PNEINT      GIC_SRC_SPI(114) // (146)
#define AW_IRQ_THS		   GIC_SRC_SPI(115) // (147)
#define AW_IRQ_DEBE2       GIC_SRC_SPI(116) // (148)
#define AW_IRQ_DEFE2       GIC_SRC_SPI(117) // (149)
#define AW_IRQ_EDP     	   GIC_SRC_SPI(118) // (150)
#define AW_IRQ_PHEINT      GIC_SRC_SPI(120) // (152)
#define AW_IRQ_CSI0CCI     GIC_SRC_SPI(122) // (154)
#define AW_IRQ_CSI1CCI     GIC_SRC_SPI(123) // (155)

#define GIC_IRQ_NUM			(AW_IRQ_CSI1CCI + 1)

#else
#define GIC_SRC_SPI(_n)		(32 + (_n))

#define AW_IRQ_NMI         GIC_SRC_SPI(0)   // only use to standy

//test for 1639 fpga/////////
//#define AW_IRQ_LCD0        GIC_SRC_SPI(0) 	// (32)
//#define AW_IRQ_LCD1		   GIC_SRC_SPI(0)	// (32)
#define AW_IRQ_LCD0        GIC_SRC_SPI(86 ) // (118)
#define AW_IRQ_LCD1        GIC_SRC_SPI(87 ) // (119)
////////////////////////////
#define AW_IRQ_EDP         GIC_SRC_SPI(0)	// (32)
#define AW_IRQ_DEBE0       GIC_SRC_SPI(1)	// (33)
#define AW_IRQ_DEBE1       GIC_SRC_SPI(1)	// (33)
#define AW_IRQ_DEBE2       GIC_SRC_SPI(1)	// (33)
#define AW_IRQ_DEFE0       GIC_SRC_SPI(2)	// (34)
#define AW_IRQ_DEFE1       GIC_SRC_SPI(2)	// (34)
#define AW_IRQ_DEFE2       GIC_SRC_SPI(2)	// (34)
#define AW_IRQ_DRC01       GIC_SRC_SPI(3)	// (35)
#define AW_IRQ_DEU01       GIC_SRC_SPI(3)  // (35)
#define AW_IRQ_CSI0        GIC_SRC_SPI(4)  // (36)
#define AW_IRQ_CSI1        GIC_SRC_SPI(4)  // (36)
#define AW_IRQ_MIPICSI     GIC_SRC_SPI(4)  // (36)
#define AW_IRQ_MIPIDSI     GIC_SRC_SPI(5)  // (37)
#define AW_IRQ_GPU         GIC_SRC_SPI(6)  // (38)
#define AW_IRQ_VE      	   GIC_SRC_SPI(7)  // (39)
#define AW_IRQ_FD      	   GIC_SRC_SPI(8)  // (40)
#define AW_IRQ_NAND0       GIC_SRC_SPI(9)  // (41)
#define AW_IRQ_NAND1       GIC_SRC_SPI(10) // (42)
#define AW_IRQ_MMC0        GIC_SRC_SPI(11) // (43)
#define AW_IRQ_MMC1        GIC_SRC_SPI(11) // (43)
#define AW_IRQ_MMC2   	   GIC_SRC_SPI(11) // (43)
#define AW_IRQ_MMC3   	   GIC_SRC_SPI(11) // (43)
#define AW_IRQ_USB_OTG     GIC_SRC_SPI(12) // (44)
#define AW_IRQ_USBEHCI0    GIC_SRC_SPI(13) // (45)
#define AW_IRQ_USBOHCI0    GIC_SRC_SPI(14) // (46)
#define AW_IRQ_HSI    	   GIC_SRC_SPI(15) // (47)
#define AW_IRQ_TSC         GIC_SRC_SPI(15) // (47)
#define AW_IRQ_TWI0        GIC_SRC_SPI(16) // (48)
#define AW_IRQ_TWI1        GIC_SRC_SPI(16) // (48)
#define AW_IRQ_TWI2        GIC_SRC_SPI(16) // (48)
#define AW_IRQ_TWI3        GIC_SRC_SPI(16) // (48)
#define AW_IRQ_TWI4        GIC_SRC_SPI(16) // (48)
#define AW_IRQ_SPI0        GIC_SRC_SPI(17) // (49)
#define AW_IRQ_SPI1        GIC_SRC_SPI(17) // (49)
#define AW_IRQ_SPI2        GIC_SRC_SPI(17) // (49)
#define AW_IRQ_SPI3        GIC_SRC_SPI(17) // (49)
#define AW_IRQ_IIS0        GIC_SRC_SPI(18) // (50)
#define AW_IRQ_IIS1        GIC_SRC_SPI(18) // (50)
#define AW_IRQ_SPDIF       GIC_SRC_SPI(18) // (50)
#define AW_IRQ_AC97    	   GIC_SRC_SPI(18) // (50)
#define AW_IRQ_SS    	   GIC_SRC_SPI(19) // (51)
#define AW_IRQ_CIR_TX      GIC_SRC_SPI(19) // (51)
#define AW_IRQ_RRSB        GIC_SRC_SPI(20) // (52)
#define AW_IRQ_RTWI0       GIC_SRC_SPI(21) // (53)
#define AW_IRQ_RTWI1       GIC_SRC_SPI(21) // (53)
#define AW_IRQ_1WIRE       GIC_SRC_SPI(22) // (54)
#define AW_IRQ_RCIR        GIC_SRC_SPI(22) // (54)
#define AW_IRQ_RUART       GIC_SRC_SPI(22) // (54)
//test for 1639 fpga//////
//#define AW_IRQ_TIMER0      GIC_SRC_SPI(23) // (55)
//#define AW_IRQ_TIMER4      GIC_SRC_SPI(23) // (55)
#define AW_IRQ_TIMER0      GIC_SRC_SPI(18 ) // (50)
#define AW_IRQ_TIMER4      GIC_SRC_SPI(22 ) // (54)
//////////////////////////
#define AW_IRQ_WATCHDOG    GIC_SRC_SPI(23) // (55)
#define AW_IRQ_DMA         GIC_SRC_SPI(24) // (56)
#define AW_IRQ_HSTMR0      GIC_SRC_SPI(25) // (57)
#define AW_IRQ_HSTMR1      GIC_SRC_SPI(25) // (57)
#define AW_IRQ_HSTMR2      GIC_SRC_SPI(25) // (57)
#define AW_IRQ_HSTMR3      GIC_SRC_SPI(25) // (57)
#define AW_IRQ_HSTMR4      GIC_SRC_SPI(25) // (57)
#define AW_IRQ_RDMA        GIC_SRC_SPI(26) // (58)
#define AW_IRQ_RTIMER0     GIC_SRC_SPI(27) // (59)
#define AW_IRQ_RTIMER1     GIC_SRC_SPI(27) // (59)
#define AW_IRQ_RWATCHDOG   GIC_SRC_SPI(27) // (59)
#define AW_IRQ_MBOX        GIC_SRC_SPI(27) // (59)
#define AW_IRQ_TWD         GIC_SRC_SPI(28) // (60)
#define AW_IRQ_RPLEINT     GIC_SRC_SPI(29) // (61)
#define AW_IRQ_RPMEINT     GIC_SRC_SPI(29) // (61) not exist in FPGA
#define AW_IRQ_PAEINT      GIC_SRC_SPI(29) // (61) not exist in FPGA
#define AW_IRQ_PBEINT      GIC_SRC_SPI(29) // (61) not exist in FPGA
#define AW_IRQ_PEEINT      GIC_SRC_SPI(29) // (61) not exist in FPGA
#define AW_IRQ_PGEINT      GIC_SRC_SPI(29) // (61) not exist in FPGA
#define AW_IRQ_TZASC       GIC_SRC_SPI(30) // (62)
#define AW_IRQ_GPUPWR      GIC_SRC_SPI(31) // (63)

//test for 1639 fpga///////
//#define GIC_IRQ_NUM			(AW_IRQ_GPUPWR + 1)
#define GIC_IRQ_NUM			(120)
//////////////////////////

#endif	//fpga irq mapping

/* processer target */
#define GIC_CPU_TARGET(_n)	(1 << (_n))
#define GIC_CPU_TARGET0		GIC_CPU_TARGET(0)
#define GIC_CPU_TARGET1		GIC_CPU_TARGET(1)
#define GIC_CPU_TARGET2		GIC_CPU_TARGET(2)
#define GIC_CPU_TARGET3		GIC_CPU_TARGET(3)
#define GIC_CPU_TARGET4		GIC_CPU_TARGET(4)
#define GIC_CPU_TARGET5		GIC_CPU_TARGET(5)
#define GIC_CPU_TARGET6		GIC_CPU_TARGET(6)
#define GIC_CPU_TARGET7		GIC_CPU_TARGET(7)
/* trigger mode */
#define GIC_SPI_LEVEL_TRIGGER	(0)	//2b'00
#define GIC_SPI_EDGE_TRIGGER	(2)	//2b'10

extern void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data);
extern void irq_free_handler(int irq);
extern int irq_enable(int irq_no);
extern int irq_disable(int irq_no);

int arch_interrupt_init (void);

int arch_interrupt_exit (void);


#endif
