/*
 * sxid - suid, sgid file and directory checking
 * conffile.c - configuration file functions
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include "sxid.h"

struct config_opt config_options;
static char conf_default[] = CONF_FILE;
char *confname = conf_default;

static
void get_value (char *key, char *value, int required)
{
    FILE *conffile;
    char buffer[MAXBUF];
    int count = 0;
    int line_no = 0;

    bzero (buffer, sizeof (buffer));

    if ((conffile = fopen (confname, "r")) == NULL) {
	fprintf (stderr, "E: error opening config file, %s.\n", confname);
	exit (1);
    } else {
	while ((fgets (buffer, sizeof (buffer), conffile)) != NULL) {
	    if (buffer[0] != '#') {
		if (strstr (buffer, key) != (char) NULL) {
		    if (strstr (buffer, "\"") == (char) NULL) {
			fprintf (stderr, "E: in \'%s\', line %d.\n", confname, line_no);
			fclose (conffile);
			exit (1);
		    }
		    strncpy (buffer, strstr (buffer, "\"") + 1, MAXBUF);
		    buffer[MAXBUF - 1] = '\0';

		    count = 0;
		    while (count < sizeof (buffer)) {
			if ((isprint (buffer[count])) && buffer[count] != '"')
			    value[count] = buffer[count];
			else {
			    value[count] = '\0';
			    break;
			}
			count++;
		    }
		    value[sizeof (buffer) - 1] = '\0';
		    fclose (conffile);
		    return;
		}
	    }
	    line_no++;
	}
	fclose (conffile);
	if (required) {
	    fprintf (stderr, "E: required option \'%s\' not found in \'%s\'.\n",
		key, confname);
	    exit (1);
	}
	return;
    }
}

void init_conf (void)
{
    char temp[MAXBUF];

    /* Set defaults */
    strcpy ((char *) config_options.exclude_paths, "");
    strcpy ((char *) config_options.email, "root");
    strcpy ((char *) config_options.forbidden_paths, "");
    strcpy ((char *) config_options.ignore_dirs, "");
    config_options.extra_list[0] = '\0';
    strcpy ((char *) config_options.mail_prog, MAIL_PROG);
    config_options.keep_logs = 3;

    if (config_options.flags & FLAG_SPOT) {
	if(getcwd(config_options.search_paths, MAXBUF) == NULL) {
	    fprintf(stderr, "E: current working directory path is too long\n");
	    exit (1);
	}
	nomail = 1;
    } else
	get_value ("SEARCH", config_options.search_paths, 1);

    get_value ("EMAIL", config_options.email, 0);
    get_value ("EXCLUDE", config_options.exclude_paths, 0);
    get_value ("IGNORE_DIRS", config_options.ignore_dirs, 0);
    get_value ("LOG_FILE", config_options.log_file, 1);
    get_value ("MAIL_PROG", config_options.mail_prog, 0);
    get_value ("EXTRA_LIST", config_options.extra_list, 0);
    get_value ("FORBIDDEN", config_options.forbidden_paths, 0);

    get_value ("KEEP_LOGS", temp, 0);
    config_options.keep_logs = atoi (temp);

    get_value ("ALWAYS_NOTIFY", temp, 0);
    if (isyes (temp))
	config_options.flags |= FLAG_ANOT;

    get_value ("ALWAYS_ROTATE", temp, 0);
    if (isyes (temp))
	config_options.flags |= FLAG_AROT;

    get_value ("ENFORCE", temp, 0);
    if (isyes (temp))
	config_options.flags |= FLAG_EFOR;

    get_value ("LISTALL", temp, 0);
    if (isyes (temp))
	config_options.flags |= FLAG_LALL;

    return;
}
