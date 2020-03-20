/***********************************************************************
  @author Kritesh tripathi
  

  Pylon SDK Camera Stream header

 * @file pylonCam.h
 *
 * This is main header file for Pylon Camera as Video source
 ***********************************************************************/

#ifndef PYLONCAM_H_
#define PYLONCAM_H_

#include <thread> 
#include <vector> 
#include <queue> 

#include "sharedQueue.h"
#include "camera.h"

using namespace std;
class cPylonSdkCamera: public cCamera
{
	bool m_running;

	// Start the grabbing of m_countOfImagesToGrab images.
	// The camera device is parameterized with a default configuration which
	// sets up free-running continuous acquisition.
	int32_t m_countOfImagesToGrab;

	// The parameter m_countMaxNumBuffer can be used to control the count of buffers
	// allocated for grabbing. The default value of this parameter is 10.
	uint32_t m_countMaxNumBuffer;
	vector<uint8_t *> *m_pBufs;
	uint8_t** m_pInternalBufs;
	uint8_t m_internalBufPtr, m_internalBufMax;
	SharedQueue<uint8_t *> m_pInternalQueue;
	thread* m_th1;
	thread* m_th2;
	uint32_t m_width;
	uint32_t m_height;

	void getFrames(SharedQueue<uint8_t *> &bufs);
	void updateVideoBuffer(SharedQueue<uint8_t *> &bufs);

public:
	enum eConstants
	{
		PYLON_NUMBER_OF_COLOR_COMPONENTS = 3,
		PYLON_NUMBER_OF_COLOR_COMPONENTS_WITH_ALPHA = 4
	};

	enum eFormat
	{
		PYLON_FORMAT_RGB565,
		PYLON_FORMAT_RGB888,
		PYLON_FORMAT_RGBA8888
	};

	cPylonSdkCamera(vector<uint8_t *> &bufs, int32_t countOfImagesToGrab, eFormat format);
	~cPylonSdkCamera();
	void start(SharedQueue<uint8_t *> &bufs);
	void setVideoBuffer(uint8_t** bufs, uint8_t max);
	bool isRunning();
	uint32_t getWidth();
	uint32_t getHeight();

protected:
	eFormat m_format;
};


#endif // PYLONCAM_H_
