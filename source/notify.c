/*
 * sxid - suid, sgid file and directory checking
 * notify.c - notification functions
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <unistd.h>
#include <limits.h>

#include "sxid.h"

char current_time[MAXBUF];

static
void mail_header (FILE * output)
{
    char hostname_val[MAXBUF];

    fprintf (output, "sXid Vers  : %s\n", sxid_version);
    fprintf (output, "Check run  : %s\n", current_time);
    fprintf (output, "This host  : %s\n", gethostname (hostname_val,
	    sizeof (hostname_val)) ? "localhost" : hostname_val);
    if (config_options.flags & FLAG_SPOT)
	fprintf (output, "Spotcheck  : %s\n", config_options.search_paths);
    else
	fprintf (output, "Searching  : %s\n", config_options.search_paths);
    fprintf (output, "Excluding  : %s\n", config_options.exclude_paths);
    fprintf (output, "Ignore Dirs: %s\n", config_options.ignore_dirs);
    if (config_options.extra_list[0] != '\0') {
	fprintf (output, "Extra List : %s\n", config_options.extra_list);
    }
    fprintf (output, "Forbidden  : %s\n", config_options.forbidden_paths);
    if (config_options.flags & FLAG_EFOR)
	fprintf (output, "  (enforcing removal of s[ug]id bits in forbidden paths)\n");

    fprintf (output, "\n\n");

    return;
}

void notify_email (void)
{
    FILE *mail_pipe;
    char buffer[PATH_MAX + 64];

    rewind (mail_file);

#ifdef CL_SUBJ
    snprintf (buffer, sizeof(buffer), "%s -s \"%s\" \"%s\"",
	config_options.mail_prog, changed ?
	"List of changed s[ug]id files and folders" :
	notify_diff ? "No changes found (persistent problems)" :
	"No changes found (notice only)", config_options.email);
#else
    snprintf (buffer, sizeof(buffer), "%s \"%s\"",
	config_options.mail_prog, config_options.email);
#endif

    if (!nomail)
	mail_pipe = popen (buffer, "w");
    else
	mail_pipe = stdout;

    if (mail_pipe == NULL) {
	fprintf (stderr, "E: could not open output to %s\n",
	    nomail ? "stdout" : config_options.mail_prog);
	if (!nomail)
	    fclose (mail_file);
	exit (1);
    }
#ifndef CL_SUBJ
    if (!nomail)
	fprintf (mail_pipe, "Subject: %s\n\n", changed ?
	    "List of changed s[ug]id files and folders" :
	    notify_diff ? "No changes found (persistent problems)" :
	    "No changes found (notice only)");
#endif

    mail_header (mail_pipe);

    if (nomail && !changed)
	fprintf (mail_pipe, "%s\n\n", notify_diff ?
	    "No changes found (persistent problems)" : "No changes found");

    if (changed || notify_diff || (config_options.flags & FLAG_LALL))
	while ((fgets (buffer, sizeof (buffer), mail_file)) != NULL)
	    fputs (buffer, mail_pipe);

    if (!nomail)
	pclose (mail_pipe);

    return;
}
