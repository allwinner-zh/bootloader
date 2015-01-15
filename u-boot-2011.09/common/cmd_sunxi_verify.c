/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Allwinner boot verify trust-chain
 */
#include <common.h>
#include <openssl_ext.h>
#include <private_toc.h>
#include <asm/arch/ss.h>
#include <sunxi_verify.h>
#include <sunxi_board.h>

int sunxi_verify_signature(void *buff, uint len, const char *cert_name)
{
	u8 hash_of_file[32];
	int ret;
	struct sbrom_toc1_head_info  *toc1_head;
	struct sbrom_toc1_item_info  *toc1_item;
	sunxi_certif_info_t  sub_certif;
	int i;

	memset(hash_of_file, 0, 32);
	sunxi_ss_open();
	ret = sunxi_sha_calc(hash_of_file, 32, buff, len);
	if(ret)
	{
		printf("sunxi_verify_signature err: calc hash failed\n");
		//sunxi_ss_close();

		return -1;
	}
	//sunxi_ss_close();
	printf("show hash of file\n");
	sunxi_dump(hash_of_file, 32);
	//获取来自toc1的证书序列
	toc1_head = (struct sbrom_toc1_head_info *)CONFIG_TOC1_STORE_IN_DRAM_BASE;
	toc1_item = (struct sbrom_toc1_item_info *)(CONFIG_TOC1_STORE_IN_DRAM_BASE + sizeof(struct sbrom_toc1_head_info));

	for(i=1;i<toc1_head->items_nr;i++, toc1_item++)
	{
		if(toc1_item->type == TOC_ITEM_ENTRY_TYPE_BIN_CERTIF)
		{
			printf("find cert name %s\n", toc1_item->name);
			if(!strcmp((const char *)toc1_item->name, cert_name))
			{
				//取出证书的扩展项
				if(sunxi_certif_probe_ext(&sub_certif, (u8 *)(CONFIG_TOC1_STORE_IN_DRAM_BASE + toc1_item->data_offset), toc1_item->data_len))
				{
					printf("%s error: cant verify the content certif\n", __func__);

					return -1;
				}
				//比较扩展项和hash
				printf("show hash in certif\n");
				sunxi_dump(sub_certif.extension.value[0], 32);
				if(memcmp(hash_of_file, sub_certif.extension.value[0], 32))
				{
					printf("hash compare is not correct\n");
					printf(">>>>>>>hash of file<<<<<<<<<<\n");
					sunxi_dump(hash_of_file, 32);
					printf(">>>>>>>hash in certif<<<<<<<<<<\n");
					sunxi_dump(sub_certif.extension.value[0], 32);

					return -1;
				}

				return 0;
			}
		}
	}

	printf("cant find a certif belong to %s\n", cert_name);

	return -1;
}


