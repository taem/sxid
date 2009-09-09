/*
 * sxid - suid, sgid file and directory checking
 * search.c - directory search functions, also functions to scan
 *    the file_entry struct for changes
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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>

#include "sxid.h"

/*
 * Tells the search function to check all files,
 * not just +s ones (used with EXTRA_LIST)
 */
static int search_all = 0;

static
int excluded (char *dir)
{
    char *tok, *e, *p;

    e = strdup (config_options.exclude_paths);

    tok = strtok_r (e, " ", &p);
    while (tok != NULL) {
	if (!strcmp (tok, dir)) {
	    free(e);
	    return 1;
	}
	tok = strtok_r (NULL, " ", &p);
    }

    free(e);
    return 0;
}

static __inline__ void check_entry (struct dirent *, char *);

void search_path (char *dir)
{
    DIR *dir_list;
    struct dirent *dir_entry;

    dir_list = opendir (dir);
    if (dir_list == NULL)
	return;

    for (dir_entry = readdir (dir_list); dir_entry != NULL; dir_entry = readdir (dir_list))
	check_entry(dir_entry, dir);

    closedir (dir_list);
    return;
}

static __inline__
void check_entry (struct dirent *dir_entry, char *dir)
{
    char buf[PATH_MAX];
    struct stat dirent_stat;

    if (dir_entry == NULL || (strcmp (dir_entry->d_name, "..") &&
	    strcmp (dir_entry->d_name, "."))) {
	if (dir_entry != NULL)
	    snprintf (buf, sizeof(buf), "%s%s%s", dir, strlen (dir) == 1 ? "" : "/",
		dir_entry->d_name);
	else
	    snprintf (buf, sizeof(buf), "%s", dir);

	lstat (buf, &dirent_stat);

	if (dirent_stat.st_mode & S_IFDIR) {
	    if ((dirent_stat.st_mode & (S_ISUID | S_ISGID)) &&
		    !check_paths(buf, config_options.ignore_dirs))
		add_file_entry (buf, dirent_stat, NULL);
	    if (!excluded (buf))
		search_path (buf);
	} else {
	    if (dirent_stat.st_mode & S_IFREG &&
		(dirent_stat.st_mode & (S_ISUID | S_ISGID) || search_all)) {
		FILE *fp = NULL;
		unsigned char digest[16 + 1];
		int i;
		char digest_hex[(16 * 2) + 1], *p;

		fp = fopen (buf, "r");
		if (fp == NULL) {
		    fprintf (stderr, "W: opening %s for md5 read\n", buf);
		    return;
		}
		if (mdfile (fp, digest)) {
		    fprintf (stderr, "W: reading %s for md5\n", buf);
		    return;
		}
		fclose (fp);

		for (i = 0, p = digest_hex; i < 16; ++i)
		    p += sprintf (p, "%02x", digest[i]);

		add_file_entry (buf, dirent_stat, digest_hex);
	    }
	}
    }
    return;
}

void start_search (void)
{
    char *tok, *s;

    /* First the selected paths */
    s = strdup (config_options.search_paths);
    for (tok = strtok (s, " "); tok != NULL ; tok = strtok (NULL, " "))
	if (!excluded (tok))
	    search_path(tok);
    free (s);

    /* Now the forbidden paths get explicit recursion */
    s = strdup (config_options.forbidden_paths);
    for (tok = strtok (s, " "); tok != NULL ; tok = strtok (NULL, " "))
	if (!check_paths (tok, config_options.search_paths))
	    search_path(tok);
    free (s);

    /* Finally we check each line in extra_list */
    if (config_options.extra_list[0] != '\0') {
	FILE *list;
	char b[MAXBUF];

	bzero (b, sizeof (b));

	if ((list = fopen (config_options.extra_list, "r")) == NULL) {
	    fprintf (stderr, "E: error opening config file, %s.\n",
		config_options.extra_list);
	    exit(1);
	} else {
	    struct stat dirent_stat;
	    search_all = 1;
	    while ((fgets (b, sizeof (b), list)) != NULL) {
		if (b[0] == '/') { /* make sure it's a full path */
		    if (b[strlen(b)-1] == '\n')
			b[strlen(b)-1] = '\0'; /* get rid of \n */
		    stat (b, &dirent_stat);
		    if (dirent_stat.st_mode & S_IFREG)
			check_entry(NULL, b);
		    else if (dirent_stat.st_mode & S_IFDIR)
			search_path(b);
		}
	    }
	    search_all = 0;
	}
    }
    return;
}

int check_paths (char *dir, char *paths)
{
    char *tok, *e, *p;
    int matched = 0;

    e = strdup (paths);

    tok = strtok_r (e, " ", &p);
    while (tok != NULL && !matched) {
	if (!strncmp (tok, dir, strlen (tok) - 1))
	    matched = 1;
	tok = strtok_r (NULL, " ", &p);
    }

    free(e);
    return matched;
}

#ifndef HAVE_STRTOK_R
/* Borrowed from string2.h in glibc 2.1.1, Copyrights apply */
char *strtok_r (char *s, char sep, char **nextp)
{
    char *result;
    if (s == NULL)
	s = *nextp;
    while (*s == sep)
	++s;
    if (*s == '\0')
	result = NULL;
    else {
	result = s;
        while (*s != '\0' && *s != sep)
	    ++s;
        if (*s == '\0')
	    *nextp = s;
        else {
	    *s = '\0';
	    *nextp = s + 1;
	}
    }
    return result;
}
#endif
