#include "ftd2xx.h"

#include <iostream>
using namespace std;


int main()
{
	FT_STATUS ftStatus;
	FT_HANDLE ftHandle;
	ULONG Status;

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
			for (int i = 0; i < numDevs; i++) 
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


	ftStatus = FT_Open(0, &ftHandle);
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
	ULONG bytesTrans;
	char rxBuf[20];
	for (;;)
	{
		if(FT_Read(ftHandle, rxBuf, 1, &bytesTrans) == FT_OK)
		{
			//rxBuf[bytesTrans] = 0;
			//cout << "Read " << bytesTrans << " bytes: "
			//     << rxBuf << endl;
			cout << rxBuf[0] << flush;
		}
	}



/*
	// read and stuff
	for (unsigned int i = 1; i < 10; ++i)
	{
		ULONG bytesTrans;
		char rxBuf[1000000];
		char txBuf[3000000];

		//for (unsigned int k = 0; k < i; ++k)
		//	if(FT_Write(ftHandle, (void*)"HEJSAN!", 2, &bytesTrans) != FT_OK)
		//		return 1;

		txBuf[0] = 0x55;

		//if (FT_Write(ftHandle, txBuf, 60000, &bytesTrans) == FT_OK)
		//	cout << bytesTrans << " bytes sent\n";
		
	//if(FT_Write(ftHandle, (void*)"HEJSAN!", 7, &bytesTrans) == FT_OK)
	//	cout << "Wrote " << bytesTrans << " bytes." << endl;

		ULONG rxBufOcc;
		if (FT_GetStatus(ftHandle, &rxBufOcc, &bytesTrans, &ftStatus) == FT_OK)
			cout << bytesTrans << " bytes in transmit queue, " 
				<< rxBufOcc << " bytes in rx queue, "
				<< " status: " << ftStatus << endl;
		//if(FT_GetQueueStatus(ftHandle, &bytesTrans) == FT_OK)
		//	cout << bytesTrans << " bytes in receive queue." << endl;
	

		//if(FT_Read(ftHandle, rxBuf, i, &bytesTrans) == FT_OK)
		if(FT_Read(ftHandle, rxBuf, rxBufOcc, &bytesTrans) == FT_OK)
		{
			cout << "Read " << bytesTrans << " bytes\n";
			for (unsigned int i = 0; i < bytesTrans; ++i)
				cout << (int)rxBuf[i] << ", ";
			cout << endl;
			//rxBuf[bytesTrans] = 0;
			//cout << rxBuf << endl;
		}
	}

*/






	ftStatus = FT_Close(ftHandle);
	if (ftStatus == FT_OK)
		cout << "Device closed" << endl;

	return 0;
}
