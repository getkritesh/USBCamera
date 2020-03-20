/***********************************************************************
  @author Kritesh tripathi
  

  Core DRM header

 * @file Core_DRM.h
 *
 * This is  source file for drm buffer handling for Camera Stream.
 ***********************************************************************/
 
#ifndef _NV2DRM_H_
#define _NV2DRM_H_

#include <inttypes.h>

struct drm_buffer
{
	int req_dev_fd;
	uint32_t req_height;
	uint32_t req_width;
	uint32_t req_bpp;       //32
	uint32_t req_fourcc;    //#define OUTPUT_PIX_FMT v4l2_fourcc('Y','U','Y','V')
	uint32_t size;          //(height * pitch + width) * bpp / 4?
	uint32_t pitch;         //!=stride
	uint32_t stride;
	uint32_t gem_handle;
	uint32_t fb_id;
	int32_t dmabuf_fd;
	uint8_t *buffer;
};

int nv2drm_open(int *f_fd, const char *f_node);   /* This helper function opens the DRM device which is given as @node and check for the DRM_CAP_DUMB_BUFFER capability */
int nv2drm_prime_buffer_new(drm_buffer *f_b); /* Helps Initialize and export the buffer */
int nv2drm_prime_buffer_map(drm_buffer *f_b); /* Requests the DRM subsystem to prepare the buffer for memory-mapping and returns a offset that can be used with mmap() */
int nv2drm_prime_buffer_addfb(drm_buffer *f_b); /* Creates framebuffers with drmModeAddFB() and use it for mode-setting */
void nv2drm_prime_buffer_unmap(void *buf, unsigned int size); /* Unmap buffers */
void nv2drm_prime_buffer_free(drm_buffer *f_b); /* Buffer release & cleanup function to be called in end */

#endif /* _NV2DRM_H_ */
