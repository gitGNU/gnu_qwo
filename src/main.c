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

#include <init.h>
#include <unistd.h>
#include <ctype.h>

#include <X11/keysym.h>
#include <locale.h>

#define MAX_GESTURES_BUFFER      10

#define LONG_EXPOSURE_DELAY		2000L

static const char *copyright_notice = "\
Copyright (C) 2008-2009 Charles Clement\n\
qwo comes with ABSOLUTELY NO WARRANTY. This is free software,\n\
and you are welcome to redistribute it under certain conditions.\n";

char charset[][MAX_REGIONS] = {
		"a,zz""?>>c",
		"debb???d",
		"gfihjj?g",
		"?ll>kk??",
		"?mmnopqq",
		"???ttsrr",
		"???..vuw",
		"yy???xx<"
};

KeySym char_free[MAX_REGIONS][MAX_REGIONS] = {
	{XK_5, 0xffff, XK_Up, 0xffff, XK_Right, XK_backslash, XK_0, XK_slash, XK_Left},
	{XK_grave, XK_1, XK_Tab, XK_equal, 0xffff, XK_braceleft, XK_bracketleft, XK_parenleft, XK_bar},
	{XK_Down, XK_asciicircum, XK_2, XK_minus, XK_greater, 0xffff, 0xffff, 0xffff, XK_less},
	{XK_apostrophe, XK_percent, XK_dollar, XK_3, XK_Insert, XK_parenright, XK_bracketright, XK_braceright, 0xffff},
	{0xffff, 0xffff, 0xffff, XK_exclam, XK_6, XK_Return, XK_Alt_L, 0xffff, XK_Home},
	{0xffff, 0xffff, 0xffff, XK_colon, XK_semicolon, XK_9, XK_asterisk, XK_ampersand, 0xffff},
	{0xffff, 0xffff, 0xffff, 0xffff, 0xffff, XK_underscore, XK_8, XK_at, 0xffff},
	{0xffff, XK_numbersign, 0xffff, 0xffff, 0xffff, XK_plus, XK_asciitilde, XK_7, XK_quotedbl},
	{0xffff, XK_Escape, 0xffff, 0xffff, XK_End, 0xffff, XK_Control_L, XK_Select, XK_4}
};

static KeyCode Shift_code, Control_code, Alt_code;

void usage(){
	fprintf(stdout,
		"Usage: qwo [options]\n\n"
		"Options:\n"
		"  -g, --geometry [size][{+-}<xoffset>[{+-}<yoffset>]]\n"
		"			specify window size and/or position\n"
		"  -c, --config <file>	use configuration file <file> instead of ~/.qworc\n"
		"  -f, --foreground <color>	set the foreground color\n"
		"  -b, --background <color>	set the background color\n"
		"  -d, --delimiter-color <color> set the region delimiter color\n"
		"        Where color is one of 0xrrggbb or a named color as red...\n"
		"  -h, --help      	Print this help\n"
		"  -v, --version   	Print version information\n"
		);
}

void print_version(){
	fprintf(stdout, PACKAGE " " VERSION " ");
	fprintf(stdout, copyright_notice);
}

int init_keycodes(){
	Shift_code = XKeysymToKeycode(dpy, XK_Shift_L);
	Control_code = XKeysymToKeycode(dpy, XK_Control_L);
	Alt_code = XKeysymToKeycode(dpy, XK_Alt_L);

	return 0;
}

char get_region_name(Window win) {
	char *region_name;
	char value;

	XFetchName(dpy, win, &region_name);
	value = region_name[0] - 48;
	XFree(region_name);

	return value;
}

void send_key(KeyCode code, KeyCode modifier)
{
	if (modifier)
		XTestFakeKeyEvent(dpy, modifier, True, 0);
	XTestFakeKeyEvent(dpy, code, True, 0);
	XTestFakeKeyEvent(dpy, code, False, 0);
	if (modifier)
		XTestFakeKeyEvent(dpy, modifier, False, 0);

}

