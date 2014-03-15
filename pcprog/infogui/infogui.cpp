#include <gtkmm.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <canrec.h>

/*
Door image info:
öppnad 30 grader
vit och colored blå
colorize: hue 0, sat 100, lightness passande, 80 för grå bakgrund
front left door image pos. 145, 185.

flash
336, 429

todos?:
färdbroms
blinkers?
ljus?
bälten
*/

class Updater;

const double pi =3.141592653589793;

Gtk::Window* pMain = 0;
Updater* pTheUpdater = 0;

Gtk::Label* plblKeyState = 0;

Gtk::Label* plblRPM = 0;
Gtk::Label* plblEnT = 0;
Gtk::Label* plblLPH = 0;
Gtk::Label* plblVolt = 0;
Gtk::Label* plblFuelL = 0;
Gtk::Label* plblFuelR = 0;

Gtk::Label* plblKPH = 0;
Gtk::Label* plblTrnRt = 0;
Gtk::Label* plblKPL = 0;
Gtk::Label* plblLPK = 0;
Gtk::Label* plblInT = 0;

Gtk::Label* plblOutT = 0;
Gtk::Label* plblOutLight = 0;

Gtk::Label* plblWrpmLF = 0;
Gtk::Label* plblWrpmRF = 0;
Gtk::Label* plblWrpmLR = 0;
Gtk::Label* plblWrpmRR = 0;
Gtk::Label* plblWrpmDFront = 0;
Gtk::Label* plblWrpmDRear = 0;
Gtk::Label* plblWrpmDLeft = 0;
Gtk::Label* plblWrpmDRight = 0;

Gtk::Image* pimgFLDoorOpen = 0;
Gtk::Image* pimgFRDoorOpen = 0;
Gtk::Image* pimgRLDoorOpen = 0;
Gtk::Image* pimgRRDoorOpen = 0;
Gtk::Image* pimgTkDoorOpen = 0;

Gtk::Image* pimgROutlet = 0;

Gtk::Label* plblIndRev = 0;
Gtk::Label* plblIndPark = 0;

const double Wcirc = 0.60*pi; // wheel circumference
const double gB = 1/1.56;     // 1 / spårvidd bak


typedef std::map<Gtk::Widget*, bool> UpdateImgvisMap;
UpdateImgvisMap updateImgvisMap;
Glib::Mutex updateImgvisMutex;

typedef std::map<Gtk::Label*, std::string> UpdateMap;
UpdateMap updateMap;
Glib::Mutex updateQueueMutex;


static
void on_button_clicked()
{
	std::cout << "Pushed, ptr " << pMain << std::endl;
  if(pMain)
    pMain->hide(); //hide() will cause main::run() to end.
}

/*
static
void on_label_clicked()
{
	std::cout << "Pushed, ptr " << plblLabel << std::endl;
	plblLabel->set_text("Ny text");
  //if(pDialog)
    //pDialog->hide(); //hide() will cause main::run() to end.
}
*/


class DerivByteFilter
{
public:
    // time derivative (per second) of increasing unsigned char data with wraparound, takes care of wraparound
    // Tmeas_ms time (in milliseconds) between measurements.
    DerivByteFilter(double Tmeas_ms, unsigned int filterpoints = 81, double sigma = 10)
        : Tmeas_ms(Tmeas_ms), filterpoints(filterpoints), sigma(sigma),
          databuf(filterpoints), derivfilt(filterpoints)
    {
        for (unsigned int i = 0; i < filterpoints; ++i)
	    {
		    databuf[i]=0;
		    const double x = i-(filterpoints-1)/2.0;
		    derivfilt[i] = exp(.5*(-x*x)/(sigma*sigma))*1/(sqrt(2*pi)*sigma) 
				* x/(sigma*sigma)*(1000.0/Tmeas_ms);
        }
    }

