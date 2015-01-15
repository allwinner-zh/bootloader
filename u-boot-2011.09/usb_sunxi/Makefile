#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libsunxi_usb.o


ifndef CONFIG_SUNXI_SPINOR_PLATFORM

COBJS-y += usb_mass.o
COBJS-y += usb_efex.o
COBJS-y += usb_fastboot.o
COBJS-y += usb_burn.o
COBJS-$(CONFIG_SUNXI_HDCP_IN_SECURESTORAGE) += key_deal.o
COBJS-y += usb_base_common.o
COBJS-y += buf_queue.o
COBJS-y += efex_queue.o
COBJS-y += usb_efex_test.o

COBJS-$(CONFIG_SUN6I) += usb_base_31.o
COBJS-$(CONFIG_ARCH_SUN8IW3P1)   += usb_base_50.o
COBJS-$(CONFIG_ARCH_SUN8IW3P1)   += usb_dma_control.o
COBJS-$(CONFIG_ARCH_SUN8IW5P1)   += usb_base_67.o
COBJS-$(CONFIG_ARCH_SUN8IW5P1)   += usb_dma_control.o
COBJS-$(CONFIG_ARCH_SUN8IW6P1)   += usb_base_73.o
COBJS-$(CONFIG_ARCH_SUN8IW6P1)   += usb_dma_control.o
COBJS-$(CONFIG_ARCH_SUN9IW1P1)   += usb_base_39.o
COBJS-$(CONFIG_SUN7I) 			 += usb_base_51.o
COBJS-$(CONFIG_SUN5I)			 += usb_base_13.o
COBJS-$(CONFIG_ARCH_SUN8IW7P1)   += usb_base_80.o
COBJS-$(CONFIG_ARCH_SUN8IW7P1)   += usb_dma_control.o
COBJS-$(CONFIG_ARCH_SUN8IW8P1)   += usb_base_81.o
COBJS-$(CONFIG_ARCH_SUN8IW8P1)   += usb_dma_control.o
COBJS-$(CONFIG_ARCH_SUN8IW9P1)   += usb_base_77.o
COBJS-$(CONFIG_ARCH_SUN8IW9P1)   += usb_dma_control.o

endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
