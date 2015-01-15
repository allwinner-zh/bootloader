#
# (C) Copyright 2000-2007
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

LIB	:= $(obj)libstandby.o

SOBJS   := standby_s.o

COBJS-y += clock_standby.o
COBJS-y += dram_standby.o
COBJS-y += int_standby.o
COBJS-y += power_ops_standby.o
COBJS-y += sw_key_check.o
COBJS-y += dram_standby_init.o
COBJS-y += power_ops.o
COBJS-y += standby.o
COBJS-y += serial_standby.o
COBJS-y += iic_standby.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c) $(SOBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
