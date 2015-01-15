#
# (C) Copyright 2000-2002
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

CROSS_COMPILE ?= $(CURDIR)/../gcc-linaro/bin/arm-linux-gnueabi-

ifndef CONFIG_STANDALONE_LOAD_ADDR
ifeq ($(SOC),omap3)
CONFIG_STANDALONE_LOAD_ADDR = 0x80300000
else
CONFIG_STANDALONE_LOAD_ADDR = 0xc100000
endif
endif

PLATFORM_CPPFLAGS += -DCONFIG_ARM -D__ARM__ -mfpu=neon -D__NEON_SIMD__

# Explicitly specifiy 32-bit ARM ISA since toolchain default can be -mthumb:
ifndef CONFIG_SPL
PLATFORM_CPPFLAGS += $(call cc-option,-marm,)
endif

# Try if EABI is supported, else fall back to old API,
# i. e. for example:
# - with ELDK 4.2 (EABI supported), use:
#	-mabi=aapcs-linux -mno-thumb-interwork
# - with ELDK 4.1 (gcc 4.x, no EABI), use:
#	-mabi=apcs-gnu -mno-thumb-interwork
# - with ELDK 3.1 (gcc 3.x), use:
#	-mapcs-32 -mno-thumb-interwork
ifndef CONFIG_SPL
PLATFORM_CPPFLAGS += $(call cc-option,\
				-mabi=aapcs-linux -mno-thumb-interwork,\
				$(call cc-option,\
					-mapcs-32,\
					$(call cc-option,\
						-mabi=apcs-gnu,\
					)\
				) $(call cc-option,-mno-thumb-interwork,)\
			)
else
PLATFORM_CPPFLAGS += $(call cc-option,\
				-mabi=aapcs-linux -mthumb-interwork,\
				$(call cc-option,\
					-mapcs-32,\
					$(call cc-option,\
						-mabi=apcs-gnu,\
					)\
				) $(call cc-option,-mthumb-interwork,)\
			)
endif

# For EABI, make sure to provide raise()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
# This file is parsed many times, so the string may get added multiple
# times. Also, the prefix needs to be different based on whether
# CONFIG_SPL_BUILD is defined or not. 'filter-out' the existing entry
# before adding the correct one.
ifdef CONFIG_SPL_BUILD
PLATFORM_LIBS := $(SPLTREE)/arch/arm/lib/eabi_compat.o \
	$(filter-out %/arch/arm/lib/eabi_compat.o, $(PLATFORM_LIBS))
else
PLATFORM_LIBS := $(OBJTREE)/arch/arm/lib/eabi_compat.o \
	$(filter-out %/arch/arm/lib/eabi_compat.o, $(PLATFORM_LIBS))
endif
endif

ifdef CONFIG_SYS_LDSCRIPT
# need to strip off double quotes
LDSCRIPT := $(subst ",,$(CONFIG_SYS_LDSCRIPT))
else
ifdef CONFIG_CPUS_STANDBY
LDSCRIPT := $(SRCTREE)/$(CPUDIR)/u-boot-cpus.lds
else
ifdef CONFIG_NO_BOOT_STANDBY
LDSCRIPT := $(SRCTREE)/$(CPUDIR)/u-boot-nostandby.lds
else
LDSCRIPT := $(SRCTREE)/$(CPUDIR)/u-boot.lds
endif
endif
endif

# needed for relocation
ifndef CONFIG_NAND_SPL
ifndef CONFIG_SPL
LDFLAGS_u-boot += -pie
endif
endif
