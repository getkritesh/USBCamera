/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream header

 * @file XEGLIntf.h
 *
 * This file implements all the functions related to EGL bindings.
 ***********************************************************************/
#ifndef XEGLINTF_H
#define XEGLINTF_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

class CoreEGL
{
private:
	static EGLConfig findConfig(bool strictMatch);
	static EGLint configAttributes [];
	static EGLint contextAttributes [];
	static EGLint windowAttributes [];
	
public:
	static void setEGLSamples(EGLint requiredEGLSamples);
	enum OpenGLESVersion {OPENGLES1, OPENGLES2, OPENGLES3, OPENGLES31};
	static EGLDisplay display;
	static EGLContext context;
	static EGLConfig config;
	static EGLSurface surface;
	static void initializeEGL(OpenGLESVersion requestedAPIVersion);
	static void terminateEGL(void);
};

#endif /* XEGLINTF_H */
