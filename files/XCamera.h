/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream header

 * @file XCamera.h
 *
 * This file implements all the functions related to Camera baseclass.
 ***********************************************************************/

#ifndef XCamera_H
#define XCamera_H

#include <cstdio>

#include "XEGLIntf.h"
#include "XCVector.h"
#include "Xfbdev_window.h"
#define ENABLE_X11 1
class XCamera
{
public:
	enum WindowStatus {
		WINDOW_IDLE, 
		WINDOW_EXIT, 
		WINDOW_CLICK};
	CVec2 mouseClick;
	fbdev_window *Fwindow;
	Window window;
	Display* display;
	virtual void prepareWindow(int width, int height) = 0;
	virtual WindowStatus checkWindow(void) = 0;
	virtual void destroyWindow(void) = 0;
	static void log(const char* format, ...);
	static XCamera* getHandler();
};

#ifdef ENABLE_FBDEV
#include "XLinuxTarget.h"
#endif

#ifdef ENABLE_X11
#include "XLinuxCamera.h"
#endif

#endif /* XCamera_H */
