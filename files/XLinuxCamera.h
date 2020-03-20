/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream header

* @file XLinuxCamera.h
 *
 * This file implements all the functions related to Linux X11 windowing 
 * system API's for application.
 ***********************************************************************/

#ifndef XLINUXCAMERA_H
#define XLINUXCAMERA_H

#include <cstdlib>
#include <EGL/egl.h>

#include "XCamera.h"

class XLinuxCamera : public XCamera
{
private:

	int windowWidth;
	int windowHeight;
	Colormap colormap;
	XVisualInfo *visual;
	static XCamera* instance;
	XLinuxCamera(void);
	static Bool wait_for_map(Display *display, XEvent *event, char *windowPointer);
public:
	static XCamera* getHandler(void);
	virtual void prepareWindow(int width, int height);
	virtual void destroyWindow(void);
	virtual WindowStatus checkWindow(void);
	bool createX11Window(void); 
};
#endif /* XLINUXCAMERA_H */
