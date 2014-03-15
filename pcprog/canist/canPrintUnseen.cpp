#include "canrec.h"
#include "canreccan.h"
#include <iostream>
#include <set>


int main()
{
	//CanrecFile can("../dtest/datadumps/background.dump");
	//CanrecFile can("../dtest/datadumps/rutoruppner.dump");
	//CanrecCan can;
	CanrecDecfile can("data.dumpdec2");
	std::set<Canframe> frameset;
	std::pair<std::set<Canframe>::iterator, bool> increturn;
	unsigned long pkgcount = 0;

	for(;;)
	{	
	
		try
		{
			Canframe frame;
			frame = can.getNextFrame();
			++pkgcount;
			increturn = frameset.insert(frame);
			if (increturn.second)
			{
				std::cout << frame.ident << "    \t";
				for(unsigned int ii = 0; ii < 8; ++ii)
				{
					std::cout << (int)(frame.data[ii]) << "\t";
					//for(unsigned char bit = 128; bit > 0; bit/=2)
					//	std::cout << (int)((frame.data[ii] & bit)/bit);
					//std::cout << " ";
				}
				std::cout << pkgcount << std::endl;
			}

		}
		catch (const char* ch)
		{
			std::cout << std::endl << "Error: " << ch << std::endl;	
		}
	}
}



bool operator<(const Canframe& r, const Canframe& k)
{
	if (r.ident < k.ident)
		return true;
	else if (r.ident > k.ident)
		return false;
	else if (r.data[0] < k.data[0])
		return true;
	else if (r.data[0] > k.data[0])
		return false;
	else if (r.data[1] < k.data[1])
		return true;
	else if (r.data[1] > k.data[1])
		return false;
	else if (r.data[2] < k.data[2])
		return true;
	else if (r.data[2] > k.data[2])
		return false;
	else if (r.data[3] < k.data[3])
		return true;
	else if (r.data[3] > k.data[3])
		return false;
	else if (r.data[4] < k.data[4])
		return true;
	else if (r.data[4] > k.data[4])
		return false;
	else if (r.data[5] < k.data[5])
		return true;
	else if (r.data[5] > k.data[5])
		return false;
	else if (r.data[6] < k.data[6])
		return true;
	else if (r.data[6] > k.data[6])
		return false;
	else return r.data[7] < k.data[7];
} 


