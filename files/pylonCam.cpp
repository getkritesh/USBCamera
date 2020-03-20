/***********************************************************************
  @author Kritesh tripathi
  

  Pylon SDK Camera Stream source

 * @file pylonCam.cpp
 *
 * This is main header file for Pylon Camera as Video source
 ***********************************************************************/

#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>

#include <pylon/PylonIncludes.h>
#include "pylonCam.h"
#include "sharedQueue.h"

using namespace std;
using namespace Pylon;


class MyBufferFactory : public IBufferFactory
{
protected:
	unsigned long m_lastBufferContext;
private:
	vector<uint8_t *> *ptrBufs;
public:
	MyBufferFactory()
		: m_lastBufferContext(0)
	{
	}
	virtual ~MyBufferFactory()
	{
	}

	// Will be called when the Instant Camera object needs to allocate a buffer.
	// Return the buffer and context data in the output parameters.
	virtual void AllocateBuffer( size_t bufferSize, void** pCreatedBuffer, intptr_t& bufferContext)
	{
		try
		{
			// Provide buffer for pixel data.
			*pCreatedBuffer = reinterpret_cast <void *>(ptrBufs->at(m_lastBufferContext));
			// The context information is never changed by the Instant Camera and can be used
			// by the buffer factory to manage the buffers.
			// The context information can be retrieved from a grab result by calling
			// ptrGrabResult->GetBufferContext();
			bufferContext = ++m_lastBufferContext;
		}
		catch (const std::exception&)
		{
			// Rethrow exception.
			// AllocateBuffer can also just return with *pCreatedBuffer = NULL to indicate
			// that no buffer is available at the moment.
			throw;
		}
	}

	// Frees a previously allocated buffer.
	virtual void FreeBuffer( void* pCreatedBuffer, intptr_t bufferContext)
	{
		// Do nothing because this buffer was never allocated by this class
	}
	// Destroys the buffer factory.
	// This will be used when you pass the ownership of the buffer factory instance to pylon
	// by defining Cleanup_Delete. pylon will call this function to destroy the instance
	// of the buffer factory. If you don't pass the ownership to pylon (Cleanup_None)
	// this method will be ignored.
	virtual void DestroyBufferFactory()
	{
		delete this;
	}

	virtual void SetMemory(vector<uint8_t *> *pBufs)
	{
		ptrBufs = pBufs;
	}
};

cPylonSdkCamera::cPylonSdkCamera(vector<uint8_t *> &bufs, int32_t countOfImagesToGrab, eFormat format):
	m_countOfImagesToGrab(countOfImagesToGrab),
	m_running(true),
	m_width(0),
	m_height(0),
	m_th1(NULL),
	m_th2(NULL),
	m_pInternalBufs(NULL),
	m_internalBufPtr(0),
	m_internalBufMax(0),
	m_format(format)
{
	m_pBufs = &bufs;
	m_countMaxNumBuffer = m_pBufs->size();
}

cPylonSdkCamera::~cPylonSdkCamera()
{
	m_running = false;
	if (m_th1 != NULL)
	{
		m_th1->join();
	}
	if (m_th2 != NULL)
	{
		m_th2->join();
	}
	delete m_th1;
	delete m_th2;
	// Releases all pylon resources. 
	cout << "Clearing Pylon" << endl;
	PylonTerminate();  
}

bool cPylonSdkCamera::isRunning()
{
	return m_running;
}

void cPylonSdkCamera::start(SharedQueue<uint8_t *> &bufs)
{
	if(m_pInternalBufs)
	{
		m_th2 = new thread(&cPylonSdkCamera::updateVideoBuffer, this, std::ref(bufs));
	}
	m_th1 = new thread(&cPylonSdkCamera::getFrames, this, std::ref(bufs));
}

void cPylonSdkCamera::setVideoBuffer(uint8_t** bufs, uint8_t max)
{
	m_internalBufMax = max;
	m_pInternalBufs = bufs;
}

uint32_t cPylonSdkCamera::getWidth()
{
	return m_width;
}

uint32_t cPylonSdkCamera::getHeight()
{
	return m_height;
}

