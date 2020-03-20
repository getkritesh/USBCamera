/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream header

* @file XLinuxTarget.h
 *
 * This file implements all the functions related to Linux XCamera class 
 * for application.
 ***********************************************************************/
#ifndef XLINUXTARGET_H
#define XLINUXTARGET_H

#include <cstdlib>
#include "XCamera.h"
#include "Xfbdev_window.h"

class XLinuxMali : public XCamera
{
private:
	static XCamera* instance;
	XLinuxMali(void);
public:
	static XCamera* getHandler(void);
	virtual void prepareWindow(int width, int height);
	virtual void destroyWindow(void);
	virtual WindowStatus checkWindow(void);
};
#endif /* XLINUXTARGET_H */

