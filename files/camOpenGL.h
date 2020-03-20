/***********************************************************************
  @author Kritesh tripathi
  

  Open GL handling header

 * @file camOpenGL.h
 *
 * This is header file for OpenGL rendering
 ***********************************************************************/

#ifndef CAMOPENGL_H_
#define CAMOPENGL_H_

#include <vector>

// Enable for accessing X11
// #define X11_WINDOW

// Include files to use openGL
#include <GLES2/gl2.h>
#include <cairo/cairo-gl.h>

#ifdef X11_WINDOW
#include "XLinuxCamera.h"
#include "XEGLIntf.h"
#endif

#include "Core_egl_zero_copy.h"

using std::vector;

struct sSpot
{
	GLfloat spotPosition[2];
	GLfloat spotColor[4];
};

class cOpenGL
{
private:
	/* Here will be the instance stored. */
	static cOpenGL* m_Instance;
	EGLSurface m_EglSurf;
	EGLContext m_EglCtx;
	EGLDisplay m_EglDpy;
	GLuint m_TextureID[eDMABuffer::eDMABuffer_MaxSize];
	GLuint m_VertexShaderID;
	GLuint m_FragmentShaderID;
	GLuint m_ProgramID;
	GLint m_InternalFormat;
	GLint m_iLocPosition;
	GLint m_iLocTexCoord;
	GLint m_iLocSampler;
	GLint m_iLocSpot;
	GLint m_iLocSpotColor;
	GLint m_iLocSpotNumber;
	GLenum m_Format;
	cairo_format_t  m_CairoPixelFormat;
	uint32_t m_WindowWidth;
	uint32_t m_WindowHeight;
	uint32_t m_BytesPerPixel;
#ifdef X11_WINDOW
	XCamera *m_Camera;
#endif

	/* Private constructor to prevent instancing. */
	cOpenGL();
	void makeFbWindow();

public:
	/* Static access method. */
	static cOpenGL* getInstance();
	void initialize(uint32_t windowWidth, uint32_t windowHeight, uint32_t bytesPerPixel);
	void prepareGraphics();
	bool checkWindow();
	EGLDisplay getEGLDisplay();
	void graphicsUpdate(buffer *pDmaBuffer, 
						uint8_t *pImageBuffer, 
						uint32_t width, 
						uint32_t height, 
						vector<sSpot> &s);
	void graphicsCleanUp();

	/*
	* pImageBuffer : frame image
	* x, y : starting position
	* width , height : rectangle width and height
	* frameWidth, frameHeight : frame width and height
	* r, g, b : rectangle fill color
	* text_r, text_g, text_b : font color
	* font, font_size , message : size of font and message to display on rectangle
	*/
	void addTextBox(uint8_t *pImageBuffer,
						uint32_t x, uint32_t y,
						uint32_t width, uint32_t height,
						uint32_t frameWidth, uint32_t frameHeight,
						uint32_t r, uint32_t g,uint32_t b, uint32_t a,
						uint32_t text_r, uint32_t text_g, uint32_t text_b, uint32_t text_a,
						const char *font,
						uint32_t font_size,
						const char *message);
};


#endif // CAMOPENGL_H_
