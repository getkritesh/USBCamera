/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream Source

 * @file camstream.cpp
 *
 * This is main source file for OpenGL rendering of Camera Stream Source.
 ***********************************************************************/
#include <iostream>
#include <vector>
#include <unistd.h>

// Enable for accessing DMA BUFFER
// #define DMA_BUF_IMPORT

#include "camOpenGL.h"
#include "camera.h"
#include "pylonCam.h"
#include "sharedQueue.h"
#include "Core_egl_zero_copy.h"
#include "camutils.h"

using namespace std;

// Maximum sensor frame dimensions:
static const unsigned int SCENE_WIDTH = 1280;
static const unsigned int SCENE_HEIGHT = 960;

// Default display size:
static const unsigned int DISPLAY_WIDTH = 860;
static const unsigned int DISPLAY_HEIGHT = 640;

// Pixel size:
// 4 Bytes = R G B A 8 8 8 8
// 2 Bytes = R G B 5 6 5
static const unsigned int BYTES_PER_PIXEL = 4;

// 2 Buffers are enough. We use many buffers to show a replay
// More buffers means longer replay
static const unsigned int NUMBER_OF_IMAGE_BUFFERS = 10;

// Font in use
static const char* FAVORITE_FONT = "serif";
static const unsigned int FAVORITE_FONT_SIZE = 20;

static void ParseArguments(int c, char* v[], uint32_t &windowWidth, uint32_t  &windowHeight, uint32_t  &bytesPerPixel)
{
	uint32_t opt, temparg;
	bool flag = false; // Error flag
	while (((opt = getopt(c, v, "w:h:b:")) != -1) && !flag) {
		switch (opt) {
		case 'w':
			temparg = atoi(optarg);
			if (temparg%4 == 0)
				windowWidth = temparg;
			else
				flag = true;
			break;
		case 'h':
			temparg = atoi(optarg);
			if (temparg%4 == 0)
				windowHeight = temparg;
			else
				flag = true;
			break;
		case 'b':
			temparg = atoi(optarg);
			if (temparg == 2 || temparg == 4)
				bytesPerPixel = temparg;
			else
				flag = true;
			break;
		default:
			flag = true;
		}
	}
	if(flag)
	{
		cout << "Usage: " << v[0] << " [-w width -h height -b bytesPerPixel]" << endl;
		cout << "width & height must be multiple of 4. Bytes per pixel must be 2 or 4" << endl;
		exit(0);
	}
}

static void UpdateText(cOpenGL* ptr, uint8_t *pImageBuffer,uint32_t frameWidth, uint32_t frameHeight)
{
	// Sample text boxes
	// If no text is provided, blank rectangles will be drawn
	ptr->addTextBox(pImageBuffer, 200,200,300,60, frameWidth, frameHeight, 1,0,0,1, 1,1,1,1, FAVORITE_FONT, FAVORITE_FONT_SIZE, "Target Information");
	ptr->addTextBox(pImageBuffer, 200,260,300,55, frameWidth, frameHeight, 1,1,1,1, 0,0,0,1, FAVORITE_FONT, FAVORITE_FONT_SIZE, "Conductivity: 42");
	ptr->addTextBox(pImageBuffer, 200,317,300,55, frameWidth, frameHeight, 1,1,1,1, 0,0,0,1, FAVORITE_FONT, FAVORITE_FONT_SIZE, "Non-Ferrous");
	ptr->addTextBox(pImageBuffer, 200,374,300,55, frameWidth, frameHeight, 1,1,1,1, 0,0,0,1, FAVORITE_FONT, FAVORITE_FONT_SIZE, "Depth : 3 in");
}

