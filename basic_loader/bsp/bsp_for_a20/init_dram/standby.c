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

#include "arch.h"
#include "jump_in_asm.h"

extern __u32 super_standby_flag;
//extern void jump_to( __u32 entry_addr );

#ifdef DEBUG
#define msg(fmt,args...)				UART_printf2(fmt ,##args)
#else
#define msg(fmt,args...)
#endif

#ifdef DRAM_CRC_TEST
#define SW_PA_TIMERC_IO_BASE              0x01c20c00
#define RTC_DATA_REG(x)				(SW_PA_TIMERC_IO_BASE + 0x120 + 4*(x))
#define lread(n)                    (*((volatile unsigned int *)(n)))
#define lwrite(addr,value)   (*((volatile unsigned int *)(addr)) = (value))


void standby_save2rtc(int index, unsigned int data)
{
    lwrite(RTC_DATA_REG(index), data);
}

unsigned int standby_get_from_rtc(int index)
{
    return lread(RTC_DATA_REG(index));
}

void standby_check_crc(void)
{
    int i, j;
    int *tmp = (int *)0x40000000;
    int crc = 0;
    int crc_check_bytes = 0;
    volatile int dbg=0x55;
    crc_check_bytes = standby_get_from_rtc(7);
    crc_check_bytes = crc_check_bytes & 7;
    standby_save2rtc(7, crc_check_bytes + 1);
    tmp += crc_check_bytes;
    msg("offset %d words!\n", crc_check_bytes);
    for(i = 0; i < 8; i++)
    {
        crc = 0;
        for(j = 0; j < 128 * 1024 * 1024; j+=32)
        {
            crc += *tmp;
            tmp += 8;
        }
        if (crc != standby_get_from_rtc(8 + i))
        {
            msg("%d M dram crc err!\n", i*128);
            msg("while here!\n");

            while(dbg==0x55);
        }
        else
        {
            msg("%d M dram crc ok!\n", i*128);
        }
    }
}
#endif
void check_super_standby_flag()
{
	msg("super standby flag = %d,%x\n", super_standby_flag, *(volatile int *)0x52000000);
	if (1 == super_standby_flag)
	{
		#ifdef DRAM_CRC_TEST
		standby_check_crc();
		#endif
		jump_to_standby(0x52000000);
	}
}
