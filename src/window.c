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

Pixmap char_pixmaps[3];

const char image_names[][MAX_IMAGE_NAME] = { "normal", "caps", "extra" };

Atom wmDeleteMessage, mtp_im_invoker_command, mb_im_invoker_command;

GC gc;

static unsigned int window_size;

void init_coordinates(XPoint point[], int width, int height, int delta){

	/*
	 * 0-7 are outer points
	 * 8-15 are inner points
	 * 16, 17, 18, 19 are north east, nort west, south west, south east
	 */

	point[16].x = point[16].y = point[17].y = point[19].x = 0;
	point[17].x = point[18].x = width;
	point[18].y = point[19].y = height;

	point[0].y = point[1].y = point[6].x = point[7].x = 0;

	point[0].x = point[5].x = width / 3;
	point[1].x = point[4].x = 2*(width / 3);
	point[2].x = point[3].x = width;

	point[2].y = point[7].y = height / 3;
	point[3].y = point[6].y = 2*(height / 3);
	point[4].y = point[5].y = height;

	point[8].x = point[13].x = (width / 3) + delta;
	point[10].y = point[15].y = (height / 3) + delta;

	point[10].x = point[11].x = 2*(width / 3) + delta;
	point[12].y = point[13].y = 2*(height / 3) + delta;

	point[9].x = point[12].x = 2*(width / 3) - delta;
	point[11].y = point[14].y = 2*(height / 3) - delta;

	point[14].x = point[15].x = (width / 3) - delta;
	point[8].y = point[9].y = (height / 3) - delta;

}

void init_regions(Display *dpy, Window toplevel, XPoint point[], int size)
{
	Window region_window;
	Region region;
	char window_name[2];
	int number;
	XWMHints *wm_hints;

	wm_hints = XAllocWMHints();

	if (wm_hints) {
		wm_hints->input = False;
		wm_hints->flags = InputHint;
	}

	XPoint regions[MAX_REGIONS][MAX_POINTS] = {
		FILL_REGION8(point[8], point[9], point[10], point[11], point[12],
				point[13], point[14], point[15]),
		FILL_REGION5(point[16], point[0], point[8], point[15], point[7]),
		FILL_REGION4(point[0], point[1], point[9], point[8]),
		FILL_REGION5(point[1], point[17], point[2], point[10], point[9]),
		FILL_REGION4(point[2], point[3], point[11], point[10]),
		FILL_REGION5(point[3], point[18], point[4], point[12], point[11]),
		FILL_REGION4(point[4], point[5], point[13], point[12]),
		FILL_REGION5(point[5], point[19], point[6], point[14], point[13]),
		FILL_REGION4(point[6], point[7], point[15], point[14])
	};

	for (number = 0; number < MAX_REGIONS; number++){
		region = XPolygonRegion(regions[number], ARRAY_SIZE(regions[number]),
				EvenOddRule);
		region_window = XCreateWindow(dpy, toplevel, 0, 0, size, size, 0,
				CopyFromParent, InputOnly, CopyFromParent, 0, NULL);
		sprintf(window_name, "%i", number);
		XStoreName(dpy, region_window, window_name);
		XShapeCombineRegion(dpy, region_window, ShapeBounding, 0, 0, region,
				ShapeSet);
		XSetWMHints(dpy, region_window, wm_hints);
		XSelectInput(dpy, region_window, EnterWindowMask | LeaveWindowMask |
				ButtonPressMask | ButtonReleaseMask);
		XMapWindow(dpy, region_window);
	}
	XFree(wm_hints);
}

void draw_grid(Display *dpy, Pixmap pixmap, XPoint point[])
{
	XColor grid_color, exact;
	Colormap cmap;

	unsigned long blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	cmap = DefaultColormap(dpy, DefaultScreen(dpy));

	XAllocNamedColor(dpy, cmap, GRID_COLOR, &grid_color, &exact);

	XSetForeground(dpy, gc, grid_color.pixel);

	XDrawLine(dpy, pixmap, gc, point[0].x, point[0].y, point[8].x, point[8].y);
	XDrawLine(dpy, pixmap, gc, point[1].x, point[1].y, point[9].x, point[9].y);

	XDrawLine(dpy, pixmap, gc, point[2].x, point[2].y, point[10].x, point[10].y);
	XDrawLine(dpy, pixmap, gc, point[3].x, point[3].y, point[11].x, point[11].y);

	XDrawLine(dpy, pixmap, gc, point[4].x, point[4].y, point[12].x, point[12].y);
	XDrawLine(dpy, pixmap, gc, point[5].x, point[5].y, point[13].x, point[13].y);

	XDrawLine(dpy, pixmap, gc, point[6].x, point[6].y, point[14].x, point[14].y);
	XDrawLine(dpy, pixmap, gc, point[7].x, point[7].y, point[15].x, point[15].y);

	XSetForeground(dpy, gc, blackColor);

}

