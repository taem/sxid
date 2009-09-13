/*
 * sxid - suid, sgid file and directory checking
 * logging.c - logging functions
 *
 * Copyright (C) 1999, 2000, 2002 Ben Collins <bcollins@debian.org>
 * Copyright (C) 2009 Timur Birsh <taem@linukz.org>
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>
#include <limits.h>

#include "sxid.h"

struct file_entry *global_log;
FILE *mail_file;

static
char *fill_space (int count)
{
    char *p;
    int i;

    /* For longer lines, we try to format with some sort of tabs */
    if (count == 0)
	count = 5;
    else if (count < 0) {
	count = 0 - count;
	for (i = 5 ; i < count ; i += 5)
	    ;
	count = i;
    }
    p = (char *) malloc (count + 1);
    if (p == NULL) {
	fprintf (stderr, "E: could not malloc() in fill_space()\n");
	exit (1);
    }
    memset (p, ' ', count);
    p[count] = '\0';
    return p;
}

static
char *format_ugid (uid_t uid, gid_t gid, mode_t mode)
{
    struct passwd *pw = getpwuid (uid);
    struct group *gr = getgrgid (gid);
    char *ret_val, *temp;

    ret_val = temp = (char *) malloc ((NAME_MAX * 2) + 2);
    if (ret_val == NULL) {
	fprintf (stderr, "E: could not malloc() in format_ugid()\n");
	exit (1);
    }
    if (mode & S_ISUID)
	temp++[0] = '*';
    if (pw == NULL)
	temp += sprintf (temp, "%d", (int) uid);
    else
	temp += sprintf (temp, "%s", pw->pw_name);

    temp++[0] = ':';

    if (mode & S_ISGID)
	temp++[0] = '*';
    if (gr == NULL)
	temp += sprintf (temp, "%d", (int) gid);
    else
	temp += sprintf (temp, "%s", gr->gr_name);

    return ret_val;
}

static
char *super_format_ugid (struct file_entry *p)
{
    struct passwd *pw;
    struct group *gr;
    char *ret_val, *temp;

    ret_val = temp = (char *) malloc ((NAME_MAX * 4) + 6);
    if (ret_val == NULL) {
	fprintf (stderr, "E: could not malloc() in super_format_ugid()\n");
	exit (1);
    }
    if (p->flags & FE_CHUSER) {
	pw = getpwuid (p->old_uid);
	if (pw == NULL)
	    temp += sprintf (temp, "%d->", (int) p->old_uid);
	else
	    temp += sprintf (temp, "%s->", pw->pw_name);
    }
    pw = getpwuid (p->uid);
    if (pw == NULL)
	temp += sprintf (temp, "%d", (int) p->uid);
    else
	temp += sprintf (temp, "%s", pw->pw_name);

    temp++[0] = ':';

    if (p->flags & FE_CHGRP) {
	gr = getgrgid (p->old_gid);
	if (gr == NULL)
	    temp += sprintf (temp, "%d->", (int) p->old_gid);
	else
	    temp += sprintf (temp, "%s->", gr->gr_name);
    }
    gr = getgrgid (p->gid);
    if (gr == NULL)
	temp += sprintf (temp, "%d", (int) p->gid);
    else
	temp += sprintf (temp, "%s", gr->gr_name);

    return ret_val;
}

static
void check_add_rem (FILE * mail_file)
{
    struct file_entry *p = global_log;
    char init;

    fprintf (mail_file, "Checking for any additions or removals:\n");

    while (p != NULL) {
	init = 0;
	if (p->flags & FE_NEW)
	    init = '+';
	else if (p->flags & FE_OLD)
	    init = '-';
	if (init) {
	    char *perms, *space1, *space2;

	    perms = format_ugid (p->uid, p->gid, p->mode);
	    space1 = fill_space (20 - strlen (perms));
	    space2 = fill_space (32 - strlen (p->path) - ((p->mode & S_IFDIR)
		    ? 1 : 0));

	    fprintf (mail_file, " %c %s%s%s%s%s%o\n", init, p->path,
		(p->mode & S_IFDIR) ? "/" : "", space2, perms, space1,
		(int) p->mode & 0x0FFF);

	    free (space1);
	    free (space2);
	    free (perms);
	    changed = 1;
	}
	p = p->next;
    }
    fprintf (mail_file, "\n\n");
    return;
}

