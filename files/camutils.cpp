/***********************************************************************
  @author Kritesh tripathi
  

  Camera Utils source

 * @file camutils.cpp
 *
 * This is utility source file
 ***********************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <unistd.h>
#include <sys/time.h>
#include "camutils.h"

using namespace std;

void cCamUtils::get_cpu_times(vector<size_t> &times)
{
	ifstream proc_stat("/proc/stat");
	proc_stat.ignore(5, ' '); // Skip the 'cpu' prefix.
	for (size_t time; proc_stat >> time; times.push_back(time));
}
 
bool cCamUtils::get_cpu_times(size_t &idle_time, size_t &total_time)
{
	vector<size_t> cpu_times;
	get_cpu_times(cpu_times);
	if (cpu_times.size() < 4)
		return false;
	idle_time = cpu_times[3];
	total_time = accumulate(cpu_times.begin(), cpu_times.end(), 0);
	return true;
}

float cCamUtils::cpu_usage()
{
	// Based on /proc/stat CPU time
	static size_t previous_idle_time = 0, previous_total_time = 0;
	size_t idle_time, total_time;
	get_cpu_times(idle_time, total_time);
	const float idle_time_delta = idle_time - previous_idle_time;
	const float total_time_delta = total_time - previous_total_time;
	const float utilization = 100.0 * (1.0 - idle_time_delta / total_time_delta);
	previous_idle_time = idle_time;
	previous_total_time = total_time;
	return utilization;
}

unsigned cCamUtils::ComputeTickCount()
{
	timeval tv;
	if (gettimeofday (&tv, NULL) != 0)
		return 0;
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void cCamUtils::ComputeFrameRate()
{
	static float framesPerSecond = 0.0f;
	static float lastTime = 0.0f;
	float currentTime = ComputeTickCount () * 0.001f;
	++framesPerSecond;
	if (currentTime - lastTime > 1.0f)
	{
		lastTime = currentTime;
		cout << "Frames Per Second & CPU usage: \t" << framesPerSecond << "\t" << cpu_usage() <<"\t%"<< endl;
		framesPerSecond = 0;
	}
}

