/***********************************************************************
  @author Kritesh tripathi
  

  Open GL handling source

 * @file camOpenGL.cpp
 *
 * This is source file for OpenGL rendering
 ***********************************************************************/

#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>

// Include files to use openGL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

// Include files to use DRM
#include <drm/drm_fourcc.h>

//Include files to use cairo
#include <cairo/cairo-gl.h>

// Enable for accessing X11
// #define X11_WINDOW

// Enable for accessing DMA BUFFER
// #define DMA_BUF_IMPORT

// Local includes
#ifdef X11_WINDOW
#include "XLinuxCamera.h"
#include "XEGLIntf.h"
#endif
#include "XGLSLCompile.h"

#include "Core_egl_zero_copy.h"
#include "camOpenGL.h"

#define GLES_VERSION 2

using std::cout;
using std::endl;
using std::vector;

// Maximum number of Spots that can be displayed
static const unsigned int NUMBER_OF_SPOTS = 50;

static const GLfloat textureCoordinates[] =
{
	0.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
};

static const GLfloat vertices[] =
{
	-1.0f, -1.0f,
	1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, 1.0f,
};


static char vertexShader[] =
	"#version 100						\n"
	"attribute vec4 a_v4Position;		\n"
	"attribute vec2 a_v2TexCoord;		\n"
	"varying vec2 v_v2TexCoord;			\n"
	"void main()						\n"
	"{									\n"
	"	v_v2TexCoord = a_v2TexCoord;	\n"
	"	gl_Position = a_v4Position;		\n"
	"}									\n";

static char fragmentShader[] =
	"#version 100																\n"
#ifdef DMA_BUF_IMPORT
	"#extension GL_OES_EGL_image_external : require								\n"
#endif
	"precision mediump float;													\n"
#ifdef DMA_BUF_IMPORT
	"uniform samplerExternalOES u_s2dTexture1;									\n"
#else
	"uniform sampler2D u_s2dTexture1;											\n"
#endif
	"uniform vec2 u_v2spot[%d];													\n"
	"uniform int u_iNumberOfSpot;												\n"
	"uniform vec4 u_v4spot_color[%d];											\n"
	"varying vec2 v_v2TexCoord;													\n"
	"void main()																\n"
	"{																			\n"
	"	vec4 bkg_color = texture2D(u_s2dTexture1,v_v2TexCoord), circle_color;	\n"
	"	float radius = 0.05;													\n"
	"	vec2 circle_center, uv;													\n"
	"	float dist, t;															\n"
	"	int spots = u_iNumberOfSpot;											\n"
	"	for(int iter = 0; iter < spots; iter++)									\n"
	"	{																		\n"
	"		circle_center = u_v2spot[iter];										\n"
	"		circle_color = u_v4spot_color[iter] * 0.05 + bkg_color * 0.95;		\n"
	"		uv = v_v2TexCoord.xy - circle_center;								\n"
	"		dist = sqrt(dot(uv, uv));											\n"
	"		t = step(radius, dist);												\n"
	"		bkg_color = mix(circle_color, bkg_color, t);						\n"
	"	}																		\n"
	"	bkg_color.w = 1.0;														\n"
	"	gl_FragColor = bkg_color;												\n"
	"}																			\n";

cOpenGL* cOpenGL::m_Instance = NULL;

cOpenGL* cOpenGL::getInstance()
{
	if (m_Instance == NULL)
	{
		m_Instance = new cOpenGL();
	}
	return m_Instance;
}

cOpenGL::cOpenGL():
	m_VertexShaderID(0),
	m_FragmentShaderID(0),
	m_ProgramID(0),
	m_iLocPosition(-1),
	m_iLocTexCoord(-1),
	m_iLocSampler(-1),
	m_iLocSpot(-1),
	m_iLocSpotColor(-1),
	m_iLocSpotNumber(-1),
	m_WindowWidth(0),
	m_WindowHeight(0),
	m_BytesPerPixel(0)
{
	memset(m_TextureID, 0, sizeof(GLuint)*eDMABuffer::eDMABuffer_MaxSize);
}

