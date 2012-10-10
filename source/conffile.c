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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> /* strerror() */
#include <errno.h>
#include <ctype.h>

#include "sxid.h"

struct config_opt config_options;
static char conf_default[] = CONF_FILE;
char *confname = conf_default;

static void get_value(char *value, char *key, int required)
{
	FILE *conffile;
	char buffer[MAXBUF];
	int line_no = 0;

	memset(buffer, 0, sizeof(buffer));

	if ((conffile = fopen(confname, "r")) == NULL) {
		fprintf(stderr, "E: error opening config file, %s.\n", confname);
		exit(EXIT_FAILURE);
	}

	while ((fgets(buffer, sizeof(buffer), conffile)) != NULL) {
		line_no++;
		if (buffer[0] == '#')
			continue;
		if (strstr(buffer, key) == (char *)NULL)
			continue;
		if (strstr(buffer, "\"") == (char *)NULL) {
			fprintf(stderr, "E: in \'%s\', line %d.\n", confname,
				line_no);
			fclose(conffile);
			exit(EXIT_FAILURE);
		}
		strncpy(buffer, strstr(buffer, "\"") + 1, MAXBUF);
		buffer[MAXBUF - 1] = '\0';

		int count = 0;
		while (count < sizeof(buffer)) {
			if ((isprint(buffer[count])) &&	buffer[count] != '"')
				value[count] = buffer[count];
			else {
				value[count] = '\0';
				break;
			}
			count++;
		}
		value[sizeof(buffer) - 1] = '\0';
		fclose(conffile);
		return;
	}
	fclose(conffile);

	if (required) {
		fprintf(stderr,
			"E: required option \'%s\' not found in \'%s\'.\n",
			key, confname);
		exit(EXIT_FAILURE);
	}

	return;
}

void init_conf(void)
{
	char value[MAXBUF];

	/* Set defaults */
	strcpy((char*) config_options.exclude_paths, "");
	strcpy((char*) config_options.email, "root");
	strcpy((char*) config_options.forbidden_paths, "");
	strcpy((char*) config_options.ignore_dirs, "");
	config_options.extra_list[0] = '\0';
	strcpy((char*) config_options.mail_prog, MAIL_PROG);
	config_options.keep_logs = 3;

	if (config_options.flags & FLAG_SPOT) {
		if (getcwd(config_options.search_paths, MAXBUF) == NULL) {
			fprintf(stderr, "E: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		nomail = 1;
	} else
		get_value(config_options.search_paths, "SEARCH", 1);

	get_value(config_options.email, "EMAIL", 0);
	get_value(config_options.exclude_paths, "EXCLUDE", 0);
	get_value(config_options.ignore_dirs, "IGNORE_DIRS", 0);
	get_value(config_options.log_file, "LOG_FILE", 1);
	get_value(config_options.mail_prog, "MAIL_PROG", 0);
	get_value(config_options.extra_list, "EXTRA_LIST", 0);
	get_value(config_options.forbidden_paths, "FORBIDDEN", 0);

	get_value(value, "KEEP_LOGS", 0);
	config_options.keep_logs = atoi(value);

	get_value(value, "ALWAYS_NOTIFY", 0);
	if (isyes(value))
		config_options.flags |= FLAG_ANOT;

	get_value(value, "ALWAYS_ROTATE", 0);
	if (isyes(value))
		config_options.flags |= FLAG_AROT;

	get_value(value, "ENFORCE", 0);
	if (isyes(value))
		config_options.flags |= FLAG_EFOR;

	get_value(value, "LISTALL", 0);
	if (isyes(value))
		config_options.flags |= FLAG_LALL;

	return;
}