int main(int argc, char **argv)
{
	char *display_name;
	Window toplevel;

	int event_base, error_base;
	int shape_ext_major, shape_ext_minor;

	int parse_code = 0;
	int run = 1;
	int visible = 1;
	int buffer[MAX_GESTURES_BUFFER];
	int buffer_count = 0;
	int shift_modifier = 0;
	int ctrl_modifier = 0;
	int alt_key = 0;
	int help_screen = 0;
	int sent = 0;
	Time last_cross_timestamp = 0L;
	Time last_pressed = 0L;

	display_name = XDisplayName(NULL);
	dpy = XOpenDisplay(display_name);

	parse_code = parse_command_line(argc, argv);

	if (parse_code == 1)
		print_version();
	else if (parse_code == 2)
		usage();

	if (parse_code)
		exit(0);

	read_config();

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

	XShapeQueryVersion(dpy, &shape_ext_major, &shape_ext_minor);

	toplevel = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, DEFAULT_WIDTH,
			DEFAULT_HEIGHT, 0, CopyFromParent, CopyFromParent, CopyFromParent,
			0, NULL);

	if (init_window(toplevel)){
		fprintf(stderr, "Can't initialise window\n");
		exit(3);
	}

	init_keycodes();

	update_display(toplevel, shift_modifier, help_screen);

	while(run) {
		XEvent e;
		int region;
		int state_mod = 0;
		KeyCode code;
		KeySym ksym;

		XNextEvent(dpy, &e);
		switch (e.type) {
		case ButtonPress:
			region = get_region_name(e.xbutton.window);
			XUngrabPointer(dpy, CurrentTime);
			buffer[0] = region;
			last_pressed = e.xbutton.time;
			buffer_count = 1;
			sent = 0;
			break;
		case ButtonRelease:
			region = get_region_name(e.xbutton.window);
			if (buffer[0] == 0 && !region && sent)
				break;
			ksym = char_free[buffer[0]][region];
			if (ksym == XK_Alt_L) {
				alt_key = 1;
			} else if (ksym == XK_Control_L) {
				ctrl_modifier = 1;
			} else if (ksym == XK_Insert) {
				code = XKeysymToKeycode(dpy, ksym);
				send_key(code, Shift_code);
			} else if (ksym == XK_Select) {
				help_screen = !help_screen;
				update_display(toplevel, shift_modifier, help_screen);
			} else {
				code = XKeysymToKeycode(dpy, ksym);

				for (state_mod = 0; state_mod < 4; state_mod++) {
					if (XKeycodeToKeysym(dpy, code, state_mod) == ksym ) {
						break;
					}
				}

				if (state_mod)
					send_key(code, Shift_code);
				else
					send_key(code, 0);
			}
			buffer_count = 0;
			break;
		case EnterNotify:
			region = get_region_name(e.xcrossing.window);
			KeySym index;

			char c = '\0';

			if ((region == 0) && buffer_count > 1 && buffer[0] == 0){

				if ((buffer[1] == buffer[buffer_count - 1]) && (buffer_count > 2)) {
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
					send_key(code, 0);
				} else if (c == '>') {
					if (buffer_count == 2) {
						if (e.xcrossing.time - last_cross_timestamp > LONG_EXPOSURE_DELAY) {
							code = XKeysymToKeycode(dpy, XK_Return);
						} else {
							code = XKeysymToKeycode(dpy, XK_space);
						}
						send_key(code, 0);
					} else if (shift_modifier) {
						shift_modifier = 0;
					} else if (buffer_count == 4) {
						shift_modifier = 1;
					} else if (buffer_count == 5) {
						shift_modifier = 2;
					}

					if (buffer_count != 2) {
						XClearWindow(dpy, toplevel);
						update_display(toplevel, shift_modifier, help_screen);
						buffer_count = 1;
						buffer[0] = 0;
						sent = 1;
						break;
					}
				} else if (code) {
					if ((shift_modifier && isalpha(c)) || state_mod) {
						send_key(code, Shift_code);
					} else if (ctrl_modifier) {
						send_key(code, Control_code);
						ctrl_modifier = 0;
					} else if (alt_key) {
						send_key(code, Alt_code);
						alt_key = 0;
					} else {
						send_key(code, 0);
					}
					if (shift_modifier == 1) {
						shift_modifier = 0;
						XClearWindow(dpy, toplevel);
						update_display(toplevel, shift_modifier, help_screen);
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

			if(buffer_count == 9 && buffer[0] == buffer[buffer_count - 1]){
				int diff;
				XNextEvent(dpy, &e);

				while(e.type != ButtonRelease){
					XNextEvent(dpy, &e);
				}

				region = get_region_name(e.xcrossing.window);
				diff = region - buffer[8];
				if (!diff) {
					sent = 1;
					buffer_count = 0;
					break;
				}
				if (!(DIRECTION(buffer[7], buffer[8]))){
					if (diff > 0) {
						diff = diff - MAX_REGIONS + 1;
					}
				} else {
					if (diff < 0) {
						diff = diff + MAX_REGIONS - 1;
					}
				}
				toplevel = resize_window(toplevel, diff);
				sent = 1;
				buffer_count = 0;
				update_display(toplevel, shift_modifier, help_screen);
			}
			break;
		case Expose:
		case ConfigureNotify:
			XMapWindow(dpy, toplevel);
			update_display(toplevel, shift_modifier, help_screen);
			break;
		case ClientMessage:
			if ((e.xclient.message_type == mb_im_invoker_command) ||
				(e.xclient.message_type == mtp_im_invoker_command)) {
				if (e.xclient.data.l[0] == KeyboardShow) {
					XMapWindow(dpy, toplevel);
					update_display(toplevel, shift_modifier, help_screen);
					visible = 1;
				}
				if (e.xclient.data.l[0] == KeyboardHide) {
					XUnmapWindow(dpy, toplevel);
					XSync(dpy, False);
					visible = 0;
				}
				if (e.xclient.data.l[0] == KeyboardToggle) {
					if (visible) {
						XUnmapWindow(dpy, toplevel);
						XSync(dpy, False);
						visible = 0;
					} else {
						XMapWindow(dpy, toplevel);
						update_display(toplevel, shift_modifier, help_screen);
						visible = 1;
					}
				}
				break;
			}
			if (e.xclient.data.l[0] == wmDeleteMessage) {
				run = 0;
				break;
			}
			break;
		}
	}

	close_window(toplevel);

	exit(0);
}