static
void check_attr (FILE * mail_file)
{
    struct file_entry *p = global_log;
    char ino, md5;

    fprintf (mail_file, "Checking for changed attributes or sums/inodes:\n");

    while (p != NULL) {
	if (p->flags & FE_CHANGEDATTR) {
	    char old_pmode[7], *perms, *space1, *space2;

	    if (p->mode & S_IFREG)
		ino = p->flags & FE_CHSUM ? 'm' : ' ';
	    else
		ino = ' ';
	    md5 = p->flags & FE_CHINODE ? 'i' : ' ';

	    if (p->flags & FE_CHMODE)
		sprintf (old_pmode, "%o->", (int) p->old_mode & 0x0FFF);
	    else
		old_pmode[0] = '\0';

	    perms = super_format_ugid (p);

	    space1 = fill_space (20 - strlen (perms));
	    space2 = fill_space (32 - strlen (p->path) - ((p->mode & S_IFDIR)
		    ? 1 : 0));

	    fprintf (mail_file, "%c%c %s%s%s%s%s%s%o\n", ino, md5, p->path,
		(p->mode & S_IFDIR) ? "/" : "", space2, perms, space1,
		old_pmode, (int) p->mode & 0x0FFF);

	    free (space1);
	    free (space2);
	    free (perms);
	    changed = 1;
	}
	p = p->next;
    }
    fprintf (mail_file, "\n\n");
    return;
}

static
void check_u_g (FILE * mail_file)
{
    struct file_entry *p = global_log;

    fprintf (mail_file, "Checking for no user/group matches:\n");

    while (p != NULL) {
	struct passwd *pw = getpwuid (p->uid);
	struct group *gr = getgrgid (p->gid);

	if ((pw == NULL || gr == NULL) && !(p->flags & FE_OLD)) {
	    char *perms, *space1, *space2;

	    perms = format_ugid (p->uid, p->gid, p->mode);
	    space1 = fill_space (20 - strlen (perms));
	    space2 = fill_space (32 - strlen (p->path) - ((p->mode & S_IFDIR)
		    ? 1 : 0));

	    fprintf (mail_file, "   %s%s%s%s%s%o\n", p->path,
		(p->mode & S_IFDIR) ? "/" : "", space2, perms, space1,
		(int) p->mode & 0x0FFF);

	    free (space1);
	    free (space2);
	    free (perms);
	    notify_diff = 1;
	}
	p = p->next;
    }
    fprintf (mail_file, "\n\n");
    return;
}

static
void check_forbidden (FILE * mail_file)
{
    struct file_entry *p = global_log;
    char init;

    fprintf (mail_file, "Checking for forbidden s[ug]id items:\n");

    while (p != NULL) {
	if (check_paths (p->path, config_options.forbidden_paths)) {
	    char *perms, *space1, *space2;

	    init = ' ';
	    perms = format_ugid (p->uid, p->gid, p->mode);

	    if (config_options.flags & FLAG_EFOR) {
		p->mode &= ~(S_ISUID | S_ISGID);
		if (chmod (p->path, p->mode))
		    init = '!';
		else {
		    p->flags &= ~FE_NEW;
		    p->flags |= FE_OLD;
		    init = 'r';
		}
	    }
	    space1 = fill_space (20 - strlen (perms));
	    space2 = fill_space (32 - strlen (p->path) - ((p->mode & S_IFDIR)
		    ? 1 : 0));

	    fprintf (mail_file, " %c %s%s%s%s%s%o\n", init, p->path,
		(p->mode & S_IFDIR) ? "/" : "", space2, perms,
		space1, (int) p->mode & 0x0FFF);

	    free (space1);
	    free (space2);
	    free (perms);
	    notify_diff = 1;
	}
	p = p->next;
    }

    fprintf (mail_file, "\n\n");
    return;
}

static
void do_list_all (FILE * mail_file)
{
    struct file_entry *p = global_log;

    fprintf (mail_file, "Complete list of entries:\n");

    while (p != NULL) {
	if (!p->flags & FE_OLD) {
	    char *perms, *space1, *space2;

	    perms = format_ugid (p->uid, p->gid, p->mode);
	    space1 = fill_space (20 - strlen (perms));
	    space2 = fill_space (32 - strlen (p->path) - ((p->mode & S_IFDIR)
		    ? 1 : 0));

	    fprintf (mail_file, "   %s%s%s%s%s%o\n", p->path,
		(p->mode & S_IFDIR) ? "/" : "", space2, perms,
		space1, (int) p->mode & 0x0FFF);

	    free (space1);
	    free (space2);
	    free (perms);
	}
	p = p->next;
    }
    fprintf (mail_file, "\n\n");
    return;
}

/* Loads the entries from the last log file */
void init_file_entries (void)
{
    FILE *log;
    char buf[MAXBUF], *s, *tok, search_tok[2] = { 0x1e, '\0' };

    global_log = NULL;
    log = fopen (config_options.log_file, "r");
    if (log == NULL)
	return;

    while ((fgets (buf, sizeof (buf), log)) != NULL) {
	struct file_entry *p;

	buf[strlen (buf) - 1] = '\0';
	s = buf;

	tok = strtok (s, search_tok);

	if((config_options.flags & FLAG_SPOT) && 
		strncasecmp(config_options.search_paths,
		s, strlen(config_options.search_paths)))
	    continue;

	p = (struct file_entry *) malloc (sizeof (struct file_entry));

	if (p == NULL) {
	    fprintf (stderr, "E: could not malloc() in init_file_entries()\n");
	    fclose (log);
	    exit (1);
	}
	p->next = global_log;
	global_log = p;

	p->path = strdup (tok);

	p->uid = (uid_t) atoi (strtok (NULL, search_tok));
	p->gid = (gid_t) atoi (strtok (NULL, search_tok));
	p->mode = (mode_t) atoi (strtok (NULL, search_tok));
	p->inode = (ino_t) atoi (strtok (NULL, search_tok));
	p->old_uid = -1;
	p->old_gid = -1;
	p->old_mode = -1;

	tok = strtok (NULL, search_tok);
	if (tok != NULL)
	    p->md5 = strdup (tok);
	else
	    p->md5 = NULL;
	p->flags = 0;
	p->flags |= FE_OLD;
    }
    fclose (log);
    return;
}

