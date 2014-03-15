#include "canbuffer.h"
#include "canrec.h"
#include "canreccan.h"
#include "framehistogram.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;


int main()
{
	bool storeData = true;
	std::ofstream framelogstream("data.dumpdec2");
	std::cout << sizeof(unsigned int) << std::endl;
try{
	//CanrecFile can("background_key1.dump");
	//CanrecFile can("../dtest/datadumps/background.dump");
	//CanrecFile can("../dtest/datadumpsIII/olstorpmm.dump");
	//CanrecCan can;

	//CanBuffer can("../dtest/datadumpsIII/olstorpmm.dump");
	CanBuffer can;

	FrameHistogram hist;


	unsigned char lastBtn = 0;
	Canframe frame;
	for(;;) // SWM button test
	{
		try {
			frame = can.getNextFrame();                    // usleep(500000);

			if (storeData)
			{
				frame.write(framelogstream);
				framelogstream.flush();
			}

			// SWM module (one of two messages) and button change
			if (frame.ident == 2097254 && frame.data[7] != lastBtn)
			{
				lastBtn = frame.data[7];
				cout << (int)lastBtn << ", buffersize " << can.getSize() << endl;

				if (lastBtn == 125) // next
					system("dbus-send --print-reply --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Next");

				if (lastBtn == 126) // previous
					system("dbus-send --print-reply --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Previous");
	
				if (lastBtn == 121 || lastBtn == 122) //prev/next + minus (pause)
					system("dbus-send --print-reply --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Pause");
				
				if (lastBtn == 117 || lastBtn == 118) //prev/next + plus (playpause)
					system("dbus-send --print-reply --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.PlayPause");
			}

		}
		catch (const char* msg)
		{ 
			cout << "Error: " << msg << endl; 
		}
	}
			


//	tkn = '1';
//	while (tkn != 'q')
	

	//for(unsigned int big = 0; big < 40; ++big)
	for(;;)
	{	
		//cout << big << endl;
		for (unsigned int k = 0; k < 1000; ++k)
		{
			//cout << k << endl;
			try
			{
				Canframe frame;
				frame = can.getNextFrame();
				//cout << frame.ident << endl;
				hist.add(frame);
				//cout << ".";

				//cout << can.getNextFrame().ident << ", " << endl;
				//hist.add(can.getNextFrame());
			}
			catch (const char* ch)
			{
				cout << endl << "Error: " << ch << endl;	
			}
		}

		hist.print();
	}

	return 0;
}
catch(const char* ch)
{
cout << endl << "TERMINATIONERROR: " << ch << endl;
}
}