void cOpenGL::initialize(uint32_t windowWidth, uint32_t windowHeight, uint32_t bytesPerPixel)
{
	m_WindowWidth = windowWidth;
	m_WindowHeight = windowHeight;
	m_BytesPerPixel = bytesPerPixel; 

	if(bytesPerPixel == 4) {
		m_CairoPixelFormat = CAIRO_FORMAT_ARGB32;
		m_InternalFormat = GL_RGBA;
		m_Format = GL_UNSIGNED_BYTE;
	}else if(bytesPerPixel == 2){
		m_CairoPixelFormat = CAIRO_FORMAT_RGB16_565;
		m_InternalFormat = GL_RGB;
		m_Format = GL_UNSIGNED_SHORT_5_6_5;
	}

#ifdef X11_WINDOW
	m_Camera = XCamera::getHandler();
	m_Camera->prepareWindow(windowWidth, windowHeight);
	CoreEGL::initializeEGL(CoreEGL::OPENGLES2);
	m_EglDpy = CoreEGL::display;
	m_EglSurf = CoreEGL::surface;
	m_EglCtx = CoreEGL::context;
	if(!eglMakeCurrent(m_EglDpy, m_EglSurf, m_EglSurf, m_EglCtx))
	{
		cout << "eglMakeCurrent() failed" << endl;
		exit(1);
	}
#else
	EGLint egl_major, egl_minor;

	m_EglDpy = eglGetDisplay((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);
	if (!m_EglDpy) {
		cout << "Error: eglGetDisplay() failed" << endl;
		exit(1);
	}

	if (!eglInitialize(m_EglDpy, &egl_major, &egl_minor)) {
		cout << "Error: eglInitialize() failed" << endl;
		exit(1);
	}
	makeFbWindow();

	if (!eglMakeCurrent(m_EglDpy, m_EglSurf, m_EglSurf, m_EglCtx)) {
		cout << "Error: eglMakeCurrent failed" << endl;
		exit(1);
	}
	glViewport(0, 0, (GLint) windowWidth, (GLint) windowHeight);
	eglSwapInterval(m_EglDpy, 0);  // disable vsync
#endif
}

void cOpenGL::makeFbWindow()
{
	EGLint red_size;
	EGLint green_size;
	EGLint blue_size;
	EGLint depth_size;

	switch(m_BytesPerPixel){
		case 2:
			red_size = 5;
			green_size = 6;
			blue_size = 5;
			depth_size = 0;
			break;

		case 4:
		default:
			red_size = 8;
			green_size = 8;
			blue_size = 8;
			depth_size = 8;
			break;
	}

	static const EGLint attribs[] = {
		EGL_RED_SIZE, red_size,
		EGL_GREEN_SIZE, green_size,
		EGL_BLUE_SIZE, blue_size,
		EGL_DEPTH_SIZE, depth_size,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	static const EGLint ctx_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLConfig config;
	EGLint num_configs;
	EGLint vid;
	EGLint val;

	if (!eglChooseConfig( m_EglDpy, attribs, &config, 1, &num_configs)) {
		cout << "Error: couldn't get an EGL visual config" << endl;
		exit(1);
	}

	assert(config);
	assert(num_configs > 0);

	if (!eglGetConfigAttrib(m_EglDpy, config, EGL_NATIVE_VISUAL_ID, &vid)) {
		cout << "Error: eglGetConfigAttrib() failed" << endl;
		exit(1);
	}

	eglBindAPI(EGL_OPENGL_ES_API);

	m_EglCtx = eglCreateContext(m_EglDpy, config, EGL_NO_CONTEXT, ctx_attribs );
	if (!m_EglCtx) {
		cout << "Error:  eglCreateContext failed" << endl;
		exit(1);
	}

	eglQueryContext(m_EglDpy, m_EglCtx, EGL_CONTEXT_CLIENT_VERSION, &val);
	assert(val == 2);
	/* Create EGL Surface and Context for rendering window */
	m_EglSurf = eglCreateWindowSurface(m_EglDpy, config, (EGLNativeWindowType) NULL, NULL);
	if (!m_EglSurf) {
		cout << "Error: eglCreateWindowSurface failed" << endl;
		exit(1);
	}

	// sanity checks
	eglQuerySurface(m_EglDpy, m_EglSurf, EGL_WIDTH, &val);
	eglQuerySurface(m_EglDpy, m_EglSurf, EGL_HEIGHT, &val);
	assert(eglGetConfigAttrib(m_EglDpy, config, EGL_SURFACE_TYPE, &val));
	assert(val & EGL_WINDOW_BIT);
}

void cOpenGL::prepareGraphics()
{
	uint32_t fragmentShaderSize = strlen(fragmentShader) + 20;
	char fragmentShaderStr[fragmentShaderSize] = {0,};

	snprintf (fragmentShaderStr, fragmentShaderSize, fragmentShader, NUMBER_OF_SPOTS, NUMBER_OF_SPOTS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader::processShaderStr(&m_VertexShaderID, vertexShader, GL_VERTEX_SHADER);
	Shader::processShaderStr(&m_FragmentShaderID, fragmentShaderStr, GL_FRAGMENT_SHADER);
	m_ProgramID = glCreateProgram();
	if (!m_ProgramID) 
	{
		cout << "Error: Could not create program" << endl;
		exit(1);
	}

	glAttachShader(m_ProgramID, m_VertexShaderID);
	glAttachShader(m_ProgramID, m_FragmentShaderID);
	glLinkProgram(m_ProgramID);
	glUseProgram(m_ProgramID);

	m_iLocPosition = glGetAttribLocation(m_ProgramID, "a_v4Position");
	if(m_iLocPosition == -1)
	{
		cout << "Warning: Attribute not found: a_v4Position" << endl;
	}

	m_iLocTexCoord = glGetAttribLocation(m_ProgramID, "a_v2TexCoord");
	if(m_iLocTexCoord == -1)
	{
		cout << "Warning: Attribute not found: a_v2TexCoord" << endl;
	}

	m_iLocSampler = glGetUniformLocation(m_ProgramID, "u_s2dTexture1");
	if(m_iLocSampler == -1)
	{
		cout << "Warning: Uniform not found: u_s2dTexture1" << endl;
	}
	else
	{
		glUniform1i(m_iLocSampler, 0);
	}

	m_iLocSpot = glGetUniformLocation(m_ProgramID, "u_v2spot");
	if(m_iLocSpot == -1)
	{
		cout << "Warning: Uniform not found: u_v2spot" << endl;
	}

	m_iLocSpotColor = glGetUniformLocation(m_ProgramID, "u_v4spot_color");
	if(m_iLocSpotColor == -1)
	{
		cout << "Warning: Uniform not found: u_v4spot_color" << endl;
	}

	m_iLocSpotNumber = glGetUniformLocation(m_ProgramID, "u_iNumberOfSpot");
	if(m_iLocSpotNumber == -1)
	{
		cout << "Warning: Uniform not found: u_iNumberOfSpot" << endl;
	}

	glUseProgram(m_ProgramID);

	glEnableVertexAttribArray(m_iLocPosition);
	glVertexAttribPointer(m_iLocPosition, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	if(m_iLocTexCoord != -1)
	{
		glVertexAttribPointer(m_iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, textureCoordinates);
		glEnableVertexAttribArray(m_iLocTexCoord);
	}

	glClearColor(0.0f, 0.0f, 0.2f, 1.0);
	glFinish();

	glGenTextures(eDMABuffer::eDMABuffer_MaxSize, m_TextureID);
}

bool cOpenGL::checkWindow()
{
#ifdef X11_WINDOW
	if(m_Camera->checkWindow() != XCamera::WINDOW_IDLE)
	{
		return false;
	}
#endif
	return true;
}

EGLDisplay cOpenGL::getEGLDisplay()
{
	return m_EglDpy;
}

void cOpenGL::graphicsUpdate(buffer *pDmaBuffer, 
					uint8_t *pImageBuffer, 
					uint32_t width, 
					uint32_t height, 
					vector<sSpot> &s)
{
	uint32_t iter, ptr;
	static GLfloat spotPosition[NUMBER_OF_SPOTS << 1];
	static GLfloat spotColor[NUMBER_OF_SPOTS << 2];
	GLint size = s.size();
	vector<sSpot>::iterator it;
	size = (size < NUMBER_OF_SPOTS) ? size : NUMBER_OF_SPOTS;
	for (it = s.begin(), iter = 0 ; it != s.end(); ++it, iter++)
	{
		ptr = iter << 1;
		spotPosition[ptr++] = it->spotPosition[0];
		spotPosition[ptr] = it->spotPosition[1];
		ptr = iter << 2;
		spotColor[ptr++] = it->spotColor[0];
		spotColor[ptr++] = it->spotColor[1];
		spotColor[ptr++] = it->spotColor[2];
		spotColor[ptr] = it->spotColor[3];
	}
	glUniform2fv(m_iLocSpot, size, spotPosition);
	glUniform4fv(m_iLocSpotColor, size, spotColor);
	glUniform1iv(m_iLocSpotNumber, 1, &size);

#ifdef DMA_BUF_IMPORT
	for(iter = 0, ptr = 1000; iter < eDMABuffer::eDMABuffer_MaxSize; iter++)
	{
		if (pDmaBuffer->buf[iter] == pImageBuffer)
		{
			ptr = iter;
			break;
		}
	}
	glActiveTexture(GL_TEXTURE0 + ptr);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_TextureID[0]);
	update_texture(pDmaBuffer, ptr);
#else 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureID[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, width, height, 0, m_InternalFormat, m_Format, pImageBuffer);
#endif

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glFinish();
	eglSwapBuffers(m_EglDpy, m_EglSurf);
}

void cOpenGL::addTextBox(uint8_t *pImageBuffer,
						uint32_t x, uint32_t y,
						uint32_t width, uint32_t height,
						uint32_t frameWidth, uint32_t frameHeight,
						uint32_t r, uint32_t g,uint32_t b, uint32_t a,
						uint32_t text_r, uint32_t text_g, uint32_t text_b, uint32_t text_a,
						const char *font,
						uint32_t font_size,
						const char *message)
{
	if(x + width > frameWidth || y+height > frameHeight){
		cout << "Rectangle out of surface " << endl;
		return;
	}
	cairo_surface_t *cairo_surface = cairo_image_surface_create_for_data((uint8_t *)pImageBuffer,
									 m_CairoPixelFormat, frameWidth, frameHeight,frameWidth*m_BytesPerPixel);
	cairo_t *cr = cairo_create(cairo_surface);

	cairo_rectangle (cr, x, y, width, height);
	if(m_CairoPixelFormat == CAIRO_FORMAT_ARGB32)
		cairo_set_source_rgba (cr, b, g, r, a);
	else
		cairo_set_source_rgba (cr, r, g, b, a);
	cairo_fill (cr);

	//Target Information
	cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, font_size);

	if(m_CairoPixelFormat == CAIRO_FORMAT_ARGB32)
		cairo_set_source_rgba(cr, text_b, text_g, text_r, text_a);
	else 
		cairo_set_source_rgba(cr, text_r, text_g, text_b, text_a);

	cairo_move_to(cr,x+10,y+40);
	cairo_show_text (cr, message);

	cairo_destroy(cr);
	cairo_surface_destroy(cairo_surface);
}

void cOpenGL::graphicsCleanUp()
{
	glDeleteTextures(eDMABuffer::eDMABuffer_MaxSize, m_TextureID);
	glDeleteProgram(m_ProgramID);

#ifdef X11_WINDOW
	CoreEGL::terminateEGL();
	m_Camera->destroyWindow();
#else
	eglDestroyContext(m_EglDpy, m_EglCtx);
	eglDestroySurface(m_EglDpy, m_EglSurf);
	eglTerminate(m_EglDpy);
#endif
}