/* Initializes comparison functions */
void compare_output (void)
{
    char *tmp_dir, *tmp_file;

    tmp_dir = getenv("TMPDIR");
    if (tmp_dir == NULL)
	tmp_dir = "/tmp";

    tmp_file = malloc(strlen(tmp_dir) + strlen("/sxid.XXXXXX") + 1);
    if (tmp_file == NULL) {
	fprintf (stderr, "E: could not malloc() in compare_output().\n");
	exit (1);
    }
    strcpy(tmp_file, tmp_dir);
    strcat(tmp_file, "/sxid.XXXXXX");

    mail_file = fdopen(mkstemp(tmp_file), "w+b");
    unlink(tmp_file);
    free(tmp_file);

    if (mail_file == NULL) {
	fprintf (stderr, "E: could not open temp file in compare_output().\n");
	exit (1);
    }
    check_add_rem (mail_file);
    check_attr (mail_file);
    check_u_g (mail_file);
    check_forbidden (mail_file);
    if (config_options.flags & FLAG_LALL)
	do_list_all (mail_file);

    return;
}

/* Lookup entry in global struct by path */
struct file_entry *get_fe_bypath (char *path)
{
    struct file_entry *p;

    p = global_log;

    while (p != NULL) {
	if (!strcmp (p->path, path))
	    return p;
	p = p->next;
    }
    p = (struct file_entry *) malloc (sizeof (struct file_entry));

    if (p == NULL) {
	fprintf (stderr, "E: could malloc() in get_fe_bypath()\n");
	exit (1);
    }
    p->next = global_log;
    p->path = NULL;
    global_log = p;

    return p;
}

int add_file_entry (char *path, struct stat dirent_stat, char *md5)
{
    struct file_entry *p;

    p = get_fe_bypath (path);

    if (p->path == NULL) {
	p->path = strdup (path);
	p->uid = dirent_stat.st_uid;
	p->gid = dirent_stat.st_gid;
	p->mode = dirent_stat.st_mode;
	p->inode = dirent_stat.st_ino;
	p->old_uid = -1;
	p->old_gid = -1;
	p->old_mode = -1;

	if (md5 != NULL)
	    p->md5 = strdup (md5);
	else
	    p->md5 = NULL;
	p->flags = 0;
	p->flags |= FE_NEW;
    } else {
	p->flags = 0;
	if (p->uid != dirent_stat.st_uid) {
	    p->old_uid = p->uid;
	    p->uid = dirent_stat.st_uid;
	    p->flags |= FE_CHUSER;
	}
	if (p->gid != dirent_stat.st_gid) {
	    p->old_gid = p->gid;
	    p->gid = dirent_stat.st_gid;
	    p->flags |= FE_CHGRP;
	}
	if (p->mode != dirent_stat.st_mode) {
	    p->old_mode = p->mode;
	    p->mode = dirent_stat.st_mode;
	    p->flags |= FE_CHMODE;
	}
	if (p->inode != dirent_stat.st_ino) {
	    p->inode = dirent_stat.st_ino;
	    p->flags |= FE_CHINODE;
	}
	if (md5 != NULL) {
	    if (do_check (path, p->md5)) {
		strcpy (p->md5, md5);
		p->flags |= FE_CHSUM;
	    }
	} else
	    p->md5 = NULL;
    }
    return 0;
}

void save_and_rotate (void)
{
    struct file_entry *p = global_log;
    FILE *new_log;
    int i;

    for (i = config_options.keep_logs - 1; i >= 0; i--) {
	char o[PATH_MAX], n[PATH_MAX];

	sprintf (o, "%s%c%d", config_options.log_file, i == 0 ?
	    '\0' : '.', i - 1);
	sprintf (n, "%s.%d", config_options.log_file, i);

	rename (o, n);
    }

    new_log = fopen (config_options.log_file, "w");
    if (new_log == NULL) {
	fprintf (stdout, "W: Could not open new log file %s, not saving information\n",
	    config_options.log_file);
	return;
    }
    while (p != NULL) {
	if (!(p->flags & FE_OLD))
	    fprintf (new_log, "%s%c%d%c%d%c%d%c%d%c%s\n", p->path, 0x1e,
		(int) p->uid, 0x1e, (int) p->gid, 0x1e, (int) p->mode,
		0x1e, (int) p->inode, 0x1e, p->md5 != NULL ? p->md5 : "");
	p = p->next;
    }

    fclose (new_log);
    return;
}
