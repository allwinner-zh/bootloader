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
#include <common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SUNXI_SECURE_SYSTEM

static __inline uint smc_readl_normal(uint addr);
static __inline void smc_writel_normal(uint value, uint addr);

uint (* smc_readl_pt)(uint addr) = smc_readl_normal;
void (* smc_writel_pt)(uint value, uint addr) = smc_writel_normal;

extern void tee_irq_handler(void);

static __inline uint smc_readl_secos(uint addr);
static __inline void smc_writel_secos(uint value, uint addr);
/* TEE SMC command type */
#define TEE_SMC_INIT_CALL		0x0FFFFFF1
#define TEE_SMC_PLAFORM_OPERATION	0x0FFFFFF2
#define TEE_SMC_OPEN_SESSION		0x0FFFFFF3
#define TEE_SMC_CLOSE_SESSION		0x0FFFFFF4
#define TEE_SMC_INVOKE_COMMAND	        0x0FFFFFF5
#define TEE_SMC_REGISTER_IRQ_HANDLER	0x0FFFFFF6
#define TEE_SMC_NS_IRQ_DONE		0x0FFFFFF7
#define TEE_SMC_NS_KERNEL_CALL		0x0FFFFFF8

#define TEE_SMC_PLATFORM_REGRW      0x0FFFFFFC

/* platform smc command define */
#define TEE_SMC_READ_REG            (0xFFFF0000)
#define TEE_SMC_WRITE_REG           (0xFFFF0001)
#define TEE_SMC_CPU_POWERUP         (0xFFFF0002)
#define TEE_SMC_CPU_POWERDOWN       (0xFFFF0003)
#define TEE_SMC_CLUSTER_POWERUP     (0xFFFF0004)
#define TEE_SMC_CLUSTER_POWERDOWN   (0xFFFF0005)

#define TEE_SMC_EFUSE_WRITE_REG			(0xFFFF000A)
#define TEE_SMC_EFUSE_READ_REG			(0xFFFF000B)
#define TEE_SMC_AES_BSSK_EN_TO_DRAM		(0xFFFF000C)
#define TEE_SMC_AES_BSSK_DE_TO_KEYSRAM	(0xFFFF000D)
#define TEE_SMC_CREATE_HUK          (0xFFFF000E)
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
u32 sunxi_smc_call(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
static u32 do_smc(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
	return sunxi_smc_call(arg0, arg1, arg2, arg3);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int tee_main_entry(void)
{
	int ret;

	printf("semelis initialize\n");
	ret = do_smc(TEE_SMC_INIT_CALL, 0, 0, 0);
	if (ret == 0) {
		printf("semelis init succeeded\n");
	}
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static __inline uint smc_readl_secos(uint addr)
{
	return do_smc(TEE_SMC_PLATFORM_REGRW, TEE_SMC_READ_REG, addr, 0);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static __inline void smc_writel_secos(uint value, uint addr)
{
	do_smc(TEE_SMC_PLATFORM_REGRW, TEE_SMC_WRITE_REG, addr, value);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int smc_init(void)
{
	if(gd->securemode == SUNXI_SECURE_MODE_WITH_SECUREOS)
	{
		smc_readl_pt = smc_readl_secos;
		smc_writel_pt = smc_writel_secos;

		return tee_main_entry();
	}
	else if(gd->securemode == SUNXI_SECURE_MODE_NO_SECUREOS )
	{
		printf("semelis initialize skip when non-boot mode\n");
	}
	else
	{
		printf("normal mode\n");
	}
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
uint smc_readl(uint addr)
{
	return smc_readl_pt(addr);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void smc_writel(uint value, uint addr)
{
	smc_writel_pt(value, addr);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int smc_efuse_writel(void *key_buf)
{
	return do_smc(TEE_SMC_PLAFORM_OPERATION, TEE_SMC_EFUSE_WRITE_REG, (uint)key_buf, 0);
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int smc_efuse_readl(void *key_buf, void *read_buf)
{
	return do_smc(TEE_SMC_PLAFORM_OPERATION, TEE_SMC_EFUSE_READ_REG, (uint)key_buf, (uint)read_buf);
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
typedef struct
{
	uint srcbuf;
	uint srclen;
	uint dstbuf;
	uint dstlen;
}
smc_ss_en_de_config;


int smc_aes_bssk_encrypt_to_dram(void *srcdata, int srclen, void *dstbuffer, int *dst_len)
{
	smc_ss_en_de_config  ss_en_config;

	ss_en_config.srcbuf = (uint)srcdata;
	ss_en_config.srclen = (uint)srclen;
	ss_en_config.dstbuf = (uint)dstbuffer;
	ss_en_config.dstlen = (uint)dst_len;

	return do_smc(TEE_SMC_PLAFORM_OPERATION, TEE_SMC_AES_BSSK_EN_TO_DRAM, (uint)&ss_en_config, 0);
}

int smc_aes_bssk_decrypt_to_keysram(void *srcdata, int srclen)
{
	return do_smc(TEE_SMC_PLAFORM_OPERATION, TEE_SMC_AES_BSSK_DE_TO_KEYSRAM, (uint)srcdata, srclen);
}

int smc_create_huk(void *huk_data, int len)
{
	return do_smc(TEE_SMC_PLAFORM_OPERATION, TEE_SMC_CREATE_HUK, (uint)huk_data, len);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static __inline uint smc_readl_normal(uint addr)
{
	return readl(addr);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static __inline void smc_writel_normal(uint value, uint addr)
{
	writel(value, addr);
}
#else
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int smc_init(void)
{
	return 0;
}
#endif