    double operator()(unsigned char newData)
    {
	    for (unsigned int i = 1; i < filterpoints; ++i)
		    databuf[i-1] = databuf[i];
		databuf[filterpoints-1] = newData;
			
	    double dataoffset = 0;
        double deriv = ((double)databuf[0]) * derivfilt[0];
        for (int i = 1; i < filterpoints; ++i)
        {
            if (databuf[i-1] > databuf[i])
                dataoffset += 256;
            deriv += (dataoffset + ((double)databuf[i])) * derivfilt[i];
        }
        return deriv;
    }

    void clearDataBuffer()
    {
        for (unsigned int i = 0; i < filterpoints; ++i)
		    databuf[i]=0;
    }

private:
    const double Tmeas_ms;
    const unsigned int filterpoints;
    const double sigma;
	//const double pi;

	std::vector<unsigned char> databuf;
	std::vector<double>         derivfilt;
};




class Updater {
  public:
 
    Updater() : fuelderiv(115,19,3), thread(0), stop(false), number(0), keyState(-10),
            wheelspeed(4, DerivByteFilter(25.0*4, 21, 3)) // 1 second delay.
    {}
               
 
    // Called to start the processing on the thread
    void start () {
      thread = Glib::Thread::create(sigc::mem_fun(*this, &Updater::run), true);
    }
 
    // When shutting down, we need to stop the thread
    ~Updater() {
      {
        Glib::Mutex::Lock lock (mutex);
        stop = true;
      }
      if (thread)
        thread->join(); // Here we block to truly wait for the thread to complete
    }

    bool updateGui() // Called by gui thread for updating labels in gui
    {
        {   // update labels
            Glib::Mutex::Lock lock(updateQueueMutex);
            //std::cout << "Updating gui ";
            const UpdateMap::iterator end = updateMap.end();
            for (UpdateMap::iterator it = updateMap.begin(); it != end; ++it)
            {
                //std::cout << it->first << " ";
                it->first->set_markup(it->second);
            }
            //std::cout << std::endl;
            updateMap.clear();
        }
        {   // update image visibility
            Glib::Mutex::Lock lock(updateImgvisMutex);
            const UpdateImgvisMap::iterator end = updateImgvisMap.end();
            for (UpdateImgvisMap::iterator it = updateImgvisMap.begin(); it != end; ++it)
            {
                it->first->set_visible(it->second);
            }
            updateImgvisMap.clear();
        }
        return true;
    }

 
    //Glib::Dispatcher sig_done;
 
  protected:
    // This is where the real work happens
    void run () 
    {

        //CanrecDecfile can("../canist/snokedjor_mot_flyget.dumpdec2", false, true);
        //CanrecDecfile can("../canist/lkp_varm.dumpdec2", false, true);
        //CanrecDecfile can("../canist/data.dumpdec2", false, true);
        CanrecDecfile can("../canist/data.dumpdec2");
        Canframe frame;
        int wheelTicksIntervalCounter = 0;
 
      while(true) {
        {
          Glib::Mutex::Lock lock (mutex);
          if (stop) 
          {
            std::cout << "Stopping!" << std::endl;
            break;
          }
        }
    
        frame = can.getNextFrame();
        switch (frame.ident)
        {
        case 0x1601028:
            updateRPM(frame);
            break;
        case 0x1213408:
            updateEnT(frame);
            break;
        case 0x1800048:
            updateFuel(frame);
            break;

        case 0x513FFC:
            updateCEM513(frame);
            break;
        case 0x613FFC:
            if (frame.data[0] == 2) updateCEM2(frame);
            break;

        case 0x71002c:
            wheelTicksIntervalCounter = (wheelTicksIntervalCounter+1) % 4; // only updates about every fourth frame
            if (wheelTicksIntervalCounter == 0) updateWheelTicks(frame);
            break;

        case 0x900002:
            updateLightness(frame);
            break;

        case 0x101102a:
            updateREMinfo(frame);
            break;

        //case 0x1b00002: // parkeringsbroms
        case 0x1a00242:
            updateCEMInd(frame);
            break;

        case 0x313df8:  // backväxel
            updateCemInd2(frame);
            break;

        default:
            break;
        }
        
        //sleep(1);
        //std::cout << "Thread write" << number << "!" << std::endl;
	    //std::ostringstream oss;
	    //oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << number << "</b></span>";
	    //plblRPM->set_markup(oss.str());
	    //++number;
        //sig_done();
        //break;
      }
    }

