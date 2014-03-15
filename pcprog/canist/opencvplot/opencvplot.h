#ifndef OPENCVPLOT_H
#define OPENCVPLOT_H

#include <vector>
//#include <opencv2/opencv.hpp>

//#include<thread>
//#include<atomic>

class PlotSettings
{
public:

	double maxval;
	double minval;
	double valgrid;
	
	unsigned int plotHeight;
	unsigned int plotWidth;

	PlotSettings() {};
	PlotSettings(double maxval, 
                     double minval = 0, 
                     double valgrid = 0,
                     unsigned int plotHeight = 256,
                     unsigned int plotWidth = 1000)
	: maxval(maxval), minval(minval), valgrid(valgrid), 
	  plotHeight(plotHeight), plotWidth(plotWidth)
	 {};
};


class OpencvPlot
{
public:
	OpencvPlot(const char* windowname, const PlotSettings& settings, 
			unsigned int updateinterval = 1);
	~OpencvPlot();

	void update(double value);

private:
	const char* const windowname;
	unsigned int updateinterval;
	unsigned int updatecounter;
	unsigned int plotlength;
	unsigned int plotheight;
	double       plotOffset;
	double       plotScale;
	//cv::Mat_<unsigned char> plot; 
	void*        plotp;

	const unsigned char bgval;
	const unsigned char fgval;
	const unsigned char gridval;
	std::vector<unsigned int> gridpoints;

	// vector for more plots, background in one plot.. or moving grid..

	//volatile std::atomic<bool> keepRunning;
	//std::atomic<bool> keepRunning;
	//std::thread mthread;
	//void run();


};





#endif
