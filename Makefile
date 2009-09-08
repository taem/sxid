# sxid - suid, sgid file and directory checking
#
# Copyright (C) 1999, 2000, 2002 Ben Collins <bcollins@debian.org>
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2,
# or (at your option) any later version.
#
# This is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with sxid; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307  USA

all: configure source/Makefile
	cd source && $(MAKE) all

clean:
	-cd source && $(MAKE) clean

deb:
	debian/rules binary-arch

distclean: clean
	-rm -f config.log config.status source/Makefile config.cache \
		source/config.h docs/sxid.1 docs/sxid.conf.5 `find . -name core`

cvsclean: distclean
	-rm -f configure

install: all
	cd source && $(MAKE) install DESTDIR=$(DESTDIR)

source/Makefile: source/Makefile.in source/config.h.in docs/sxid.1.in docs/sxid.conf.5.in config.status
	@./config.status

config.status:
	@./configure --prefix=/usr --sysconfdir=/etc

configure: configure.in
	autoconf
