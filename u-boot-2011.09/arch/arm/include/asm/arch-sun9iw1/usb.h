/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Yana <yangnaitian@allwinnertech.com>
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
#ifndef  __USB_39_OPERATION_H__
#define  __USB_39_OPERATION_H__

#include <asm/arch/cpu.h>
//#define CONFIG_SUSB_1_1_DEVICE
#define CONFIG_SUSB_2_0_DEVICE
//#define CONFIG_SUSB_3_0_DEVICE

#define USB3_EVENT_BUF_SIZE   128
#define USB3_EVENT_BUF_ALIGN  (~0x7f)

#define SUSB_TRB0_CNT   8
#define SUSB_TRBs_CNT   8
#define SUSB_EVENT_MAX  8

/* usb device switch */
#define  USBC_DEVICE_SWITCH_OFF    0
#define  USBC_DEVICE_SWITCH_ON     1

/* usb OTG mode */
#define  USBC_OTG_HOST        0
#define  USBC_OTG_DEVICE      1

/* usb device type */
#define  USBC_DEVICE_HSDEV    0
#define  USBC_DEVICE_FSDEV    1
#define  USBC_DEVICE_LSDEV    2

/*  usb transfer type  */
#define  USBC_TS_TYPE_IDLE    0
#define  USBC_TS_TYPE_CTRL    1
#define  USBC_TS_TYPE_ISO     2
#define  USBC_TS_TYPE_INT     3
#define  USBC_TS_TYPE_BULK    4

/*  usb transfer mode  */
#define  USBC_TS_MODE_UNKOWN  0
#define  USBC_TS_MODE_LS      1
#define  USBC_TS_MODE_FS      2
#define  USBC_TS_MODE_HS      3
#define  USBC_TS_MODE_SS      4

/* usb id type */
#define  USBC_ID_TYPE_DISABLE      	0
#define  USBC_ID_TYPE_HOST         	1
#define  USBC_ID_TYPE_DEVICE       	2
//-----------------------------------------------------------------------
//   reg offset
//-----------------------------------------------------------------------

/* Global Registers */
#define SUSB_o_GSBUSCFG0		 0xc100
#define SUSB_o_GSBUSCFG1		 0xc104
#define SUSB_o_GTXTHRCFG		 0xc108
#define SUSB_o_GRXTHRCFG		 0xc10c
#define SUSB_o_GCTL		         0xc110
#define SUSB_o_GEVTEN		     0xc114
#define SUSB_o_GSTS		         0xc118
#define SUSB_o_GSNPSID		     0xc120
#define SUSB_o_GGPIO		     0xc124
#define SUSB_o_GUID		         0xc128
#define SUSB_o_GUCTL		     0xc12c
#define SUSB_o_GBUSERRADDR0	     0xc130
#define SUSB_o_GBUSERRADDR1	     0xc134
#define SUSB_o_GPRTBIMAP0		 0xc138
#define SUSB_o_GPRTBIMAP1		 0xc13c
#define SUSB_o_GHWPARAMS0		 0xc140
#define SUSB_o_GHWPARAMS1		 0xc144
#define SUSB_o_GHWPARAMS2		 0xc148
#define SUSB_o_GHWPARAMS3		 0xc14c
#define SUSB_o_GHWPARAMS4		 0xc150
#define SUSB_o_GHWPARAMS5		 0xc154
#define SUSB_o_GHWPARAMS6		 0xc158
#define SUSB_o_GHWPARAMS7		 0xc15c
#define SUSB_o_GDBGFIFOSPACE	 0xc160
#define SUSB_o_GDBGLTSSM		 0xc164
#define SUSB_o_GPRTBIMAP_HS0	 0xc180
#define SUSB_o_GPRTBIMAP_HS1	 0xc184
#define SUSB_o_GPRTBIMAP_FS0	 0xc188
#define SUSB_o_GPRTBIMAP_FS1	 0xc18c

#define SUSB_o_GUSB2PHYCFG(n)	(0xc200 + (n * 0x04))
#define SUSB_o_GUSB2I2CCTL(n)	(0xc240 + (n * 0x04))

#define SUSB_o_GUSB2PHYACC(n)	(0xc280 + (n * 0x04))

#define SUSB_o_GUSB3PIPECTL(n)	(0xc2c0 + (n * 0x04))

#define SUSB_o_GTXFIFOSIZ(n)	(0xc300 + (n * 0x04))
#define SUSB_o_GRXFIFOSIZ(n)	(0xc380 + (n * 0x04))

