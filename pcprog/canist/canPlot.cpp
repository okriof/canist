#include "opencvplot/opencvplot.h"
#include "canrec.h"
#include <cmath>

// x = -4:4;
// sigma = 1;
// ddfilt = exp(.5 * -x.^2/sigma^2)*1/(sqrt(2*pi)*sigma) .* (x/(sigma^2));
// fuelcon = conv2(dsel/5000,ddfilt.','same')*1000/115*3600;

int main()
{
	const double pi =  3.141592653589793;

	const int filterpts = 81; //41;
	unsigned char fuelbuf[filterpts];
	double         fuelreq[filterpts];
	double         derivfilt[filterpts];
	double         speed = 0;

	for (int i = 0; i < filterpts; ++i)
	{
		fuelbuf[i]=0;
		fuelreq[i]=0;
		const double sigma = 10; //6;
		const double x = i-(filterpts-1)/2;
		derivfilt[i] = exp(.5*(-x*x)/(sigma*sigma))*1/(sqrt(2*pi)*sigma) 
				* x/(sigma*sigma)*(1000.0/115.0)*(3600.0/5000.0); // liter/h
		std::cout << derivfilt[i] << std::endl;
	}

	//OpencvPlot speedplot("Fart", PlotSettings(120,0,20), 3);
	//OpencvPlot unkn("Fråga", PlotSettings(256,0,32), 0);
	//OpencvPlot revs("RPM", PlotSettings(6000,0,1000), 0);
	//OpencvPlot lph("Liter per timme", PlotSettings(10,0,1), 0);
	//OpencvPlot lpkm("Liter per 100 km", PlotSettings(15,0,1), 0);
	OpencvPlot tank6("Tank D6", PlotSettings(80,0,10), 0);
	OpencvPlot tank7("Tank D7", PlotSettings(80,0,10), 3);

	//CanrecCan can;
	//CanrecDecfile can("olstexample.dumpdec2");
	//CanrecDecfile can("datadumpsVIII/skgskol_schntest.dumpdec2", false, true);
	//CanrecDecfile can("data.dumpdec2", false, true);
	CanrecDecfile can("data.dumpdec2");

	for(;;)
	{	
	
		try
		{
			const Canframe& frame = can.getNextFrame();
			//if (pkgcount == 0)
			//	timerefframe = frame;
			// TODO: real timescale..

			if (frame.ident == 5324796) // speed
			{
				speed = 0.25*((frame.data[6] % 2)*256.0 + frame.data[7]);
				//speedplot.update(speed);
			}
	
			if (frame.ident == 25165896) // fuel consumption
			{
				for (int i = 1; i < filterpts; ++i)
					fuelbuf[i-1] = fuelbuf[i];
				fuelbuf[filterpts-1] = frame.data[1];
			
				double fueloffset = 0;
				fuelreq[0] = ((double)fuelbuf[0]);
				for (int i = 1; i < filterpts; ++i)
				{
					if (fuelbuf[i-1] > fuelbuf[i])
						fueloffset += 256;
					fuelreq[i] = fueloffset + ((double)fuelbuf[i]);
				}

				double fuel_lph = 0;
				for (int i = 0; i < filterpts; ++i)
					fuel_lph += derivfilt[i]*fuelreq[i];
				
				//lph.update(fuel_lph);
				
				double lp100km = fuel_lph/speed*100;

				//lpkm.update(lp100km);
				std::cout << lp100km << "\t " << fuel_lph << std::endl;

			}

			//if (frame.ident == 20983848)
			//	unkn.update(frame.data[5]);
			//if (frame.ident == 18953224)
			//	unkn.update(frame.data[4]);
	
			if (frame.ident == 23072808)
			{
				//revs.update( (frame.data[6] % 16)*256.0 + frame.data[7]);
			}

			if (frame.ident == 16846890)
			{
				tank6.update( frame.data[6] );
				tank7.update( frame.data[7] );
			}

		}
		catch (const char* ch)
		{
			std::cout << std::endl << "Error: " << ch << std::endl;	
		}
	}
	
	return 0;
}
