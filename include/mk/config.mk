#
# Copyright (C) 2016 Alexey Kodanev <akodanev@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

ifndef CONFIG_LOADED
CONFIG_LOADED = 1

VERSION		= 2.0

CFLAGS_DBG	= $(subst DEBUG,-DDEBUG,$(DBG))
INCLUDE		= -I$(top_dir)/include
WCFLAGS		= -Wall -W
CFLAGS		= -g -O2 -pipe -Wp,-D_FORTIFY_SOURCE=2 $(WCFLAGS) $(CFLAGS_DBG)
LDFLAGS		= -g -Wl,-O1

ifneq ($(CROSS_DIR),"/")
SDK_DIR		:= $(patsubst %/,%,$(CROSS_DIR))
else
SDK_DIR		:= $(CROSS_DIR)
endif

#
# Cross-compiling for LinPac SDK
#
ifneq ($(filter lincon%, $(CROSS)),)

ifeq ($(SDK_DIR),)
SDK_DIR		:= ~/$(CROSS)
endif

EXIST ?= $(shell test -d "$(SDK_DIR)"; echo $$?)
ifeq ($(EXIST),1)
$(error You must have 'lincon' SDK directory: $(SDK_DIR))
endif

CC_PREFIX	= arm-linux-
SDK_TOOLS_DIR	= $(SDK_DIR)/tools

PATH		:= $(SDK_TOOLS_DIR)/bin:$(SDK_TOOLS_DIR)/sbin:$(PATH)
INCPATH		= $(SDK_TOOLS_DIR)/include
LIBPATH		= $(SDK_TOOLS_DIR)/lib
INCI8K		= $(SDK_DIR)/i8k/include
LIBI8K		= $(SDK_DIR)/i8k/lib
INCLUDE		+= -I$(INCPATH) -I$(INCI8K)
CFLAGS		+= -DCONFIG_LINPAC_TARGET
LDFLAGS		+= -L$(LIBI8K)
LDLIBS		= -li8k
else
ifneq ($(CROSS),)
$(error Not supported cross-compilation: $(CROSS))
endif
endif

CROSS		?= $(shell uname -i)

CC		= $(CC_PREFIX)gcc
AR		= $(CC_PREFIX)ar

.PHONY: clean clean-all

endif