    unsigned int oldRPM;
    void updateRPM(const Canframe& frame)
    {
        unsigned int rpm;
        rpm = ((unsigned int)(frame.data[6] % 16))*256 + frame.data[7];
	    std::ostringstream oss;
	    oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << rpm << "</b></span>";
	    if (oldRPM != rpm) 
        {
            Glib::Mutex::Lock lock(updateQueueMutex);
            updateMap[plblRPM] = oss.str();
        }
        oldRPM = rpm;
    }

    int oldEnT;
    void updateEnT(const Canframe& frame)
    {
        int T;
        T = ((unsigned int)(frame.data[4]))-60;
        if( oldEnT != T)
        {
	        std::ostringstream oss;
            if (T < 0)
                oss << "<span foreground=\"#0000a0\" size=\"x-large\"><b>" << T << "</b></span>";
            else    
	            oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << T << "</b></span>";
            Glib::Mutex::Lock lock(updateQueueMutex);
	        updateMap[plblEnT] = oss.str();
            oldEnT = T;
        }
    }

    DerivByteFilter fuelderiv;
    //double oldfuellph;
    void updateFuel(const Canframe& frame)
    {
        double lph = fuelderiv(frame.data[1])*3600.0/5000.0;
    //    if (oldfuellph != lph)
    //    {
    //        oldfuellph = lph;
            std::ostringstream oss;
            oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << std::fixed << std::setprecision(2) << lph << "</b></span>";

            std::ostringstream osskpl;
            const double kpl = SpeedKPH / lph;
            if (lph == 0 || kpl >= 100) osskpl << "<span foreground=\"#00a000\" size=\"x-large\"><b>--</b></span>";
            else osskpl << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << std::fixed << std::setprecision(1) << kpl << "</b></span>";

            std::ostringstream osslpk;
            const double lpk = lph / SpeedKPH * 100;
            if (SpeedKPH == 0 || lpk >= 100) osslpk << "<span foreground=\"#00a000\" size=\"x-large\"><b>--</b></span>";
            else osslpk << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << std::fixed << std::setprecision(1) << lpk << "</b></span>";

            Glib::Mutex::Lock lock(updateQueueMutex);
            updateMap[plblLPH] = oss.str();
            updateMap[plblKPL] = osskpl.str();
            updateMap[plblLPK] = osslpk.str();
    //    }
    }

    int oldOutT;
    int oldInT;
    int oldVolt;
    void updateCEM2(const Canframe& frame)
    {
        int T;
        T = ((int)(frame.data[1]))-60;
        if( oldInT != T)
        {
	        std::ostringstream oss;
            if (T == 254-60)
                oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>--</b></span>";
            else
            {
                if (T <= 0)
                    oss << "<span foreground=\"#0000a0\" size=\"x-large\"><b>" << T << "</b></span>";
                else    
	                oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << T << "</b></span>";
            }
            Glib::Mutex::Lock lock(updateQueueMutex);
	        updateMap[plblInT] = oss.str();
            oldInT = T;
        }

        int16_t outTT = 256*((unsigned int) frame.data[2]) + frame.data[3];
        //std::cout << (int)frame.data[3] << " " << (int)frame.data[2] << ", " << (unsigned int)outTT << ": ";
        if (outTT & 0x0200) outTT |= 0xfc00; else outTT &= 0x03ff; // sign extension
        int outT = outTT;
        //std::cout << (unsigned int)outTT << "  :  " << outT << " sz: " << sizeof(outTT) << std::endl;
        if (oldOutT != outT)
        {
            oldOutT = outT;
            std::ostringstream oss;
            if (outT <= 0)
                oss << "<span foreground=\"#0000a0\" size=\"x-large\"><b>" << std::fixed << std::setprecision(1) << outT/4.0 << "</b></span>";
            else    
                oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << std::fixed << std::setprecision(1) << outT/4.0 << "</b></span>";
            Glib::Mutex::Lock lock(updateQueueMutex);
            updateMap[plblOutT] = oss.str();
        }

        int Volt;
        Volt = frame.data[4];
        if( oldVolt != Volt )
        {
	        std::ostringstream oss;
            oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>"<< std::fixed << std::setprecision(1) << Volt/8.0 << "</b></span>";
            Glib::Mutex::Lock lock(updateQueueMutex);
	        updateMap[plblVolt] = oss.str();
            oldVolt = Volt;
        }    
    }


