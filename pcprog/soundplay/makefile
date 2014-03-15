#LOCALINCL = -I /home/internal/deps/local/include
#LOCALLIB = -L /home/internal/deps/local/lib
#CVLIBS = $(LOCALLIB) -lopencv_core -lopencv_highgui
#LIBPATH = -Xlinker -rpath=/home/internal/deps/local/lib
#CPP_FLAGS = -g -O0 -std=c++0x
CC_FLAGS = -O0 -g -march=native
CPP_FLAGS = -O0 -g -march=native -std=c++0x
LD_FLAGS = -lpulse 
#-lpthread -ldl -lrt -std=c++0x 
#LD_FLAGS = -L . -lftd2xx -lpthread -ldl -lrt -std=c++0x 

#all : canist canFilterPrinter canPrintUnseen canPlot canPosest
all : audiotest

audiopulse.o : audiopulse.c audiopulse.h makefile
	gcc -c $(CC_FLAGS) audiopulse.c

audiotest.o : audiotest.cpp audiopulse.h makefile
	g++ -c $(CPP_FLAGS) audiotest.cpp


audiotest : audiopulse.o audiotest.o makefile
	g++ -o audiotest audiopulse.o audiotest.o ../canrec.o ../canframe.o  $(LD_FLAGS)


clean :
	rm -f *.o audiotest

