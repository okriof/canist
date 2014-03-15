#include "framehistogram.h"
#include <iostream>
#include <iomanip>

using namespace std;

FrameHistogram::FrameHistogram()
	: totcount(0) 
{ }




void FrameHistogram::add(const Canframe& frame)
{
	++totcount;
	bool found = false;
	const list<unsigned int>::iterator end = idents.end();
	list<unsigned int>::iterator count = counts.begin();

	for (list<unsigned int>::iterator it = idents.begin(); it != end; ++it)
	{
		if (*it == frame.ident)
		{
			found = true;
			++(*count);
		}
		++count;
	}

	if (!found)
	{
		idents.push_back(frame.ident);
		counts.push_back(1);
		oldcounts.push_back(0);
	}
}




void FrameHistogram::print()
{
	cout << "Total: " << totcount << "\n";
	list<unsigned int>::iterator ident = idents.begin();
	list<unsigned int>::iterator count = counts.begin();
	list<unsigned int>::iterator oldcount = oldcounts.begin();
	const unsigned int isize = idents.size();
	for (unsigned int k = 0; k < isize; ++k)
	{
		cout << setw(4) << k << ". "
			<< hex << setw(8) << *ident << " "
			<< dec << setw(7) << *count << " "
			<< dec << setw(5);
		if (*count-*oldcount)
			cout << *count-*oldcount;
		else
			cout << 0;
		cout << "\n";
		*oldcount = *count;
		++ident;
		++count;
		++oldcount;
	}
	cout << endl;
}