    int oldKPH;
    unsigned char oldDoorState;
    double SpeedKPH;
    void updateCEM513(const Canframe& frame)
    {
        int newKeyState = -1;
        if (frame.data[0] & 0x04)
            newKeyState = frame.data[0] & 0x03;
        if (keyState != newKeyState)
        {
            keyState = newKeyState;
            std::ostringstream oss;
            oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>";
            switch (keyState)
            {
            case -1: oss << "--"; break;
            case 0:  oss << "0"; break;
            case 3:  oss << "I";
            case 2:  oss << "I";
            case 1:  oss << "I";
            default: break;
            }
            oss << "</b></span>";
            updateMap[plblKeyState] = oss.str();
            //TODO: update things not updating when key changes..
        }

        if (oldDoorState != frame.data[5])
        {
            oldDoorState = frame.data[5];
            Glib::Mutex::Lock lockim(updateImgvisMutex);
            updateImgvisMap[pimgFLDoorOpen] = oldDoorState & 0x01;
            updateImgvisMap[pimgFRDoorOpen] = oldDoorState & 0x02;
            updateImgvisMap[pimgRLDoorOpen] = oldDoorState & 0x04;
            updateImgvisMap[pimgRRDoorOpen] = oldDoorState & 0x08;
            updateImgvisMap[pimgTkDoorOpen] = oldDoorState & 0x10;
        }
                
        int KPH;
        KPH = ((int)(frame.data[6] % 4))*256 + frame.data[7];
        //std::cout << KPH/4.0 << std::endl;
        if( oldKPH != KPH )
        {
	        std::ostringstream oss;
            SpeedKPH = KPH/4.0;
            oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>"<< std::fixed << std::setprecision(1) << SpeedKPH << "</b></span>";
            Glib::Mutex::Lock lock(updateQueueMutex);
            updateMap[plblKPH] = oss.str();
            oldKPH = KPH;
        }             
    }

