/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream Source

 * @file XLinuxMali.cpp
 *
 * This file implements all the functions related to Linux XCamera class 
 * for application.
 ***********************************************************************/

#include <iostream>
#include <cstdlib>
#include <sys/fcntl.h>
#include <sys/unistd.h>

#include "XLinuxTarget.h"

using std::cout;
using std::endl;

XCamera* XLinuxMali::instance = NULL;

XLinuxMali::XLinuxMali(void)
{

}

XCamera* XLinuxMali::getHandler(void)
{
	if (instance == NULL)
	{
		instance = new XLinuxMali();
	}
	return instance;
}

void XLinuxMali::prepareWindow(int width, int height)
{
	Fwindow = (fbdev_window *)calloc(1, sizeof(fbdev_window));
	if(Fwindow == NULL)
	{
		cout << "Out of memory at " << __FILE__ << ":" << __LINE__ << endl;
		exit(1);
	}
	Fwindow->width = width;
	Fwindow->height = height;
}

void XLinuxMali::destroyWindow(void)
{
	free(Fwindow);
}

XCamera::WindowStatus XLinuxMali::checkWindow(void)
{
	return XCamera::WINDOW_IDLE;
}
