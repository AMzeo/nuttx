############################################################################
# boards/arm/pic32czca90/pic32czca90-curiosity/src/Makefile
#
# SPDX-License-Identifier: Apache-2.0
############################################################################

include $(TOPDIR)/Make.defs

CSRCS  = pic32czca90_boot.c

ifeq ($(CONFIG_ARCH_LEDS),y)
CSRCS += pic32czca90_autoleds.c
else
CSRCS += pic32czca90_userleds.c
endif

ifeq ($(CONFIG_NSH_ARCHINIT),y)
CSRCS += pic32czca90_appinit.c
endif

ifeq ($(CONFIG_BOARDCTL),y)
CSRCS += pic32czca90_bringup.c
endif

include $(TOPDIR)/boards/Board.mk