void cPylonSdkCamera::updateVideoBuffer(SharedQueue<uint8_t *> &bufs)
{
	uint8_t *pImageBuffer = NULL;
	int iter, count;
	uint8_t* rgbdst;
	uint8_t* rgb;

	while(m_running)
	{
		if (m_pInternalQueue.empty())
		{
			usleep(1);
		}
		else
		{
			pImageBuffer = m_pInternalQueue.front();
			m_pInternalQueue.pop_front();
			if (m_format == cPylonSdkCamera::PYLON_FORMAT_RGBA8888)
			{
				// Convert RGB888 to RGBA8888
				rgb = pImageBuffer;
				rgbdst = m_pInternalBufs[m_internalBufPtr];
				count = m_width * m_height;
				for(iter = count; --iter; rgbdst+=4, rgb+=3) {
					*(uint32_t*)(void*)rgbdst = *(const uint32_t*)(const void*)rgb;
				}
				for(iter = 0; iter < 3; ++iter)
				{
					rgbdst[iter] = rgb[iter];
				}
			}
			else
			{
				if (m_format == cPylonSdkCamera::PYLON_FORMAT_RGB565)
				{
					// Convert RGB888 to RGB565
					rgb = pImageBuffer;
					rgbdst = m_pInternalBufs[m_internalBufPtr];
					count = m_width * m_height;
					for(iter = count; --iter; rgbdst+=2, rgb+=3)
					{
						*(uint16_t*)rgbdst = ((rgb[0] & 0xF8) << 8) | ((rgb[1] & 0xFC) << 3) | (rgb[2] >> 3);
					}
				}
				else
				{
					// Provide RGB888 without conversion
					memcpy(m_pInternalBufs[m_internalBufPtr], pImageBuffer, m_width * m_height * 3);
				}
			}
			bufs.push_back(m_pInternalBufs[m_internalBufPtr]);
			m_internalBufPtr++;
			if (m_internalBufPtr >= m_internalBufMax)
			{
				m_internalBufPtr = 0;
			}
		}
	}
}

void cPylonSdkCamera::getFrames(SharedQueue<uint8_t *> &bufs)
{
	uint8_t *pImageBuffer = 0;
	uint8_t *rgbdst, *rgb;
	uint32_t sizeRGB, sizeRGBA, sizeImage;
	int iter;

	// Before using any pylon methods, the pylon runtime must be initialized. 
	PylonInitialize();
	try
	{
		MyBufferFactory myFactory;
		myFactory.SetMemory(m_pBufs);
		// Create an instant camera object with the camera device found first.
		CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());

		// Print the model name of the camera.
		cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;
		camera.SetBufferFactory(&myFactory, Cleanup_None);
		camera.MaxNumBuffer = m_countMaxNumBuffer;
		camera.StartGrabbing(m_countOfImagesToGrab);

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;

		while (camera.IsGrabbing() && m_running)
		{
			// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
			camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{
				if (m_width == 0 || m_height == 0)
				{
					m_width = ptrGrabResult->GetWidth();
					m_height = ptrGrabResult->GetHeight();
					sizeImage = m_width * m_height;
					sizeRGB = (sizeImage * 3) - 3;
					sizeRGBA = (sizeImage * 4) - 4;
				}
				pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
				if(m_pInternalBufs)
				{
					m_pInternalQueue.push_back(pImageBuffer);
				}
				else
				{
					if (m_format == cPylonSdkCamera::PYLON_FORMAT_RGBA8888)
					{
						// Convert RGB to RGBA (in-place)
						rgbdst = rgb = pImageBuffer;
						rgbdst += sizeRGBA;
						rgb += sizeRGB;
						for(iter= sizeImage; --iter; rgbdst-=4, rgb-=3)
						{
							*(uint32_t*)rgbdst = *(uint32_t*)rgb;
						}
					}
					if (m_format == cPylonSdkCamera::PYLON_FORMAT_RGB565)
					{
						// Convert RGB888 to RGB565 (in-place)
						rgbdst = rgb = pImageBuffer;
						for(iter= sizeImage; --iter; rgbdst+=2, rgb+=3)
						{
							*(uint16_t*)rgbdst = ((rgb[0] & 0xF8) << 8) | ((rgb[1] & 0xFC) << 3) | (rgb[2] >> 3);
						}
					}
					// Send frame
					bufs.push_back(pImageBuffer);
				}
			}
			else
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
			if(--m_countOfImagesToGrab <= 0)
			{
				m_running = false;
			}
		}
	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl << e.GetDescription() << endl;
		m_running = false;
	}
}
