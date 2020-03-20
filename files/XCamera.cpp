/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream Source

 * @file XCamera.cpp
 *
 * This file implements all the functions related to Camera baseclass.
 ***********************************************************************/

#include <cstdio>
#include <cstdarg>

#include "XCamera.h"


XCamera* XCamera::getHandler()
{
#ifdef ENABLE_FBDEV
	return XLinuxMali::getHandler();
#endif

#ifdef ENABLE_X11
	return XLinuxCamera::getHandler();
#endif
}


