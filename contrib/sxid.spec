Summary: suid, sgid file and directory checking
Name: sxid
Version: 4.0.5
Release: 1
Copyright: GPL
Group: System Environment/Base
#Source: http://www.phunnypharm.org/pub/sxid/sxid_4.0.4.tar.gz
Source: sxid_4.0.4.tar.gz
Packager: Ben Collins <bcollins@debian.org>
BuildRoot: /var/tmp/sxid-root

%description
This program runs as a cronjob. Basically it tracks any changes in
your s[ug]id files and folders. If there are any new ones, ones that
aren't set any more, or they have changed bits or other modes then it
reports the changes. You can also run this manually for spot checking.

It tracks s[ug]id files by md5 checksums. This helps detect if your files
have been tampered with, would not show under normal name and permissions
checking. Directories are tracked by inodes.
	
%changelog
* Mon Jun 14 1999 Alexandr D. Kanevskiy <kad@blackcatlinux.com>
- Initial build for BCL 6.0
* Mon Jun 14 1999 Ben Collins <bcollins@debian.org>
- Added spec file to main source (thanks Alex)
* Mon Jun 13 2000 Ben Collins <bcollins@debian.org>
- Updated spec

%prep
%setup 

%build
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"
 
%install
rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install

mkdir -p $RPM_BUILD_ROOT/etc/cron.daily
install debian/cron.daily $RPM_BUILD_ROOT/etc/cron.daily/sxid

%clean
rm -rf $RPM_BUILD_ROOT

%files
%attr(0644, root, root) %doc README docs/TODO
%attr(0755, root, root) /usr/bin/*
%attr(0755, root, root) /etc/cron.daily/sxid
%attr(0644, root, root) /usr/man/*
%attr(0644, root, root) %config /etc/sxid.conf
