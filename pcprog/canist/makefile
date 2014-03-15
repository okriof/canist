LOCALINCL = -I /home/internal/deps/local/include
LOCALLIB = -L /home/internal/deps/local/lib
CVLIBS = $(LOCALLIB) -lopencv_core -lopencv_highgui
LIBPATH = -Xlinker -rpath=/home/internal/deps/local/lib
#CPP_FLAGS = -g -O0 -std=c++0x
CPP_FLAGS = -O3 -march=native -std=c++11
LD_FLAGS = -L . -lftd2xx -lpthread -ldl -lrt -std=c++11 

all : canist canFilterPrinter canPrintUnseen canPlot canPosest

canframe.o : canframe.cpp canframe.h makefile
	g++ -c $(CPP_FLAGS) canframe.cpp

canreccan.o : canreccan.cpp canreccan.h makefile
	g++ -c $(CPP_FLAGS) canreccan.cpp

canrec.o : canrec.cpp canrec.h makefile
	g++ -c $(CPP_FLAGS) canrec.cpp

canbuffer.o : canbuffer.cpp canbuffer.h makefile canrec.h canreccan.h
	g++ -c $(CPP_FLAGS) canbuffer.cpp

canbuffertest.o : canbuffertest.cpp canbuffer.h makefile
	g++ -c $(CPP_FLAGS) canbuffertest.cpp

framehistogram.o : framehistogram.cpp framehistogram.h canframe.h makefile
	g++ -c $(CPP_FLAGS) framehistogram.cpp

opencvplot.o : opencvplot/opencvplot.cpp opencvplot/opencvplot.h makefile
	g++ -c $(CPP_FLAGS) opencvplot/opencvplot.cpp $(LOCALINCL)


canbuffertest : canbuffer.o canbuffertest.o canrec.o canreccan.o canframe.o
	g++ -o canbuffertest canbuffertest.o canbuffer.o canrec.o canreccan.o canframe.o $(LD_FLAGS)
	


canist.o : canist.cpp canbuffer.h canrec.h canreccan.h framehistogram.h makefile
	g++ -c $(CPP_FLAGS) canist.cpp

canist : canist.o canbuffer.o canrec.o framehistogram.o canreccan.o canframe.o makefile
	g++ -o canist canist.o canbuffer.o canrec.o framehistogram.o canreccan.o canframe.o $(LD_FLAGS)



canPrintUnseen.o : canPrintUnseen.cpp canrec.h canreccan.h canframe.h makefile
	g++ -c $(CPP_FLAGS) canPrintUnseen.cpp

canPrintUnseen : canPrintUnseen.o canrec.o canreccan.o canframe.o makefile
	g++ -o canPrintUnseen canPrintUnseen.o canrec.o canreccan.o canframe.o $(LD_FLAGS)



canFilterPrinter.o : canFilterPrinter.cpp canrec.h canreccan.h canframe.h makefile
	g++ -c $(CPP_FLAGS) canFilterPrinter.cpp

canFilterPrinter : canFilterPrinter.o canrec.o canreccan.o canframe.o makefile
	g++ -o canFilterPrinter canFilterPrinter.o canrec.o canreccan.o canframe.o $(LD_FLAGS)



canPlot.o : canPlot.cpp opencvplot/opencvplot.h canrec.h makefile
	g++ -c $(CPP_FLAGS) canPlot.cpp

canPlot : canPlot.o opencvplot.o canrec.o canframe.o
	g++ -o canPlot canPlot.o opencvplot.o canrec.o canframe.o $(LD_FLAGS) $(CVLIBS) $(LIBPATH)



canPosest.o : canPosest.cpp canrec.h makefile
	g++ -c $(CPP_FLAGS) canPosest.cpp

canPosest : canPosest.o canrec.o canframe.o
	g++ -o canPosest canPosest.o canrec.o canframe.o $(LD_FLAGS)


clean :
	rm -f *.o canist canFilterPrinter canPrintUnseen canPlot canPosest

