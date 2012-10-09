/*
 * sxid - suid, sgid file and directory checking
 * sxid.h - defines and declerations for the main program
 *
 * Copyright (C) 1999, 2000, 2002 Ben Collins <bcollins@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with sxid; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef SXID_H
#define SXID_H 1

#include "config.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

/* conffile.c */
void init_conf(void);
#define isyes(x)        (x[0] == 'y' || x[0] == 'Y' || x[0] == '1')

/* notify.c */
void notify_email(void);

/* search.c */
int check_paths(char *, char *);
void start_search(void);

/* md5sum.c */
int mdfile(FILE * fp, unsigned char *digest);
int do_check(char *path, char *md5sum);

/* logging.c */
void init_file_entries(void);
void compare_output(void);
void save_and_rotate(void);
int add_file_entry(char *, struct stat, char *);

#ifndef HAVE_STRTOK_R
extern char *strtok_r(char *, char, char **);
#endif

#ifndef MAXBUF
#define MAXBUF 2048
#endif

/* Configuration info */
struct config_opt {
	char search_paths[MAXBUF + 1];
	char exclude_paths[MAXBUF + 1];
	char ignore_dirs[MAXBUF + 1];
	char email[MAXBUF + 1];
	char log_file[MAXBUF + 1];
	char forbidden_paths[MAXBUF + 1];
	char mail_prog[MAXBUF + 1];
	char extra_list[MAXBUF + 1];
	int keep_logs;
	unsigned int flags;
};

/* Flags for config file */
#define FLAG_ANOT       0x0001
#define FLAG_AROT       0x0002
#define FLAG_EFOR       0x0004
#define FLAG_LALL       0x0010
#define FLAG_SPOT       0x0020

/* File entry information */
struct file_entry {
	struct file_entry *next;
	char *path;
	uid_t uid;
	gid_t gid;
	uid_t old_uid;
	gid_t old_gid;
	mode_t mode;
	mode_t old_mode;
	char *md5;
	ino_t inode;
	u_int flags;
};

/* Flags for file entries */
#define FE_OLD          0x0001  /* Old entry (default, get's unset if current) */
#define FE_NEW          0x0002  /* New entry */
#define FE_CHMODE       0x0004  /* Changed mode (r,w or x) */
#define FE_CHSUM        0x0010  /* Changed md5sum (files only) */
#define FE_CHINODE      0x0020  /* Changed inode */
#define FE_CHUSER       0x0040  /* User id changed */
#define FE_CHGRP        0x0100  /* Group id changed */
#define FE_CHANGEDATTR  0x0174  /* Mask for anything changed */

extern char *confname;
extern struct config_opt config_options;
extern int changed;
extern int notify_diff;
extern int nomail;
extern char current_time[MAXBUF];
extern struct file_entry *global_log;
extern FILE *mail_file;
extern char sxid_version[];

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#endif /* SXID_H */
