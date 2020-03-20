/***********************************************************************
  @author Kritesh tripathi
  

  Zero copy header

 * @file Core_egl_zero_copy.h
 *
 * This is  source file for implementing zero copy 
 * rendering using EGL_EXT_image_dma_buf_import extension.
 ***********************************************************************/
#ifndef ZERO_BENCH
#define ZERO_BENCH

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

enum fbdev_pixmap_flags
{
	FBDEV_PIXMAP_DEFAULT = 0,
	FBDEV_PIXMAP_SUPPORTS_UMP = (1 << 0),
	FBDEV_PIXMAP_ALPHA_FORMAT_PRE = (1 << 1),
	FBDEV_PIXMAP_COLORSPACE_sRGB = (1 << 2),
	FBDEV_PIXMAP_EGL_MEMORY = (1 << 3),
	FBDEV_PIXMAP_DMA_BUF = (1 << 4),
};

enum eDMABuffer
{
	// Number of DMA buffers to be used
	eDMABuffer_MaxSize = 5
};

struct fbdev_pixmap {
	unsigned int height;
	unsigned int width;
	unsigned int bytes_per_pixel;
	unsigned char buffer_size;
	unsigned char red_size;
	unsigned char green_size;
	unsigned char blue_size;
	unsigned char alpha_size;
	unsigned char luminance_size;
	fbdev_pixmap_flags flags;
	unsigned short *data;
	unsigned int format;
};

struct buffer {
	int dbuf_fd;
	EGLImageKHR egl_img[eDMABuffer::eDMABuffer_MaxSize];
	void *buf[eDMABuffer::eDMABuffer_MaxSize];
	unsigned int length;
};

extern void init_dma(buffer *buff, EGLDisplay egl_dpy, uint32_t width, uint32_t height,uint32_t bytes_per_pixel);
extern void exit_dma(buffer *buff, EGLDisplay egl_dpy);
extern void update_texture(buffer *buff, uint32_t bufnum);


#endif // ZERO_BENCH
