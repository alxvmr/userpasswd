#
# $Id$
# Copyright (C) 2002  Stanislav Ievlev <inger@altlinux.org>
# Copyright (C) 2002  Dmitry V. Levin <ldv@altlinux.org>
# 
# Makefile for pofiles
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

INSTALL = /usr/bin/install
XGETTEXT=/usr/bin/xgettext
MSGFMT=/usr/bin/msgfmt
MSGMERGE=/usr/bin/msgmerge

PACKAGE = userpasswd
POTFILE = $(PACKAGE).po
CATALOGS = ru
MOFILES = $(CATALOGS:=.mo)
SOURCES = $(wildcard ../src/*.c)

localedir = $(RPM_BUILD_ROOT)/usr/share/locale

.PHONY:	all install clean update

all: $(MOFILES)

install:
	@for n in $(CATALOGS); \
		do $(INSTALL) -vpD -m644 $$n.mo $(localedir)/$$n/LC_MESSAGES/$(PACKAGE).mo; \
	done

clean: 
	$(RM) $(MOFILES) $(POTFILE) *.old *.new

update: $(POTFILE)
	@for lang in $(CATALOGS); do \
		mv $$lang.po $$lang.old.po; \
		echo -n "$$lang "; \
		if $(MSGMERGE) $$lang.old.po $(POTFILE) -o $$lang.po; then \
			$(RM) $$lang.old.po; \
		else \
			echo "failed!"; \
			$(RM) $$lang.po; \
			mv $$lang.old.po $$lang.po; \
		fi; \
	done

$(POTFILE): $(SOURCES)
	$(XGETTEXT) --keyword=_ -d $(PACKAGE) $^

%.mo: %.po
	$(MSGFMT) $(OUTPUT_OPTION) $<
