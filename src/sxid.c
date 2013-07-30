/*
 * sxid - suid, sgid file and directory checking
 * sxid.c - main program routines
 *
 * Copyright © 1999, 2000, 2002 Ben Collins <bcollins@debian.org>
 * Copyright © 2009, 2013 Timur Birsh <taem@linukz.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* access() */
#include <string.h> /* strerror() */
#include <errno.h>
#include <time.h>
#include <getopt.h>

#include "sxid.h"

struct option cmd_options[] =
{
	{ "nomail",    no_argument,	     0, 'n' },
	{ "help",      no_argument,	     0, 'h' },
	{ "version",   no_argument,	     0, 'V' },
	{ "listall",   no_argument,	     0, 'l' },
	{ "spotcheck", no_argument,	     0, 'k' },
	{ "config",    required_argument,    0, 'c' },
	{ NULL,	       0,		     0, 0   }
};

char sxid_version[] = VERSION;

int changed = 0;
int notify_diff = 0;
int nomail = 0;

static
void sxid_usage()
{
	fprintf(stdout, "Usage: sxid [options]\n");
	fprintf(stdout, "-h, --help              This help output\n");
	fprintf(stdout, "-V, --version           Print version and exit\n");
	fprintf(stdout, "-k, --spotcheck         Run this using spotcheck rules "
		"(see man page)\n");
	fprintf(stdout, "-n, --nomail            Send output to stdout instead "
		"of sending email\n");
	fprintf(stdout, "-l, --listall           List all found s[ug]id files "
		"regardless of changes\n");
	fprintf(stdout, "-c, --config <file>     Specifies alternative config "
		"file\n");
}

static
void print_version()
{
	fprintf(stdout, "sXid %s\n", sxid_version);
	fprintf(stdout, "Copyright (C) 1999, 2000, 2002 Ben Collins\n");
	fprintf(stdout, "Copyright (C) 2009, 2013 Timur Birsh\n");
	fprintf(stdout, "This is free software; see the source for copying "
		"conditions.  There is NO\n"
		"warranty; not even for MERCHANTABILITY or FITNESS FOR A "
		"PARTICULAR PURPOSE.\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	FILE *conffile;
	int opt;
	time_t time_mark;

	umask(077);
	config_options.flags = 0x00;

	while ((opt = getopt_long_only(argc, argv, "nhVlkc:", cmd_options,
						NULL)) != EOF) {
		switch (opt) {
		case 'c':
			confname = optarg;
			break;
		case 'n':
			nomail = 1;
			break;
		case 'l':
			config_options.flags |= FLAG_LALL;
			break;
		case 'k':
			config_options.flags |= FLAG_SPOT;
			break;
		case 'V':
			print_version();
			break;
		case 'h':
			sxid_usage();
			exit(EXIT_SUCCESS);
		default:
			sxid_usage();
			exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (access(confname, R_OK) != 0) {
		fprintf(stderr, "E: %s: %s\n", strerror(errno), confname);
		exit(EXIT_FAILURE);
	}

	/* Ok, let's time stamp */
	time_mark = time(NULL);
	if (!strftime(current_time, sizeof(current_time), "%c",
		      localtime(&time_mark)))
		current_time[sizeof(current_time) - 1] = '\0';

	init_conf();
	init_file_entries();
	start_search();
	compare_output();

	if ((changed || (config_options.flags & FLAG_AROT)) &&
	    !(config_options.flags & FLAG_SPOT))
		save_and_rotate();

	if (changed || notify_diff || nomail ||
	    (config_options.flags & (FLAG_ANOT | FLAG_LALL)))
		notify_email();

	fclose(mail_file);
	return 0;
}
