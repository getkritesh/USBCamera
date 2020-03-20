/***********************************************************************
  @author Kritesh tripathi
  

  Camera Utils header

 * @file camutils.h
 *
 * This is utility header file
 ***********************************************************************/

#ifndef CAMUTILS_H_
#define CAMUTILS_H_

#include <vector>
#include <unistd.h>

using std::vector;

class cCamUtils
{
	static unsigned ComputeTickCount();
	static bool get_cpu_times(size_t &idle_time, size_t &total_time);
	static void get_cpu_times(vector<size_t> &times);
public:
	static void ComputeFrameRate();
	static float cpu_usage();
};


#endif // CAMUTILS_H_
