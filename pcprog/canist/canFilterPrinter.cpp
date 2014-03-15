#include "canrec.h"
#include "canreccan.h"
#include <iostream>
#include <sstream>
#include <vector>



int main(int argc, char** argv)
{
	bool showBits = false;
	int argid = 1;
	while ((argid < argc) && (argv[argid])[0] == '+')
	{
		switch((argv[argid])[1])
		{
		case 0:
			break;
		case 'b':
			showBits = true;
			break;

		default:
			std::cout << "Unknown: +" << (argv[argid])[1] << std::endl;
			break;
		}
		++argid;
	}
	std::vector<int> filters(9,-1);
	for (int ii = argid-1; ii < std::min(argc-1, 9); ++ii)
	{
		std::istringstream istr(argv[ii+1]);
		int filt;
		istr >> filt;
		filters[ii-argid+1] = filt;
		//std::cout << filt << std::endl;
	}

	//CanrecFile can("../dtest/datadumps/background.dump");
	//CanrecFile can("../dtest/datadumps/rutoruppner.dump");
	//CanrecCan can;
	//CanrecDecfile can("olstexample.dumpdec2");
	CanrecDecfile can("data.dumpdec2");
	unsigned long pkgcount = 0;
	Canframe timerefframe;

	for(;;)
	{	
	
		try
		{
			const Canframe& frame = can.getNextFrame();
			if (pkgcount == 0)
				timerefframe = frame;

			++pkgcount;
			bool display = (frame.ident == filters[0] || filters[0] < 0);
			for(unsigned int ii = 0; ii < 8; ++ii)
				display = display && (frame.data[ii] == filters[ii+1] || filters[ii+1] < 0);
			if (display)
			{
				std::cout << frame.ident << "    \t";
				for(unsigned int ii = 0; ii < 8; ++ii)
				{
					if (showBits)
					{
						for(unsigned char bit = 128; bit > 0; bit/=2)
							std::cout << ( (int)((frame.data[ii] & bit)/bit) ? '1' : '.');
						std::cout << " ";
					}
					else
						std::cout << (int)(frame.data[ii]) << "\t";
				}
				std::cout << pkgcount 
					<< "\t" << frame.timediff(timerefframe) << std::endl;

			}

		}
		catch (const char* ch)
		{
			std::cout << std::endl << "Error: " << ch << std::endl;	
		}
	}
}


