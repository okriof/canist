#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencvplot.h"

int main()
{
	OpencvPlot fooplot("MyWindow",PlotSettings(2,-1,.5),10);
	OpencvPlot fumplot("other",PlotSettings(1000,200,100),10);

//	cv::namedWindow("foowindow",CV_WINDOW_AUTOSIZE);
//	cv::Mat_<unsigned char> plot1(256,1000);
//	plot1 = 255;
	for (unsigned int i = 0; i < 10000; ++i)
	{
		fooplot.update(std::sin(i/100.0));
		fumplot.update(i);
	}

	std::cout << "Foo" << std::endl;

	// close all windows..
	return 0;
}