#define SUSB_o_GEVNTADRLO(n)	(0xc400 + (n * 0x10))
#define SUSB_o_GEVNTADRHI(n)	(0xc404 + (n * 0x10))
#define SUSB_o_GEVNTSIZ(n)	    (0xc408 + (n * 0x10))
#define SUSB_o_GEVNTCOUNT(n)	(0xc40c + (n * 0x10))

#define SUSB_o_GHWPARAMS8		 0xc600

/* Device Registers */
#define SUSB_o_DCFG		         0xc700
#define SUSB_o_DCTL		         0xc704
#define SUSB_o_DEVTEN		     0xc708
#define SUSB_o_DSTS		         0xc70c
#define SUSB_o_DGCMDPAR		     0xc710
#define SUSB_o_DGCMD		     0xc714
#define SUSB_o_DALEPENA		     0xc720
#define SUSB_o_DEPCMDPAR2(n)	(0xc800 + (n * 0x10))
#define SUSB_o_DEPCMDPAR1(n)	(0xc804 + (n * 0x10))
#define SUSB_o_DEPCMDPAR0(n)	(0xc808 + (n * 0x10))
#define SUSB_o_DEPCMD(n)		(0xc80c + (n * 0x10))


//-----------------------------------------------------------------------
//   register
//-----------------------------------------------------------------------

/* Global Registers */
#define SUSB_GSBUSCFG0		                 (SUNXI_USBOTG_BASE + SUSB_o_GSBUSCFG0		)
#define SUSB_GSBUSCFG1		                 (SUNXI_USBOTG_BASE + SUSB_o_GSBUSCFG1		)
#define SUSB_GTXTHRCFG		                 (SUNXI_USBOTG_BASE + SUSB_o_GTXTHRCFG		)
#define SUSB_GRXTHRCFG		                 (SUNXI_USBOTG_BASE + SUSB_o_GRXTHRCFG		)
#define SUSB_GCTL		                     (SUNXI_USBOTG_BASE + SUSB_o_GCTL		    )
#define SUSB_GEVTEN		                     (SUNXI_USBOTG_BASE + SUSB_o_GEVTEN		    )
#define SUSB_GSTS		                     (SUNXI_USBOTG_BASE + SUSB_o_GSTS		    )
#define SUSB_GSNPSID		                 (SUNXI_USBOTG_BASE + SUSB_o_GSNPSID		)
#define SUSB_GGPIO		                     (SUNXI_USBOTG_BASE + SUSB_o_GGPIO     		)
#define SUSB_GUID		                     (SUNXI_USBOTG_BASE + SUSB_o_GUID      		)
#define SUSB_GUCTL		                     (SUNXI_USBOTG_BASE + SUSB_o_GUCTL		    )
#define SUSB_GBUSERRADDR0	                 (SUNXI_USBOTG_BASE + SUSB_o_GBUSERRADDR0	)
#define SUSB_GBUSERRADDR1	                 (SUNXI_USBOTG_BASE + SUSB_o_GBUSERRADDR1	)
#define SUSB_GPRTBIMAP0		                 (SUNXI_USBOTG_BASE + SUSB_o_GPRTBIMAP0		)
#define SUSB_GPRTBIMAP1		                 (SUNXI_USBOTG_BASE + SUSB_o_GPRTBIMAP1		)
#define SUSB_GHWPARAMS0		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS0		)
#define SUSB_GHWPARAMS1		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS1		)
#define SUSB_GHWPARAMS2		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS2		)
#define SUSB_GHWPARAMS3		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS3		)
#define SUSB_GHWPARAMS4		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS4		)
#define SUSB_GHWPARAMS5		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS5		)
#define SUSB_GHWPARAMS6		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS6		)
#define SUSB_GHWPARAMS7		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS7		)
#define SUSB_GDBGFIFOSPACE	                 (SUNXI_USBOTG_BASE + SUSB_o_GDBGFIFOSPACE	)
#define SUSB_GDBGLTSSM		                 (SUNXI_USBOTG_BASE + SUSB_o_GDBGLTSSM		)
#define SUSB_GPRTBIMAP_HS0	                 (SUNXI_USBOTG_BASE + SUSB_o_GPRTBIMAP_HS0	)
#define SUSB_GPRTBIMAP_HS1	                 (SUNXI_USBOTG_BASE + SUSB_o_GPRTBIMAP_HS1	)
#define SUSB_GPRTBIMAP_FS0	                 (SUNXI_USBOTG_BASE + SUSB_o_GPRTBIMAP_FS0	)
#define SUSB_GPRTBIMAP_FS1	                 (SUNXI_USBOTG_BASE + SUSB_o_GPRTBIMAP_FS1	)

