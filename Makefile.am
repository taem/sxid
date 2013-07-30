# sxid - suid, sgid file and directory checking
# Copyright © 1999, 2000, 2002 Ben Collins <bcollins@debian.org>
# Copyright © 2009, 2013 Timur Birsh <taem@linukz.org>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

SUBDIRS = src

EXTRA_DIST = contrib examples src/getopt.c src/getopt1.c src/getopt.h \
	     autogen.sh README.md

man_MANS = docs/sxid.1 docs/sxid.conf.5
doc_DATA = README.md NEWS COPYING ChangeLog

contribdir = $(docdir)/contrib
contrib_DATA = contrib/sxid.spec

examplesdir = $(docdir)/examples
examples_DATA = examples/sxid.conf examples/sxid.cron
