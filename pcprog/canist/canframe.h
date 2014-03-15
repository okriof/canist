#ifndef CANFRAME_H
#define CANFRAME_H
#include<iostream>
//#include<sys/time.h>

class Canframe
{
public:
	unsigned int 	ident; //uint32               4
	bool		extended;   //                1 for all flags inc, ACK
	bool		SRR;
	bool 		IDE;
	bool		RTR; // 1: remote, 0: data
	bool		r1;
	bool		r0;
	unsigned char	datalen;	//            1
	unsigned char  data[8];        //            8
	unsigned int	calcCRC;	// uint16     2
	unsigned int 	recCRC;		// uint16     2
	bool		ACK;
	unsigned int   ctime_sec;	//	      4  capture time seconds part
	unsigned int   ctime_usec;     //	      4  capture time, microseconds part
	
	double timediff(const Canframe& reference) const;

	void write(std::ostream& os) const; ///< Write frame data to ostream!

	Canframe() {};
	const Canframe& operator=(const unsigned char* const byteseq); ///< fill frame from bytes, requires getSizeof() bytes.
	Canframe(const unsigned char* const byteseq); ///< Create frame from bytes, requires getSizeof() bytes.

	static unsigned int getSizeof() {return 4+4+4+1+1+8+2+2 +2;};
};

#endif



