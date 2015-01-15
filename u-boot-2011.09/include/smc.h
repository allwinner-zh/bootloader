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

#ifndef __SMC_H__
#define __SMC_H__

#ifndef CONFIG_SUNXI_SECURE_SYSTEM

#include <asm/io.h>

#define smc_readl          readl
#define smc_writel         writel

#else

unsigned int smc_readl(unsigned int addr);
void smc_writel(unsigned int value, unsigned int addr);
int smc_efuse_writel(void *key_buf);
int smc_efuse_readl(void *key_buf, void *read_buf);
int smc_aes_bssk_encrypt_to_dram(void *keydata, int keylen, void *buffer, int *real_len);
int smc_aes_bssk_decrypt_to_keysram(void *srcdata, int srclen);
int smc_create_huk(void *huk_data, int len);

#endif


#endif