    std::vector<DerivByteFilter> wheelspeed;
    void updateWheelTicks(const Canframe& frame)
    {
        double wRPMrf = wheelspeed[0](frame.data[1])/48*60; // right front
        double wRPMlf = wheelspeed[1](frame.data[5])/48*60; // left front
        double wRPMrr = wheelspeed[2](frame.data[6])/48*60; // right rear
        double wRPMlr = wheelspeed[3](frame.data[7])/48*60; // left rear
        double dCoursedt = (wRPMlr-wRPMrr)*gB*Wcirc/60; // rad/s
        std::ostringstream ossrf;
        std::ostringstream osslf;
        std::ostringstream ossrr;
        std::ostringstream osslr;
        std::ostringstream ossdfr;
        std::ostringstream ossdre;
        std::ostringstream ossdle;
        std::ostringstream ossdri;
        std::ostringstream ossTrnRt;
//        ossrf << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(1) << wRPMrf*Wcirc*60/1000 << "</span>";
//        osslf << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(1) << wRPMlf*Wcirc*60/1000 << "</span>";
//        ossrr << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(1) << wRPMrr*Wcirc*60/1000 << "</span>";
//        osslr << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(1) << wRPMlr*Wcirc*60/1000 << "</span>";
        ossrf << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(0) << wRPMrf << "</span>";
        osslf << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(0) << wRPMlf << "</span>";
        ossrr << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(0) << wRPMrr << "</span>";
        osslr << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(0) << wRPMlr << "</span>";
        ossdfr << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(1) << wRPMlf-wRPMrf << "</span>";
        ossdre << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(1) << wRPMlr-wRPMrr << "</span>";
        ossdle << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(1) << wRPMlf-wRPMlr << "</span>";
        ossdri << "<span foreground=\"#00a000\" size=\"x-large\">"<< std::fixed << std::setprecision(1) << wRPMrf-wRPMrr << "</span>";
        ossTrnRt << "<span foreground=\"#00a000\" size=\"x-large\"><b>"<< std::fixed << std::setprecision(1) << dCoursedt*180.0/pi << "</b></span>";
        Glib::Mutex::Lock lock(updateQueueMutex);
        updateMap[plblWrpmRF] = ossrf.str();
        updateMap[plblWrpmLF] = osslf.str();
        updateMap[plblWrpmRR] = ossrr.str();
        updateMap[plblWrpmLR] = osslr.str();
        updateMap[plblWrpmDFront] = ossdfr.str();
        updateMap[plblWrpmDRear] = ossdre.str();
        updateMap[plblWrpmDLeft] = ossdle.str();
        updateMap[plblWrpmDRight] = ossdri.str();
        updateMap[plblTrnRt] = ossTrnRt.str();
    }

    int oldLightness;
    void updateLightness(const Canframe& frame)
    {
        int newLightness = (frame.data[5] & 0xf0)/16;
        if (newLightness != oldLightness)
        {
            oldLightness = newLightness;
            std::ostringstream oss;
            oss << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << newLightness << "</b></span>";
            Glib::Mutex::Lock lock(updateQueueMutex);
            updateMap[plblOutLight] = oss.str();
        }
    }
    int oldFuel;
    bool oldRearOutlet;
    void updateREMinfo(const Canframe& frame)
    {
        int newFuel = ((int) frame.data[6])*256 + frame.data[7];
        if (newFuel != oldFuel)
        {
            oldFuel = newFuel;
            std::ostringstream ossL;
            std::ostringstream ossR;
            ossL << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << (int)frame.data[7] << "</b></span>";
            ossR << "<span foreground=\"#00a000\" size=\"x-large\"><b>" << (int)frame.data[6] << "</b></span>";
            Glib::Mutex::Lock lock(updateQueueMutex);
            updateMap[plblFuelL] = ossL.str();
            updateMap[plblFuelR] = ossR.str();
        }

        bool rearOutlet = (frame.data[2] & 0x01);
        if (oldRearOutlet != rearOutlet)
        {
            oldRearOutlet = rearOutlet;
            Glib::Mutex::Lock lock(updateImgvisMutex);            
            updateImgvisMap[pimgROutlet] = rearOutlet;
        }
    }
    bool oldIndPbrake;
    void updateCEMInd(const Canframe& frame)
    {
        bool pBrake = (frame.data[3] & 0x01);
        if (oldIndPbrake != pBrake)
        {
            oldIndPbrake = pBrake;
            Glib::Mutex::Lock lock(updateImgvisMutex);
            updateImgvisMap[plblIndPark] = pBrake;
        }
    }
    bool oldIndPRev;
    void updateCemInd2(const Canframe& frame)
    {
        bool revInd = (frame.data[4] & 0x40);
        if (oldIndPRev != revInd)
        {
            oldIndPRev = revInd;
            Glib::Mutex::Lock lock(updateImgvisMutex);
            updateImgvisMap[plblIndRev] = revInd;
        }            
    }


 
    Glib::Thread * thread;
    Glib::Mutex mutex;
    bool stop;
    int  number;
    int  keyState;
};





