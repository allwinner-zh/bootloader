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

#ifndef _PIO_H_
#define _PIO_H_


#include "hd_scrpt.h"


/*
*********************************************************************************************************
*   PIO Controller define          < maintained by Jerry >
*********************************************************************************************************
*/
  /* offset */
#define PIOC_REG_o_OFFSET               0x04

#define PIOC_REG_o_CFG0                 0x00
#define PIOC_REG_o_CFG1                 0x04
#define PIOC_REG_o_CFG2                 0x08
#define PIOC_REG_o_CFG3                 0x0C
#define PIOC_REG_o_DATA                 0x10
#define PIOC_REG_o_DRV0                 0x14
#define PIOC_REG_o_DRV1                 0x18
#define PIOC_REG_o_PUL0                 0x1C
#define PIOC_REG_o_PUL1                 0x20
#define PIOC_REG_o_INT_CFG0             0x200
#define PIOC_REG_o_INT_CFG1             0x204
#define PIOC_REG_o_INT_CFG2             0x208
#define PIOC_REG_o_INT_CFG3             0x20C
#define PIOC_REG_o_INT_CTL              0x210
#define PIOC_REG_o_INT_STA              0x214
#define PIOC_REG_o_INT_DEB              0x218
#define PIOC_REG_o_PAD_DRV              0x220
#define PIOC_REG_o_PAD_PUL              0x224

#define GPIO_GROUP_A                    (0)
#define GPIO_GROUP_B                    (1)
#define GPIO_GROUP_C                    (2)
#define GPIO_GROUP_D                    (3)
#define GPIO_GROUP_E                    (4)
#define GPIO_GROUP_F                    (5)
#define GPIO_GROUP_G                    (6)
#define GPIO_GROUP_H                    (7)
#define GPIO_GROUP_I                    (8)

  /* registers */
#define PIOC_REG_CFG0(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_CFG0 )
#define PIOC_REG_CFG1(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_CFG1 )
#define PIOC_REG_CFG2(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_CFG2 )
#define PIOC_REG_CFG3(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_CFG3 )
#define PIOC_REG_DATA(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_DATA )
#define PIOC_REG_DRV0(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_DRV0 )
#define PIOC_REG_DRV1(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_DRV1 )
#define PIOC_REG_PUL0(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_PUL0 )
#define PIOC_REG_PUL1(port)             __REG( PIOC_REGS_BASE + 0x24 * port + PIOC_REG_o_PUL1 )
#define PIOC_REG_INT_CFG0               __REG( PIOC_REGS_BASE + PIOC_REG_o_INT_CFG0           )
#define PIOC_REG_INT_CFG1               __REG( PIOC_REGS_BASE + PIOC_REG_o_INT_CFG1           )
#define PIOC_REG_INT_CFG2               __REG( PIOC_REGS_BASE + PIOC_REG_o_INT_CFG2           )
#define PIOC_REG_INT_CFG3               __REG( PIOC_REGS_BASE + PIOC_REG_o_INT_CFG3           )
#define PIOC_REG_INT_CTL                __REG( PIOC_REGS_BASE + PIOC_REG_o_INT_CTL            )
#define PIOC_REG_INT_STA                __REG( PIOC_REGS_BASE + PIOC_REG_o_INT_STA            )
#define PIOC_REG_INT_DEB                __REG( PIOC_REGS_BASE + PIOC_REG_o_INT_DEB            )
#define PIOC_REG_PAD_DRV                __REG( PIOC_REGS_BASE + PIOC_REG_o_PAD_DRV            )
#define PIOC_REG_PAD_PUL                __REG( PIOC_REGS_BASE + PIOC_REG_o_PAD_PUL            )


#define PIO_REG_CFG(n, i)               ((unsigned int *)( PIOC_REGS_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00))
#define PIO_REG_DLEVEL(n, i)            ((unsigned int *)( PIOC_REGS_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14))
#define PIO_REG_PULL(n, i)              ((unsigned int *)( PIOC_REGS_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C))
#define PIO_REG_DATA(n) 	            ((unsigned int *)( PIOC_REGS_BASE + ((n)-1)*0x24 + 0x10))

#define PIO_REG_CFG_VALUE(n, i)          __REG( PIOC_REGS_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x00)
#define PIO_REG_DLEVEL_VALUE(n, i)       __REG( PIOC_REGS_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x14)
#define PIO_REG_PULL_VALUE(n, i)         __REG( PIOC_REGS_BASE + ((n)-1)*0x24 + ((i)<<2) + 0x1C)
#define PIO_REG_DATA_VALUE(n) 	         __REG( PIOC_REGS_BASE + ((n)-1)*0x24 + 0x10)

#define PIO_REG_BASE(n)                 ((unsigned int *)( PIOC_REGS_BASE + ((n)-1)*0x24))

#endif    // #ifndef _PIO_H_
