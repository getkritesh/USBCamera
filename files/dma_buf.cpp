/***********************************************************************
  @author Kritesh tripathi
  

  DRM buffer source

 * @file dma_buf.cpp
 *
 * This is  source file containing init and render functions for 
 * zero copy rendering using EGL_EXT_image_dma_buf_import extension.
 ***********************************************************************/
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <drm/drm_fourcc.h>
#include "Core_egl_zero_copy.h"
#include "Core_DRM.h"

using std::cout;
using std::endl;

static drm_buffer drmBuffer;

/************************************************************************************************************************************/
/***************  Initialize and Map the DRM buffers & create EGL Image *************************************************************/
/************************************************************************************************************************************/
void init_dma(buffer *buff, EGLDisplay egl_dpy, uint32_t width, uint32_t height,uint32_t bytes_per_pixel)
{

	int ret, iter;
	EGLint bpp = (bytes_per_pixel == 4)?DRM_FORMAT_RGBA8888:DRM_FORMAT_RGB565;

	//init drm buffers
	ret= nv2drm_open(&drmBuffer.req_dev_fd, 0);
	if (ret < 0) {
		cout << __FUNCTION__ << __LINE__ << ret << endl;
	}

	drmBuffer.req_width = width;
	drmBuffer.req_height = height;
	drmBuffer.req_bpp = bytes_per_pixel * 8;

	for(iter = 0; iter < eDMABuffer::eDMABuffer_MaxSize; iter++)
	{
		//allocate drm buffers
		ret = nv2drm_prime_buffer_new(&drmBuffer);
		if (ret < 0) {
			cout << __FUNCTION__ << __LINE__ << ret << endl;
		}

		//map drm buffers
		ret = nv2drm_prime_buffer_map(&drmBuffer);
		if (ret < 0) {
			cout << __FUNCTION__ << __LINE__ << ret << endl;
		}

		buff->dbuf_fd = drmBuffer.dmabuf_fd;
		buff->buf[iter] = drmBuffer.buffer;
		buff->length = drmBuffer.size;

		EGLint egl_img_attr[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
		EGL_DMA_BUF_PLANE0_FD_EXT, buff->dbuf_fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint)(width * bytes_per_pixel),
		EGL_WIDTH, (EGLint)width,
		EGL_HEIGHT, (EGLint)height,
		EGL_LINUX_DRM_FOURCC_EXT, bpp,
		EGL_NONE, EGL_NONE };

		glActiveTexture(GL_TEXTURE0 + iter) ;
		buff->egl_img[iter] = eglCreateImageKHR(egl_dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)0, egl_img_attr);
	}
}

/************************************************************************************************************************************/
/************************  Update the EGL Image texture with new buffer  ************************************************************/
/************************************************************************************************************************************/

void update_texture(buffer *buff, uint32_t bufnum)
{
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, buff->egl_img[bufnum]);
}

void exit_dma(buffer *buff, EGLDisplay egl_dpy)
{
	int iter, ret;
	for (iter = 0; iter < eDMABuffer::eDMABuffer_MaxSize; iter++)
	{
		nv2drm_prime_buffer_unmap(buff->buf[iter], buff->length);
		eglDestroyImageKHR(egl_dpy, buff->egl_img[iter]);
	}
	nv2drm_prime_buffer_free(&drmBuffer);
	ret = close(drmBuffer.req_dev_fd);
	if (ret < 0) {
		cout << __FUNCTION__ << __LINE__ << ret << endl;
	}
}
