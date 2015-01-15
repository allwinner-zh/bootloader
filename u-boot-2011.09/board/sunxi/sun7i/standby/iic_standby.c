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
#include <asm/arch/cpu.h>
#include <asm/arch/twi.h>
#include <linux/types.h>
#include <common.h>
#define	I2C_WRITE		0
#define I2C_READ		1

#define I2C_OK			0
#define I2C_NOK			1
#define I2C_NACK		2
#define I2C_NOK_LA		3	/* Lost arbitration */
#define I2C_NOK_TOUT	4	/* time out */

#define I2C_START_TRANSMIT     0x08
#define I2C_RESTART_TRANSMIT   0x10
#define I2C_ADDRWRITE_ACK	   0x18
#define I2C_ADDRREAD_ACK	   0x40
#define I2C_DATAWRITE_ACK      0x28
#define I2C_READY			   0xf8
#define I2C_DATAREAD_NACK	   0x58
#define I2C_DATAREAD_ACK	   0x50

#define __reg(n)    (*(volatile unsigned int *)(n))
#define I2C_ADDR    __reg(SUNXI_TWI0_BASE + 0x0)
#define I2C_XADDR   __reg(SUNXI_TWI0_BASE + 0x4) 
#define I2C_DATA    __reg(SUNXI_TWI0_BASE + 0x8) 
#define I2C_CTL     __reg(SUNXI_TWI0_BASE + 0xc) 
#define I2C_STATUS  __reg(SUNXI_TWI0_BASE + 0x10) 
#define I2C_CLK     __reg(SUNXI_TWI0_BASE + 0x14) 
#define I2C_SRST    __reg(SUNXI_TWI0_BASE + 0x18) 
#define I2C_EFT     __reg(SUNXI_TWI0_BASE + 0x1c) 
#define I2C_LCR     __reg(SUNXI_TWI0_BASE + 0x20) 
#define I2C_DVFS    __reg(SUNXI_TWI0_BASE + 0x24) 