int load_charset(Display *dpy, int num, int width, int height){
	Visual *vis;
	Colormap cm;
	int depth;
	char image_path[MAX_IMAGE_PATH];
	Imlib_Image image;

	unsigned long blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	unsigned long whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

	XSetForeground(dpy, gc, whiteColor);
	XFillRectangle(dpy, char_pixmaps[num], gc, 0, 0, width, height);
	XSetForeground(dpy, gc, blackColor);

	vis = DefaultVisual(dpy, DefaultScreen(dpy));
	depth = DefaultDepth(dpy, DefaultScreen(dpy));
	cm = DefaultColormap(dpy, DefaultScreen(dpy));

	imlib_context_set_display(dpy);
	imlib_context_set_visual(vis);
	imlib_context_set_colormap(cm);
	imlib_context_set_progress_function(NULL);

	imlib_context_set_drawable(char_pixmaps[num]);

	strncpy(image_path, DATA_PATH, MAX_IMAGE_PATH);
	strncat(image_path + strlen(DATA_PATH), image_names[num],
		MAX_IMAGE_PATH - strlen(DATA_PATH));
	strncat(image_path + strlen(DATA_PATH) + strlen(image_names[num]),
			IMAGE_SUFFIXE, MAX_IMAGE_PATH - strlen(DATA_PATH) - strlen(image_names[num]));
	image = imlib_load_image(image_path);
	if (!image) {
		fprintf(stderr, "%s : Can't open file\n", image_path);
		return 1;
	}
	imlib_context_set_image(image);
	imlib_render_image_on_drawable_at_size(0, 0, width, height);

	imlib_free_image();

	return 0;
}

int set_window_properties(Display *dpy, Window toplevel){
	XWMHints *wm_hints;
	Atom net_wm_state_skip_taskbar, net_wm_state_skip_pager, net_wm_state,
		net_wm_window_type, net_wm_window_type_toolbar,
		net_wm_allowed_actions, net_wm_action_resize;
	Atom actions[1];
	XSizeHints	size_hints;

	size_hints.flags = PMinSize | PMaxSize ;
	size_hints.min_width = size_hints.min_height = MIN_WIDTH;
	size_hints.max_width = size_hints.max_height = MAX_WIDTH;

	XSetWMNormalHints(dpy, toplevel, &size_hints);

	XStoreName(dpy, toplevel, "Keyboard");

	wm_hints = XAllocWMHints();

	if (wm_hints) {
		wm_hints->input = False;
		wm_hints->flags = InputHint;
		XSetWMHints(dpy, toplevel, wm_hints);
		XFree(wm_hints);
	}

	wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	mtp_im_invoker_command = XInternAtom(dpy, "_MTP_IM_INVOKER_COMMAND", False);
	mb_im_invoker_command = XInternAtom(dpy, "_MB_IM_INVOKER_COMMAND", False);
	XSetWMProtocols(dpy, toplevel, &wmDeleteMessage, 1);

	net_wm_state_skip_pager = XInternAtom(dpy, "_NET_WM_STATE_SKIP_PAGER", False);
	net_wm_state_skip_taskbar = XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", False);
	net_wm_state = XInternAtom (dpy, "_NET_WM_STATE", False);
	net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE" , False);
	net_wm_window_type_toolbar = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);

	net_wm_allowed_actions = XInternAtom(dpy, "_NET_WM_ALLOWED_ACTIONS", False);
	net_wm_action_resize = XInternAtom(dpy, "_NET_WM_ACTION_RESIZE", False);

	actions[0] = net_wm_action_resize;

	XChangeProperty (dpy, toplevel, net_wm_state, XA_ATOM, 32, PropModeAppend,
			(unsigned char *)&net_wm_state_skip_taskbar, 1);
	XChangeProperty (dpy, toplevel, net_wm_state, XA_ATOM, 32, PropModeAppend,
			(unsigned char *)&net_wm_state_skip_pager, 1);
	XChangeProperty(dpy, toplevel, net_wm_window_type, XA_ATOM, 32,
			PropModeReplace, (unsigned char *) &net_wm_window_type_toolbar, 1);
	XChangeProperty(dpy, toplevel, net_wm_allowed_actions, XA_ATOM, 32,
			PropModeReplace, (unsigned char *) &actions, 1);

	return 0;
}

