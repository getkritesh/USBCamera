/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream Source

 * @file XEGLIntf.cpp
 *
 * This file implements all the functions related to EGL bindings.
 ***********************************************************************/
#include <iostream>
#include <cstdlib>

#include "XEGLIntf.h"
#include "XCamera.h"

using std::cout;
using std::endl;

EGLDisplay CoreEGL::display;
EGLContext CoreEGL::context;
EGLSurface CoreEGL::surface;
EGLConfig CoreEGL::config;

EGLint CoreEGL::configAttributes[] =
{
	EGL_SAMPLES,             4,
	EGL_ALPHA_SIZE,          0,
#ifdef ENABLE_FBDEV
	EGL_RED_SIZE,            8,
	EGL_GREEN_SIZE,          8,
	EGL_BLUE_SIZE,           8,
	EGL_BUFFER_SIZE,         32,
#else
	EGL_RED_SIZE,            5,
	EGL_GREEN_SIZE,          6,
	EGL_BLUE_SIZE,           5,
	EGL_BUFFER_SIZE,         16,
#endif
	EGL_STENCIL_SIZE,        0,
	EGL_RENDERABLE_TYPE,     0,    /* This field will be completed according to application request. */
	EGL_SURFACE_TYPE,        EGL_WINDOW_BIT ,
	EGL_DEPTH_SIZE,          16,
	EGL_NONE
};

EGLint CoreEGL::contextAttributes[] =
{
	EGL_CONTEXT_CLIENT_VERSION, 0, 
	EGL_NONE, EGL_NONE,            
	EGL_NONE
};

EGLint CoreEGL::windowAttributes[] =
{
	EGL_NONE
};


void CoreEGL::initializeEGL(OpenGLESVersion requestedAPIVersion)
{
	XCamera* platform = XCamera::getHandler();

	EGLBoolean success = EGL_FALSE;

#ifdef ENABLE_FBDEV
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#else
	platform->display = XOpenDisplay(NULL);
	display = eglGetDisplay(platform->display);
#endif

	if(display == EGL_NO_DISPLAY)
	{
		EGLint error = eglGetError();
		cout << "eglGetError():" << error << endl;
		cout << "No EGL Display available at" << __FILE__ << __LINE__ << endl;
		exit(1);
	}

	success = eglInitialize(display, NULL, NULL);
	if(success != EGL_TRUE)
	{
		EGLint error = eglGetError();
		cout << "eglGetError():" << error << endl;
		cout << "Failed to initialize EGL at" << __FILE__ << __LINE__ << endl;
		exit(1);
	}

	if(requestedAPIVersion == OPENGLES1)
	{
		configAttributes[15] = EGL_OPENGL_ES_BIT;
		contextAttributes[1] = 1;
		contextAttributes[2] = EGL_NONE;
	}
	else if(requestedAPIVersion == OPENGLES2)
	{
		configAttributes[15] = EGL_OPENGL_ES2_BIT;
		contextAttributes[1] = 2;
		contextAttributes[2] = EGL_NONE;
	}
#ifdef ENABLE_FBDEV
	config = findConfig(true);
#else
	config = findConfig(false);
#endif

#ifdef ENABLE_X11
	((XLinuxCamera*)(platform))->createX11Window();
#endif

	surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)(platform->window), windowAttributes);
	if(surface == EGL_NO_SURFACE)
	{
		EGLint error = eglGetError();
		cout << "eglGetError():" << error << endl;
		cout << "Failed to create EGL surface at" << __FILE__ << __LINE__ << endl;
		exit(1);
	}

	eglBindAPI(EGL_OPENGL_ES_API);

	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
	if(context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		cout << "eglGetError():" << error << endl;
		cout << "Failed to create EGL context at" << __FILE__ << __LINE__ << endl;
		exit(1);
	}
}

void CoreEGL::setEGLSamples(EGLint requiredEGLSamples)
{
	configAttributes[1] = requiredEGLSamples;
}

void CoreEGL::terminateEGL(void)
{
	eglBindAPI(EGL_OPENGL_ES_API);
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(display, context);
	eglDestroySurface(display, surface);
	eglTerminate(display);
}

