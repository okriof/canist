#include "canreccan.h"
#include <iostream>

using namespace std;

CanrecCan::CanrecCan()
{ 
	FT_STATUS ftStatus;
	int deviceID = 0;
	ftStatus = FT_Open(deviceID, &ftHandle);
	if (ftStatus != FT_OK)
	{
		cout << "Error while opening: " << ftStatus << FT_DEVICE_NOT_OPENED << endl;
		throw("Could not open device");
	}
	cout << "Device opened" << endl;

	
	if (FT_SetBaudRate(ftHandle, 1250000) == FT_OK)
		cout << "Baud rate set!" << endl;
	
	if (FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE)
		== FT_OK)
		cout << "Data format set!" << endl;

	if (FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0) == FT_OK)
		cout << "Flow set!" << endl;

};

unsigned char CanrecCan::importByte()
{
	unsigned char newb;
	unsigned int bytesToGet = 0;
	unsigned int bytesGot;
	while (bytesToGet < 1)
		if(FT_GetQueueStatus(ftHandle, &bytesToGet) != FT_OK)
		{
			cout << "Get avail len error" << endl;
			throw(10); //TODO better throw
		}

	FT_Read(ftHandle, &newb, 1, &bytesGot);
	if (bytesGot != 1)
	{
		cout << "Got incorrect number of bytes" << endl;
		throw(20); //TODO better throw
	}
//	cout << "Got " << bytesGot << " wanted one" << endl;

	return newb;
}

void CanrecCan::importBytes(std::vector<unsigned char>& bytes, unsigned int count)
{
	bytes.resize(count);
	unsigned int bytesToGet = 0;
	unsigned int bytesGot;
	while (bytesToGet < count)
		if(FT_GetQueueStatus(ftHandle, &bytesToGet) != FT_OK)
		{
			cout << "Get avail len error" << endl;
			throw(30); //TODO better throw
		}

	FT_Read(ftHandle, (char*) &bytes[0], count, &bytesGot);
	if (bytesGot != count)
	{
		cout << "Got incorrect number of bytes\n"
			<< "got " << bytesGot << " wanted " << count
			<< " (" << bytesToGet << " bytes to get)"
			<< endl;
		throw(40); //TODO better throw
	}
	//cout << "got " << bytesGot << " wanted " << count << endl;
	//cout << ".";
}

CanrecCan::~CanrecCan()
{
	FT_STATUS ftStatus = FT_Close(ftHandle);
	if (ftStatus == FT_OK)
		cout << "Device closed" << endl;
}


