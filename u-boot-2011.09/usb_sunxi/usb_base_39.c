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
#include "usb_base.h"
#include <scsi.h>
#include <asm/arch/intc.h>
#include <asm/io.h>

#define  SUNXI_USB_EP0_BUFFER_SIZE   (512)

#define  HIGH_SPEED_EP_MAX_PACKET_SIZE   (512)
#define  FULL_SPEED_EP_MAX_PACKET_SIZE	 (64)

#define  BULK_FIFOSIZE                   (512)

#define  SUNXI_USB_CTRL_EP_INDEX		0
#define  SUNXI_USB_BULK_IN_EP_INDEX		1	/* tx */
#define  SUNXI_USB_BULK_OUT_EP_INDEX	1	/* rx */

#define  SUNXI_USB_BULK_PREPARE         0
#define  SUNXI_USB_BULK_CBW_BEGIN       1
#define  SUNXI_USB_BULK_CBW_OK          2

#define  SUNXI_USB_CONTROL_CMD_OK       0
#define  SUNXI_USB_CONTROL_STAT         1
#define  SUNXI_USB_CONTROL_STAT_OK      2
#define  SUNXI_USB_CONTROL_EARLY_INIT   3



static uchar sunxi_usb_ep0_buffer[SUNXI_USB_EP0_BUFFER_SIZE];

static int  ep0_zero_package_flag;
sunxi_udc_t  			  sunxi_udc_source;
static sunxi_ubuf_t   			  sunxi_ubuf;
sunxi_usb_setup_req_t     *sunxi_udev_active;
static susb_mem_cfg               usb_mem_cfg;

static void __usb_bulk_ep_reset (void);
static int __process_event_buf(__u32 buf_Num);
static int __usb_prepare_mem(void);
static int __usb_release_mem(void);
static void __ep0_send_zero_package(void);

extern int fastboot_data_flag;
extern volatile int sunxi_usb_burn_from_boot_init;
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
void sunxi_usb_irq(void *data)
{
	__u32 i, num;

	num = SUSB_Get_EvtCount();
	for(i=0; i<num; i++)
	{
		__process_event_buf(i);
	}

	return ;
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
*    note          :   usb初始化动作，完成后，即可开启中断后，使用中断处理程序
*
*
************************************************************************************************************
*/
int sunxi_usb_init(int delaytime)
{
	if(sunxi_udev_active->state_init())
	{
		printf("sunxi usb err: fail to init usb device\n");

		return -1;
	}
	//预先关闭usb中断
	irq_disable(AW_IRQ_USB_OTG);
	//初始化 sunxi_udc用到的资源
    memset(&sunxi_udc_source, 0, sizeof(sunxi_udc_t));
    sunxi_udc_source.standard_reg = (struct usb_device_request *)malloc_noncache(sizeof(struct usb_device_request));
    if(!sunxi_udc_source.standard_reg)
    {
    	printf("sunxi usb err: fail to malloc memory for standard reg\n");

    	return -1;
    }
	//断开usb
	SUSB_Dev_ConectSwitch(USBC_DEVICE_SWITCH_OFF);
	SUBS_Dev_CoreReset();
	//预先关闭usb时钟
	usb_close_clock();
	//延时 delaytime ms
	__msdelay(delaytime);

	//假定usb运行在高速模式下
	sunxi_udc_source.address = 0;
	sunxi_udc_source.speed = USB_SPEED_HIGH;

	usb_mem_cfg.bulk_ep_max = HIGH_SPEED_EP_MAX_PACKET_SIZE;
	usb_mem_cfg.phy_out_ep = SUSB_Dev_Log_to_Physical_Ep(SUNXI_USB_BULK_OUT_EP_INDEX, 1);
	usb_mem_cfg.phy_in_ep = SUSB_Dev_Log_to_Physical_Ep(SUNXI_USB_BULK_IN_EP_INDEX, 0);
    usb_mem_cfg.ep0_maxpkt = 512;
	usb_mem_cfg.xfer_res = 0;

	memset(&sunxi_ubuf, 0, sizeof(sunxi_ubuf_t));
	//malloc memory for rx buffer
	sunxi_ubuf.rx_base_buffer = (uchar *)malloc(1024);
	if(!sunxi_ubuf.rx_base_buffer)
	{
		printf("sunxi usb err: fail to malloc memory for rx command buffer\n");

		goto __sunxi_usb_init_fail;
	}
	sunxi_ubuf.rx_req_buffer = sunxi_ubuf.rx_base_buffer;
	//open clock
	usb_open_clock();
	//Alloc Event Buffer
	if(__usb_prepare_mem())
	{
		printf("sunxi usb err : SUSB_Alloc_Event_Buffer failed\n");

	  	goto __sunxi_usb_init_fail;
	}
	//Setup Event Buffer
    SUSB_Event_Buffers_Setup(usb_mem_cfg.event_buf[0]);
    //Make sure that USB Disconnect
    SUSB_Dev_ConectSwitch(USBC_DEVICE_SWITCH_OFF);
	//设置为device模式
    SUSB_Force_Role(USBC_ID_TYPE_DEVICE);
    //Enable Phy
    SUSB_Dev_PHY_Config(1, USBC_TS_MODE_HS);
    //Config FIFO Size
    SUSB_Config_Dev_Size();
	//加强信号的驱动能力
    SUSB_EnhanceSignal();
	//Select Speed Mode: default high speed
#if defined(CONFIG_SUSB_1_1_DEVICE)
	 SUSB_Dev_ConfigTransferMode(USBC_TS_MODE_FS);
#elif defined(CONFIG_SUSB_2_0_DEVICE)
	 SUSB_Dev_ConfigTransferMode(USBC_TS_MODE_HS);
#elif defined(CONFIG_SUSB_3_0_DEVICE)
	 SUSB_Dev_ConfigTransferMode(USBC_TS_MODE_SS);
#else
#endif
    /* disable all interrupt */
	SUSB_Dev_Interrupt_Ctl(0);
    /* begin interrupt that need */
	SUSB_Dev_Interrupt_Ctl(SUSB_INTERRUPT_NO_SOF_EN);
    /*Issue a DEPSTARTCFG command for physical ep0*/
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 0, SUSB_DEPCMD_DEPSTARTCFG, 0))
	{
		printf("sunxi usb err: fail to Issue a DEPSTARTCFG command for physical ep0\n");
	  	goto __sunxi_usb_init_fail;
	}
	/*Issue DEPXFERCFG command for physical ep0 and ep1*/
    if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 0, SUSB_DEPCMD_SETEPCONFIG, 0))
	{
	  	printf("sunxi usb err: fail to Issue a DEPXFERCFG command for physical ep0\n");
	  	goto __sunxi_usb_init_fail;
	}
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 1, SUSB_DEPCMD_SETEPCONFIG, 0))
	{
	  	printf("sunxi usb err: fail to Issue a DEPXFERCFG command for physical ep1\n");
	  	goto __sunxi_usb_init_fail;
	}
	if(sunxi_udc_source.speed == USB_SPEED_SUPER)
	{
	 	usb_mem_cfg.ep0_maxpkt = 0x200;
	}
	else if(sunxi_udc_source.speed == USB_SPEED_HIGH)
	{
	 	usb_mem_cfg.ep0_maxpkt = 0x40;
	}
	else
	{
	 	usb_mem_cfg.ep0_maxpkt = 0x08;
	}
     /*Issue SUSB_DEPCMD_SETTRANSFRESOURCE command for physical ep 0&1*/
	 if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 0, SUSB_DEPCMD_SETTRANSFRESOURCE, 0))
	{
	  	printf("sunxi usb err: fail to Issue a DEPXFERCFG command for physical ep0\n");
	}
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 1, SUSB_DEPCMD_SETTRANSFRESOURCE, 0))
	{
	  	printf("sunxi usb err: fail to Issue a DEPXFERCFG command for physical ep0\n");
	}
	/*Enable Physical ep0 */
	SUSB_Dev_EnaDisa_Ep_Function(1, 0);
	/*Enable Physical ep1*/
	SUSB_Dev_EnaDisa_Ep_Function(1, 1);
	/*Start Device, softconnect begis*/
    SUSB_Dev_ConectSwitch(USBC_DEVICE_SWITCH_ON);
	/*Register Interrupt Handle */
	irq_install_handler(AW_IRQ_USB_OTG, sunxi_usb_irq, NULL);
	irq_enable(AW_IRQ_USB_OTG);

	return 0;