#define SUSB_GUSB2PHYCFG(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GUSB2PHYCFG(n) )
#define SUSB_GUSB2I2CCTL(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GUSB2I2CCTL(n) )

#define SUSB_GUSB2PHYACC(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GUSB2PHYACC(n) )

#define SUSB_GUSB3PIPECTL(n)	             (SUNXI_USBOTG_BASE + SUSB_o_GUSB3PIPECTL(n))

#define SUSB_GTXFIFOSIZ(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GTXFIFOSIZ(n)  )
#define SUSB_GRXFIFOSIZ(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GRXFIFOSIZ(n)  )

#define SUSB_GEVNTADRLO(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GEVNTADRLO(n)  )
#define SUSB_GEVNTADRHI(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GEVNTADRHI(n)  )
#define SUSB_GEVNTSIZ(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GEVNTSIZ(n)    )
#define SUSB_GEVNTCOUNT(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_GEVNTCOUNT(n)  )

#define SUSB_GHWPARAMS8		                 (SUNXI_USBOTG_BASE + SUSB_o_GHWPARAMS8	    )

/* Device Registers */
#define SUSB_DCFG		                     (SUNXI_USBOTG_BASE + SUSB_o_DCFG	        )
#define SUSB_DCTL		                     (SUNXI_USBOTG_BASE + SUSB_o_DCTL	        )
#define SUSB_DEVTEN		                     (SUNXI_USBOTG_BASE + SUSB_o_DEVTEN	        )
#define SUSB_DSTS		                     (SUNXI_USBOTG_BASE + SUSB_o_DSTS	        )
#define SUSB_DGCMDPAR		                 (SUNXI_USBOTG_BASE + SUSB_o_DGCMDPAR	    )
#define SUSB_DGCMD		                     (SUNXI_USBOTG_BASE + SUSB_o_DGCMD	        )
#define SUSB_DALEPENA		                 (SUNXI_USBOTG_BASE + SUSB_o_DALEPENA	    )
#define SUSB_DEPCMDPAR2(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_DEPCMDPAR2(n)  )
#define SUSB_DEPCMDPAR1(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_DEPCMDPAR1(n)  )
#define SUSB_DEPCMDPAR0(n)	                 (SUNXI_USBOTG_BASE + SUSB_o_DEPCMDPAR0(n)  )
#define SUSB_DEPCMD(n)		                 (SUNXI_USBOTG_BASE + SUSB_o_DEPCMD(n)      )


//-----------------------------------------------------------------------
//   bit position
//-----------------------------------------------------------------------

/* Global Configuration Register */
#define SUSB_GCTL_CORESOFTRESET	(1 << 11)

/* Global USB2 PHY Configuration Register */
#define SUSB_GUSB2PHYCFG_PHYSOFTRST (1 << 31)
#define SUSB_GUSB2PHYCFG_SUSPHY	(1 << 6)

/* Global USB3 PIPE Control Register */
#define SUSB_GUSB3PIPECTL_PHYSOFTRST (1 << 31)
#define SUSB_GUSB3PIPECTL_SUSPHY (1 << 17)

/*Device Configuration Register*/
#define SUSB_DCFG_DEVADDR_MASK	SUSB_DCFG_DEVADDR(0x7f)

/*Device Control*/
#define SUSB_DEV_RUN_STOP    (0x01U<<31)
#define SUSB_DCTL_TSTCTRL_MASK	(0xf << 1)

//-----------------------------------------------------------------------
//  Function Type
//-----------------------------------------------------------------------

//Interrupt Enable Bit
#define SUSB_INTERRUPT_ALL_EN  0x1e9f
#define SUSB_INTERRUPT_NO_SOF_EN   0x1e1f

//Device Interupt Type
#define SUSB_EVENT_TYPE_DEV	0
#define SUSB_EVENT_TYPE_CARKIT	3
#define SUSB_EVENT_TYPE_I2C	4

//Device Interrupt Event Type
#define SUSB_DEVICE_EVENT_DISCONNECT		0
#define SUSB_DEVICE_EVENT_RESET			1
#define SUSB_DEVICE_EVENT_CONNECT_DONE		2
#define SUSB_DEVICE_EVENT_LINK_STATUS_CHANGE	3
#define SUSB_DEVICE_EVENT_WAKEUP		4
#define SUSB_DEVICE_EVENT_EOPF			6
#define SUSB_DEVICE_EVENT_SOF			7
#define SUSB_DEVICE_EVENT_ERRATIC_ERROR		9
#define SUSB_DEVICE_EVENT_CMD_CMPL		10
#define SUSB_DEVICE_EVENT_OVERFLOW		11

