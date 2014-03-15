all: main datadump

main: main.cpp
	g++ -o main main.cpp -L . -lftd2xx -lpthread -ldl -lrt

datadump: datadump.cpp
	g++ -o datadump datadump.cpp -O2 -Wall -L . -lftd2xx -lpthread -ldl -lrt

