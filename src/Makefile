#
# $Id$
# Copyright (C) 2002  Stanislav Ievlev <inger@altlinux.org>
# Copyright (C) 2002  Dmitry V. Levin <ldv@altlinux.org>
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

CPPFLAGS = $(RPM_OPT_FLAGS) -g -Wall -D_GNU_SOURCE $(shell gtk-config --cflags)
LDFLAGS = -lgtk -lgdk
INSTALL = install
PACKAGE = userpasswd

SRC = child.c conv.c main.c parent.c ui.c
OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)

bindir = $(RPM_BUILD_ROOT)/usr/bin

.PHONY:	all install clean

all: $(PACKAGE)

$(PACKAGE): $(OBJ)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

install: all
	$(INSTALL) -pD -m755 $(PACKAGE) $(bindir)/$(PACKAGE)

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