static __s32 standby_i2c_sendstart(void)
{
    __s32  time = 0xfffff;
    __u32  tmp_val;

    I2C_EFT = 0;
    I2C_SRST = 1;
	I2C_CTL |= 0x20;

    while((time--)&&(!(I2C_CTL & 0x08)));
	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}

	tmp_val = I2C_STATUS;
    if(tmp_val != I2C_START_TRANSMIT)
    {
		return -I2C_START_TRANSMIT;
    }

    return I2C_OK;
}
static __s32 standby_i2c_sendRestart(void)
{
    __s32  time = 0xffff;
    __u32  tmp_val;

	tmp_val = I2C_CTL & 0xC0;
	tmp_val |= 0x20;
	I2C_CTL = tmp_val;

    while( (time--) && (!(I2C_CTL & 0x08)) );
	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}

	tmp_val = I2C_STATUS;
    if(tmp_val != I2C_RESTART_TRANSMIT)
    {
		return -I2C_RESTART_TRANSMIT;
    }

    return I2C_OK;
}
static __s32 standby_i2c_sendslaveaddr(__u32 saddr,  __u32 rw)
{
    __s32  time = 0xffff;
    __u32  tmp_val;

	rw &= 1;
	I2C_DATA = ((saddr & 0xff) << 1)| rw;
	I2C_CTL &=  0xF7;

	while(( time-- ) && (!( I2C_CTL & 0x08 )));
	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}

	tmp_val = I2C_STATUS;
	if(rw == I2C_WRITE)//+write
	{
		if(tmp_val != I2C_ADDRWRITE_ACK)
		{
			return -I2C_ADDRWRITE_ACK;
		}
	}
	else//+read
	{
		if(tmp_val != I2C_ADDRREAD_ACK)
		{
			return -I2C_ADDRREAD_ACK;
		}
	}

	return I2C_OK;
}
static __s32 standby_i2c_sendbyteaddr(__u32 byteaddr)
{
    __s32  time = 0xffff;
    __u32  tmp_val;

	I2C_DATA = byteaddr & 0xff;
	I2C_CTL &= 0xF7;

	while( (time--) && (!(I2C_CTL & 0x08)) );
	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}

	tmp_val = I2C_STATUS;
	if(tmp_val != I2C_DATAWRITE_ACK)
	{
		return -I2C_DATAWRITE_ACK;
	}

	return I2C_OK;
}
static __s32 standby_i2c_getdata(__u8 *data_addr, __u32 data_count)
{
    __s32  time = 0xffff;
    __u32  tmp_val, i;

	if(data_count == 1)
	{
		I2C_CTL &= 0xF7;

		while( (time--) && (!(I2C_CTL & 0x08)) );
		if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
		for(time=0;time<100;time++);
		*data_addr = I2C_DATA;

		tmp_val = I2C_STATUS;
		if(tmp_val != I2C_DATAREAD_NACK)
		{
			return -I2C_DATAREAD_NACK;
		}
	}
	else
	{
		for(i=0; i< data_count - 1; i++)
		{
			time = 0xffff;
			tmp_val = I2C_CTL & 0xF7;
			tmp_val |= 0x04;
		    I2C_CTL = tmp_val;

			while( (time--) && (!(I2C_CTL & 0x08)) );
			if(time <= 0)
			{
				return -I2C_NOK_TOUT;
			}
			for(time=0;time<100;time++);
			time = 0xffff;
			data_addr[i] = I2C_DATA;
		    while( (time--) && (I2C_STATUS != I2C_DATAREAD_ACK) );
			if(time <= 0)
			{
				return -I2C_NOK_TOUT;
			}
		}

        time = 0xffff;
		I2C_CTL &= 0xF3;
		while( (time--) && (!(I2C_CTL & 0x08)) );
		if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
		for(time=0;time<100;time++);
		data_addr[data_count - 1] = I2C_DATA;
	    while( (time--) && (I2C_STATUS != I2C_DATAREAD_NACK) );
		if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
	}

	return I2C_OK;
}
static __s32 standby_i2c_senddata(__u8  *data_addr, __u32 data_count)
{
    __s32  time = 0xffff;
    __u32  i;

	for(i=0; i< data_count; i++)
	{
		time = 0xffff;
		I2C_DATA = data_addr[i];
		I2C_CTL &= 0xF7;

		while( (time--) && (!(I2C_CTL & 0x08)) );
		if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
		time = 0xffff;
		while( (time--) && (I2C_STATUS != I2C_DATAWRITE_ACK) );
		if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
	}

	return I2C_OK;
}
static __s32 standby_i2c_stop(void)
{
    __s32  time = 0xffff;
    __u32  tmp_val;

	tmp_val = (I2C_CTL & 0xC0) | 0x10;
	I2C_CTL = tmp_val;
	while( (time--) && (I2C_CTL & 0x10) );
	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}
	time = 0xffff;
	while( (time--) && (I2C_STATUS != I2C_READY) );
	tmp_val = I2C_STATUS;
	if(tmp_val != I2C_READY)
	{
		return -I2C_NOK_TOUT;
	}

	return I2C_OK;
}
int standby_i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
    int   i, ret, ret0, addrlen;
    char  *slave_reg;

	ret0 = -1;
    ret = standby_i2c_sendstart();
    if(ret)
	{
		goto i2c_read_err_occur;
	}

    ret = standby_i2c_sendslaveaddr(chip, I2C_WRITE);
    if(ret)
    {
	    goto i2c_read_err_occur;
	}
    //send byte address
    if(alen >= 3)
    {
    	addrlen = 3;
    }
    else if(alen <= 1)
    {
    	addrlen = 0;
    }
    else
    {
    	addrlen = 2;
    }
    slave_reg = (char *)&addr;
    for (i = addrlen; i>=0; i--)
    {
    	ret = standby_i2c_sendbyteaddr(slave_reg[i] & 0xff);
    	if(ret)
    	{
    		goto i2c_read_err_occur;
    	}
    }
    ret = standby_i2c_sendRestart();
    if(ret)
	{
		goto i2c_read_err_occur;
	}
    ret = standby_i2c_sendslaveaddr(chip, I2C_READ);
    if(ret)
    {
        goto i2c_read_err_occur;
	}
    //get data
	ret = standby_i2c_getdata(buffer, len);
	if(ret)
	{
		goto i2c_read_err_occur;
	}
    ret0 = 0;

i2c_read_err_occur:
	standby_i2c_stop();

	return ret0;
}
/*
****************************************************************************************************
*
*             TWIC_Write
*
*  Description:
*       DRV_MOpen
*
*  Parameters:
*
*  Return value:
*       EPDK_OK
*       EPDK_FAIL
****************************************************************************************************
*/
int standby_i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
    int   i, ret, ret0, addrlen;
    char  *slave_reg;

	ret0 = -1;
    ret = standby_i2c_sendstart();
    if(ret)
	{
		goto i2c_write_err_occur;
	}

    ret = standby_i2c_sendslaveaddr(chip, I2C_WRITE);
    if(ret)
    {
	    goto i2c_write_err_occur;
	}
    //send byte address
    if(alen >= 3)
    {
    	addrlen = 3;
    }
    else if(alen <= 1)
    {
    	addrlen = 0;
    }
    else
    {
    	addrlen = 2;
    }
    slave_reg = (char *)&addr;
    for (i = addrlen; i>=0; i--)
    {
    	ret = standby_i2c_sendbyteaddr(slave_reg[i] & 0xff);
    	if(ret)
    	{
    		goto i2c_write_err_occur;
    	}
    }
	ret = standby_i2c_senddata(buffer, len);
	if(ret)
	{
		goto i2c_write_err_occur;
	}
    ret0 = 0;

i2c_write_err_occur:
	standby_i2c_stop();

	return ret0;
}