//Endpoint Interrupt Type
#define SUSB_DEPEVT_XFERCOMPLETE	  0x01
#define SUSB_DEPEVT_XFERINPROGRESS	0x02
#define SUSB_DEPEVT_XFERNOTREADY	  0x03
#define SUSB_DEPEVT_RXTXFIFOEVT		  0x04
#define SUSB_DEPEVT_STREAMEVT		    0x06
#define SUSB_DEPEVT_EPCMDCMPLT		  0x07

//Endpoint Command Type
#define SUSB_DEPCMD_DEPSTARTCFG		(0x09 << 0)
#define SUSB_DEPCMD_ENDTRANSFER		(0x08 << 0)
#define SUSB_DEPCMD_UPDATETRANSFER	(0x07 << 0)
#define SUSB_DEPCMD_STARTTRANSFER	(0x06 << 0)
#define SUSB_DEPCMD_CLEARSTALL		(0x05 << 0)
#define SUSB_DEPCMD_SETSTALL		(0x04 << 0)
#define SUSB_DEPCMD_GETSEQNUMBER	(0x03 << 0)
#define SUSB_DEPCMD_SETTRANSFRESOURCE	(0x02 << 0)
#define SUSB_DEPCMD_SETEPCONFIG		(0x01 << 0)

//-----------------------------------------------------------------------
//   USB3 Struct
//-----------------------------------------------------------------------

/**
 * struct susb_event_buffer - Software event buffer representation
 * @buf_start_l: low 32 bits of event buffer basic address pointer
 * @buf_start_h: high 32 bits of event buffer basic address pointer
 * @length: size of this buffer
 */
struct susb_event_buffer
{
	u32   *buf_start_l;
	u32   *buf_start_h;

	u32	  length;
	u32	  lpos;
};

/**
 * struct susb_gadget_ep_cmd_params
 */
struct susb_gadget_ep_cmd_params
{
	u32 param2;
	u32 param1;
	u32 param0;
	u32 res0;
};

/**
 * struct susb_trb_hw
 */
typedef struct susb_trb_hw
{
	u32 bpl;
	u32 bph;
	u32 size;
	u32 ctl;
}SUSBTRB, *pSUSBTRB;


typedef struct
{
	struct susb_event_buffer   *event_buf[SUSB_EVENT_MAX];
	char                       *usb_evt_buffer;
	char                       *trb0_buffer;
	char                       *trbx_buffer;
	struct susb_trb_hw         *pSDEVTRB0;
	struct susb_trb_hw         *pSDEVTRBx;

	u32 bulk_ep_max;
	u32 ep0_maxpkt;

	int xfer_res;
	int phy_out_ep;
	int phy_in_ep;
}
susb_mem_cfg;


extern void SUSB_Dev_Stop_Active_Transfer(void);
extern void SUSB_Dev_Stop_All_Stall_Ep(void);
extern void SUSB_Dev_SetAddress(int address);
extern void SUSB_Dev_Test_Mode_Cofig(uint is_Normal_Test);
extern void SUSB_Dev_ConectSwitch(__u32 is_on);

extern void SUSB_Force_Role(__u32 id_type);
extern void SUSB_EnhanceSignal(void);
extern void SUSB_Dev_ConfigTransferMode(__u32 speed_mode);
extern void SUSB_Dev_EnaDisa_Ep_Function(__u32 is_Ena, __u32 bit_map);
extern void SUSB_Dev_PHY_Config(__u32 is_On, __u8 speed_mode);

extern void SUBS_Dev_CoreReset(void);
extern void SUSB_Config_Dev_Size(void);
extern void SUSB_Dev_Core_Rest(void);

extern s32  SUSB_Ep_CMD_Implement(susb_mem_cfg usb_cfg, __u32 p_trb, unsigned phy_ep, unsigned cmd, unsigned is_IOC);
extern void SUSB_Dev_Interrupt_Ctl(__u32 Ena_Val);
extern unsigned int SUSB_Dev_Log_to_Physical_Ep(int logi_ep, int is_Out);

extern uint SUSB_Dev_GetEvtCount(int evt_index);
extern void SUSB_Dev_SetEvtCount(uint value, int evt_index);
extern uint SUSB_Get_Speed_Mode(void);
extern uint SUSB_Get_EvtCount(void);
extern void SUSB_Set_Epmap_Enable(int phy_ep_no);
extern void SUSB_Event_Buffers_Setup(struct susb_event_buffer *event_buffer);

extern int usb_open_clock(void);
extern int usb_close_clock(void);


#endif   //__USB_39_OPERATION_H__

