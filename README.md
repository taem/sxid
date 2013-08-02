sXid
====

suid, sgid file and directory checking.

Overview
========

This program is meant to run as a cronjob. I have it run once a day, but
busy shell boxes may want to run it twice a day. Basically it tracks any
changes in your s[ug]id files and folders. If there are any new ones,
ones that aren't set any more, or they have changed bits or other modes
then it reports the changes. You can also run this manually for spot
checking.

It also tracks s[ug]id files by SHA-256 checksums. This helps detect
if a root kit has been installed which would not show under normal
name and permissions checking. Directories are tracked by inodes.

To install, set the options in sxid.conf (/etc/sxid.conf when installed)
and add an entry in root's crontab (it needs root permission to check ALL
files and folders). All log files are created mode 600 so no one will be
able to get a list.

Notes on reading the output:

 - In the add remove section, new files are preceded by a '+', old ones
   are preceded by a '-' NOTE: that removed does not mean gone from the
   filesystem, just that it is no longer sgid or suid.
 - Most of it is pretty easy to understand. On the sections that show
   changes in the file's info (uid, gid, modes...) the format is
   old->new. So if the old owner was 'mail' and it is now 'root' then it
   shows it as mail->root.
 - The list of files in the checks is in the following format:

      /full/path              *user.group    MODE

   MODE is the 4 digit mode, as in 4755.

   In the changes section, if the line is preceded by an 'i' then that
   item has changed inodes since the last check (regardless of any
   s[ug]id change), if there is an 'm' then the SHA-256 checksum has
   changed.
 - If a user or group entry is preceded by a '*' then that is +s
   (ie. *root.wheel is suid, root.*wheel is sgid, *root.*wheel is +s)
 - On the forbidden directories, it enfore is enabled a 'r' will precede
   forbidden items that were succesfully -s'd, and a '!' will show that
   the was unsuccesfully -s'd (for what ever reason.

Installation
============

To install sXid simply (requires GNU make):

	make install

This will configure and compile the program then install it into
/usr/local/bin by default. You can use sample config file
examples/sxid.conf. By default sXid looks for config in /usr/local/etc.
Please edit the conf file, it is well commented and very basic so no worries.

Alternatively you can run ./configure manually with any options you may
wish (./configure --help for options) then run 'make install'

Afterwards place an entry into root's crontab. You can use the
line in examples/sxid.cron.

sXid is known to compile on these platforms:

* Solaris 2.7/2.6/2.5.1x)
* Linux GLIBC 2.0/2.1 and Libc5 on kernels 2.0.x to 2.2.x
* AIX 4.x (and possibly 3.x)
* HP/UX ([Peter Sulecki](mailto:psulecki@ios.krakow.pl))
* Tru64 UNIX ([Marc Baudoin](mailto:babafou@pasteur.fr))

It should compile on others as well.

Finally
=======

NOTE: If you were using any version prior to 3.2.4 you need to archive and
remove the old logs since they aren't compatible with this version and
will cause improbable errors in the output.

Development
===========

If you have patches, please fork sXid at GitHub
https://github.com/taem/sxid. Or simply send your patches by e-mail (see
below).

If you found a bug, please report it at https://github.com/taem/sxid/issues.

License
=======

sXid is distributed under the terms GNU General Public License Version 2
or greater. Please see COPYING for full GNU GPL text.

Homepage
========

Latest version can be found at http://linukz.org/sxid.shtml.

Author
======

Original author is [Ben Collins](mailto:bcollins@debian.org).
Later [Timur Birsh](mailto:taem@linukz.org) picked up maintenance.
