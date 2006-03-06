#
# $Id$
# Copyright (C) 2002  Stanislav Ievlev <inger@altlinux.org>
# Copyright (C) 2002, 2005, 2006  Dmitry V. Levin <ldv@altlinux.org>
# 
# Makefile for sources
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
#

CPPFLAGS = $(RPM_OPT_FLAGS) -D_GNU_SOURCE $(shell pkg-config gtk+-2.0 --cflags)
LDLIBS = $(shell pkg-config gtk+-2.0 --libs)
INSTALL = install
PACKAGE = userpasswd

SRC = child.c conv.c main.c parent.c ui.c
OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)

bindir = /usr/bin
datadir = /usr/share

.PHONY:	all install clean

all: $(PACKAGE)

$(PACKAGE): $(OBJ)
	$(LINK.o) $^ $(LDLIBS) -o $@

install: all
	$(INSTALL) -pD -m755 $(PACKAGE) $(RPM_BUILD_ROOT)$(bindir)/$(PACKAGE)
	$(INSTALL) -pD -m755 loop.sh $(RPM_BUILD_ROOT)$(datadir)/$(PACKAGE)/loop

clean:
	$(RM) $(DEP) $(OBJ) $(PACKAGE) *~

# We need dependencies only if goal isn't "clean"
ifneq ($(MAKECMDGOALS),clean)

%.d:	%.c
	@echo Making dependences for $<
	@$(SHELL) -ec "$(CC) -MM $(CPPFLAGS) $< | sed 's|\($*\)\.o[ :]*|\1.o $@ : |g' > $@; [ -s $@ ] || $(RM) $@"

ifneq ($(DEP),)
-include $(DEP)
endif

endif
