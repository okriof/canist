#ifndef CANBUFFER_H
#define CANBUFFER_H

#include "canframe.h"

#include<thread>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<queue>


class CanBuffer
{
public:
	//CanBuffer(); // can
	CanBuffer(const char* source = ""); // file, or "" can
	~CanBuffer();

	const Canframe& getNextFrame(); // reference to frame valid until next call
	unsigned int getSize();


private:
	const char* const source;
	std::atomic<bool> keepRunning;
	std::thread mthread;
	std::queue<Canframe> frameQueue;
	std::mutex queueMutex;			//protects the frame queue
	std::condition_variable queueCond;      //wait for queue to fill..

	Canframe poppedframe; // last popped frame from queue
	void run();
};



#endif