int main(int argc, char* argv[])
{
	uint32_t allocateSize;
	uint32_t frameWidth = 0, frameHeight = 0, iter, iter2;	
	uint32_t windowWidth = DISPLAY_WIDTH, windowHeight = DISPLAY_HEIGHT, bytesPerPixel = BYTES_PER_PIXEL;
	vector<uint8_t *> imageBufs;
	vector<sSpot> spots;
	SharedQueue<uint8_t *> imageBufCtx, imageBufReplay;
	SharedQueue<uint8_t *> *imageBufPtr, *imageBufPtrBackup;
	uint8_t *pImageBuffer;
	buffer dmaBuffer;
	cCamera *pCam;
	cOpenGL *pOpengl = cOpenGL::getInstance();

	ParseArguments(argc, argv, windowWidth, windowHeight, bytesPerPixel);
	cout << "Using width = " << windowWidth << " height = " << windowHeight << " and bytes per pixel = " << bytesPerPixel << endl;

	// Define a sample spot
	float mover1 = 250.0, mover2 = 250.0;
	sSpot spot1 = {
		.spotPosition = { 0.0, 0.0 },
		.spotColor = { 0.0f, 0.0f, 20.0f, 0.05f }
	};
	spots.push_back(spot1);

	pOpengl->initialize(windowWidth, windowHeight, bytesPerPixel);

#ifdef DMA_BUF_IMPORT
	init_dma(&dmaBuffer, pOpengl->getEGLDisplay(), SCENE_WIDTH, SCENE_HEIGHT, bytesPerPixel);
	allocateSize = SCENE_WIDTH * SCENE_HEIGHT * cPylonSdkCamera::PYLON_NUMBER_OF_COLOR_COMPONENTS;
#else
	allocateSize = SCENE_WIDTH * SCENE_HEIGHT * cPylonSdkCamera::PYLON_NUMBER_OF_COLOR_COMPONENTS_WITH_ALPHA;
#endif

	for (iter = 0; iter < NUMBER_OF_IMAGE_BUFFERS; iter++)
	{
		pImageBuffer = new uint8_t[allocateSize];
		imageBufs.push_back(pImageBuffer);
	}
	pImageBuffer = NULL;
	cout << "Idle state" << endl;
	cCamUtils::ComputeFrameRate();

	pOpengl->prepareGraphics();

	// Provide image buffer banks and get 200 image frames
	// The image buffers will get re-used in cyclic order
	if(bytesPerPixel == 4) {
		pCam = new cPylonSdkCamera(imageBufs, 200, cPylonSdkCamera::PYLON_FORMAT_RGBA8888);
	}else if(bytesPerPixel == 2){
		pCam = new cPylonSdkCamera(imageBufs, 200, cPylonSdkCamera::PYLON_FORMAT_RGB565);
	}

#ifdef DMA_BUF_IMPORT
	pCam->setVideoBuffer((uint8_t**)dmaBuffer.buf, eDMABuffer::eDMABuffer_MaxSize);
#endif

	pCam->start(imageBufCtx);

	while (pCam->isRunning())
	{
		if (!imageBufCtx.empty())
		{
			if (frameWidth == 0 || frameHeight == 0)
			{
				frameWidth = pCam->getWidth();
				frameHeight = pCam->getHeight();
			}
			pImageBuffer = imageBufCtx.front();
			imageBufCtx.pop_front();
			imageBufReplay.push_back(pImageBuffer);
			if(!pOpengl->checkWindow())
			{
				cout << "Exiting Camera capture" << endl;
				break;
			}
			cCamUtils::ComputeFrameRate();

			// Move the spot
			spot1.spotPosition[0] = (mover1+=4)/frameWidth;
			spot1.spotPosition[1] = (mover2+=2)/frameHeight;
			spots.pop_back();
			spots.push_back(spot1);

			pOpengl->graphicsUpdate(&dmaBuffer, pImageBuffer, frameWidth, frameHeight, spots);
		}
		else
		{
			usleep(1);
		}
	}

	// clean Camera object and resources
	delete pCam;

	for (iter2 = 0 ; iter2 < 2; iter2++)
	{
		spots.clear();
		spot1.spotColor[1] = 10.0f;
		spot1.spotColor[2] = 0.0f;
		spot1.spotPosition[1] = (200.0f)/frameHeight;
		mover1 = 100.0f;
		cout << "Idle state" << endl;
		sleep(1);
		cCamUtils::ComputeFrameRate();

		for (iter = 0 ; iter < 5; iter++)
		{
			if(iter2 == 0)
				cout << "Now Replay frames with spots:\t"<< spots.size() << endl;
			else
				cout << "Now Replay frames with text and spots:\t"<< spots.size() << endl;

			imageBufPtr = (imageBufCtx.empty()) ? &imageBufReplay : &imageBufCtx;
			imageBufPtrBackup = (imageBufCtx.empty()) ? &imageBufCtx : &imageBufReplay;
			while (!imageBufPtr->empty())
			{
				pImageBuffer = imageBufPtr->front();
				imageBufPtr->pop_front();
				imageBufPtrBackup->push_back(pImageBuffer);
				cCamUtils::ComputeFrameRate();
				if(iter2 == 1)
					UpdateText(pOpengl, pImageBuffer, frameWidth, frameHeight);
				pOpengl->graphicsUpdate(&dmaBuffer, pImageBuffer, frameWidth, frameHeight, spots);
			}
			cout << endl;
			// Add 1 more spot
			spot1.spotPosition[0] = mover1/frameWidth;
			spot1.spotPosition[1] = mover1/frameHeight;
			mover1 += 125.0f;
			spots.push_back(spot1);
		}
	}

#ifdef DMA_BUF_IMPORT
	exit_dma(&dmaBuffer, pOpengl->getEGLDisplay());
#endif
	while (!imageBufs.empty())
	{
		pImageBuffer = imageBufs.back();
		delete [] pImageBuffer;
		imageBufs.pop_back();
	}
	pOpengl->graphicsCleanUp();

	return 0;
}