EGLConfig CoreEGL::findConfig(bool strictMatch)
{
	EGLConfig *configsArray = NULL;
	EGLint numberOfConfigs = 0;
	EGLBoolean success = EGL_FALSE;

	/* Enumerate available EGL configurations which match or exceed our required attribute list. */
	success = eglChooseConfig(display, configAttributes, NULL, 0, &numberOfConfigs);
	if(success != EGL_TRUE)
	{
		EGLint error = eglGetError();
		cout << "eglGetError():" << error << endl;
		cout << "Failed to enumerate EGL configs at" << __FILE__ << __LINE__ << endl;
		exit(1);
	}

	if (numberOfConfigs == 0)
	{
		cout << "Disabling AntiAliasing to try and find a config" << endl;
		configAttributes[1] = EGL_DONT_CARE;
		success = eglChooseConfig(display, configAttributes, NULL, 0, &numberOfConfigs);
		if(success != EGL_TRUE)
		{
			EGLint error = eglGetError();
			cout << "eglGetError():" << error << endl;
			cout << "Failed to enumerate EGL configs at" << __FILE__ << __LINE__ << endl;
			exit(1);
		}

		if (numberOfConfigs == 0)
		{
			cout << "No configs found with the requested attributes" << endl;
			exit(1);
		}
		else
		{
			cout << "Configs found when antialiasing disabled" << endl;
		}
	}

	configsArray = (EGLConfig *)calloc(numberOfConfigs, sizeof(EGLConfig));
	if(configsArray == NULL)
	{
		cout << "Out of memory at " << __FILE__ << __LINE__ << endl;
		exit(1);
	}
	success = eglChooseConfig(display, configAttributes, configsArray, numberOfConfigs, &numberOfConfigs);
	if(success != EGL_TRUE)
	{
		EGLint error = eglGetError();
		cout << "eglGetError():" << error << endl;
		cout << "Failed to enumerate EGL configs at" << __FILE__ << __LINE__ << endl;
		exit(1);
	}

	bool matchFound = false;
	int matchingConfig = -1;

	if (strictMatch)
	{
		EGLint redSize = configAttributes[5];
		EGLint greenSize = configAttributes[7];
		EGLint blueSize = configAttributes[9];

		for(int configsIndex = 0; (configsIndex < numberOfConfigs) && !matchFound; configsIndex++)
		{
			EGLint attributeValue = 0;

			success = eglGetConfigAttrib(display, configsArray[configsIndex], EGL_RED_SIZE, &attributeValue);
			if(success != EGL_TRUE)
			{
				EGLint error = eglGetError();
				cout << "eglGetError():" << error << endl;
				cout << "Failed to get EGL attribute at" << __FILE__ << __LINE__ << endl;
				exit(1);
			}

			if(attributeValue == redSize)
			{
				success = eglGetConfigAttrib(display, configsArray[configsIndex], EGL_GREEN_SIZE, &attributeValue);
				if(success != EGL_TRUE)
				{
					EGLint error = eglGetError();
					cout << "eglGetError():" << error << endl;
					cout << "Failed to get EGL attribute at" << __FILE__ << __LINE__ << endl;
					exit(1);
				}

				if(attributeValue == greenSize)
				{
					success = eglGetConfigAttrib(display, configsArray[configsIndex], EGL_BLUE_SIZE, &attributeValue);
					if(success != EGL_TRUE)
					{
						EGLint error = eglGetError();
						cout << "eglGetError():" << error << endl;
						cout << "Failed to get EGL attribute at" << __FILE__ << __LINE__ << endl;
						exit(1);
					}

					if(attributeValue == blueSize) 
					{
						matchFound = true;
						matchingConfig = configsIndex;
					}
				}
			}
		}
	}
	else
	{
		matchingConfig = 0;
		matchFound = true;
	}

	if(!matchFound)
	{
		cout << "Failed to find matching EGL config at " << __FILE__ << __LINE__ << endl;
		exit(1);
	}

	EGLConfig configToReturn = configsArray[matchingConfig];

	free(configsArray);
	configsArray = NULL;

	return configToReturn;
}
