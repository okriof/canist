#include <iostream>
#include <fstream>
#include <unistd.h>
#include "audiopulse.h"
#include "../canrec.h"

int main()
{
//	CanrecDecfile can("../datadumpsIX/skgTskolan.dumpdec2", false, true);
	CanrecDecfile can("../data.dumpdec2");

	std::ifstream audioin("schn");
	const unsigned long frames = 5676032;

	int16_t* frame = new int16_t[2*frames];
	audioin.read((char*)(void*)frame, frames*4);
	std::cout << "Fileisgood " << audioin.good() << std::endl;

	setSndBufPtr(frame, frames);

	const unsigned int sleeptime = 25000;
	std::cout << "Init" << std::endl;
	spotInitAudio();
	sleep(1);
	spotPlayAudio(2, 44100, frame, 1);
	//std::cout << "Writeable: " << spotWritableFrames() << std::endl;
	sleep(1);

	double speed = 0;
	double bufpos = 0;
	double playspeed = 0;
	bool forward = true;
	for(;;)
	{
		const Canframe& frame = can.getNextFrame();
		if (frame.ident == 5324796)
		{
			speed = (0.25*((frame.data[6] % 2)*256.0 + frame.data[7]));
			if (forward)
			{
				if (speed < 15)
					playspeed = speed/10; //bufpos = setPlaySpeed(speed/10);
				else
				{
					double newspeed = speed;
					while (newspeed > 15) newspeed -= 10;
					playspeed = newspeed/10;
				}
			}
			else
				playspeed = -speed/10; //bufpos = setPlaySpeed(-speed/10);
			bufpos = setPlaySpeed(playspeed);
			std::cout << (forward ? "FWD" : "REV") << "\t " << speed << "\t" << playspeed << "\t" << bufpos*100 << std::endl;
		}
		if (frame.ident == 3227128)
		{
			forward = !(frame.data[4] & 64);
		}
	}//*/


/*	double num = 0;
	while (num >= -6)
	{
		std::cin >> num;
		std::cout << setPlaySpeed(num)*100 << std::endl;
	} //*/
	//std::cout << "Writeables: " << spotWritableFrames() << std::endl;
//	unsigned int fts = frames;
//	while (fts != 0)
//	{
//		unsigned int wr = spotPlayAudio(2, 44100, frame, fts);
//		//std::cout << "Wrote " << wr << std::endl;
//		fts -= wr;
//		frame += wr*2;
//	}
	

	usleep(sleeptime);
	unsigned int framesInBuffer;
//	do 
//	{
//		framesInBuffer = spotFramesInBuffer();
//		std::cout << "inbuffer: " << framesInBuffer << std::endl;
//		usleep(sleeptime);
//	} while (framesInBuffer > 0);

	std::cout << "Unload" << std::endl;
	spotUnloadAudio();
	std::cout << "Done" << std::endl;
	return 0;
}
