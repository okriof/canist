#ifndef FRAMEHISTOGRAM_H
#define FRAMEHISTOGRAM_H

#include <vector>
#include <list>
#include "canframe.h"

class FrameHistogram
{
public:
	FrameHistogram();

	void add(const Canframe& frame);
	void print();
		

private:
	std::list<unsigned int> idents;
	std::list<unsigned int> counts;
	std::list<unsigned int> oldcounts;
	unsigned int totcount;
};



#endif
