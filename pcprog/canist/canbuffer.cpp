#include <iostream>
#include "canbuffer.h"
#include "canrec.h"
#include "canreccan.h"

//CanBuffer::CanBuffer()
//	: source(""), keepRunning(true), mthread(&CanBuffer::run, this)
//	{}

CanBuffer::CanBuffer(const char* source)
	: source(source) ,keepRunning(true), mthread(&CanBuffer::run, this)
	{}


CanBuffer::~CanBuffer()
{
	keepRunning=false;
	mthread.join();
}


const Canframe& CanBuffer::getNextFrame()
{
	{
		std::unique_lock<std::mutex> queueLock(queueMutex);
		while (frameQueue.empty())
			queueCond.wait(queueLock);
		
		poppedframe = frameQueue.front();
		frameQueue.pop();
	}
	
	return poppedframe;
}
// need semaphores to wait for new frame..



unsigned int CanBuffer::getSize()
{
	unsigned int size;
	{
		std::lock_guard<std::mutex> queueLock(queueMutex);
		size = frameQueue.size();
	}
	return size;
}



void CanBuffer::run()
{
	Canrec* can;
	if (source[0] == 0) // empty string, read from real can bus
	{
		std::cout << "Connecting to bus" << std::endl;
		can = new CanrecCan();
	}
	else
	{
		std::cout << "Opening file: " << source << std::endl;
		can = new CanrecFile(source);
	}

	while(keepRunning)
	{

		try
		{
			// wait for and get reference to next frame
			const Canframe& nextFrame = can->getNextFrame(); 
			
			{ // insert in queue (mutex-protected)
				std::lock_guard<std::mutex> queueLock(queueMutex);
				frameQueue.push(nextFrame);
				queueCond.notify_all(); // notify if waiting for frames.
			}
		}
		catch(const char* msg)
		{
			std::cout << "Error: " << msg << std::endl;
			// TODO: fix exceptiopns (frame reading exception)
		}
		catch(CanFileReadError& e)
		{	// TODO: unrecoverable exception
			std::cout << "Unable to read can input file" << std::endl;
			keepRunning = false;
		}
	}
}



