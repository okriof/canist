LOCALINCL = -I /home/internal/deps/local/include
LOCALLIB = -L /home/internal/deps/local/lib
CVLIBS = $(LOCALLIB) -lopencv_core -lopencv_highgui
LIBPATH = -Xlinker -rpath=/home/internal/deps/local/lib
#CPP_FLAGS = -g -O0 -std=c++0x
CPP_FLAGS = -O3 -march=native -std=c++0x
LD_FLAGS = -std=c++0x

all : opencvplottest


opencvplottest.o : opencvplottest.cpp opencvplot.h
	g++ -c $(CPP_FLAGS)  opencvplottest.cpp $(LOCALINCL)

opencvplot.o : opencvplot.cpp opencvplot.h
	g++ -c $(CPP_FLAGS) opencvplot.cpp $(LOCALINCL)


opencvplottest : opencvplottest.o opencvplot.o
	g++ $(LD_FLAGS) -o opencvplottest opencvplottest.o opencvplot.o $(CVLIBS) $(LIBPATH)



clean :
	rm -f *.o opencvplottest