__sunxi_usb_init_fail:
	__usb_release_mem();

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
int sunxi_usb_exit(void)
{
	irq_disable(AW_IRQ_USB_OTG);
	irq_free_handler(AW_IRQ_USB_OTG);

	if(sunxi_ubuf.rx_req_buffer)
	{
		free(sunxi_ubuf.rx_req_buffer);
	}

	usb_close_clock();

	sunxi_udev_active->state_exit();

	__usb_release_mem();

	memset(&sunxi_ubuf, 0, sizeof(sunxi_ubuf_t));

	return 0;
}

/*
*******************************************************************************
*                     __usb_bulk_ep_reset
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void __usb_bulk_ep_reset (void)
{
	SUSB_Set_Epmap_Enable(0);
	SUSB_Set_Epmap_Enable(1);

	/*Stop any active transfer except for the endpoint0*/
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 1, SUSB_DEPCMD_ENDTRANSFER, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_ENDTRANSFER command for physical ep1\n");
	}
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_in_ep, SUSB_DEPCMD_ENDTRANSFER, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_ENDTRANSFER command for physical ep1\n");
	}
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_out_ep, SUSB_DEPCMD_ENDTRANSFER, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_ENDTRANSFER command for physical ep1\n");
	}

	/*Endpoint1 re DEPCFG*/
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 1, SUSB_DEPCMD_SETEPCONFIG, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_ENDTRANSFER command for physical ep1\n");
	}

	/*Endpoint0 re DEPSTARTCFG with XferRscldx set 2*/
	usb_mem_cfg.xfer_res = 2;
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 1, SUSB_DEPCMD_DEPSTARTCFG, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_DEPSTARTCFG command for physical ep1\n");
	}

	/*Issue DEPCFG for bulk out&in*/
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_out_ep, SUSB_DEPCMD_SETEPCONFIG, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_SETEPCONFIG command for physical usb_mem_cfg.phy_out_ep\n");
	}
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_in_ep, SUSB_DEPCMD_SETEPCONFIG, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_SETEPCONFIG command for physical usb_mem_cfg.phy_in_ep\n");
	}

	/*Issue DEPXFERCFG command for bulk out&in*/
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_out_ep, SUSB_DEPCMD_SETTRANSFRESOURCE, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_SETTRANSFRESOURCE command for physical usb_mem_cfg.phy_out_ep\n");
	}
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_in_ep, SUSB_DEPCMD_SETTRANSFRESOURCE, 0))
	{
		printf("__usb_bulk_ep_reset: fail to Issue a SUSB_DEPCMD_SETTRANSFRESOURCE command for physical usb_mem_cfg.phy_in_ep\n");
	}

	/*Eanble the Logic Endpoint*/
	SUSB_Set_Epmap_Enable(usb_mem_cfg.phy_out_ep);
	SUSB_Set_Epmap_Enable(usb_mem_cfg.phy_in_ep);

	return;
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
void sunxi_udc_ep_reset(void)
{
	__usb_bulk_ep_reset();

	sunxi_udc_source.eps_bulk_step = SUNXI_USB_BULK_PREPARE;
	sunxi_udc_source.ep0_control_step = SUNXI_USB_CONTROL_CMD_OK;
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
void sunxi_udc_send_setup(uint bLength, void *buffer)
{
	pSUSBTRB data_trb;

	data_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRB0[2];

	data_trb->bpl = (__u32)buffer;
	data_trb->bph = 0;
	data_trb->size = (bLength &(0x0ffffff)); //Max packet size or act size????
	data_trb->ctl = ((0x01<<11)|(0x05<<4)|(0x01<<1)|(0x01<<0));

	sunxi_udc_source.ep0_control_step = SUNXI_USB_CONTROL_STAT;

	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)data_trb, 1, SUSB_DEPCMD_STARTTRANSFER, 0))
	{
		printf("ep0 send setup: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep1\n");
	}

	return ;
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
int sunxi_udc_set_configuration(int config_param)
{
	/*Config USB Device*/
	SUSB_Set_Epmap_Enable(0);
    SUSB_Set_Epmap_Enable(1);

    if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 1, SUSB_DEPCMD_ENDTRANSFER, 0))
	{
		printf("sunxi_udc_set_configuration: fail to Issue a SUSB_DEPCMD_ENDTRANSFER command for physical ep1\n");
	}

	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 1, SUSB_DEPCMD_SETEPCONFIG, 0))
	{
		printf("sunxi_udc_set_configuration: fail to Issue a SUSB_DEPCMD_SETEPCONFIG command for physical ep1\n");
	}

	usb_mem_cfg.xfer_res = 2;
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 0, SUSB_DEPCMD_DEPSTARTCFG, 0))
	{
		printf("sunxi_udc_set_configuration: fail to Issue a SUSB_DEPCMD_DEPSTARTCFG command for physical ep0\n");
	}

     /*Issue DEPCFG for bulk out&in*/
    if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_out_ep, SUSB_DEPCMD_SETEPCONFIG, 0))
	{
		printf("sunxi_udc_set_configuration: fail to Issue a SUSB_DEPCMD_SETEPCONFIG command for physical ep%d\n", usb_mem_cfg.phy_out_ep);
	}
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_in_ep, SUSB_DEPCMD_SETEPCONFIG, 0))
	{
		printf("sunxi_udc_set_configuration: fail to Issue a SUSB_DEPCMD_SETEPCONFIG command for physical ep%d\n",usb_mem_cfg.phy_in_ep);
	}

	/*Issue DEPXFERCFG command for bulk out&in*/
    if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_out_ep, SUSB_DEPCMD_SETTRANSFRESOURCE, 0))
	{
		printf("sunxi_udc_set_configuration: fail to Issue a SUSB_DEPCMD_SETTRANSFRESOURCE command for physical ep%d\n", usb_mem_cfg.phy_out_ep);
	}
	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, usb_mem_cfg.phy_in_ep, SUSB_DEPCMD_SETTRANSFRESOURCE, 0))
	{
		printf("sunxi_udc_set_configuration: fail to Issue a SUSB_DEPCMD_SETTRANSFRESOURCE command for physical ep%d\n", usb_mem_cfg.phy_out_ep);
	}

	/*Eanble the Logic Endpoint*/
	SUSB_Set_Epmap_Enable(usb_mem_cfg.phy_out_ep);
	SUSB_Set_Epmap_Enable(usb_mem_cfg.phy_in_ep);

	//__ep0_send_zero_package();
    ep0_zero_package_flag = 1;

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
int sunxi_udc_set_address(uchar address)
{
	__u32 speed_mode;

	/*Config New Address*/
	SUSB_Dev_SetAddress(address);

    speed_mode = SUSB_Get_Speed_Mode();
    if(speed_mode== 0x4)
    {
  	    sunxi_udc_source.speed = USB_SPEED_SUPER;
		sunxi_usb_dbg("usb speed: SS\n");
    }
    else if(speed_mode == 0x0)
    {
  	    sunxi_udc_source.speed = USB_SPEED_HIGH;
		sunxi_usb_dbg("usb speed: HIGH\n");
    }
	else
	{
	    sunxi_udc_source.speed = USB_SPEED_FULL;
		sunxi_usb_dbg("usb speed: FULL\n");
	}

	//__ep0_send_zero_package();
    ep0_zero_package_flag = 1;

	return SUNXI_USB_REQ_SUCCESSED;
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
int sunxi_udc_send_data(void *buffer, unsigned int buffer_size)
{
	pSUSBTRB bulk_in_trb;

	bulk_in_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRBx[1];

	bulk_in_trb->bpl = (u32)(buffer);
	bulk_in_trb->bph = 0;
	bulk_in_trb->size = (buffer_size&0x0ffffff);
	bulk_in_trb->ctl = ((0x01<<11)|(1<<4)|(0x01<<1)|(0x01<<0));

    sunxi_udc_source.eps_bulk_step = SUNXI_USB_BULK_PREPARE;

	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)bulk_in_trb, usb_mem_cfg.phy_in_ep, SUSB_DEPCMD_STARTTRANSFER, 0))
	{
	    printf("ep0 notready out: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep1\n");
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
int sunxi_udc_start_recv_by_dma(uint mem_buf, uint length)
{
	pSUSBTRB data_trb;

	data_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRBx[2];

	data_trb->bpl = (u32)(mem_buf);
	data_trb->bph = 0;
	length = (length + 511) & (~511);
	data_trb->size = max(0x200, (length&0x0ffffff));
	data_trb->ctl = ((0x01<<11)|(1<<4)|(0x01<<1)|(0x01<<0));

	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)data_trb, usb_mem_cfg.phy_out_ep, SUSB_DEPCMD_STARTTRANSFER, 0))
	{
		printf("sunxi_udc_start_recv_by_dma: fail to Issue a SUSB_DEPCMD_STARTTRANSFER for physical epsout\n");
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
int sunxi_udc_get_ep_max(void)
{
	return usb_mem_cfg.bulk_ep_max;
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
int sunxi_udc_get_ep_in_type(void)
{
	return  (0x80 | SUNXI_USB_BULK_IN_EP_INDEX);
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
int sunxi_udc_get_ep_out_type(void)
{
	return  SUNXI_USB_BULK_OUT_EP_INDEX;
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
static void __ep0_send_zero_package(void)
{
	pSUSBTRB status_trb;

  	/*Setup New TRB for next Control Transfer*/
	status_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRB0[3];

	status_trb->bpl = 0;
	status_trb->bph = 0;
	status_trb->size = (0x0&0x0ffffff);
	status_trb->ctl = ((0x01<<11)|(0x04<<4)|(0x01<<1)|(0x01<<0));

	sunxi_udc_source.ep0_control_step = SUNXI_USB_CONTROL_STAT_OK;

	if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)status_trb, 1, SUSB_DEPCMD_STARTTRANSFER, 0))
	{
		printf("ep0 notready out: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep1\n");
	}
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
static void __eps_notready_out(void)
{
	if(sunxi_udc_source.eps_bulk_step == SUNXI_USB_BULK_PREPARE)
	{
		pSUSBTRB cbw_trb;

		cbw_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRBx[0];

		if(!fastboot_data_flag)
		{
			memset(sunxi_ubuf.rx_req_buffer, 0, 64);
		}

		cbw_trb->bpl  = (u32)sunxi_ubuf.rx_req_buffer;
		cbw_trb->bph  = 0;
		cbw_trb->size = (0x200&0x0ffffff);
		cbw_trb->ctl  = ((0x01<<11)|(0x01<<4)|(0x01<<1)|(0x01<<0));

		if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)cbw_trb, usb_mem_cfg.phy_out_ep, SUSB_DEPCMD_STARTTRANSFER, 0))
		{
			printf("ep0 notready out: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep1\n");
		}

		sunxi_udc_source.eps_bulk_step = SUNXI_USB_BULK_CBW_BEGIN;
	}

	return ;
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
static int __eps_complete_out(void)
{
	pSUSBTRB cbw_trb;

	cbw_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRBx[0];

	if(sunxi_udc_source.eps_bulk_step == SUNXI_USB_BULK_CBW_BEGIN)
	{
		sunxi_udc_source.eps_bulk_step = SUNXI_USB_BULK_CBW_OK;
		sunxi_ubuf.rx_ready_for_data = 1;
		sunxi_ubuf.rx_req_length = 0x200 - (cbw_trb->size & 0x00ffffff);
		sunxi_usb_dbg("sunxi_ubuf.rx_req_length=0x%x\n", sunxi_ubuf.rx_req_length);
		if(fastboot_data_flag)
		{
			sunxi_ubuf.rx_req_buffer += sunxi_ubuf.rx_req_length;
			sunxi_udc_source.eps_bulk_step = SUNXI_USB_BULK_PREPARE;
		}
	}
	else if(sunxi_udc_source.eps_bulk_step == SUNXI_USB_BULK_CBW_OK)
	{
		sunxi_udev_active->dma_rx_isr(NULL);
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
static int __eps_complete_in(void)
{
	sunxi_udev_active->dma_tx_isr(NULL);

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
static int __ep0_notready_out(void)
{
	/*Ep0 Control-3 Out Status*/
	if(sunxi_udc_source.ep0_control_step == SUNXI_USB_CONTROL_STAT)
	{
		pSUSBTRB status_trb;

	    status_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRB0[4];

    	status_trb->bpl = 0;
    	status_trb->bph = 0;
    	status_trb->size = (0x0&0x0ffffff);
    	status_trb->ctl = ((0x01<<11)|(0x04<<4)|(0x01<<1)|(0x01<<0));

		sunxi_udc_source.ep0_control_step = SUNXI_USB_CONTROL_EARLY_INIT;

		if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)status_trb, 0, SUSB_DEPCMD_STARTTRANSFER, 0))
	    {
			printf("ep0 notready out: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep1\n");
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
*    note          :
*
*
************************************************************************************************************
*/
static int __ep0_notready_in(void)
{
    if(ep0_zero_package_flag == 1)
    {
        __ep0_send_zero_package();
        ep0_zero_package_flag = 0;
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
static int __ep0_complete_out(void)
{
	int ret = -1;

	/*Handle CBW*/
	if(sunxi_udc_source.ep0_control_step == SUNXI_USB_CONTROL_CMD_OK)
	{
		if(USB_REQ_TYPE_STANDARD == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_TYPE_MASK))//Standard Req
		{
			switch(sunxi_udc_source.standard_reg->bRequest)
			{
				case USB_REQ_GET_STATUS:		//   0x00
				{
					/* device-to-host */
					if(USB_DIR_IN == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						ret = sunxi_udev_active->standard_req_op(USB_REQ_GET_STATUS, sunxi_udc_source.standard_reg, sunxi_usb_ep0_buffer);
					}

					break;
				}
				case USB_REQ_CLEAR_FEATURE:		//   0x01
				{
					/* host-to-device */
					if(USB_DIR_OUT == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						ret = sunxi_udev_active->standard_req_op(USB_REQ_CLEAR_FEATURE, sunxi_udc_source.standard_reg, NULL);
					}

					break;
				}
				case USB_REQ_SET_FEATURE:		//   0x03
				{
					/* host-to-device */
					if(USB_DIR_OUT == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						ret = sunxi_udev_active->standard_req_op(USB_REQ_SET_FEATURE, sunxi_udc_source.standard_reg, NULL);
					}

					break;
				}
				case USB_REQ_SET_ADDRESS:		//   0x05
				{
					/* host-to-device */
					if(USB_DIR_OUT == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						if(USB_RECIP_DEVICE == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_RECIPIENT_MASK))
						{
							/* receiver is device */
							ret = sunxi_udev_active->standard_req_op(USB_REQ_SET_ADDRESS, sunxi_udc_source.standard_reg, NULL);
						}
					}

					break;
				}
				case USB_REQ_GET_DESCRIPTOR:		//   0x06
				{
					/* device-to-host */
					if(USB_DIR_IN == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						if(USB_RECIP_DEVICE == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_RECIPIENT_MASK))
						{
							ret = sunxi_udev_active->standard_req_op(USB_REQ_GET_DESCRIPTOR, sunxi_udc_source.standard_reg, sunxi_usb_ep0_buffer);
						}
					}

					break;
				}
				case USB_REQ_SET_DESCRIPTOR:		//   0x07
				{
					/* host-to-device */
					if(USB_DIR_OUT == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						if(USB_RECIP_DEVICE == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_RECIPIENT_MASK))
						{
							//there is some problem
							ret = sunxi_udev_active->standard_req_op(USB_REQ_SET_DESCRIPTOR, sunxi_udc_source.standard_reg, sunxi_usb_ep0_buffer);
						}
					}

					break;
				}
				case USB_REQ_GET_CONFIGURATION:		//   0x08
				{
					/* device-to-host */
					if(USB_DIR_IN == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						if(USB_RECIP_DEVICE == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_RECIPIENT_MASK))
						{
							ret = sunxi_udev_active->standard_req_op(USB_REQ_GET_CONFIGURATION, sunxi_udc_source.standard_reg, sunxi_usb_ep0_buffer);
						}
					}

					break;
				}
				case USB_REQ_SET_CONFIGURATION:		//   0x09
				{
					/* host-to-device */
					if(USB_DIR_OUT == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						if(USB_RECIP_DEVICE == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_RECIPIENT_MASK))
						{
							ret = sunxi_udev_active->standard_req_op(USB_REQ_SET_CONFIGURATION, sunxi_udc_source.standard_reg, NULL);
						}
					}

					break;
				}
				case USB_REQ_GET_INTERFACE:		//   0x0a
				{
					/* device-to-host */
					if(USB_DIR_IN == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						if(USB_RECIP_DEVICE == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_RECIPIENT_MASK))
						{
							ret = sunxi_udev_active->standard_req_op(USB_REQ_GET_INTERFACE, sunxi_udc_source.standard_reg, sunxi_usb_ep0_buffer);
						}
					}

					break;
				}
				case USB_REQ_SET_INTERFACE:		//   0x0b
				{
					/* host-to-device */
					if(USB_DIR_OUT == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						if(USB_RECIP_INTERFACE == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_RECIPIENT_MASK))
						{
							ret = sunxi_udev_active->standard_req_op(USB_REQ_SET_INTERFACE, sunxi_udc_source.standard_reg, NULL);
						}
					}

					break;
				}
				case USB_REQ_SYNCH_FRAME:		//   0x0b
				{
					/* device-to-host */
					if(USB_DIR_IN == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_DIRECTION_MASK))
					{
						if(USB_RECIP_INTERFACE == (sunxi_udc_source.standard_reg->bmRequestType & USB_REQ_RECIPIENT_MASK))
						{
							ret = sunxi_udev_active->standard_req_op(USB_REQ_SYNCH_FRAME, sunxi_udc_source.standard_reg, NULL);
						}
					}

					break;
				}

				default:
				{
					printf("ep0 control unsupported request\n");

					break;
				}
			}
		}
		else   //Non-Standard Req
		{
			ret = sunxi_udev_active->nonstandard_req_op(USB_REQ_GET_STATUS, sunxi_udc_source.standard_reg, sunxi_usb_ep0_buffer, 0);
			if(ret < 0)
			{
				printf("err: unknown bmRequestType(0x%c)\n", sunxi_udc_source.standard_reg->bmRequestType);
			}
		}
	}

    if(ret < 0)
    {
        if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 0, SUSB_DEPCMD_SETSTALL, 0))
        {
           printf("ep0 notready out: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep1\n");
        }
        sunxi_udc_source.ep0_control_step = SUNXI_USB_CONTROL_EARLY_INIT;
    }

	/*Control Status Out Complete: Setup a NEW TRB*/
	if(sunxi_udc_source.ep0_control_step == SUNXI_USB_CONTROL_EARLY_INIT)
	{
		pSUSBTRB setup_trb;

		setup_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRB0[0];

		setup_trb->bpl = (u32)(sunxi_udc_source.standard_reg);
		setup_trb->bph = (u32)0;
		setup_trb->size = (0x08&0x0ffffff); //Maxpkt size or act size here????
		setup_trb->ctl = ((0x01<<11)|(0x02<<4)|(0x01<<1)|(0x01<<0));

		sunxi_udc_source.ep0_control_step = SUNXI_USB_CONTROL_CMD_OK;

		if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)setup_trb, 0, SUSB_DEPCMD_STARTTRANSFER, 0))
	    {
		   printf("ep0 notready out: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep1\n");
	    }
	}

	return ret;
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
static int __ep0_complete_in(void)
{
	/*Control Status In Complete: Setup a NEW TRB*/
	if(sunxi_udc_source.ep0_control_step == SUNXI_USB_CONTROL_STAT_OK)
	{
		pSUSBTRB setup_trb;

		setup_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRB0[0];

		setup_trb->bpl = (u32)(sunxi_udc_source.standard_reg);
		setup_trb->bph = (u32)0;
		setup_trb->size = (0x08&0x0ffffff);
		setup_trb->ctl = ((0x01<<11)|(0x02<<4)|(0x01<<1)|(0x01<<0));

		sunxi_udc_source.ep0_control_step = SUNXI_USB_CONTROL_CMD_OK;

		if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)setup_trb, 0, SUSB_DEPCMD_STARTTRANSFER, 0))
		{
	   		printf("ep0 notready out: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep1\n");
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
*    note          :
*
*
************************************************************************************************************
*/
static void __endpoint_interrupt(__u32 event_context)
{
	switch(((event_context>>6)&0x0f))
	{
		case SUSB_DEPEVT_XFERCOMPLETE:
			if(((event_context>>1)&0x1f)==0)//ep0 out
			{
				sunxi_usb_dbg("ep0 out complete\n");
			 	__ep0_complete_out();
			}
			else if(((event_context>>1)&0x1f)==1)//ep0 in
			{
				sunxi_usb_dbg("ep0 in complete\n");
			 	__ep0_complete_in();
			}
			else if(((event_context>>1)&0x1f) == (usb_mem_cfg.phy_out_ep&0x1f))//epx out
			{
				sunxi_usb_dbg("eps out complete\n");
				__eps_complete_out();
			}
			else if(((event_context>>1)&0x1f) == (usb_mem_cfg.phy_in_ep&0x1f))//epx in
			{
				sunxi_usb_dbg("eps in complete\n");
				__eps_complete_in();
			}
			else
			{
				puts("unsupported usb SUSB_DEPEVT_XFERCOMPLETE interrupt\n");
			}
			break;

		case SUSB_DEPEVT_XFERNOTREADY:
			if(((event_context>>1)&0x1f)==0)//ep0 out
			{
				sunxi_usb_dbg("ep0 out not ready\n");
				__ep0_notready_out();
			}
			else if(((event_context>>1)&0x1f)==1)//ep0 in
			{
				sunxi_usb_dbg("ep0 in not ready\n");
				__ep0_notready_in();
			}
			else if(((event_context>>1)&0x1f) == (usb_mem_cfg.phy_out_ep&0x1f))//epx out
			{
				sunxi_usb_dbg("eps out not ready\n");
				__eps_notready_out();
			}
			else if(((event_context>>1)&0x1f) == (usb_mem_cfg.phy_in_ep&0x1f))//epx in
			{
				sunxi_usb_dbg("eps in not ready\n");
			}
			else
			{
				puts("unsupported usb SUSB_DEPEVT_XFERNOTREADY interrupt\n");
			}
			break;

		case SUSB_DEPEVT_XFERINPROGRESS:
		case SUSB_DEPEVT_RXTXFIFOEVT:
		case SUSB_DEPEVT_STREAMEVT:
		case SUSB_DEPEVT_EPCMDCMPLT:
		default:
			puts("unsupported usb interrupt\n");
			break;
	}

	return;
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
static void __gadget_interrupt(__u32 event_context)
{
	__u32 temp;
	__u32 vale;

  	temp = ((event_context>>8)&0x0f);
	switch(temp)
  	{
  		/*Disconnect*/
  		case SUSB_DEVICE_EVENT_DISCONNECT:
  			sunxi_usb_dbg("IRQ: disconnect\n");
  			break;
	  	/*RESET*/
	  	case SUSB_DEVICE_EVENT_RESET:
	  	{
			pSUSBTRB setup_trb;

			sunxi_usb_dbg("IRQ: reset\n");
			SUSB_Dev_Test_Mode_Cofig( 0);

			SUSB_Dev_Stop_Active_Transfer();
			SUSB_Dev_Stop_All_Stall_Ep();
			SUSB_Dev_SetAddress(0);

			sunxi_udc_source.address = 0;					    //default value
			sunxi_udc_source.speed = USB_SPEED_HIGH;		//default value

			//Setup TRB0 for Control Transfer
		   	setup_trb = (pSUSBTRB)&usb_mem_cfg.pSDEVTRB0[0];

		   	memset(sunxi_udc_source.standard_reg, 0, sizeof(struct usb_device_request));
		   	setup_trb->bpl = (u32)(sunxi_udc_source.standard_reg);
		   	setup_trb->bph = (u32)0;
		   	setup_trb->size = (0x08&0x0ffffff);
	  	 	setup_trb->ctl = ((0x01<<11)|(0x02<<4)|(0x01<<1)|(0x01<<0));

			sunxi_udc_source.eps_bulk_step = SUNXI_USB_BULK_PREPARE;
			sunxi_udc_source.ep0_control_step = SUNXI_USB_CONTROL_CMD_OK;

			if(SUSB_Ep_CMD_Implement(usb_mem_cfg, (u32)setup_trb, 0, SUSB_DEPCMD_STARTTRANSFER, 0))
		   	{
		    	printf("reset handle: fail to Issue a SUSB_DEPCMD_STARTTRANSFER command for physical ep0\n");
		   	}

			sunxi_udev_active->state_reset();

	  		break;
	  	}
	  	/*Connect Done*/
	  	case SUSB_DEVICE_EVENT_CONNECT_DONE:
		{
			sunxi_usb_dbg("SUSB_DEVICE_EVENT_CONNECT_DONE\n");
		    /*...*/
		    sunxi_usb_burn_from_boot_init = 1;
		    /*Issue DEPCFG for physical ep 0&1, which are control ep 0 out and ep0 in*/
			if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 0, SUSB_DEPCMD_SETEPCONFIG, 0))
			{
				printf("sunxi usb err: fail to Issue a DEPXFERCFG command for physical ep0\n");
			}
			if(SUSB_Ep_CMD_Implement(usb_mem_cfg, 0, 1, SUSB_DEPCMD_SETEPCONFIG, 0))
			{
				printf("sunxi usb err: fail to Issue a DEPXFERCFG command for physical ep0\n");
			}

		    break;
		}
	  	/*Link Status Change*/
	  	case SUSB_DEVICE_EVENT_LINK_STATUS_CHANGE:
	  	{
			sunxi_usb_dbg("IRQ: SUSB_DEVICE_EVENT_LINK_STATUS_CHANGE\n");
	  		vale = ((event_context>>16)&0x0ff);
	  		if(vale & 0x10)//SuperSpeed Mode
	  		{
				;
	  		}
	  		else//HS/FS Mode
	  		{
	  			switch(vale&0x0f)
	  			{
					case 0x00:
						sunxi_usb_dbg("IRQ: Non-SS On-State\n");
						break;
					case 0x02:
						sunxi_usb_dbg("IRQ: Non-SS Sleep\n");
						break;
					case 0x03:
						sunxi_usb_dbg("IRQ: Non-SS suspend\n");
						break;
					case 0x05:
						sunxi_usb_dbg("IRQ: Non-SS Early Suspend\n");
						break;
					default:
						break;
	  			}
	  		}
	  	}
	  	break;
	  	/*Resume*/
	  	case SUSB_DEVICE_EVENT_WAKEUP:
	  	 	sunxi_usb_dbg("IRQ: resume\n");
	  		break;

	  	/*End of Periodic Frame*/
	  	case SUSB_DEVICE_EVENT_EOPF:
	  		break;
#if 0
	  	/*SOF*/
	  	case SUSB_DEVICE_EVENT_SOF:
			sunxi_usb_burn_from_boot_init = 1;

			SUSB_Dev_Interrupt_Ctl(SUSB_INTERRUPT_NO_SOF_EN);

	  		break;
#endif
	  	/*Erratic Error*/
	    case SUSB_DEVICE_EVENT_ERRATIC_ERROR:
	    	sunxi_usb_dbg("IRQ: erratic error\n");
	    break;

	    /*Generic Command Complete*/
	    case SUSB_DEVICE_EVENT_CMD_CMPL:
	    	break;

	    /*Event Buffer Overflow*/
	    case SUSB_DEVICE_EVENT_OVERFLOW:
	    	sunxi_usb_dbg("IRQ: event buffer overflow\n");
	    	break;

	    default:
	    	sunxi_usb_dbg("IRQ: BREAK\n");
	    	break;
  	}

  	return ;
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
static void __usb_process_event_entry(__u32 event_context)
{
	if((event_context&0x01) == 0)
	{
		/*Endpoint Interrupt Handle*/
		__endpoint_interrupt(event_context);
	}
	else
	{
		/*Device Bus Interrupt Handle*/
		switch(((event_context>>1)&0x07f))
		{
			case SUSB_EVENT_TYPE_DEV:
		  		__gadget_interrupt(event_context);
		  		break;
		  	default:
		  		break;
		}
	}
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
static int __process_event_buf(__u32 buf_Num)
{
	struct susb_event_buffer *event;
	__u32 count;

	count = SUSB_Dev_GetEvtCount(buf_Num);
	count &= 0xfffc;

	if(!count)
	{
		printf("USB OTG: Error Event Count\n");
		return (-1);
	}

    event = usb_mem_cfg.event_buf[buf_Num];

	while(count > 0)
	{
        __u32 event_context;

	    event_context = readl((__u32)(event->buf_start_l)+(event->lpos));
		__usb_process_event_entry(event_context);

		event->lpos += 4;

		if(event->lpos >= USB3_EVENT_BUF_SIZE)
		{
        	event->lpos = 0;
		}

		count -= 4;
		SUSB_Dev_SetEvtCount(4, buf_Num);
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
static int __usb_prepare_mem(void)
{
	int    evt_cnt;
	int    i;
	char   *tmp_buffer;

	evt_cnt = SUSB_Get_EvtCount();
	//malloc memory for usb event global buffer
	usb_mem_cfg.usb_evt_buffer = (char *)malloc_noncache(USB3_EVENT_BUF_SIZE * (evt_cnt + 1));
	if(!usb_mem_cfg.usb_evt_buffer)
	{
		printf("sunxi usb init err: unable to malloc memory for event global buffer\n");

		return -1;
	}
	memset(usb_mem_cfg.usb_evt_buffer, 0, USB3_EVENT_BUF_SIZE * (evt_cnt + 1));
	sunxi_usb_dbg("usb_evt_buffer=0x%x\n", (uint)usb_mem_cfg.usb_evt_buffer);

	usb_mem_cfg.trb0_buffer = (char *)malloc_noncache(sizeof(struct susb_trb_hw) * (SUSB_TRB0_CNT + 1));
	if(!usb_mem_cfg.trb0_buffer)
	{
		printf("sunxi usb init err: unable to malloc memory for trb0 hardware buffer\n");

		goto __usb_prepare_mem_err1;
	}
	memset(usb_mem_cfg.trb0_buffer, 0, sizeof(struct susb_trb_hw) * (SUSB_TRB0_CNT + 1));

	usb_mem_cfg.trbx_buffer = (char *)malloc_noncache(sizeof(struct susb_trb_hw) * (SUSB_TRBs_CNT + 1));
	if(!usb_mem_cfg.trbx_buffer)
	{
		printf("sunxi usb init err: unable to malloc memory for trbx hardware buffer\n");

		goto __usb_prepare_mem_err1;
	}
	memset(usb_mem_cfg.trbx_buffer, 0, sizeof(struct susb_trb_hw) * (SUSB_TRBs_CNT + 1));
	//

	tmp_buffer = (char *)(((uint)usb_mem_cfg.usb_evt_buffer + USB3_EVENT_BUF_SIZE - 1) & USB3_EVENT_BUF_ALIGN);
	//
	for(i=0; i<evt_cnt; i++)
	{
		usb_mem_cfg.event_buf[i] = (struct susb_event_buffer *)malloc_noncache(sizeof(struct susb_event_buffer));
		if(!usb_mem_cfg.event_buf[i])
		{
			printf("sunxi usb init err: unable to malloc memory for event %d buffer\n", i);

			goto __usb_prepare_mem_err2;
		}

		usb_mem_cfg.event_buf[i]->length      = USB3_EVENT_BUF_SIZE;
		usb_mem_cfg.event_buf[i]->buf_start_l = (uint *)(tmp_buffer + i * USB3_EVENT_BUF_SIZE);
	}
	//
	tmp_buffer = (char *)(((uint)usb_mem_cfg.trb0_buffer + 0x0f) & (~0x0f));
	usb_mem_cfg.pSDEVTRB0 = (struct susb_trb_hw *)tmp_buffer;


	sunxi_usb_dbg("trb0_buffer=0x%x\n", (uint)usb_mem_cfg.trb0_buffer);
	sunxi_usb_dbg("usb_mem_cfg.pSDEVTRB0=0x%x\n", (uint)usb_mem_cfg.pSDEVTRB0);
	//
	tmp_buffer = (char *)(((uint)usb_mem_cfg.trbx_buffer + 0x0f) & (~0x0f));
	usb_mem_cfg.pSDEVTRBx = (struct susb_trb_hw *)tmp_buffer;

	sunxi_usb_dbg("trbx_buffer=0x%x\n", (uint)usb_mem_cfg.trbx_buffer);
	sunxi_usb_dbg("usb_mem_cfg.pSDEVTRBx=0x%x\n", (uint)usb_mem_cfg.pSDEVTRBx);

	return 0;

__usb_prepare_mem_err2:

	for(i=0; i<evt_cnt; i++)
	{
		if(usb_mem_cfg.event_buf[i])
		{
			free(usb_mem_cfg.event_buf[i]);
		}
		else
		{
			break;
		}
	}
__usb_prepare_mem_err1:
	if(usb_mem_cfg.trb0_buffer)
	{
		free(usb_mem_cfg.trb0_buffer);

	}
	if(usb_mem_cfg.trbx_buffer)
	{
		free(usb_mem_cfg.trbx_buffer);
	}

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
static int __usb_release_mem(void)
{
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
void sunxi_usb_main_loop(int delaytime)
{
	int ret;

	if(sunxi_usb_init(delaytime))
	{
		printf("usb init fail\n");

		sunxi_usb_exit();

		return ;
	}
	printf("usb init ok\n");

	while(1)
	{
		ret = sunxi_udev_active->state_loop(&sunxi_ubuf);
		if(ret)
		{
			break;
		}

		if(ctrlc())
		{
			break;
		}
	}

	printf("exit usb\n");
	sunxi_usb_exit();

	sunxi_update_subsequent_processing(ret);

	return ;
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
int sunxi_usb_extern_loop(void)
{
	return sunxi_udev_active->state_loop(&sunxi_ubuf);
}

