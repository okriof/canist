#include "opencvplot.h"
#include "opencv2/opencv.hpp"



OpencvPlot::OpencvPlot(const char* windowname, const PlotSettings& settings, unsigned int updateinterval) 
	: windowname(windowname), updateinterval(updateinterval), updatecounter(0),
	  plotlength(settings.plotWidth), plotheight(settings.plotHeight), 
	  plotOffset(-settings.minval), 
	  plotScale(plotheight/(settings.maxval-settings.minval)),
	  //plot(plotheight,plotlength),
	  plotp((void*) new cv::Mat_<unsigned char>(plotheight,plotlength)),
	  bgval(0), fgval(255), gridval(60)
{ 
	cv::Mat_<unsigned char>& plot = *((cv::Mat_<unsigned char>*) plotp); 
	cv::namedWindow(windowname,CV_WINDOW_AUTOSIZE);
	plot = bgval;

	if (settings.valgrid > 0)
	{
		for (double grid = settings.minval; grid < settings.maxval; grid += settings.valgrid)
		{
			int pt = (grid+plotOffset)*plotScale;
			if ((pt >= 0) && (pt < plotheight))
				gridpoints.push_back((plotheight-1-pt)*plotlength + plotlength-1);
		}
	}	

	cv::imshow(windowname, plot);
	cv::waitKey(10);	
}

OpencvPlot::~OpencvPlot() 
{ 
	//TODO: close window!
	cv::Mat_<unsigned char>* plot = (cv::Mat_<unsigned char>*) plotp;
}

void OpencvPlot::update(double value)
{
	cv::Mat_<unsigned char>& plot = *((cv::Mat_<unsigned char>*) plotp);
	for (unsigned int r = 0; r < plotheight; ++r)
	{
		unsigned char* d = plot.data + plotlength*r;
		for (unsigned int k = 1; k < plotlength; ++k)
				d[k-1] = d[k];

		d[plotlength-1] = bgval;
	}
	for (unsigned int r = 0; r < gridpoints.size(); ++r)
		plot.data[gridpoints[r]] = gridval;

	int plotval = (value+plotOffset)*plotScale;
	if (plotval < 0) plotval = 0;
	if (plotval >= plotheight) plotval = plotheight-1; 
	plot.data[plotlength*(plotheight-1-plotval) + plotlength-1] = fgval;

	cv::imshow(windowname, plot);

	if (updateinterval > 0 && updatecounter % updateinterval == 0)
		cv::waitKey(1);
	++updatecounter;
}

