#include "ftd2xx.h"

#include <iostream>
#include <fstream>
using namespace std;


int main()
{
	//ofstream of("data.dump", ios::out | ios::binary);
	ofstream of("data.dump");
	FT_STATUS ftStatus;
	FT_HANDLE ftHandle;
	//ULONG Status;

/*
	ULONG numDevs;
	ftStatus = FT_CreateDeviceInfoList(&numDevs);
	if (ftStatus != FT_OK)
		return 1;

	cout << "Found " << numDevs << " devices." << endl;


	if (numDevs > 0) 
	{
		FT_DEVICE_LIST_INFO_NODE* devInfo =
			new FT_DEVICE_LIST_INFO_NODE[numDevs];
		ftStatus = FT_GetDeviceInfoList(devInfo,&numDevs);

		if (ftStatus == FT_OK) 
		{
			for (unsigned int i = 0; i < numDevs; i++) 
			{
				cout << "Dev " << i << endl;
				cout << " Flags=" << devInfo[i].Flags << endl;
				cout << " Type=" << devInfo[i].Type << endl;
				cout << " ID=" << devInfo[i].ID << endl;
				cout << " LocId=" << devInfo[i].LocId << endl;
				cout << " SerialNumber " << devInfo[i].SerialNumber << endl;
				cout << " Description " << devInfo[i].Description << endl;
				cout << " ftHandle " << devInfo[i].ftHandle << endl;
			}
		}
	}

	char buf[64];
	FT_ListDevices(0,buf,FT_LIST_BY_INDEX|FT_OPEN_BY_DESCRIPTION);
	//FTID_GetDeviceSerialNumber(0, buf, 64);
	cout << "Buffer: " << buf << endl;
*/
	int deviceID = 0;
	ftStatus = FT_Open(deviceID, &ftHandle);
	if (ftStatus != FT_OK)
	{
		cout << "Error while opening: " << ftStatus << FT_DEVICE_NOT_OPENED << endl;
		return 1;
	}
	cout << "Device opened" << endl;

	
	if (FT_SetBaudRate(ftHandle, 1250000) == FT_OK)
		cout << "Baud rate set!" << endl;
	
	if (FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE)
		== FT_OK)
		cout << "Data format set!" << endl;

	if (FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0) == FT_OK)
		cout << "Flow set!" << endl;



	// read stuff
	ULONG bytesTrans = 100;
	ULONG bytesToGet = 0;
	//char rxBuf[2000];
	char* rxBuf = new char[20010];
	bool onScreen = true;
	//for (unsigned int k=0; k<1000;++k)
	for(;;)
	{
		//rxBuf = new char[20010];
		bytesToGet = 0;
		while (bytesToGet < 10) //000)
			if(FT_GetQueueStatus(ftHandle, &bytesToGet) != FT_OK)
			{
				cout << "Get avail len error" << endl;
				return 1;
			}

		ftStatus = FT_Read(ftHandle, rxBuf, bytesToGet, &bytesTrans);
//		if(FT_Read(ftHandle, rxBuf, 20000, &bytesTrans) == FT_OK)
		if (ftStatus == FT_OK)
		{
			of.write(rxBuf, bytesTrans);
			if (onScreen)
			{
				for (unsigned int i = 0; i < bytesTrans; ++i)
				{
					if ((unsigned char)rxBuf[i] == 250) cout << "\n";
					cout << " " << (int)(unsigned char)rxBuf[i];
				}
			}

			//cout << "\nReceived " << bytesTrans << " bytes.\n" << flush;
			cout << bytesTrans << "." << flush;
		}
		else
			return 1;
		//delete[] rxBuf;
	}





	ftStatus = FT_Close(ftHandle);
	if (ftStatus == FT_OK)
		cout << "Device closed" << endl;

	return 0;
}