int main(int argc, char *argv[])
{
	pTheUpdater = new Updater;
	if(!Glib::thread_supported()) Glib::thread_init();

	Gtk::Main kit(argc, argv);

	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("infoguiMain.glade");

	builder->get_widget("winInfoMain", pMain);

	builder->get_widget("lblKeyState", plblKeyState);

	builder->get_widget("lblRPM", plblRPM);
	builder->get_widget("lblEnT", plblEnT);
	builder->get_widget("lblLPH", plblLPH);
	builder->get_widget("lblVolt", plblVolt);
	builder->get_widget("lblFuelL", plblFuelL);
	builder->get_widget("lblFuelR", plblFuelR);

	builder->get_widget("lblKPH", plblKPH);
	builder->get_widget("lblTrnRt", plblTrnRt);
	builder->get_widget("lblKPL", plblKPL);
	builder->get_widget("lblLPK", plblLPK);
	builder->get_widget("lblInT", plblInT);

	builder->get_widget("lblOutT", plblOutT);
	builder->get_widget("lblOutLight", plblOutLight);

	builder->get_widget("lblWrpmLF", plblWrpmLF);
	builder->get_widget("lblWrpmRF", plblWrpmRF);
	builder->get_widget("lblWrpmLR", plblWrpmLR);
	builder->get_widget("lblWrpmRR", plblWrpmRR);
	builder->get_widget("lblWrpmDFront", plblWrpmDFront);
	builder->get_widget("lblWrpmDRear", plblWrpmDRear);
	builder->get_widget("lblWrpmDLeft", plblWrpmDLeft);
	builder->get_widget("lblWrpmDRight", plblWrpmDRight);

	builder->get_widget("imgFLDoorOpen", pimgFLDoorOpen);
	builder->get_widget("imgFRDoorOpen", pimgFRDoorOpen);
	builder->get_widget("imgRLDoorOpen", pimgRLDoorOpen);
	builder->get_widget("imgRRDoorOpen", pimgRRDoorOpen);
	builder->get_widget("imgTkDoorOpen", pimgTkDoorOpen);

	builder->get_widget("imgROutlet", pimgROutlet);

	builder->get_widget("lblIndRev", plblIndRev);
	builder->get_widget("lblIndPark", plblIndPark);

    

    //sigc::connection Glib::SignalTimeout::connect(const sigc::slot<bool>& slot, unsigned int interval, int priority = Glib::PRIORITY_DEFAULT);

    //sigc::mem_fun(*pTheUpdater, &Updater::run);
//§    sigc::mem_fun(*pTheUpdater, &Updater::updateGui);
    sigc::slot<bool> labelslot = sigc::mem_fun(*pTheUpdater, &Updater::updateGui);
    sigc::connection labelconn = Glib::signal_timeout().connect(labelslot, 100);

	pTheUpdater->start();

//	Gtk::Image* pImBack = 0;
//	builder->get_widget("imBack", pImBack);
//	Glib::RefPtr<Gdk::Pixbuf> pPix = Gdk::Pixbuf::create_from_file("backbuf.png");
//	pImBack->set(pPix);
//	pPix = Gdk::Pixbuf::create_from_file("btntest.png");
//	builder->get_widget("imBtnImg", pImBack);
//	pImBack->set(pPix);
//	Gtk::Button* pBtnLabel = 0;
//	builder->get_widget("btnWlabel", pBtnLabel);
//	builder->get_widget("lblBtnWlabel", plblLabel);
//	pBtnLabel->signal_clicked().connect( sigc::ptr_fun(on_label_clicked) );
	Gtk::Button* pButton = 0;
	builder->get_widget("btnClose", pButton);
	pButton->signal_clicked().connect( sigc::ptr_fun(on_button_clicked) );

	Gtk::Main::run(*pMain);
	delete pTheUpdater;
    return 0;
}


