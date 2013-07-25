dnl sxid - suid, sgid file and directory checking
dnl Copyright © 1999, 2000, 2002 Ben Collins <bcollins@debian.org>
dnl Copyright © 2009, 2013 Timur Birsh <taem@linukz.org>

dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.

dnl You should have received a copy of the GNU General Public License along
dnl with this program; if not, write to the Free Software Foundation, Inc.,
dnl 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

dnl Process this file with autoconf to produce a configure script.

AC_INIT([sXid], [4.3], [taem@linukz.org], [sxid], [http://linukz.org/sxid.shtml])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_PROG_CC
AC_PROG_INSTALL

mail_prog="mail"
AC_CHECK_PROG([UNAME], [uname], [uname])
system=`$UNAME`
AC_MSG_CHECKING([OS])
AC_MSG_RESULT([$system])
case $system in
  Linux)
    ;;
  *)
    mail_prog="sendmail"
    ;;
esac

if test "$mail_prog" = "mail"; then
  AC_DEFINE([CL_SUBJ], [1],
            [Define if your mailer accepts the subject in the body of the mail])
fi

AC_PATH_PROG([MAIL_PROG], ["$mail_prog"], [/usr/bin/mail],
             [$PATH$PATH_SEPARATOR/usr/sbin$PATH_SEPARATOR/usr/lib])
AC_DEFINE_UNQUOTED([MAIL_PROG], ["$ac_cv_path_MAIL_PROG"],
                   [Define to the location of the default mail program])

AC_CHECK_FUNC([getopt_long_only], [AC_DEFINE([HAVE_GETOPT], [1],
                                             [Define if you have getopt()])],
              [EXTRA_OBJ="getopt.o getopt1.o"])

AC_CHECK_FUNCS([strtok_r])

AC_SUBST([EXTRA_OBJ])

test "x$prefix" = xNONE && prefix="$ac_default_prefix"
TMPCONF=`eval echo $sysconfdir`
AC_SUBST([TMPCONF])
AC_DEFINE_UNQUOTED([CONF_FILE], ["${TMPCONF}/sxid.conf"],
                   [Define to the location of the default config file])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile docs/sxid.1 docs/sxid.conf.5])
AC_OUTPUT
