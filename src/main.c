/*
 *  Copyright (C) 2008, 2009 Charles Clement <caratorn _at_ gmail.com>
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

#include <window.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/keysym.h>
#include <locale.h>

#ifdef HAVE_LIBCONFIG
#include <libconfig.h>
#endif

#define CONFIG_FILE		"/.qworc"

#define MAX_CHAR_PER_REGION		 5

#define MAX_GESTURES_BUFFER      6

#define LONG_EXPOSURE_DELAY		2000L

static const char *copyright_notice = "\
Copyright (C) 2008-2009 Charles Clement\n\
qwo comes with ABSOLUTELY NO WARRANTY. This is free software,\n\
and you are welcome to redistribute it under certain conditions.\n";

char charset[][MAX_REGIONS] = {
		"a,z?""?>>c",
		"deb?????",
		"gfihj???",
		"??l>k???",
		"??mnopq?",
		"????tsr?",
		"????.vuw",
		"y?????x<"
};

KeySym char_free[MAX_REGIONS][MAX_REGIONS] = {
	{XK_KP_5, 0xffff, XK_Up, 0xffff, XK_Right, XK_backslash, XK_KP_0, XK_slash, XK_Left},
	{XK_grave, XK_KP_1, XK_Tab, XK_equal, 0xffff, XK_braceleft, XK_bracketleft, XK_parenleft, XK_bar},
	{XK_Down, XK_asciicircum, XK_KP_2, XK_minus, XK_greater, 0xffff, 0xffff, 0xffff, XK_less},
	{XK_apostrophe, XK_percent, XK_dollar, XK_KP_3, XK_Insert, XK_parenright, XK_bracketright, XK_braceright, 0xffff},
	{0xffff, 0xffff, 0xffff, XK_exclam, XK_KP_6, XK_Return, XK_Alt_L, 0xffff, XK_Home},
	{0xffff, 0xffff, 0xffff, XK_colon, XK_semicolon, XK_KP_9, XK_asterisk, XK_ampersand, 0xffff},
	{0xffff, 0xffff, 0xffff, 0xffff, 0xffff, XK_underscore, XK_KP_8, XK_at, 0xffff},
	{0xffff, XK_numbersign, 0xffff, 0xffff, 0xffff, XK_plus, XK_asciitilde, XK_KP_7, XK_quotedbl},
	{0xffff, XK_Escape, 0xffff, 0xffff, XK_End, 0xffff, XK_Control_L, XK_Select, XK_KP_4}
};

static KeyCode Shift_code, Control_code, Alt_code;

KeySym custom_charset[MAX_REGIONS - 1][MAX_CHAR_PER_REGION];

typedef enum {
	KeyboardNone = 0,
	KeyboardShow,
	KeyboardHide,
	KeyboardToggle
} KeyboardOperation;

void usage(){
	fprintf(stdout,
		"Usage: qwo [options]\n\n"
		"Options:\n"
		"  -g, --geometry [size][{+-}<xoffset>[{+-}<yoffset>]]\n"
		"			specify window size and/or position\n"
		"  -c, --config <file>	use configuration file <file> instead of ~/.qworc\n"
		"  -h, --help      	Print this help\n"
		"  -v, --version   	Print version information\n"
		);
}

void print_version(){
	fprintf(stdout, PACKAGE " " VERSION " ");
	fprintf(stdout, copyright_notice);
}

#ifdef HAVE_LIBCONFIG
int read_config(char *config_path, char **geometry)
{
	int j, i = 0;
	config_t configuration;
	FILE * file;
	const char *keysym_name, *string;
	KeySym key;
	const config_setting_t *keymap;
	const config_setting_t *line;

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

	string = config_lookup_string(&configuration, "geometry");

	if (string) {
		*geometry = (char *) malloc(sizeof(char) * strlen(string));
		strcpy(*geometry, string);
	}

	config_destroy(&configuration);
	return 1;
}
#endif

int init_keycodes(Display *dpy){
	Shift_code = XKeysymToKeycode(dpy, XK_Shift_L);
	Control_code = XKeysymToKeycode(dpy, XK_Control_L);
	Alt_code = XKeysymToKeycode(dpy, XK_Alt_L);

	return 0;
}

int main(int argc, char **argv)
{
	Display *dpy;
	char *display_name;
	Window toplevel;

	int event_base, error_base;
	int shape_ext_major, shape_ext_minor;

	char *config_path = NULL;

	char *user_geometry = NULL;
	char *config_geometry = NULL;
	char *switch_geometry = NULL;

	int loaded_config = 0;
	int run = 1;
	int visible = 0;
	int buffer[MAX_GESTURES_BUFFER];
	int buffer_count = 0;
	int shift_modifier = 0;
	int ctrl_modifier = 0;
	int alt_key = 0;
	int help_screen = 0;
	int sent = 0;
	Time last_cross_timestamp = 0L;
	Time last_pressed = 0L;
	int options;
	int option_index = 0;
	static struct option long_options[] = {
		{"help",     no_argument      , 0, 'h'},
		{"version",  no_argument      , 0, 'v'},
		{"config",   required_argument, 0, 'c'},
		{"geometry", required_argument, 0, 'g'},
		{0, 0, 0, 0}
	};


	while ((options = getopt_long(argc, argv, "c:g:hv", long_options,
					&option_index)) != -1)
	{
		switch(options){
			case 'c':
				config_path = optarg;
				break;
			case 'g':
				switch_geometry = optarg;
				break;
			case 'v':
				print_version();
				exit(0);
			default:
				usage();
				exit(0);
		}
	}

	display_name = XDisplayName(NULL);
	dpy = XOpenDisplay(display_name);

	if (!setlocale(LC_CTYPE, ""))
	{
		fprintf(stderr, "Locale not specified. Check LANG, LC_CTYPE, LC_ALL. ");
		return 1;
	}

	if (dpy == NULL){
		fprintf(stderr, "%s : Can't open display %s\n", argv[0],
				display_name);
		exit(1);
	}

	if (XShapeQueryExtension(dpy, &event_base, &error_base) != True){
		fprintf(stderr, "%s : X11 Shape extension not supported on "
				"%s\n", argv[0], display_name);
		exit(2);
	}

#ifdef HAVE_LIBCONFIG
	if (config_path) {
		loaded_config = read_config(config_path, &config_geometry);
	} else {
		char config_path[MAX_CONFIG_PATH];
		char *home_dir = getenv("HOME");
		strncpy(config_path, home_dir, MAX_CONFIG_PATH);
		strncat(config_path + strlen(home_dir), CONFIG_FILE,
				MAX_CONFIG_PATH - strlen(home_dir));
		loaded_config = read_config(config_path, &config_geometry);
	}
#endif

	XShapeQueryVersion(dpy, &shape_ext_major, &shape_ext_minor);

	toplevel = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, DEFAULT_WIDTH,
			DEFAULT_HEIGHT, 0, CopyFromParent, CopyFromParent, CopyFromParent,
			0, NULL);

	if (switch_geometry){
		user_geometry = switch_geometry;
	} else {
		user_geometry = config_geometry;
	}

	if (init_window(dpy, toplevel, user_geometry)){
		fprintf(stderr, "Can't initialise window\n");
		exit(3);
	}

	if (config_geometry) {
		free(config_geometry);
	}

	init_keycodes(dpy);

	update_display(dpy, toplevel, shift_modifier, help_screen);

	while(run) {
		XEvent e;
		char *region_name;
		int region;
		int state_mod = 0;
		KeyCode code;
		KeySym ksym;

		XNextEvent(dpy, &e);
		switch (e.type) {
			case ButtonPress:
				XFetchName(dpy, e.xbutton.window, &region_name);
				region = region_name[0] - 48;
				XFree(region_name);
				XUngrabPointer(dpy, CurrentTime);
				buffer[0] = region;
				last_pressed = e.xbutton.time;
				sent = 0;
				break;
			case ButtonRelease:
				XFetchName(dpy, e.xbutton.window, &region_name);
				region = region_name[0] - 48;
				XFree(region_name);
				if (buffer[0] == 0 && !region && sent)
					break;
				ksym = char_free[buffer[0]][region];
				if (ksym == XK_Alt_L) {
					alt_key = 1;
				} else if (ksym == XK_Control_L) {
					ctrl_modifier = 1;
				} else if (ksym == XK_Insert) {
					code = XKeysymToKeycode(dpy, ksym);
					XTestFakeKeyEvent(dpy, Shift_code, True, 0);
					XTestFakeKeyEvent(dpy, code, True, 0);
					XTestFakeKeyEvent(dpy, code, False, 0);
					XTestFakeKeyEvent(dpy, Shift_code, False, 0);
				} else if (ksym == XK_Select) {
					help_screen = !help_screen;
					update_display(dpy, toplevel, shift_modifier, help_screen);
				} else {
					code = XKeysymToKeycode(dpy, ksym);

					for (state_mod = 0; state_mod < 4; state_mod++) {
						if (XKeycodeToKeysym(dpy, code, state_mod) == ksym ) {
							break;
						}
					}

					if (state_mod)
						XTestFakeKeyEvent(dpy, Shift_code, True, 0);
					XTestFakeKeyEvent(dpy, code, True, 0);
					XTestFakeKeyEvent(dpy, code, False, 0);
					if (state_mod)
						XTestFakeKeyEvent(dpy, Shift_code, False, 0);
				}
				buffer_count = 0;
				break;
			case EnterNotify:
				XFetchName(dpy, e.xcrossing.window, &region_name);
				region = region_name[0] - 48;
				XFree(region_name);
				KeySym index;

				char c = '\0';

				if ((region == 0) && buffer_count > 1 && buffer[0] == 0){

					if ((buffer[1] == buffer[buffer_count - 1]) && loaded_config && (buffer_count > 2)) {
						buffer_count = (buffer_count - 1) >> 1;

						if (DIRECTION(buffer[1], buffer[2]))
							index = custom_charset[buffer[1] - 1][(buffer_count << 1) - 1];
						else
							index = custom_charset[buffer[1] - 1][(buffer_count << 1) - 2];

						if (shift_modifier) {
							KeySym lower, upper;
							XConvertCase(index, &lower, &upper);
							code = XKeysymToKeycode(dpy, upper);
						} else {
							code = XKeysymToKeycode(dpy, index);
						}
					} else {
						c = charset[buffer[1] - 1][buffer[buffer_count - 1] - 1];
							// X11 KeySym maps ASCII table
						code = XKeysymToKeycode(dpy, c);
						index = (KeySym) c;
					}
					for (state_mod = 0; state_mod < 4; state_mod++) {
						if (XKeycodeToKeysym(dpy, code, state_mod) == index) {
							break;
						}
					}

					if (c == '<') {
						if (e.xcrossing.time - last_cross_timestamp > LONG_EXPOSURE_DELAY) {
								// Erase all buffer
						} else {
							if (buffer_count == 2) {
							code = XKeysymToKeycode(dpy, XK_BackSpace);
							}
						}
						XTestFakeKeyEvent(dpy, code, True, 0);
						XTestFakeKeyEvent(dpy, code, False, 0);
					} else if (c == '>') {
						if (buffer_count == 2) {
							if (e.xcrossing.time - last_cross_timestamp > LONG_EXPOSURE_DELAY) {
								code = XKeysymToKeycode(dpy, XK_Return);
							} else {
								code = XKeysymToKeycode(dpy, XK_space);
							}
							XTestFakeKeyEvent(dpy, code, True, 0);
							XTestFakeKeyEvent(dpy, code, False, 0);
						} else if (shift_modifier) {
							shift_modifier = 0;
						} else if (buffer_count == 4) {
							shift_modifier = 1;
						} else if (buffer_count == 5) {
							shift_modifier = 2;
						}

						if (buffer_count != 2) {
							XClearWindow(dpy, toplevel);
							update_display(dpy, toplevel, shift_modifier, help_screen);
							buffer_count = 1;
							buffer[0] = 0;
							break;
						}
					} else if (code) {
						if ((shift_modifier && isalpha(c)) || state_mod) {
							XTestFakeKeyEvent(dpy, Shift_code, True, 0);
							XTestFakeKeyEvent(dpy, code, True, 0);
							XTestFakeKeyEvent(dpy, code, False, 0);
							XTestFakeKeyEvent(dpy, Shift_code, False, 0);
						} else if (ctrl_modifier) {
							XTestFakeKeyEvent(dpy, Control_code, True, 0);
							XTestFakeKeyEvent(dpy, code, True, 0);
							XTestFakeKeyEvent(dpy, code, False, 0);
							XTestFakeKeyEvent(dpy, Control_code, False, 0);
							ctrl_modifier = 0;
						} else if (alt_key) {
							XTestFakeKeyEvent(dpy, Alt_code, True, 0);
							XTestFakeKeyEvent(dpy, code, True, 0);
							XTestFakeKeyEvent(dpy, code, False, 0);
							XTestFakeKeyEvent(dpy, Alt_code, False, 0);
							alt_key = 0;
						} else {
							XTestFakeKeyEvent(dpy, code, True, 0);
							XTestFakeKeyEvent(dpy, code, False, 0);
						}
						if (shift_modifier == 1) {
							shift_modifier = 0;
							XClearWindow(dpy, toplevel);
							update_display(dpy, toplevel, shift_modifier, help_screen);
						}
					}

					sent = 1;
					buffer_count = 1;
					buffer[0] = 0;
					break;
				}

				if(buffer_count == 1) {
					last_cross_timestamp = e.xcrossing.time;
				}
				if (!buffer_count || (buffer[buffer_count - 1] != region)) {
					buffer[buffer_count] = region;
					buffer_count++;
				}
				break;
			case Expose:
			case ConfigureNotify:
				XMapWindow(dpy, toplevel);
				update_display(dpy, toplevel, shift_modifier, help_screen);
				break;
			case ClientMessage:
				if ((e.xclient.message_type == mb_im_invoker_command) ||
					(e.xclient.message_type == mtp_im_invoker_command)) {
					if (e.xclient.data.l[0] == KeyboardShow) {
						XMapWindow(dpy, toplevel);
						update_display(dpy, toplevel, shift_modifier, help_screen);
					}
					if (e.xclient.data.l[0] == KeyboardHide) {
						XUnmapWindow(dpy, toplevel);
						XSync(dpy, False);
					}
					if (e.xclient.data.l[0] == KeyboardToggle) {
						if (visible) {
							XUnmapWindow(dpy, toplevel);
							XSync(dpy, False);
							visible = 0;
						} else {
							XMapWindow(dpy, toplevel);
							update_display(dpy, toplevel, shift_modifier, help_screen);
							visible = 1;
						}
					}
					break;
				}
				if (e.xclient.data.l[0] == wmDeleteMessage)
					run = 0;
				break;
			}
	}

	close_window(dpy, toplevel);

	exit(0);
}

