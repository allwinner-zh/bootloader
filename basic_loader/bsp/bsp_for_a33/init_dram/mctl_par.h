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
#ifndef   MCTL_PAR_H
#define   MCTL_PAR_H

#include "types.h"

//#define FPGA_PLATFORM
//#define DDR2_FPGA_S2C_16B
#ifdef FPGA_PLATFORM
//******************************************************************************
//DDR2 (x32)
//******************************************************************************
#ifdef DDR2_FPGA_S2C_X32
//DDR2 128Mx8 (128M Byte), total 1GB
#define MCTL_DDR_TYPE			2				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#define BURST_LENGTH			4				//DDR2 burst length
#define MCTL_RANK_NUM			1				//rank number
#endif
//******************************************************************************
//DDR2 (x16)
//******************************************************************************
#ifdef DDR2_FPGA_S2C_16B
//DDR2 128Mx8 (128M Byte), total 512MB
#define MCTL_DDR_TYPE			2				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			16
#define MCTL_PAGE_SIZE			2				//unit in KByte for one rank
#define BURST_LENGTH			4				//DDR2 burst length
#define MCTL_RANK_NUM			1				//rank number
#endif

//*****************************************************************************
//LPDDR3 SDRAM(x32)
//*****************************************************************************
#ifdef LPDDR3_FPGA_S2C_X32
//DDR2 128Mx8 (128M Byte), total 1GB
#define MCTL_DDR_TYPE			7				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#define BURST_LENGTH			8				//DDR2 burst length
#define MCTL_RANK_NUM			1				//rank number
#endif
//*****************************************************************************
//LPDDR2 SDRAM(x32)
//*****************************************************************************
#ifdef LPDDR2_FPGA_S2C_X32
//LPDDR2 128Mx32 (512M Byte), total 2GB
#define MCTL_DDR_TYPE			6				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			14
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#define BURST_LENGTH			8				//DDR2 burst length
#define MCTL_RANK_NUM			1				//rank number
#endif

//*****************************************************************************
//DDR3 SDRAM(x32)
//*****************************************************************************
#ifdef DDR3_FPGA_S2C_X32
//DDR3 512Mx8 (512M Byte)
#define MCTL_DDR_TYPE			3				//1: DDR, 2: DDR2, 3: DDR3
#define MCTL_BANK_SIZE			8
#define MCTL_ROW_WIDTH			16
#define MCTL_BUS_WIDTH			32
#define MCTL_PAGE_SIZE			4				//unit in KByte for one rank
#define BURST_LENGTH			8				//DDR2 burst length
#define MCTL_RANK_NUM			1				//rank number
#endif

#if (MCTL_DDR_TYPE==2)		//DDR2-800

#define MCTL_TREFI			7800  //max. in ns
#define MCTL_TMRD			2
#define MCTL_TRFC			400			//min. in ns
#define MCTL_TRP			6
#define MCTL_TPREA			1
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			6
#define MCTL_TCWL			5
#define MCTL_TRAS			18
#define MCTL_TRC			24
#define MCTL_TRCD			6
#define MCTL_TRRD			4
#define MCTL_TRTP			3
#define MCTL_TWR			6
#define MCTL_TWTR			4
#define MCTL_TEXSR			200
#define MCTL_TXP			2
#define MCTL_TXPDLL			6
#define MCTL_TZQCS			0
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			0
#define MCTL_TCKSRX			0
#define MCTL_TCKE			3
#define MCTL_TMOD			0
#define MCTL_TRSTL			0
#define MCTL_TZQCL			0
#define MCTL_TMRR			2
#define MCTL_TCKESR			2
#define MCTL_TDPD			0

#define MCTL_MR0			0xa63
#define MCTL_MR1			0x0
#define MCTL_MR2			0x0
#define MCTL_MR3			0x0
#define MCTL_PITMG0         0x02010101

#elif(MCTL_DDR_TYPE==3)		//DDR3-1333
#define MCTL_TREFI		7800   	//max. in ns
#define MCTL_TMRD			4
#define MCTL_TRFC			350				//min. in ns
#define MCTL_TRP			9
#define MCTL_TPREA		0
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			9
#define MCTL_TCWL			8
#define MCTL_TRAS			24
#define MCTL_TRC			33
#define MCTL_TRCD			9
#define MCTL_TRRD			4
#define MCTL_TRTP			5
#define MCTL_TWR			10
#define MCTL_TWTR			5
#define MCTL_TEXSR			512
#define MCTL_TXP			5
#define MCTL_TXPDLL			16
#define MCTL_TZQCS			64
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			7
#define MCTL_TCKSRX			7
#define MCTL_TCKE			4
#define MCTL_TMOD			12
#define MCTL_TRSTL			80
#define MCTL_TZQCL			512
#define MCTL_TMRR			2
#define MCTL_TCKESR			5
#define MCTL_TDPD			0

