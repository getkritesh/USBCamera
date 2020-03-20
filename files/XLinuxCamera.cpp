/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream Source

 * @file XLinuxCamera.cpp
 *
 * This file implements all the functions related to Linux X11 windowing 
 * system API's for application.
 ***********************************************************************/
#include <iostream>
#include "XLinuxCamera.h"

using std::cout;
using std::endl;

XCamera* XLinuxCamera::instance = NULL;

XLinuxCamera::XLinuxCamera(void)
{

}

XCamera* XLinuxCamera::getHandler(void)
{
	if (instance == NULL)
	{
		instance = new XLinuxCamera();
	}
	return instance;
}

void XLinuxCamera::prepareWindow(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
}

bool XLinuxCamera::createX11Window(void)
{
	XSetWindowAttributes windowAttributes;
	XSizeHints sizeHints;
	XEvent event;
	XVisualInfo visualInfoTemplate;

	unsigned long mask;
	long screen;

	int visualID, numberOfVisuals;

	display = XOpenDisplay(NULL);

	screen = DefaultScreen(display);

	eglGetConfigAttrib(CoreEGL::display, CoreEGL::config, EGL_NATIVE_VISUAL_ID, &visualID);
	visualInfoTemplate.visualid = visualID;
	visual = XGetVisualInfo(display, VisualIDMask, &visualInfoTemplate, &numberOfVisuals);

	if (visual == NULL)
	{
		cout << "Couldn't get X visual info" << endl;
		return false;
	}

	colormap = XCreateColormap(display, RootWindow(display, screen), visual->visual, AllocNone);

	windowAttributes.colormap = colormap;
	windowAttributes.background_pixel = 0xFFFFFFFF;
	windowAttributes.border_pixel = 0;
	windowAttributes.event_mask = StructureNotifyMask | ExposureMask;

	mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

	window = XCreateWindow(display, RootWindow(display, screen), 0, 0, windowWidth, windowHeight,
		0, visual->depth, InputOutput, visual->visual, mask, &windowAttributes);
	sizeHints.flags = USPosition;
	sizeHints.x = 10;
	sizeHints.y = 10;

	XSetStandardProperties(display, window, "Camera Stream ", "", None, 0, 0, &sizeHints);
	XMapWindow(display, window);
	XIfEvent(display, &event, wait_for_map, (char*)&window);
	XSetWMColormapWindows(display, window, &window, 1);
	XFlush(display);

	XSelectInput(display, window, KeyPressMask | ExposureMask | EnterWindowMask
					| LeaveWindowMask | PointerMotionMask | VisibilityChangeMask | ButtonPressMask
					| ButtonReleaseMask | StructureNotifyMask);

	return true;
}

void XLinuxCamera::destroyWindow(void)
{
	XDestroyWindow(display, window);
	XFreeColormap(display, colormap);
	XFree(visual);
	XCloseDisplay(display);
}

XCamera::WindowStatus XLinuxCamera::checkWindow(void)
{
	XEvent event;

	while (XPending(display) > 0) 
	{
		XNextEvent(display, &event);

		if (event.type == ButtonPress)
		{
			return XCamera::WINDOW_CLICK;
		}
	}
	return XCamera::WINDOW_IDLE;
}

Bool XLinuxCamera::wait_for_map(Display *display, XEvent *event, char *windowPointer) 
{
	return (event->type == MapNotify && event->xmap.window == (*((Window*)windowPointer)));
}
