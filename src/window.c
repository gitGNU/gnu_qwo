/*
 *  Copyright (C) 2008, 2009 Charles Clement caratorn _at_ gmail.com
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

const XPoint point1 = { WIDTH / 3, 0 };
const XPoint point2 = { 2*(WIDTH / 3), 0 };
const XPoint point3 = { WIDTH, (HEIGHT / 3) };
const XPoint point4 = { WIDTH, 2*(HEIGHT / 3) };
const XPoint point5 = { 2*(WIDTH / 3), HEIGHT };
const XPoint point6 = { (WIDTH / 3), HEIGHT };
const XPoint point7 = { 0, 2*(HEIGHT/ 3) };
const XPoint point8 = { 0, (HEIGHT / 3) };
const XPoint point9 = { (WIDTH / 3) + DELTA, (HEIGHT / 3) - DELTA };
const XPoint point10 = { 2*(WIDTH / 3) - DELTA, (HEIGHT / 3) - DELTA };
const XPoint point11 =  { 2*(WIDTH / 3) + DELTA, (HEIGHT / 3) + DELTA };
const XPoint point12 = { 2*(WIDTH / 3) + DELTA, 2*(HEIGHT / 3) - DELTA };
const XPoint point13 = { 2*(WIDTH / 3) - DELTA, 2*(HEIGHT / 3) + DELTA };
const XPoint point14 = { (WIDTH / 3) + DELTA, 2*(HEIGHT / 3) + DELTA };
const XPoint point15 = { (WIDTH / 3) - DELTA, 2*(HEIGHT / 3) - DELTA };
const XPoint point16 = { (WIDTH / 3) - DELTA, (HEIGHT / 3) + DELTA };

/*
 * Cardinal points
 */

const XPoint point_ne = { 0, 0};
const XPoint point_nw = { WIDTH, 0};
const XPoint point_sw = { WIDTH, HEIGHT};
const XPoint point_se = { 0, HEIGHT};

void init_regions(Display *dpy, Window toplevel)
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
		FILL_REGION8(point9, point10, point11, point12, point13,
				point14, point15, point16),
		FILL_REGION5(point_ne, point1, point9, point16, point8),
		FILL_REGION4(point1, point2, point10, point9),
		FILL_REGION5(point2, point_nw, point3, point11, point10),
		FILL_REGION4(point3, point4, point12, point11),
		FILL_REGION5(point4, point_sw, point5, point13, point12),
		FILL_REGION4(point5, point6, point14, point13),
		FILL_REGION5(point6, point_se, point7, point15, point14),
		FILL_REGION4(point7, point8, point16, point15)
	};

	for (number = 0; number < MAX_REGIONS; number++){
		region = XPolygonRegion(regions[number], ARRAY_SIZE(regions[number]),
				EvenOddRule);
		region_window = XCreateWindow(dpy, toplevel, 0, 0, WIDTH, HEIGHT, 0,
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

void draw_grid(Display *dpy, Pixmap pixmap, GC gc)
{
	XColor grid_color, exact;
	Colormap cmap;

	unsigned long blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	cmap = DefaultColormap(dpy, DefaultScreen(dpy));

	XAllocNamedColor(dpy, cmap, GRID_COLOR, &grid_color, &exact);

	XSetForeground(dpy, gc, grid_color.pixel);

	XDrawLine(dpy, pixmap, gc, point1.x, point1.y, point9.x, point9.y);
	XDrawLine(dpy, pixmap, gc, point2.x, point2.y, point10.x, point10.y);

	XDrawLine(dpy, pixmap, gc, point3.x, point3.y, point11.x, point11.y);
	XDrawLine(dpy, pixmap, gc, point4.x, point4.y, point12.x, point12.y);

	XDrawLine(dpy, pixmap, gc, point5.x, point5.y, point13.x, point13.y);
	XDrawLine(dpy, pixmap, gc, point6.x, point6.y, point14.x, point14.y);

	XDrawLine(dpy, pixmap, gc, point7.x, point7.y, point15.x, point15.y);
	XDrawLine(dpy, pixmap, gc, point8.x, point8.y, point16.x, point16.y);

	XSetForeground(dpy, gc, blackColor);

}

int load_charset(Display *dpy, GC gc, int num){
	Visual *vis;
	Colormap cm;
	int depth;
	char image_path[MAX_IMAGE_PATH];
	Imlib_Image image;

	unsigned long blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	unsigned long whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

	XSetForeground(dpy, gc, whiteColor);
	XFillRectangle(dpy, char_pixmaps[num], gc, 0, 0, WIDTH, HEIGHT);
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
	imlib_render_image_on_drawable(0, 0);

	draw_grid(dpy, char_pixmaps[num], gc);

	imlib_free_image();

	return 0;
}

int set_window_properties(Display *dpy, Window toplevel){
	XWMHints *wm_hints;
	Atom net_wm_state_skip_taskbar, net_wm_state_skip_pager, net_wm_state,
		net_wm_window_type, net_wm_window_type_toolbar;

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

	XChangeProperty (dpy, toplevel, net_wm_state, XA_ATOM, 32, PropModeAppend,
			(unsigned char *)&net_wm_state_skip_taskbar, 1);
	XChangeProperty (dpy, toplevel, net_wm_state, XA_ATOM, 32, PropModeAppend,
			(unsigned char *)&net_wm_state_skip_pager, 1);
	XChangeProperty(dpy, toplevel, net_wm_window_type, XA_ATOM, 32,
			PropModeReplace, (unsigned char *) &net_wm_window_type_toolbar, 1);

	return 0;
}

int set_window_geometry(Display *dpy, Window win, char *geometry){
	int xpos, ypos, width, height, return_mask, gravity;
	XSizeHints	size_hints;

	size_hints.flags = PMinSize | PMaxSize;
	size_hints.min_width = size_hints.max_width = WIDTH;
	size_hints.min_height = size_hints.max_height = HEIGHT;

	XSetWMNormalHints(dpy, win, &size_hints);

	if (geometry){

		return_mask = XWMGeometry(dpy, DefaultScreen(dpy), geometry,
				NULL, 0, &size_hints, &xpos, &ypos, &width, &height, &gravity);

		if (return_mask & (WidthValue | HeightValue)){
			fprintf(stderr, "Can't resize windows\n");
		}

		if (return_mask & (XValue | YValue)){
			XMoveWindow(dpy, win, xpos, ypos);
		}
	}

	return 0;
}

void update_display(Display *dpy, Window toplevel, GC gc, int shift, int help){

	XClearWindow(dpy, toplevel);
	if (help) {
		XCopyArea(dpy, char_pixmaps[MAX_IMAGES - 1], toplevel, gc, 0,0, WIDTH, HEIGHT, 0, 0);
	} else if (shift) {
		XCopyArea(dpy, char_pixmaps[1], toplevel, gc, 0,0, WIDTH, HEIGHT, 0, 0);
	} else {
		XCopyArea(dpy, char_pixmaps[0], toplevel, gc, 0,0, WIDTH, HEIGHT, 0, 0);
	}
	XSync(dpy, False);
}