#define MCTL_MR0			0x1a50
#define MCTL_MR1			0x0
#define MCTL_MR2			0x18
#define MCTL_MR3			0x0

#elif(MCTL_DDR_TYPE==5)		//LPDDR

#elif(MCTL_DDR_TYPE==6)		//LPDDR2-800

#define MCTL_TREFI			3900
#define MCTL_TMRD			2
#define MCTL_TRFC			52
#define MCTL_TRP			8
#define MCTL_TPREA			0
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			6
#define MCTL_TCWL			4
#define MCTL_TRAS			18
#define MCTL_TRC			27
#define MCTL_TRCD			8
#define MCTL_TRRD			4
#define MCTL_TRTP			3
#define MCTL_TWR			6
#define MCTL_TWTR			3
#define MCTL_TEXSR			200
#define MCTL_TXP			3
#define MCTL_TXPDLL			6
#define MCTL_TZQCS			0
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			5
#define MCTL_TCKSRX			5
#define MCTL_TCKE			3
#define MCTL_TMOD			0
#define MCTL_TRSTL			0
#define MCTL_TZQCL			0
#define MCTL_TMRR			2
#define MCTL_TCKESR			6
#define MCTL_TDPD			0

#define MCTL_MR0			0x0
#define MCTL_MR1			0x92
#define MCTL_MR2			0x4
#define MCTL_MR3			0x2

#elif(MCTL_DDR_TYPE==7)

#define MCTL_TREFI			3900
#define MCTL_TMRD			2
#define MCTL_TRFC			52
#define MCTL_TRP			8
#define MCTL_TPREA			0
#define MCTL_TRTW			2
#define MCTL_TAL			0
#define MCTL_TCL			6
#define MCTL_TCWL			4
#define MCTL_TRAS			18
#define MCTL_TRC			27
#define MCTL_TRCD			8
#define MCTL_TRRD			4
#define MCTL_TRTP			3
#define MCTL_TWR			6
#define MCTL_TWTR			3
#define MCTL_TEXSR			200
#define MCTL_TXP			3
#define MCTL_TXPDLL			6
#define MCTL_TZQCS			0
#define MCTL_TZQCSI			0
#define MCTL_TDQS			1
#define MCTL_TCKSRE			5
#define MCTL_TCKSRX			5
#define MCTL_TCKE			3
#define MCTL_TMOD			0
#define MCTL_TRSTL			0
#define MCTL_TZQCL			0
#define MCTL_TMRR			2
#define MCTL_TCKESR			6
#define MCTL_TDPD			0

#define MCTL_MR0			0x0
#define MCTL_MR1			0x92
#define MCTL_MR2			0x4
#define MCTL_MR3			0x2
#define MCTL_PITMG0         0x02030101

#endif
#endif

#ifdef SYSTEM_SIMULATION
//******************************************************************************
//only for simulation
//******************************************************************************
#ifdef DRAM_TYPE_DDR3
#define MCTL_TCL = 5;	//11;
#define MCTL_TCWL= 4;	//8;
#define MCTL_TRCD= 6;	//11;
#define MCTL_TRC = 20;	//39;
#define MCTL_TRRD= 3;	//6;
#define MCTL_TFAW=16;   //32
#define MCTL_TCCD= 2;	//4;
#define MCTL_TRAS= 14;	//28;
#define MCTL_TRP = 6;	//11;
#define MCTL_TWTR= 3;	//6;
#define MCTL_TWR = 6;	//12;	//15ns min
#define MCTL_TRTP= 3;	//6;
#define MCTL_TAOND=0;   //0;
#define MCTL_TWLO=3;    //6;
#define MCTL_TWLMRD=20; //40;
#define MCTL_TRFC= 64;	//128;
#define MCTL_TMOD= 6;	//12;
#define MCTL_TMRD= 2;	//4;
#define MCTL_TMRW= 5;	//10;
#define MCTL_TMRR= 2;	//4;
#define MCTL_TRTW= 1;	//1;
#define MCTL_TRTODT=0;  //0
#define MCTL_TDLLK=256; //512
#define MCTL_TCKE= 2;	//4;
#define MCTL_TXP = 10;	//20;
#define MCTL_TXS =256;  //512
#define MCTL_TDQSCKMAX=0;//0
#define MCTL_TDQSCK=0;  //0
#define MCTL_TREFI= 98;	//195;
#define MCTL_TWTP = 12; //24
#define MCTL_TRASMAX=27;//54
#define MCTL_TWR2RD =8; //16
#define MCTL_TRD2WR =4; //7
#define MCTL_TCKESR	=3; //5
#define MCTL_TCKSRX	=4; //8
#define MCTL_TCKSRE	=4; //8

#define MCTL_MR0			0xa50
#define MCTL_MR1			0x4
#define MCTL_MR2			0x18
#define MCTL_MR3			0x0
#define MCTL_PARA1          0x00431af4
#endif

#endif
#endif  //MCTL_PAR_H