void update_display(Display *dpy, Window toplevel, int shift, int help){

	XClearWindow(dpy, toplevel);
	if (help) {
		XCopyArea(dpy, char_pixmaps[MAX_IMAGES - 1], toplevel, gc, 0,0, window_size, window_size, 0, 0);
	} else if (shift) {
		XCopyArea(dpy, char_pixmaps[1], toplevel, gc, 0,0, window_size, window_size, 0, 0);
	} else {
		XCopyArea(dpy, char_pixmaps[0], toplevel, gc, 0,0, window_size, window_size, 0, 0);
	}
	XSync(dpy, False);
}

int create_window(Display *dpy, Window win, int size){
	int delta, i;
	int status = 0;
	XPoint coordinates[20];

	window_size = size;

	XResizeWindow(dpy, win, window_size, window_size);

	delta = (window_size * DEFAULT_DELTA) / DEFAULT_WIDTH;

	init_coordinates(coordinates, window_size, window_size, delta);
	init_regions(dpy, win, coordinates, window_size);

	XSelectInput(dpy, win, SubstructureNotifyMask | StructureNotifyMask | ExposureMask);

	for( i = 0; i < MAX_IMAGES; i++) {
		char_pixmaps[i] = XCreatePixmap(dpy, win, window_size, window_size,
				DefaultDepth(dpy, DefaultScreen(dpy)));
		load_charset(dpy, i, window_size, window_size);
		draw_grid(dpy, char_pixmaps[i], coordinates);
	}

	XMapWindow(dpy, win);

	return status;
}

Window resize_window(Display *dpy, Window win, int number){
	int size;

	size = window_size + (number * INCREMENT);

	if ((size < MIN_WIDTH) || (size > MAX_WIDTH)){
		return win;
	}

	XUnmapWindow(dpy, win);

	XDestroySubwindows(dpy, win);

	XFreePixmap(dpy, char_pixmaps[0]);
	XFreePixmap(dpy, char_pixmaps[1]);
	XFreePixmap(dpy, char_pixmaps[2]);

	create_window(dpy, win, size);

	return win;
}

int init_window(Display *dpy, Window win, char *geometry){
	unsigned long valuemask;
	XGCValues xgc;
	unsigned int width, height;
	int xpos, ypos, return_mask, status;
	int dpy_width, dpy_height;

	xgc.foreground = BlackPixel(dpy, DefaultScreen(dpy));
	xgc.background = WhitePixel(dpy, DefaultScreen(dpy));
	xgc.line_width = 2;
	valuemask = GCForeground | GCBackground | GCLineWidth;

	gc = XCreateGC(dpy, DefaultRootWindow(dpy), valuemask, &xgc);

	set_window_properties(dpy, win);

	return_mask = XParseGeometry(geometry, &xpos, &ypos, &width, &height);

	if ((width < MIN_WIDTH) || (width > MAX_WIDTH)){
		width = DEFAULT_WIDTH;
	}

	if ((height < MIN_HEIGHT) || (height > MAX_HEIGHT)){
		height = DEFAULT_HEIGHT;
	}

	dpy_width = XDisplayWidth(dpy, DefaultScreen(dpy));
	dpy_height = XDisplayHeight(dpy, DefaultScreen(dpy));

	if (!(return_mask & XValue)){
		xpos = 0;
	} else if(return_mask & XNegative) {
		xpos+= dpy_width - width;
	}

	if (!(return_mask & YValue)){
		ypos = 0;
	} else if (return_mask & YNegative) {
		ypos+= dpy_height - height;
	}

	XMoveWindow(dpy, win, xpos, ypos);

	window_size = width;

	status = create_window(dpy, win, width);

	return status;
}

void close_window(Display *dpy, Window toplevel){

	XFreeGC(dpy, gc);
	XFreePixmap(dpy, char_pixmaps[0]);
	XFreePixmap(dpy, char_pixmaps[1]);
	XFreePixmap(dpy, char_pixmaps[2]);
	XDestroyWindow(dpy, toplevel);
	XCloseDisplay(dpy);

}


