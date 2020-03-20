/***********************************************************************
  @author Kritesh tripathi
  

  Core DRM Source

 * @file Core_DRM.cpp
 *
 * This is  source file for drm buffer handling for Camera Stream.
 ***********************************************************************/

#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <cstring>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "Core_DRM.h"

using std::cout;
using std::endl;

/************************************************************************************************************************************/
/**** This small helper function opens the DRM device which is given as @node. The new fd is stored in @out on success **************/ 
/**** On failure, a negative error code is returned. After opening the file, we also check for the DRM_CAP_DUMB_BUFFER capability ***/
/**** If the driver supports this capability, we can create simple memory-mapped buffers without any driver-dependent code **********/
/************************************************************************************************************************************/
int nv2drm_open(int *f_fd, const char *f_node)
{
	int fd, ret;
	uint64_t has_dumb;

	if(!f_node)
	{
		f_node = "/dev/dri/card0";
	}
	fd = open(f_node, O_RDWR | O_CLOEXEC);
	if(fd < 0)
	{
		ret = -errno;
		cout << "Error: cannot open " << f_node << endl;
		return ret;
	}

	if(drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb)
	{
		cout << "Error: drm device " << f_node << " does not support dumb buffers" << endl;
		close(fd);
		return -EOPNOTSUPP;
	}

	*f_fd = fd;
	return 0;
}

/************************************************************************************************************************************/
/******************************* Initializing and exporting the buffer **************************************************************/
/****** By using the framebuffer handle (f_b->gem_handle) it’s possible to acquire its dma-buf file descriptor **********************/
/****** (prime.fd) that will be shared later with the other process, for render *****************************************************/

int nv2drm_prime_buffer_new(drm_buffer *f_b)
{
	drm_mode_create_dumb creq;
	drm_mode_destroy_dumb dreq;
	int ret;

	/* create dumb buffer */
	memset(&creq, 0, sizeof(creq));
	creq.width = f_b->req_height;
	creq.height = f_b->req_width;
	creq.bpp = f_b->req_bpp;
	ret = drmIoctl(f_b->req_dev_fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
	if(ret < 0)
	{
		cout << "Error: cannot create dumb buffer " << errno << endl;
		return -errno;
	}

	f_b->pitch = creq.pitch;
	f_b->size = creq.size;
	f_b->gem_handle = creq.handle;
	f_b->stride = f_b->req_width * f_b->req_bpp / 8;

	drm_prime_handle prime;
	memset(&prime, 0, sizeof prime);
	prime.handle = f_b->gem_handle;
	/* Export gem object  to a FD */
	ret = drmIoctl(f_b->req_dev_fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
	if(ret)
	{
		cout << "Error: PRIME_HANDLE_TO_FD failed " << errno << endl;
		ret = -errno;
		goto err_destroy_gem;
	}
	f_b->dmabuf_fd = prime.fd;

	return 0;

	err_destroy_gem:
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = f_b->gem_handle;
	if(drmIoctl(f_b->req_dev_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq))
	{
		cout << "Error: cannot destroy dumb buffer " << errno << endl;
	}
	return ret;
}


/************************************************************************************************************************************/
/**************** To access the buffer, you first need to retrieve the offset of the buffer **************************************/
/**************** The DRM_IOCTL_MODE_MAP_DUMB ioctl requests the DRM subsystem to prepare the buffer for memory-mapping *************/
/**************** and returns a offset that can be used with mmap() *****************************************************************/
/************************************************************************************************************************************/

int nv2drm_prime_buffer_map(drm_buffer *f_b)
{
	drm_mode_destroy_dumb dreq;
	drm_mode_map_dumb mreq;
	int ret;

	/* prepare buffer for memory mapping */
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = f_b->gem_handle;
	ret = drmIoctl(f_b->req_dev_fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if(ret)
	{
		cout << "Error: cannot map dumb buffer " << errno << endl;
		ret = -errno;
		goto err_destroy_prime;
	}

	/* perform actual memory mapping */
	f_b->buffer = (uint8_t *)mmap(0, f_b->size, PROT_READ | PROT_WRITE, MAP_SHARED, f_b->req_dev_fd, mreq.offset);
	if(f_b->buffer == MAP_FAILED)
	{
		cout << "Error: cannot mmap dumb buffer " << errno << endl;
		ret = -errno;
		goto err_destroy_prime;
	}

	return 0;

	err_destroy_prime:
	close(f_b->dmabuf_fd);

	err_destroy_gem:
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = f_b->gem_handle;
	if(drmIoctl(f_b->req_dev_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq))
	{
		cout << "Error: cannot destroy dumb buffer " << errno << endl;
	}
	return ret;
}


/************************************************************************************************************************************/
/**************** The DRM_IOCTL_MODE_CREATE_DUMB ioctl can be used to create a dumb buffer. *****************************************/
/**************** The kernel will return a 32bit handle that can be used to manage the buffer with the DRM API. *********************/
/**************** You can create framebuffers with drmModeAddFB() and use it for mode-setting and scanout. **************************/
/************************************************************************************************************************************/

int nv2drm_prime_buffer_addfb(drm_buffer *f_b)
{
	drm_mode_destroy_dumb dreq;
	int ret;

	uint32_t bo_handles[4] = { f_b->gem_handle };
	uint32_t pitches[4] = { f_b->stride };
	uint32_t offsets[4] = { 0 };

	/* request the creation of frame buffers */
	ret = drmModeAddFB2(f_b->req_dev_fd, f_b->req_width, f_b->req_height, f_b->req_fourcc, bo_handles,
		pitches, offsets, &f_b->fb_id, 0);
	if(ret)
	{
		cout << "Error: drmModeAddFB2 failed " << errno << endl;
		ret = -errno;
		goto err_destroy_prime;
	}

	return 0;

	err_destroy_prime:
	close(f_b->dmabuf_fd);

	err_destroy_gem:
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = f_b->gem_handle;
	if(drmIoctl(f_b->req_dev_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq))
	{
		cout << "Error: cannot destroy dumb buffer " << errno << endl;
	}
	return ret;
}


/************************************************************************************************************************************/
/**************************  Release the buffers , Cleanup function  ****************************************************************/
/************************************************************************************************************************************/
void nv2drm_prime_buffer_unmap(void *buf, unsigned int size)
{
	/* unmap buffer */
	if(buf)
	{
		munmap(buf, size);
	}
}

void nv2drm_prime_buffer_free(drm_buffer *f_b)
{
	drm_mode_destroy_dumb dreq;

	/* delete framebuffer */
	if(f_b->fb_id)
	{
		drmModeRmFB(f_b->req_dev_fd, f_b->fb_id);
	}

	/* delete dumb buffer */
	if(f_b->dmabuf_fd)
	{
		close(f_b->dmabuf_fd);
	}

	if(f_b->gem_handle)
	{
		memset(&dreq, 0, sizeof(dreq));
		dreq.handle = f_b->gem_handle;
		drmIoctl(f_b->req_dev_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
	}
}


