/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Ported 'dynenv' to 'nand env.oob' command
 * (C) 2010 Nanometrics, Inc.
 * 'dynenv' -- Dynamic environment offset in NAND OOB
 * (C) Copyright 2006-2007 OpenMoko, Inc.
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 *
 * Copyright 2010 Freescale Semiconductor
 * The portions of this file whose copyright is held by Freescale and which
 * are not considered a derived work of GPL v2-only code may be distributed
 * and/or modified under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */

 /*
 A. 加密解密
 1. 密钥的产生
     1) 找出两个相异的大素数P和Q，令N＝P×Q，M＝（P－1）*（Q－1）。
     2) 找出与M互素的大数E，且E<M；用欧氏算法计算出大数D，使D×E≡1 MOD M。
     3) 丢弃P和Q，公开E，D和N。E和N即加密密钥(公钥)，D和N即解密密钥(私钥)。
 2. 加密的步骤
     1) 计算N的有效位数tn（以字节数计），将最高位的零忽略掉，令tn1＝tn－1。比如N＝0x012A05，其有效位数tn＝5，tn1＝4
     2) 将明文数据A分割成tn1位（以字节数计）的块，每块看成一个大数，块数记为bn。从而，保证了每块都小于N。
     3) 对A的每一块Ai进行Bi＝Ai^E MOD N运算。Bi就是密文数据的一块，将所有密文块合并起来，就得到了密文数据B。
 3. 解密的步骤
     1) 同加密的第一步。
     2) 将密文数据B分割成tn位（以字节数计）的块，每块看成一个大数，块数记为bn。
     3) 对B的每一块Bi进行Ci＝Bi^D MOD N运算。Ci就是密文数据的一块，将所有密文块合并起来，就得到了密文数据C。
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <superblock.h>
#include <fastboot.h>
#include <sys_partition.h>
#include <sunxi_mbr.h>
#include <android_misc.h>

DECLARE_GLOBAL_DATA_PTR;


int signature_erase_all(sunxi_mbr_t  *mbr);

#ifndef CONFIG_OPENSSL

typedef struct public_key_pairs_t
{
 unsigned int  public_key;     // e
 unsigned int  divider;      // n
}
public_key_pairs;

typedef struct private_key_pairs_t
{
 unsigned int  private_key;    // d
 unsigned int  divider;      // n
}
private_key_pairs;

#define  P   (127)
#define  Q   (401)
#define  N   ((P) * (Q))
#define  M   ((P-1) * (Q-1))
#define  E   (53)

public_key_pairs   pblc_keys;
private_key_pairs  prvt_keys;

void rsa_dump(void);

static unsigned int  probe_gcd(unsigned int  divdend, unsigned int  divder)
{
     unsigned int  ret = divdend % divder;

     while(ret)
     {
         divdend = divder;
         divder  = ret;
         ret = divdend % divder;
     }

     return divder;
}

unsigned probe_high_level_power_mod(unsigned int  base_value, unsigned int  power, unsigned int  divider)
{
     unsigned int ret = 1;

     base_value %= divider;
     while(power > 0)
     {
         if(power & 1)
         {
             ret = (ret * base_value) % divider;
         }
         power /= 2;
         base_value = (base_value * base_value) % divider;
     }

     return ret;
}

unsigned rsa_init(void)
{
     unsigned int k;
     unsigned int product;
     unsigned int m_value;

     m_value = M;

     k = 1;
     if(probe_gcd(m_value, E) == 1)      //e,M互质
     {
         do
         {
             product = M * k + 1;
             if(!(product % E))
             {
                 pblc_keys.public_key = E;
                 pblc_keys.divider = N;

                 prvt_keys.private_key = product/E;
                 prvt_keys.divider = N;

#ifdef DEBUG_MODE
                 rsa_dump();
#endif

                 return 0;
             }
             k ++;
         }
         while(1);
     }

     return -1;
}


void rsa_dump(void)
{
     printf("base value\n");
     printf("M = %d(%d * %d), N = %d(%d * %d)\n", M, P-1, Q-1, N, P, Q);

     printf("public key: \n");
     printf("{e, n} = %d, %d\n", pblc_keys.public_key, pblc_keys.divider);

     printf("private key: \n");
     printf("{d, n} = %d, %d\n", prvt_keys.private_key, prvt_keys.divider);
}

void rsa_encrypt( unsigned int *input, unsigned int length, unsigned int *output )
{
     unsigned int i;

     for(i=0;i<length;i++)
     {
         output[i] = probe_high_level_power_mod(input[i], pblc_keys.public_key, pblc_keys.divider);
     }

     return ;
}


void rsa_decrypt( unsigned int *input, unsigned int length, unsigned int *output )
{
     unsigned int i;

     for(i=0;i<length;i++)
     {
         output[i] = probe_high_level_power_mod(input[i], prvt_keys.private_key, prvt_keys.divider);
     }

     return ;
}


/*
************************************************************************************************************

                                             hash function
************************************************************************************************************
*/
unsigned int cryptTable[0x500];
unsigned int seed1 = 0x7FED7FED;
unsigned int seed2 = 0xEEEEEEEE;
int key = 0;
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
*    note          :  返回数组
*
*
************************************************************************************************************
*/
void prepareCryptTable(void)
{
//	unsigned int seed = 0x00100001, index1 = 0, index2 = 0, i;
//
//	for( index1 = 0; index1 < 0x100; index1++ )
//	{
//		for( index2 = index1, i = 0; i < 5; i++, index2 += 0x100 )
//		{
//			unsigned long temp1, temp2;
//
//			seed = (seed * 125 + 3) % 0x2AAAAB;
//			temp1 = (seed & 0xFFFF) << 0x10;
//			seed = (seed * 125 + 3) % 0x2AAAAB;
//			temp2 = (seed & 0xFFFF);
//
//			cryptTable[index2] = ( temp1 | temp2 );
//		}
//	}
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
*    note          :  返回hash值
*
*
************************************************************************************************************
*/
unsigned int HashString( unsigned char *str, unsigned int dwHashType, unsigned int length )
{
//	unsigned char *key = (unsigned char *)str;
//	int ch;
//
//	while( length > 0 )
//	{
//		ch = *key++;
//		seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
//		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
//		length --;
//	}
//
//	return seed1;
	key += add_sum((void *)str, length);

	return (unsigned int)key;
}

void HashString_init(void)
{
	key = 0;
}

#define  HASH_BUFFER_BYTES                (32 * 1024)
#define  HASH_BUFFER_SECTORS              (HASH_BUFFER_BYTES/512)

//static void change_boot_signature(int partition_index)
//{
//	unsigned int summary = 0;
//	uint boot_start ;
//	uchar buffer[HASH_BUFFER_BYTES];
//	uint read_bytes;
//	unsigned int s_value[4], h_value[4];
//
//	memset(buffer,0,HASH_BUFFER_BYTES);
//	prepareCryptTable();
//	boot_start = sunxi_partition_get_offset(partition_index);
//	if(boot_start == 0)
//	{
//		printf("get the boot partition offest error!!\n");
//		return ;
//	}
//	HashString_init();
//	read_bytes = sizeof(struct fastboot_boot_img_hdr);
//	if(!sunxi_flash_read(boot_start, (read_bytes + 511)/512, buffer))
//	{
//		printf("signature0 read flash sig1 err\n");
//
//		return ;
//	}
//	summary = HashString(buffer, 1, read_bytes);	//1类hash
//	 printf("the summary is 0x%x\n",summary);
//	read_bytes = sizeof(struct image_header);
//	if(!sunxi_flash_read(boot_start+ CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE/512, (read_bytes + 511)/512, buffer))
//	{
//		printf("signature0 read flash sig2 err\n");
//
//		return ;
//	}
//	summary = HashString(buffer, 1, read_bytes);	//1类hash
//	 printf("the summary is 0x%x\n",summary);
//	 rsa_init();
//	 h_value[3] = (summary>>24)&0xff;
//	 h_value[2] = (summary>>16)&0xff;
//	 h_value[1] = (summary>>8)&0xff;
//	 h_value[0] = (summary>>0)&0xff;
//
//	 rsa_encrypt(h_value,4,s_value);
//
//	 if(!sunxi_flash_read(boot_start, 2, buffer))
//	 {
//		printf("signature0 read flash sig3 err\n");
//		return;
//	 }
//
//	 *(unsigned int *)(buffer + 608) = s_value[0];
//	 *(unsigned int *)(buffer + 612) = s_value[1];
//	 *(unsigned int *)(buffer + 616) = s_value[2];
//	 *(unsigned int *)(buffer + 620) = s_value[3];
//	 debug("change_boot_signature!!!!\n");
//	 debug("s_value[0] = %d\n",s_value[0]);
//	 debug("s_value[1] = %d\n",s_value[1]);
//	 debug("s_value[2] = %d\n",s_value[2]);
//	 debug("s_value[3] = %d\n",s_value[3]);
//	if(!sunxi_flash_write(boot_start,2, buffer))
//	{
//		printf("cant write the signature into flash!!\n");
//		return ;
//	}
//
//	 return ;
//}
//
//static void change_system_signature(int partition_index)
//{
//	  unsigned int summary = 0;
//	  unsigned int system_start;
//	  unsigned int s_value[4], h_value[4];
//	  unsigned char buffer[HASH_BUFFER_BYTES];
//	  unsigned int read_bytes;
//	  struct ext4_super_block  *sblock;
//	  system_start = sunxi_partition_get_offset(partition_index);
//
//	  HashString_init();
//	  read_bytes = sizeof(struct ext4_super_block);
//	  if(!sunxi_flash_read(system_start + CFG_SUPER_BLOCK_SECTOR, (read_bytes + 511)/512, buffer))
//	  {
//	  	printf("read the system fail!!!\n");
//		return ;
//	  }
//	  sblock = (struct ext4_super_block *)buffer;
//	  sblock->s_mtime	  = CFG_SUPER_BLOCK_STAMP_VALUE;
//	  sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
//	  memset(sblock->s_last_mounted, 0, 64);
//
//	  summary = HashString(buffer, 1, (unsigned int)&(((struct ext4_super_block *)0)->s_snapshot_list));	//1类hash
//	  printf("the summary is 0x%x\n",summary);
//	  rsa_init();
//
//	  h_value[3] = (summary>>24)&0xff;
//	  h_value[2] = (summary>>16)&0xff;
//	  h_value[1] = (summary>>8)&0xff;
//	  h_value[0] = (summary>>0)&0xff;
//
//	  rsa_encrypt(h_value,4,s_value);
//
//	 if(!sunxi_flash_read(system_start, 2, buffer))
//	 {
//	 	printf("fail to read the system signature!!!\n");
//		return;
//	 }
//
//	  *(unsigned int *)(buffer + 1000 - 10 * 4 + 0) = s_value[0];
//	  *(unsigned int *)(buffer + 1000 - 10 * 4 + 4) = s_value[1];
//	  *(unsigned int *)(buffer + 1000 - 10 * 4 + 8 ) = s_value[2];
//	  *(unsigned int *)(buffer + 1000 - 10 * 4 + 12 ) = s_value[3] ;
//
//		debug("change_system_signature!!!!\n");
//		debug("s_value[0] = %d\n",s_value[0]);
//		debug("s_value[1] = %d\n",s_value[1]);
//		debug("s_value[2] = %d\n",s_value[2]);
//		debug("s_value[3] = %d\n",s_value[3]);
//
//	  if(!sunxi_flash_write(system_start,2, buffer))
//	  {
//	 	 printf("write the system signature fail!!\n");
//	   	 return ;
//	  }
//  return ;
//}



//static void change_signature(void)
//{
//	int partition_num = 0;
//	int i;
//	char partition_name[16];
//	partition_num = sunxi_partition_get_total_num();
//	if((partition_num <= 0) || (partition_num > SUNXI_MBR_MAX_PART_COUNT))
//	{
//		printf("mbr not exist\n");
//		return;
//	}
//	for(i=0;i<partition_num;i++)
//	{
//		sunxi_partition_get_name(i,partition_name);
//		if(!strcmp("boot",partition_name))
//		{
//			printf("find the boot partition!!\n");
//			change_boot_signature(i);
//		}
//		if(!strcmp("system",partition_name))
//		{
//			printf("find the system partition!!\n");
//			change_system_signature(i);
//		}
//	}
//	return ;
//}

static int __signature_add_for_boot_format(uint start, uint *sig_value)
{
	unsigned char buffer[4096];
	unsigned int h_value[4], s_value[4];
	uint hash_value;

	HashString_init();
	if(sunxi_flash_read(start, 4096/512, buffer) != (4096/512))
	{
		puts("sunxi add signature fail: read flash err\n");

		return -1;
	}

	hash_value = HashString(buffer, 1, sizeof(struct fastboot_boot_img_hdr));	//1类hash
	hash_value = HashString(buffer + CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE, 1, sizeof(struct image_header));	//1类hash

	rsa_init();
	h_value[0] = (hash_value>>0) & 0xff;
	h_value[1] = (hash_value>>8) & 0xff;
	h_value[2] = (hash_value>>16) & 0xff;
	h_value[3] = (hash_value>>24) & 0xff;

	rsa_encrypt( h_value, 4, s_value);

	sig_value[0] = s_value[0];
	sig_value[1] = s_value[1];
	sig_value[2] = s_value[2];
	sig_value[3] = s_value[3];

	return 0;
}

static int __signature_add_for_ext4_format(uint start, uint *sig_value)
{
	unsigned char buffer[4096];
	unsigned int h_value[4], s_value[4];
	struct ext4_super_block  *sblock;
	uint hash_value;

	HashString_init();
	if(sunxi_flash_read(start + CFG_SUPER_BLOCK_SECTOR, 4096/512, buffer) != (4096/512))
	{
		puts("sunxi add signature fail: read flash err\n");

		return -1;
	}

	sblock = (struct ext4_super_block *)buffer;
	sblock->s_mtime     = CFG_SUPER_BLOCK_STAMP_VALUE;
	sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
	memset(sblock->s_last_mounted, 0, 64);
	hash_value = HashString(buffer, 1, (unsigned int)&(((struct ext4_super_block *)0)->s_snapshot_list));	//1类hash

	rsa_init();
	h_value[0] = (hash_value>>0) & 0xff;
	h_value[1] = (hash_value>>8) & 0xff;
	h_value[2] = (hash_value>>16) & 0xff;
	h_value[3] = (hash_value>>24) & 0xff;

	rsa_encrypt( h_value, 4, s_value);

	sig_value[0] = s_value[0];
	sig_value[1] = s_value[1];
	sig_value[2] = s_value[2];
	sig_value[3] = s_value[3];

	return 0;
}

static int __signature_add(sunxi_partition *partition)
{
	int ret;

	if(partition->sig_verify == 0x8000)
	{
		ret = __signature_add_for_boot_format(partition->addrlo, partition->sig_value);
	}
	else if(partition->sig_verify == 0x8001)
	{
		ret = __signature_add_for_ext4_format(partition->addrlo, partition->sig_value);
	}
	else
	{
		printf("the part is not need to add signature\n");

		ret = 0;
	}

	if(ret < 0)
	{
		printf("sunxi add signature for part %s failed\n", partition->name);

		return -1;
	}

	return 0;
}


static int __signature_verify_for_boot_format(uint start, uint *sig_value)
{
	unsigned char buffer[4096];
	unsigned int h_value[4], s_value[4];
	uint summary1, summary2;

	HashString_init();
	if(sunxi_flash_read(start, 4096/512, buffer) != (4096/512))
	{
		puts("sunxi add signature fail: read flash err\n");

		return -1;
	}

	summary1 = HashString(buffer, 1, sizeof(struct fastboot_boot_img_hdr));	//1类hash
	summary1 = HashString(buffer + CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE, 1, sizeof(struct image_header));	//1类hash

	s_value[0] = sig_value[0];
	s_value[1] = sig_value[1];
	s_value[2] = sig_value[2];
	s_value[3] = sig_value[3];

	rsa_init();
	rsa_decrypt( s_value, 4, h_value );
	summary2 = (h_value[0]<<0) | (h_value[1]<<8) | (h_value[2]<<16) | (h_value[3]<<24);

	if(summary1 != summary2)
	{
		printf("summary by hash 0x%x\n", summary1);
		printf("summary by key 0x%x\n", summary2);

		printf("boot signature invalid\n");

		return -1;
	}

	return 0;
}

static int __signature_verify_for_ext4_format(uint start, uint *sig_value)
{
	unsigned char buffer[4096];
	unsigned int h_value[4], s_value[4];
	struct ext4_super_block  *sblock;
	uint summary1, summary2;

	HashString_init();
	if(sunxi_flash_read(start + CFG_SUPER_BLOCK_SECTOR, 4096/512, buffer) != (4096/512))
	{
		puts("sunxi add signature fail: read flash err\n");

		return -1;
	}

	sblock = (struct ext4_super_block *)buffer;
	sblock->s_mtime     = CFG_SUPER_BLOCK_STAMP_VALUE;
	sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
	memset(sblock->s_last_mounted, 0, 64);
	summary1 = HashString(buffer, 1, (unsigned int)&(((struct ext4_super_block *)0)->s_snapshot_list));	//1类hash

	s_value[0] = sig_value[0];
	s_value[1] = sig_value[1];
	s_value[2] = sig_value[2];
	s_value[3] = sig_value[3];

	rsa_init();
	rsa_decrypt( s_value, 4, h_value );
	summary2 = (h_value[0]<<0) | (h_value[1]<<8) | (h_value[2]<<16) | (h_value[3]<<24);

	if(summary1 != summary2)
	{
		printf("summary by hash 0x%x\n", summary1);
		printf("summary by key 0x%x\n", summary2);

		printf("system signature invalid\n");

		return -1;
	}

	return 0;
}

static int __signature_verify(sunxi_partition *partition)
{
	int ret;

	if(partition->sig_verify == 0x8000)
	{
		ret = __signature_verify_for_boot_format(partition->addrlo, partition->sig_value);
	}
	else if(partition->sig_verify == 0x8001)
	{
		ret = __signature_verify_for_ext4_format(partition->addrlo, partition->sig_value);
	}
	else
	{
		printf("the part is not need to verify signature\n");

		ret = 0;
	}

	if(ret < 0)
	{
		printf("sunxi verify signature for part %s failed\n", partition->name);

		return -1;
	}

	return 0;
}
//static int __signature_verify(char *part_name)
//{
//	unsigned int tmp_start;
//	unsigned int summary1, summary2;
//	unsigned int s_value[4], h_value[4];
//	unsigned char buffer[HASH_BUFFER_BYTES];
//	unsigned int read_bytes;
//
//	memset(buffer, 0, HASH_BUFFER_BYTES);
//	//计算hash值
//	prepareCryptTable();		//准备hash表
//	//获取签名
//	printf("ras init\n");
//	rsa_init();
//	printf("ras start\n");
//
//	if(!strcmp("boot", part_name))
//	{
//		tmp_start = sunxi_partition_get_offset_byname(part_name);
//
//		printf("find part %s\n", part_name);
//		HashString_init();
//		read_bytes = sizeof(struct fastboot_boot_img_hdr);
//		if(!sunxi_flash_read(tmp_start, (read_bytes + 511)/512, buffer))
//		{
//			printf("signature0 read flash sig1 err\n");
//
//			return -1;
//		}
//		summary1 = HashString(buffer, 1, read_bytes);	//1类hash
//		read_bytes = sizeof(struct image_header);
//		if(!sunxi_flash_read(tmp_start + CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE/512, (read_bytes + 511)/512, buffer))
//		{
//			printf("signature0 read flash sig2 err\n");
//
//			return -1;
//		}
//		summary1 = HashString(buffer, 1, read_bytes);	//1类hash
//
//		//获取保存的签名
//		if(!sunxi_flash_read(tmp_start, 2, buffer))
//		{
//			printf("signature0 read flash sig3 err\n");
//
//			return -1;
//		}
//		s_value[0] = *(unsigned int *)(buffer + 608);
//		s_value[1] = *(unsigned int *)(buffer + 612);
//		s_value[2] = *(unsigned int *)(buffer + 616);
//		s_value[3] = *(unsigned int *)(buffer + 620);
//
//		rsa_decrypt( s_value, 4, h_value );
//
//		summary2 = (h_value[0]<<0) | (h_value[1]<<8) | (h_value[2]<<16) | (h_value[3]<<24);
//#if 0
//		for(j=0;j<4;j++)
//		{
//			printf("s_value[%d] = %x\n", j, s_value[j]);
//		}
//		for(j=0;j<4;j++)
//		{
//			printf("h_value[%d] = %x\n", j, h_value[j]);
//		}
//#endif
//		printf("summary by hash 0x%x\n", summary1);
//		printf("summary by rsa 0x%x\n", summary2);
//		if(summary1 != summary2)
//		{
//			printf("boot signature invalid\n");
//
//			return -1;
//		}
//	}
//	else if(!strcmp("system", part_name))
//	{
//		struct ext4_super_block  *sblock;
//
//		tmp_start = sunxi_partition_get_offset_byname(part_name);
//		printf("find part %s\n", part_name);
//
//		HashString_init();
//
//		read_bytes = sizeof(struct ext4_super_block);
//		if(!sunxi_flash_read(tmp_start + CFG_SUPER_BLOCK_SECTOR, (read_bytes + 511)/512, buffer))
//		{
//			printf("signature1 read flash sig1 err\n");
//
//			return -1;
//		}
//		sblock = (struct ext4_super_block *)buffer;
//#if 0
//		{
//			int k;
//			printf("s_inodes_count        = %x\n", sblock->s_inodes_count);
//			printf("s_blocks_count_lo     = %x\n", sblock->s_blocks_count_lo);
//			printf("s_r_blocks_count_lo   = %x\n", sblock->s_r_blocks_count_lo);
//			printf("s_free_blocks_count_lo= %x\n", sblock->s_free_blocks_count_lo);
//			printf("s_free_inodes_count   = %x\n", sblock->s_free_inodes_count);
//			printf("s_first_data_block    = %x\n", sblock->s_first_data_block);
//			printf("s_log_block_size      = %x\n", sblock->s_log_block_size);
//			printf("s_log_cluster_size    = %x\n", sblock->s_log_cluster_size);
//			printf("s_blocks_per_group    = %x\n", sblock->s_blocks_per_group);
//			printf("s_clusters_per_group  = %x\n", sblock->s_clusters_per_group);
//			printf("s_inodes_per_group    = %x\n", sblock->s_inodes_per_group);
//			printf("s_mtime               = %x\n", sblock->s_mtime);
//			printf("s_wtime               = %x\n", sblock->s_wtime);
//			printf("s_mnt_count           = %x\n", sblock->s_mnt_count);
//			printf("s_max_mnt_count       = %x\n", sblock->s_max_mnt_count);
//			printf("s_magic               = %x\n", sblock->s_magic);
//			printf("s_state               = %x\n", sblock->s_state);
//			printf("s_errors              = %x\n", sblock->s_errors);
//			printf("s_minor_rev_level     = %x\n", sblock->s_minor_rev_level);
//			printf("s_lastcheck           = %x\n", sblock->s_lastcheck);
//			printf("s_checkinterval       = %x\n", sblock->s_checkinterval);
//			printf("s_creator_os          = %x\n", sblock->s_creator_os);
//			printf("s_rev_level           = %x\n", sblock->s_rev_level);
//			printf("s_def_resuid          = %x\n", sblock->s_def_resuid);
//			printf("s_def_resgid          = %x\n", sblock->s_def_resgid);
//
//			printf("s_first_ino           = %x\n", sblock->s_first_ino);
//			printf("s_inode_size          = %x\n", sblock->s_inode_size);
//			printf("s_block_group_nr      = %x\n", sblock->s_block_group_nr);
//			printf("s_feature_compat      = %x\n", sblock->s_feature_compat);
//			printf("s_feature_incompat    = %x\n", sblock->s_feature_incompat);
//			printf("s_feature_ro_compat   = %x\n", sblock->s_feature_ro_compat);
//			for(k=0;k<16;k++)
//			{
//				printf("s_uuid[%d]        = %x\n", k, sblock->s_uuid[k]);
//			}
//			for(k=0;k<16;k++)
//			{
//				printf("s_volume_name[%d] = %x\n", k, sblock->s_volume_name[k]);
//			}
//			for(k=0;k<64;k++)
//			{
//				printf("s_last_mounted[%d]= %x\n", k, sblock->s_last_mounted[k]);
//			}
//			printf("s_algorithm_usage_bitmap= %x\n", sblock->s_algorithm_usage_bitmap);
//
//			printf("s_prealloc_blocks     = %x\n", sblock->s_prealloc_blocks);
//			printf("s_prealloc_dir_blocks = %x\n", sblock->s_prealloc_dir_blocks);
//			printf("s_reserved_gdt_blocks = %x\n", sblock->s_reserved_gdt_blocks);
//
//			for(k=0;k<16;k++)
//			{
//				printf("s_journal_uuid[%d]= %x\n", k, sblock->s_journal_uuid[k]);
//			}
//			printf("s_journal_inum        = %x\n", sblock->s_journal_inum);
//			printf("s_journal_dev         = %x\n", sblock->s_journal_dev);
//			printf("s_last_orphan         = %x\n", sblock->s_last_orphan);
//			for(k=0;k<16;k++)
//			{
//				printf("s_hash_seed[%d]   = %x\n", k, sblock->s_hash_seed[k]);
//			}
//			printf("s_def_hash_version    = %x\n", sblock->s_def_hash_version);
//			printf("s_jnl_backup_type     = %x\n", sblock->s_jnl_backup_type);
//			printf("s_desc_size           = %x\n", sblock->s_desc_size);
//			printf("s_default_mount_opts  = %x\n", sblock->s_default_mount_opts);
//			printf("s_first_meta_bg       = %x\n", sblock->s_first_meta_bg);
//			printf("s_mkfs_time           = %x\n", sblock->s_mkfs_time);
//			for(k=0;k<17;k++)
//			{
//				printf("s_jnl_blocks[%d]  = %x\n", k, sblock->s_jnl_blocks[k]);
//			}
//
//			printf("s_blocks_count_hi     = %x\n", sblock->s_blocks_count_hi);
//			printf("s_r_blocks_count_hi   = %x\n", sblock->s_r_blocks_count_hi);
//			printf("s_free_blocks_count_hi= %x\n", sblock->s_free_blocks_count_hi);
//			printf("s_min_extra_isize     = %x\n", sblock->s_min_extra_isize);
//			printf("s_want_extra_isize    = %x\n", sblock->s_want_extra_isize);
//			printf("s_flags               = %x\n", sblock->s_flags);
//
//			printf("s_raid_stride         = %x\n", sblock->s_raid_stride);
//			printf("s_mmp_update_interval = %x\n", sblock->s_mmp_update_interval);
//			printf("s_mmp_block           = %x\n", sblock->s_mmp_block);
//			printf("s_raid_stripe_width   = %x\n", sblock->s_raid_stripe_width);
//			printf("s_log_groups_per_flex = %x\n", sblock->s_log_groups_per_flex);
//			printf("s_reserved_char_pad   = %x\n", sblock->s_reserved_char_pad);
//
//			printf("s_reserved_pad        = %x\n", sblock->s_reserved_pad);
//			printf("s_kbytes_written      = %x\n", sblock->s_kbytes_written);
//			printf("s_snapshot_inum       = %x\n", sblock->s_snapshot_inum);
//			printf("s_snapshot_id         = %x\n", sblock->s_snapshot_id);
//			printf("s_snapshot_r_blocks_count= %x\n", sblock->s_snapshot_r_blocks_count);
//			printf("s_snapshot_list       = %x\n", sblock->s_snapshot_list);
//		}
//#endif
//		sblock->s_mtime     = CFG_SUPER_BLOCK_STAMP_VALUE;
//		sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
//		memset(sblock->s_last_mounted, 0, 64);
//		summary1 = HashString(buffer, 1, (unsigned int)&(((struct ext4_super_block *)0)->s_snapshot_list));	//1类hash
//
//		//获取保存的签名
//		if(!sunxi_flash_read(tmp_start, 2, buffer))
//		{
//			printf("signature1 read flash sig3 err\n");
//
//			return -1;
//		}
//		s_value[0] = *(unsigned int *)(buffer + 1000 - 10 * 4 + 0);
//		s_value[1] = *(unsigned int *)(buffer + 1000 - 10 * 4 + 4);
//		s_value[2] = *(unsigned int *)(buffer + 1000 - 10 * 4 + 8);
//		s_value[3] = *(unsigned int *)(buffer + 1000 - 10 * 4 + 12);
//
//		rsa_decrypt( s_value, 4, h_value );
//		summary2 = (h_value[0]<<0) | (h_value[1]<<8) | (h_value[2]<<16) | (h_value[3]<<24);
//#if 0
//		for(j=0;j<4;j++)
//		{
//			printf("s_value[%d] = %x\n", j, s_value[j]);
//		}
//		for(j=0;j<4;j++)
//		{
//			printf("h_value[%d] = %x\n", j, h_value[j]);
//		}
//#endif
//		printf("summary by hash %x\n", summary1);
//		printf("summary by rsa %x\n", summary2);
//		if(summary1 != summary2)
//		{
//			printf("system signature invalid\n");
//
//			return -1;
//		}
//	}
//
//	return 0;
//}
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
int do_sunxi_boot_signature(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	int i;
	sunxi_mbr_t  *mbr = (sunxi_mbr_t *)sunxi_partition_fetch_mbr();

	if(mbr == NULL)
	{
		puts("sunxi signature fail: unable to get the signature table\n");

		return -1;
	}

	if(gd->lockflag == SUNXI_NOLOCK)
	{
		puts("no signature\n");
		return 0;
	}

	if(gd->lockflag == SUNXI_UNLOCK)
	{
		puts("signature abandon\n");
		setenv("signature", "abandon");

		return 0;
	}

	for(i=0;i<mbr->PartCount;i++)
	{
		if((mbr->array[i].sig_verify & 0x8000) == 0x8000)
		{
			ret = __signature_verify(&mbr->array[i]);
			if(ret < 0)
			{
				printf("sunxi signature fail: the part %s signature is not passed\n", mbr->array[i].name);
				ret = signature_erase_all(mbr);
				if(ret < 0)
				{
					printf("sunxi signature fail: erase part fail\n");

					return -1;
				}

				if(gd->lockflag == SUNXI_LOCKING)
				{
					puts("signature failed\n");
					setenv("signature", "failed");
				}
				else if(gd->lockflag == SUNXI_RELOCKING)
				{
					puts("signature relock_failed\n");
					setenv("signature", "relock_failed");
				}

				return 0;
			}
		}
	}
	if(gd->lockflag == SUNXI_LOCKING)
	{
		puts("signature pass\n");
		setenv("signature", "pass");
	}
	else if(gd->lockflag == SUNXI_RELOCKING)
	{
		puts("signature relock_pass\n");
		setenv("signature", "relock_pass");
	}

    return 0;
}

U_BOOT_CMD(
	sunxi_boot_signature, CONFIG_SYS_MAXARGS, 1, do_sunxi_boot_signature,
	"sunxi_boot_signature sub-system",
	"no parmeters : \n"

);


static int sunxi_resignature(sunxi_mbr_t *mbr)
{
	int ret;
	int i;

	for(i=0;i<mbr->PartCount;i++)
	{
		if((mbr->array[i].sig_verify & 0x8000) == 0x8000)
		{
			printf("resignature for part %s\n", mbr->array[i].name);
			ret = __signature_add(&mbr->array[i]);
			if(ret < 0)
			{
				setenv("signature", "failed");
				printf("sunxi signature fail: the part %s signature is not passed\n", mbr->array[i].name);

				return -1;
			}
		}
	}

	return 0;
}


int sunxi_oem_op_lock(int request_flag, char *info, int force)
{
	char buffer[SUNXI_MBR_COPY_NUM * SUNXI_MBR_SIZE];
	char *tmp_buffer;
	sunxi_mbr_t  *mbr, *tmp_mbr;
	char local_info[64];
	char *info_p;
	uint store_flag;
	int  i;
	uint crc_cal;

	if(info == NULL)
	{
		memset(local_info, 0, 64);
		info_p = local_info;
	}
	else
	{
		info_p = info;
	}

	if(sunxi_flash_read(0, SUNXI_MBR_COPY_NUM * SUNXI_MBR_SIZE/512, buffer) != (SUNXI_MBR_COPY_NUM * SUNXI_MBR_SIZE/512))
	{
		strcpy(info_p, "sunxi oem operation fail, cant get oem flag");
		printf("%s\n", info_p);

		return -1;
	}

	mbr = (sunxi_mbr_t *)buffer;
	store_flag = mbr->lockflag;

	if( (request_flag == SUNXI_RELOCKING) || (request_flag == SUNXI_LOCKING) )	//要求加锁
	{
		if(  ((store_flag == SUNXI_UNLOCK) || (store_flag == SUNXI_NOLOCK)) || (force == 1)  )	//如果没有锁上
		{
			if(!sunxi_resignature(mbr))  //执行加锁操作
			{
				if(!force)
				{
					mbr->lockflag = SUNXI_RELOCKING;
				}
				else
				{
					mbr->lockflag = SUNXI_LOCKING;
				}

				tmp_mbr = mbr + 1;
				for(i=1;i<SUNXI_MBR_COPY_NUM;i++)
				{
					tmp_mbr->array[i].sig_value[0] = mbr->array[i].sig_value[0];
					tmp_mbr->array[i].sig_value[1] = mbr->array[i].sig_value[1];
					tmp_mbr->array[i].sig_value[2] = mbr->array[i].sig_value[2];
					tmp_mbr->array[i].sig_value[3] = mbr->array[i].sig_value[3];

					tmp_mbr ++;
				}
			}
			else							//如果加锁失败
			{
				strcpy(info_p, "the lock flag is invalid");

				printf("%s\n", info_p);

				return -2;
			}
		}
		//else if(lockflag == SUNXI_LOCKING)	//如果已经锁上
		else							//默认都是锁上
		{
			strcpy(info_p, "system is already locked");
			printf("%s\n", info_p);

			return -3;
		}
	}
	else if(request_flag == SUNXI_UNLOCK)	//要求解锁
	{
		if(   ((store_flag == SUNXI_UNLOCK)	|| (store_flag == SUNXI_NOLOCK)) && (force != 1) )//
		{
			strcpy(info_p, "system is not locked");
			printf("%s\n", info_p);

			return -4;
		}
		//else if(store_flag == SUNXI_LOCKING)	//如果已经锁上
		else								//默认都是锁上
		{
			puts("ready to erase all dedicate parts\n");
			if(!signature_erase_all(mbr))//开始擦除数据
			{
				if(!force)
				{
					mbr->lockflag = SUNXI_UNLOCK;
				}
				else
				{
					mbr->lockflag = SUNXI_NOLOCK;
				}
			}
			else
			{
				strcpy(info_p, "system cant erase parts");
				printf("%s\n", info_p);

				return -5;
			}
		}
	}
	else
	{
		strcpy(info_p, "the requst is invalid");
		printf("%s\n", info_p);

		return -6;
	}

	tmp_mbr = mbr;
	tmp_buffer = buffer;
	for(i=0;i<SUNXI_MBR_COPY_NUM;i++)
	{
		tmp_mbr->lockflag = mbr->lockflag;

		crc_cal = crc32(0, (const unsigned char *)tmp_buffer + 4, SUNXI_MBR_SIZE-4);
		*(unsigned int *)tmp_buffer = crc_cal;
		tmp_buffer += SUNXI_MBR_SIZE;
		tmp_mbr ++;
	}

	if(sunxi_flash_write(0, SUNXI_MBR_COPY_NUM * SUNXI_MBR_SIZE/512, buffer) != (SUNXI_MBR_COPY_NUM * SUNXI_MBR_SIZE/512))
	{
		strcpy(info_p, "sunxi oem operation fail, cant set oem flag");
		printf("%s\n", info_p);

		return -7;
	}
	sunxi_partition_refresh(buffer, SUNXI_MBR_SIZE);
	gd->lockflag = mbr->lockflag;

	return 0;
}


#else   /* CONFIG_OPENSSL */


#define SUNXI_SIGN_READ_MAX   (1024 * 1024)


#include <sunxi_openssl.h>
#include <sunxi_debug.h>
#include <sunxi_board.h>
#include "../fs/aw_fs/ff.h"

#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/pem.h>

static FILE sign_file;

#define SUNXI_SIGN_READ_MAX   (1024 * 1024)
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
int __probe_sign_file(unsigned char *partition_name, char *sign_buffer)
{
	unsigned char file_name[32];
	int  ret;
	int  index;
	uint rbytes;

	memset(file_name, 0, 32);
	strcpy((char *)file_name, (char *)partition_name);

	index = strlen((const char *)file_name);

	file_name[index++] = '.';
	file_name[index++] = 's';
	file_name[index++] = 'i';
	file_name[index++] = 'g';

	ret = f_open (&sign_file, (const TCHAR *)file_name, FA_OPEN_EXISTING | FA_READ );
    if(ret)
    {
        printf("validata err: open sign file %s error\n", file_name);

        return -1;
    }
    f_read(&sign_file, sign_buffer, 256, &rbytes);
	//公钥解密rsa
	f_close(&sign_file);
	//sunxi_dump(sign_buffer, 256);

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
static int __calculate_for_bootimg_format(char *data_buffer, char *hash_buffer)
{
	SHA256_CTX	ctx;

	memset(hash_buffer, 0, SHA256_DIGEST_LENGTH);
	//计算sha256
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (u_int8_t *)data_buffer, sizeof(struct fastboot_boot_img_hdr));
    //sunxi_dump(data_buffer, sizeof(struct fastboot_boot_img_hdr));

    SHA256_Update(&ctx, (u_int8_t *)(data_buffer + CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE), sizeof(struct image_header) + 512);
    //sunxi_dump(data_buffer + CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE, sizeof(struct image_header) + 512);

    SHA256_Final((unsigned char *)hash_buffer, &ctx);

	if(hash_buffer[0] >= 0xe0)
		hash_buffer[0] = 0xcc;
	//sunxi_dump(hash_buffer, 256);

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
static int __calculate_for_sparse_format(char *data_buffer, char *hash_buffer)
{
	char  *tmp_buffer = data_buffer + CFG_SUPER_BLOCK_SECTOR * 512;
	struct ext4_super_block  *sblock;
	SHA256_CTX	ctx;
	unsigned int data_len;

	sblock = (struct ext4_super_block *)tmp_buffer;
	sblock->s_mtime     = CFG_SUPER_BLOCK_STAMP_VALUE;
	sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
	memset(sblock->s_last_mounted, 0, 64);

	//计算sha256
	data_len = (unsigned int)&(((struct ext4_super_block *)0)->s_snapshot_list);
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (u_int8_t *)tmp_buffer, data_len);
    SHA256_Final((unsigned char *)hash_buffer, &ctx);

	if(hash_buffer[0] >= 0xe0)
		hash_buffer[0] = 0xcc;
	//sunxi_dump(hash_buffer, 256);

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
static int __validata_verify(sunxi_partition *partition)
{
	int ret;
	char *data_buffer;
	char sign_buffer[256];
	char hash1_buffer[512];
	char hash2_buffer[512];

	if( (partition->sig_verify != 0x8000) && (partition->sig_verify != 0x8001) )
	{
		printf("the part is not need to verify signature\n");

		return 0;
	}

	data_buffer = malloc(SUNXI_SIGN_READ_MAX);
	memset(data_buffer, 0, SUNXI_SIGN_READ_MAX);

	if(!sunxi_flash_read(partition->addrlo, SUNXI_SIGN_READ_MAX/512, data_buffer))
	{
		printf("validata err: read partition %s err\n", partition->name);
		free(data_buffer);

		return -1;
	}
	memset(hash1_buffer, 0, 512);
	if(partition->sig_verify == 0x8000)
	{
		ret = __calculate_for_bootimg_format(data_buffer, hash1_buffer);
	}
	else
	{
		ret = __calculate_for_sparse_format(data_buffer, hash1_buffer);
	}

	//获取分区对应的签名文件
	memset(sign_buffer, 0, SHA256_DIGEST_LENGTH);
	memset(hash2_buffer, 0, 512);
	if(__probe_sign_file(partition->name, sign_buffer))
	{
		free(data_buffer);

		return -1;
	}
	ret = sunxi_rsa_publickey_decrypt(sign_buffer, hash2_buffer, 256, "0:public.pem");
	if(ret < 0)
	{
		free(data_buffer);
		printf("validata err: when public decrypt %s\n", partition->name);

		return -1;
	}
	if(hash2_buffer[0] >= 0xe0)
		hash2_buffer[0] = 0xcc;

	//printf("\n\n");
	//sunxi_dump(hash2_buffer, 256);
	//比较两个值
	if(memcmp(hash1_buffer, hash2_buffer, 256))
	{
		free(data_buffer);
		printf("validate err: when compare the signature on %s\n", partition->name);

		return -1;
	}
	free(data_buffer);
	printf("validate ok: validate the signature on %s\n", partition->name);

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
int sunxi_oem_op_lock(int request_flag, char *info, int force)
{
	return -1;
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
int do_sunxi_boot_signature(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	int i;
	sunxi_mbr_t  *mbr = (sunxi_mbr_t *)sunxi_partition_fetch_mbr();
	FATFS openssl_mount;

	if(mbr == NULL)
	{
		puts("sunxi signature fail: unable to get the signature table\n");

		return -1;
	}

	if(gd->lockflag == SUNXI_NOLOCK)
	{
		puts("sunxi validata: no signature\n");
		return 0;
	}

	if(gd->lockflag == SUNXI_UNLOCK)
	{
		puts("sunxi validata: signature abandon\n");
		setenv("signature", "abandon");

		return 0;
	}

	arm_neon_init();
	f_mount_ex(0, &openssl_mount, 0);

	for(i=0;i<mbr->PartCount;i++)
	{
		if((mbr->array[i].sig_verify & 0x8000) == 0x8000)
		{
			ret = __validata_verify(&mbr->array[i]);
			if(ret < 0)
			{
				printf("sunxi signature fail: the part %s signature is not passed\n", mbr->array[i].name);
				ret = signature_erase_all(mbr);
				if(ret < 0)
				{
					printf("sunxi signature fail: erase part fail\n");
					f_mount(0, NULL, NULL);

					return -1;
				}

				if(gd->lockflag == SUNXI_LOCKING)
				{
					puts("signature failed\n");
					setenv("signature", "failed");
				}
				else if(gd->lockflag == SUNXI_RELOCKING)
				{
					puts("signature relock_failed\n");
					setenv("signature", "relock_failed");
				}
				f_mount(0, NULL, NULL);

				return 0;
			}
		}
	}
	if(gd->lockflag == SUNXI_LOCKING)
	{
		puts("signature pass\n");
		setenv("signature", "pass");
	}
	else if(gd->lockflag == SUNXI_RELOCKING)
	{
		puts("signature relock_pass\n");
		setenv("signature", "relock_pass");
	}
	f_mount(0, NULL, NULL);

    return 0;
}

U_BOOT_CMD(
	sunxi_boot_signature, 16, 0, do_sunxi_boot_signature,
	"do validata",
	"[public key name]"
);

#endif    /* CONFIG_OPENSSL */

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
static int __signature_erase_part(uint sector_start, uint nsector, void *erase_buffer, uint buffer_sectors)
{
	uint tmp_nsectors, tmp_start;

	tmp_start = sector_start;
	tmp_nsectors = nsector;

    while(tmp_nsectors>=buffer_sectors)
    {
        if(sunxi_flash_write(tmp_start, buffer_sectors, erase_buffer) != buffer_sectors)
        {
			printf("signature err: unable to erase part when signature fail\n");

			return -1;
        }
        tmp_start    += buffer_sectors;
        tmp_nsectors -= buffer_sectors;
    }
    if(tmp_nsectors)
    {
        if(sunxi_flash_write(tmp_start, tmp_nsectors, erase_buffer) != tmp_nsectors)
        {
        	printf("signature err: unable to erase part when signature fail\n");

        	return -1;
        }
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
*    note          :  4 steps:  1, erase cache; 2 erase data; 3 erase private; 4 erase drm; 5 erase UDISK
*
*
************************************************************************************************************
*/
int signature_erase_all(sunxi_mbr_t  *mbr)
{
	int i;
	uint part_start, part_sectors;
	void *buffer;

	buffer = malloc(4 * 1024 * 1024);
	if(!buffer)
	{
		puts("sunxi signature fail: unable to malloc memory to erase parts\n");

		return -1;
	}

	puts("ready to erase dedicated parts\n");
	memset(buffer, 0xff, 4 * 1024 * 1024);
	for(i=0;i<mbr->PartCount;i++)
	{
		printf("try part %s\n", mbr->array[i].name);
		if(mbr->array[i].sig_erase == 0x8000)
		{
			part_start   = mbr->array[i].addrlo;
			//part_sectors = mbr->array[i].lenlo;
			part_sectors = 16 * 1024 * 1024/512;

			printf("erase part %s ...\n", mbr->array[i].name);
			if(__signature_erase_part(part_start, part_sectors, buffer, 4 * 1024 * 1024/512))
			{
				printf("erase part %s fail\n", mbr->array[i].name);
				free(buffer);

				return -1;
			}
			printf("erase part %s ok\n", mbr->array[i].name);
		}
	}
	free(buffer);

	return 0;
}

