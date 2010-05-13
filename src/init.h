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

#ifndef INIT_H
#define INIT_H 1

#include <getopt.h>
#include "window.h"

#ifdef HAVE_LIBCONFIG
#include <libconfig.h>
#endif

#define CONFIG_FILE		"/.qworc"

#define MAX_CHAR_PER_REGION		 5

extern KeySym custom_charset[MAX_REGIONS - 1][MAX_CHAR_PER_REGION];

extern char *user_geometry;
extern char *config_geometry;
extern char *switch_geometry;

extern char *config_path;

int parse_command_line(int argc, char **argv);

#ifdef HAVE_LIBCONFIG
int read_config();
#else
static inline int read_config() {return 0;}
#endif

#endif /* INIT_H */
