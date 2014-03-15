#include "canframe.h"


double Canframe::timediff(const Canframe& reference) const
{
	double ns = ((double)this->ctime_usec) - reference.ctime_usec;
	double s  = ((double)this->ctime_sec)  - reference.ctime_sec;
	if (ns < 0)
	{
		ns += 1000000;
		 s -= 1;
	}
	return s + ns/1000000;
}


void Canframe::write(std::ostream& os) const
{
	char dataw[28]; // 4+1+1+8+2+2+4+4 +2 initflag
	dataw[0] = 250;
	dataw[1] = 111;

	dataw[2] = (ident/(256*256*256)) % 256;
	dataw[3] = (ident/(256*256    )) % 256;
	dataw[4] = (ident/(256        )) % 256;
	dataw[5] = (ident              ) % 256;

	dataw[6] = 64*extended + 32*SRR + 16*IDE + 8*RTR + 4*r1 + 2*r0 + 1*ACK;

	dataw[7] = datalen;
	dataw[8] = data[0];
	dataw[9] = data[1];
	dataw[10] = data[2];
	dataw[11] = data[3];
	dataw[12] = data[4];
	dataw[13] = data[5];
	dataw[14] = data[6];
	dataw[15] = data[7];

	dataw[16] = (calcCRC/256) % 256;
	dataw[17] = (calcCRC)     % 256;
	dataw[18] = (recCRC/256)  % 256;
	dataw[19] = (recCRC)      % 256;

	dataw[20] = (ctime_sec/(256*256*256)) % 256;
	dataw[21] = (ctime_sec/(256*256    )) % 256;
	dataw[22] = (ctime_sec/(256        )) % 256;
	dataw[23] = (ctime_sec              ) % 256;
	dataw[24] = (ctime_usec/(256*256*256)) % 256;
	dataw[25] = (ctime_usec/(256*256    )) % 256;
	dataw[26] = (ctime_usec/(256        )) % 256;
	dataw[27] = (ctime_usec              ) % 256;
	
	os.write(dataw, 28);
}


Canframe::Canframe(const unsigned char* const dataw)
{ *this = dataw; }


const Canframe& Canframe::operator=(const unsigned char* const dataw)
{
	if ((dataw[0] != 250) || (dataw[1] != 111))
		throw "initial bytes incorrect";
	
	ident =    ((((unsigned int)dataw[2])*256 + dataw[3])*256 + dataw[4])*256 + dataw[5];

	extended = (dataw[6] & 0x40) != 0;
	SRR      = (dataw[6] & 0x20) != 0;
	IDE      = (dataw[6] & 0x10) != 0;
	RTR      = (dataw[6] & 0x08) != 0;
	r1       = (dataw[6] & 0x04) != 0;
	r0       = (dataw[6] & 0x02) != 0;
	ACK      = (dataw[6] & 0x01) != 0;

	datalen = dataw[7];
	data[0] = dataw[8];
	data[1] = dataw[9];
	data[2] = dataw[10];
	data[3] = dataw[11];
	data[4] = dataw[12];
	data[5] = dataw[13];
	data[6] = dataw[14];
	data[7] = dataw[15];

	calcCRC = ((unsigned int)dataw[16])*256 + dataw[17];
	recCRC  = ((unsigned int)dataw[18])*256 + dataw[19];

	ctime_sec = ((((unsigned int)dataw[20])*256 + dataw[21])*256 + dataw[22])*256 + dataw[23];
	ctime_usec = ((((unsigned int)dataw[24])*256 + dataw[25])*256 + dataw[26])*256 + dataw[27];

	return *this;
}

/*   // old format without timestamp
void Canframe::write(std::ostream& os)
{
	char dataw[20]; // 4+1+1+8+2+2 +2 initflag
	dataw[0] = 250;
	dataw[1] = 110;

	dataw[2] = (ident/(256*256*256)) % 256;
	dataw[3] = (ident/(256*256    )) % 256;
	dataw[4] = (ident/(256        )) % 256;
	dataw[5] = (ident              ) % 256;

	dataw[6] = 64*extended + 32*SRR + 16*IDE + 8*RTR + 4*r1 + 2*r0 + 1*ACK;

	dataw[7] = datalen;
	dataw[8] = data[0];
	dataw[9] = data[1];
	dataw[10] = data[2];
	dataw[11] = data[3];
	dataw[12] = data[4];
	dataw[13] = data[5];
	dataw[14] = data[6];
	dataw[15] = data[7];

	dataw[16] = (calcCRC/256) % 256;
	dataw[17] = (calcCRC)     % 256;
	dataw[18] = (recCRC/256)  % 256;
	dataw[19] = (recCRC)      % 256;
	
	os.write(dataw, 20);
}
*/

