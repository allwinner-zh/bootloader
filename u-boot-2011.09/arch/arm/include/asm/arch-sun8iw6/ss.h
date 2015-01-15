/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/

#ifndef _SS_H_
#define _SS_H_

#include "platform.h"

#define SS_CTL				  (SS_BASE + 0x00)
#define SS_INT_CTRL			  (SS_BASE + 0x04)
#define SS_INT_STATUS		  (SS_BASE + 0x08)
#define SS_KEY_LOW_ADR        (SS_BASE + 0x10)
#define SS_KEY_HIGH_ADR       (SS_BASE + 0x14)
#define SS_PM_LOW_ADR         (SS_BASE + 0x18)
#define SS_PM_HIGH_ADR        (SS_BASE + 0x1C)
#define SS_DATA_SRC_LOW_ADR   (SS_BASE + 0x20)
#define SS_DATA_SRC_HIGH_ADR  (SS_BASE + 0x24)
#define SS_DATA_DST_LOW_ADR   (SS_BASE + 0x28)
#define SS_DATA_DST_HIGH_ADR  (SS_BASE + 0x2C)
#define SS_DATA_LEN			  (SS_BASE + 0x30)
#define SS_CNT0(n)            (SS_BASE + 0x34 + (n)*4)
#define SS_CLK_GATING         (SS_BASE + 0x44)
#define SS_CNT1(n)            (SS_BASE + 0x48 + (n)*4)


#define		SHA1_160_MODE	0
#define		SHA2_256_MODE	1

void sunxi_ss_open(void);
void sunxi_ss_close(void);
int  sunxi_sha_calc(u8 *dst_addr, u32 dst_len,
					u8 *src_addr, u32 src_len);

s32 sunxi_rsa_calc(u8 * n_addr,   u32 n_len,
				   u8 * e_addr,   u32 e_len,
				   u8 * dst_addr, u32 dst_len,
				   u8 * src_addr, u32 src_len);

#endif    /*  #ifndef _SS_H_  */
