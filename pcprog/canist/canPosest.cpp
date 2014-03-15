#include <fstream>
#include <iostream>
#include <math.h>
#include "canrec.h"
#include <sys/time.h>


double sinxgx(double x) ///< sin(x)/x
{
	const double xsq = x*x;
	double factor = -xsq/(1*2*3);
	double n = 3;
	double oldval = 1;
	double val = 1+factor;

	while(val != oldval)
	{
		oldval = val;
		n += 2;
		factor *= -xsq/((n-1)*n);
		val += factor;
	}
	return val;
}

double omcosxgx(double x) ///< (1-cos(x))/x
{
	const double xsq = x*x;
	double factor = x/(1*2);
	double n = 2;
	double oldval = 0;
	double val = factor;

	while(val != oldval)
	{
		oldval = val;
		n += 2;
		factor *= -xsq/((n-1)*n);
		val += factor;
	}
	return val;
}


int main()
{
	// initialize plot process
	pid_t pid;
	int commpipe[2];
	if (pipe(commpipe))
	{ std::cout << "Pipe error" << std::endl; return 1; }

	if ( (pid=fork()) == -1)
	{ std::cout << "Fork error" << std::endl; return 1; }

	if (pid == 0) // the newly forked process..
	{
		std::cout << "This is the other process" << std::endl;
		dup2(commpipe[0],0); // replace stdin with pipe
		close(commpipe[1]); 

		execl("/usr/bin/gnuplot","/usr/bin/gnuplot",0);

		std::cerr << "Something is wrong" << std::endl;
		return 0;
	}

	// parent process..
	close(commpipe[0]);



//	CanrecDecfile can("lkpg_ljungsbro.dumpdec2", false, true);
//	CanrecDecfile can("skg2skol_norev.dumpdec2", false, true);
	CanrecDecfile can("data.dumpdec2");
	std::ofstream plotfile("pos");



	plotfile << "0 0\n-2 -2\n2 -2\n0 0" << std::endl;
	write(commpipe[1], "set size ratio -1\n", 18);
	write(commpipe[1], "plot 'pos' with lines\n", 22);
	
	const double pi = 3.141592653589793;
	
	const double f1 = 0.60*pi/48; // vänster bak
	const double f2 = 0.60*pi/48; // höger bak
	const double gB = 1/1.56;     // 1 / spårvidd bak
	// TODO: reparameterisera, B och omkretsratio vs hjulomkrets?

	int t1prev = 0;
	int t2prev = 0;
	int t1acc = 0;
	int t2acc = 0;
	double phi = 0;
	double px = 0;
	double py = 0;

	// ticks init
	Canframe frame = can.getNextFrame();
	while (frame.ident != 7405612)
		frame = can.getNextFrame();
	t1prev = frame.data[7];
	t2prev = frame.data[6];

	// plot refresh rate limiter
	timeval cmptime;
	timeval newtime;
	gettimeofday(&cmptime,0);


	for(;;)
	{
		frame = can.getNextFrame();
		while (frame.ident != 7405612)
			frame = can.getNextFrame();

		const int ticks1 = frame.data[7];
		const int ticks2 = frame.data[6];		
			
		//std::cout << ticks1 << "\t" << ticks2 << std::endl;
		t1acc += ((int)ticks1 - (int)t1prev + 256*(ticks1<t1prev));
		t2acc += ((int)ticks2 - (int)t2prev + 256*(ticks2<t2prev));

		t1prev = ticks1;
		t2prev = ticks2;

		if (std::max(t1acc,t2acc) > 48)
		{
			const double D = (f1*t1acc + f2*t2acc)*.5;
			const double dphi = (f1*t1acc - f2*t2acc)*gB; // delta phi
			const double dxl = sinxgx(dphi)*D;	// displacement local frame
			const double dyl = omcosxgx(dphi)*D;

			const double dxg = cos(phi)*dxl - sin(phi)*dyl; // displacement, global frame
			const double dyg = sin(phi)*dxl + cos(phi)*dyl;
		
			px += dxg;
			py += dyg;
			phi += dphi;
			
			t1acc = 0;
			t2acc = 0;

			plotfile << py << " " << px << "\n";

			gettimeofday(&newtime,0);
			if (newtime.tv_sec > cmptime.tv_sec || (newtime.tv_sec == cmptime.tv_sec && newtime.tv_usec == cmptime.tv_usec))
			{
				plotfile << std::flush;
				write(commpipe[1], "replot\n", 7); // update plot
				cmptime.tv_usec += 900000;
				if (cmptime.tv_usec >= 1000000)
				{
					cmptime.tv_sec += 1;
					cmptime.tv_usec -= 1000000;
				}
			}
		}
	}
// todo: plot every something second..

/*
    if (t(ii) > tp)
     %todo: plot every something second..
        plot(pxy(2,:),pxy(1,:)); axis equal; title(ii);
        %disp(pxy(:,end))
        pause(.1);
        tp = tp + 1002;
    end
end
 plot(pxy(2,:),pxy(1,:)); axis equal;
*/







	//std::cout << "FRAMEID: " << frame.ident << std::endl;

	/*for(double d = -10; d < 10; d+= .001)
		plotfile << d << " " << omcosxgx(d) << "\n";
	plotfile << std::flush;


	write(commpipe[1], "plot 'pos' with lines\n", 22);
*/
	int i;
	std::cin >> i;	
}
