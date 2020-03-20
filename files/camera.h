/***********************************************************************
  @author Kritesh tripathi
  

  Pylon SDK Camera Stream header

 * @file camera.h
 *
 * This is header file for Camera class defining Video source
 * This class is guidelines to implement a camera source
 ***********************************************************************/

#ifndef CAMERA_H_
#define CAMERA_H_

#include <thread> 
#include <vector> 
#include <queue> 

#include "sharedQueue.h"

using namespace std;
class cCamera
{
public:
	// Start getting frames from the camera
	// Provide a queue of allocated buffers to handle the frames
	virtual void start(SharedQueue<uint8_t *> &bufs) = 0;

	// Set Video Buffer if available
	// Buffer array and length of the array
	virtual void setVideoBuffer(uint8_t** bufs, uint8_t max) = 0;

	// Check if camera source is running
	virtual bool isRunning() = 0;

	// Get frame width
	virtual uint32_t getWidth() = 0;

	// Get frame height
	virtual uint32_t getHeight() = 0;

	// Deleting the object should close camera source.
	// Cleans up and releases resources
	virtual ~cCamera() {}
};

#endif // CAMERA_H_
