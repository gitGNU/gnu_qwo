/*
 *  Copyright (C) 2008, 2009, 2010 Charles Clement <caratorn _at_ gmail.com>
 *
 *  This file is part of qwo.
 *
 *  qwo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301  USA
 *
 */

#include "init.h"

KeySym custom_charset[MAX_REGIONS - 1][MAX_CHAR_PER_REGION];

char *config_path = NULL;

/**
 * parse_command_line - parse command line arguments
 *
 * return code
 * 0 if ok,
 * 1 to show version
 * 2 to show help
 */

int parse_command_line(int argc, char **argv)
{
	int options;
	int option_index = 0;
	int return_code = 0;
	static struct option long_options[] = {
		{"help",	no_argument      , 0, 'h'},
		{"version",	no_argument      , 0, 'v'},
		{"foreground",	required_argument, 0, 'f'},
		{"background",	required_argument, 0, 'b'},
		{"delimiter-color", required_argument, 0, 'l'},
		{"config",	required_argument, 0, 'c'},
		{"geometry",	required_argument, 0, 'g'},
		{0, 0, 0, 0}
	};

	while ((options = getopt_long(argc, argv, "c:g:f:b:d:hv", long_options,
					&option_index)) != -1)
	{
		switch(options){
			case 'c':
				config_path = optarg;
				break;
			case 'g':
				strncpy(geometry_config, optarg, MAX_GEOMETRY_STRING);
				break;
			case 'f':
				color_scheme[FG_COLOR] = convert_color(optarg);
				defined_colors |= (1 << FG_COLOR);
				break;
			case 'b':
				color_scheme[BG_COLOR] = convert_color(optarg);
				defined_colors |= (1 << BG_COLOR);
				break;
			case 'd':
				color_scheme[GRID_COLOR] = convert_color(optarg);
				defined_colors |= (1 << GRID_COLOR);
				break;
			case 'v':
				return_code = 1;
			default:
				return_code = 2;
		}
	}

	return return_code;
}

#ifdef HAVE_LIBCONFIG
int read_config()
{
	int j, i = 0;
	config_t configuration;
	FILE * file;
	const char *keysym_name, *string = NULL;
	const char *fg_color = NULL;
	const char *bg_color = NULL;
	const char *delimiter_color = NULL;
	KeySym key;
	const config_setting_t *keymap;
	const config_setting_t *line;
	char default_config_path[MAX_CONFIG_PATH];

	if (config_path == NULL) {
		char *home_dir = getenv("HOME");
		strncpy(default_config_path, home_dir, MAX_CONFIG_PATH);
		strncat(default_config_path + strlen(home_dir), CONFIG_FILE,
				MAX_CONFIG_PATH - strlen(home_dir));
		config_path = default_config_path;
	}

	if ((file = fopen(config_path, "r")) == NULL) {
		fprintf(stderr, "Can't open configuration file %s\n", config_path);
		return 0;
	}
	config_init(&configuration);
	if (config_read(&configuration, file) == CONFIG_FALSE) {
		fprintf(stderr, "File %s, Line %i : %s\n", config_path,
				config_error_line(&configuration),
				config_error_text(&configuration));
		exit(3);
	}
	fclose(file);
	keymap = config_lookup(&configuration, "charset");

	if (keymap) {
		for (i = 0 ; i < config_setting_length(keymap) ; i++) {
			line = config_setting_get_elem(keymap, i);
			for (j = 0 ; j < config_setting_length(line); j++) {
				keysym_name = config_setting_get_string_elem(line, j);
				if ((key = XStringToKeysym(keysym_name)) == NoSymbol) {
					fprintf(stderr, "KeySym not found : %s\n",
							config_setting_get_string_elem(line, j));
					exit(3);
				}
				custom_charset[i][j] = key & 0xffffff;
			}
			for (; j < MAX_CHAR_PER_REGION; j++) {
				custom_charset[i][j] = '\0';
			}
		}
		for (; i < MAX_REGIONS - 1 ; i++) {
			for ( j = 0 ; j < MAX_CHAR_PER_REGION; j++) {
				custom_charset[i][j] = '\0';
			}
		}
	}

#if LIBCONFIG_LOOKUP_RETURN_CODE
	config_lookup_string(&configuration, "geometry", &string);
	config_lookup_string(&configuration, "foreground", &fg_color);
	config_lookup_string(&configuration, "background", &bg_color);
	config_lookup_string(&configuration, "delimiter-color", &delimiter_color);
#else
	string = config_lookup_string(&configuration, "geometry");
	fg_color = config_lookup_string(&configuration, "foreground");
	bg_color = config_lookup_string(&configuration, "background");
	delimiter_color = config_lookup_string(&configuration, "delimiter-color");
#endif

	if ((string) && (strlen(geometry_config) == 0)){
		strncpy(geometry_config, string, MAX_GEOMETRY_STRING);
	}

	if (fg_color && (!(defined_colors & (1 << FG_COLOR)))) {
		color_scheme[FG_COLOR] = convert_color(fg_color);
		defined_colors |= (1 << FG_COLOR);
	}

	if (bg_color && (!(defined_colors & (1 << BG_COLOR)))) {
		color_scheme[BG_COLOR] = convert_color(bg_color);
		defined_colors |= (1 << BG_COLOR);
	}

	if (delimiter_color && (!(defined_colors & (1 << GRID_COLOR)))) {
		color_scheme[GRID_COLOR] = convert_color(delimiter_color);
		defined_colors |= (1 << GRID_COLOR);
	}

	config_destroy(&configuration);
	return 1;
}
#endif /* HAVE_LIBCONFIG */


