#
# (C) Copyright 2003-2006
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(VENDOR).o

SOBJS-$(CONFIG_USE_NEON_SIMD) += arm_neon.o

COBJS-y	+= board_common.o
COBJS-y += power_probe.o
COBJS-y	+= de.o
COBJS-y += usb_probe.o

ifndef CONFIG_SUNXI_SPINOR_PLATFORM
COBJS-y += load_check.o
COBJS-y += cartoon/sprite_cartoon.o
COBJS-y += cartoon/sprite_progressbar/sprite_progressbar.o
COBJS-y += cartoon/sprite_draw/sprite_draw.o
COBJS-y += bat_cartoon.o
COBJS-y += cartoon/sprite_char/sprite_char.o
COBJS-y += cartoon/sprite_char/sfte/FontEngine.o
COBJS-y += cartoon/sprite_char/sfte/sfte.o
COBJS-y += key_burn.o
endif


COBJS-y += sys_partition.o		\
		   sys_config.o

COBJS-$(CONFIG_OPENSSL) += openssl.o

COBJS-y += sunxi_debug.o

SOBJS   := $(SOBJS-y)
COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c) $(SOBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))

all:    $(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
