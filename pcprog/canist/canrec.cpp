#include "canrec.h"
#include <iostream>
#include <sys/time.h>
#include <unistd.h>


const Canframe& Canrec::getNextFrame()
{
	// sync
	unsigned char byte1 = importByte();
	unsigned char byte2 = importByte();
	unsigned int syncbytes = 0;
	while (! (byte1 == 0xFA && byte2 == 0x9F))
	{
		byte1 = byte2;
		byte2 = importByte();
		++syncbytes;
	}

	if (syncbytes > 0) // TODO
		std::cout << "Syncbytes: " << syncbytes << std::endl;

	unsigned int databytes = importByte();
	if (databytes < 4)
		throw "Too short message";

	databytes -= 3;

	// capture time.
	timeval ctime;
	gettimeofday(&ctime,0);
	

	importBytes(lastMessage, databytes);

	//std::cout << " databytes: " << databytes << std::endl;

	framebits.resize(databytes*8);
	for (unsigned int k = 0; k < databytes; ++k)
	{
		framebits[k*8+0] = (lastMessage[k] & 0x80) != 0;
		framebits[k*8+1] = (lastMessage[k] & 0x40) != 0;
		framebits[k*8+2] = (lastMessage[k] & 0x20) != 0;
		framebits[k*8+3] = (lastMessage[k] & 0x10) != 0;
		framebits[k*8+4] = (lastMessage[k] & 0x08) != 0;
		framebits[k*8+5] = (lastMessage[k] & 0x04) != 0;
		framebits[k*8+6] = (lastMessage[k] & 0x02) != 0;
		framebits[k*8+7] = (lastMessage[k] & 0x01) != 0;
	}
	
	
	decodeFrame(framebits);

	frame.ctime_sec  = ctime.tv_sec;
	frame.ctime_usec = ctime.tv_usec;	

	return frame;
}






Canrec::BitBuffer::BitBuffer(const std::vector<bool>& framebits, unsigned int index)
	: framebits(framebits), index(index), eqbits(0), 
		lastbit(false), crcreg(0)
{  }

// TODO: check length of framebits
bool Canrec::BitBuffer::getbitBS()
{
	if (lastbit == framebits[index])
		++eqbits;
	else
		eqbits = 1; // first bit in new sequence

	const bool bit = framebits[index];
	lastbit = bit;
	++index;

	if (eqbits >= 5)	// next bit should be a stuff bit
	{
		if (framebits[index] == lastbit) // index incremented
			throw("Stuff bit error"); 
			// TODO: better exception
		lastbit = framebits[index];
		eqbits = 1;
		++index;
	}

	bool bit15 = ((crcreg & 0x4000) != 0);
	bool crcnext = (bit xor bit15);
	crcreg = ((crcreg*2) & 0x7fff);
	if (crcnext)
		crcreg ^= 0x4599;

	return bit;
}
 

/*if (bus.lastbit == bus.data(bus.ii))
    bus.eqseq = bus.eqseq + 1; %another bit..
else
    bus.eqseq = 1; %first bit in new sequence
end

bit = bus.data(bus.ii);
bus.lastbit = bit;
bus.ii = bus.ii + 1; % read bit and increment counter

if (bus.eqseq == 5) % next bit should be a stuff bit (counter incremented)
    if (bus.data(bus.ii) == bus.lastbit)
        throw(MException('candecode:stuffError', 'Stuff bit error'));
        %error('Stuff bit error!');
    end
    % else, set lastbit seqconuter ant discard bit
    bus.lastbit = bus.data(bus.ii);
    bus.eqseq = 1;
    bus.ii = bus.ii + 1;
    
    %disp('Removed stuff bit!');
end
*/




bool Canrec::BitBuffer::getbit()
{
	eqbits = 0;
	return framebits[index++];
}







void Canrec::decodeFrame(const std::vector<bool>& framebits)
{
	//Canframe frame; // in class now
	//frame.ctime_sec  = ctime_s;
	//frame.ctime_usec = ctime_us;

	BitBuffer bitbuf(framebits);

	// start of frame
	if (bitbuf.getbitBS() != 0)
		throw("No start of frame"); //TODO

	//% arbitration field and first bit of control field
	frame.ident = 0;
	for (unsigned int k = 0; k < 11; ++k)
		frame.ident = frame.ident*2 + bitbuf.getbitBS();

	const bool RTRSRRbit = bitbuf.getbitBS();
	frame.IDE = bitbuf.getbitBS();

	if (frame.IDE)
	{
		frame.extended = true;
		frame.SRR = RTRSRRbit;

		//if (! RTRSSRbit)
			//TODO SRR bit dominant

		for (unsigned int k = 0; k < 18; ++k)
			frame.ident = frame.ident*2 + bitbuf.getbitBS();

		frame.RTR = bitbuf.getbitBS();
		frame.r1 = bitbuf.getbitBS();

		// TODO if (r1)
		//        disp('r1 bit recessive');
	}
	else
	{
		frame.extended = false;
    
		frame.RTR = RTRSRRbit;

		for (unsigned int k = 0; k < 18; ++k)
			frame.ident = frame.ident*2;
	}

    	//% control field minus first bit
	frame.r0 = bitbuf.getbitBS();
	// TODO if (r0)
	//    disp('r0 bit recessive');

	frame.datalen = 0;
	for (unsigned int k = 0; k < 4; ++k)
		frame.datalen = frame.datalen*2 + bitbuf.getbitBS();

	if (frame.datalen > 8)
		throw("Frame data lenght too long"); // TODO

	//% data field
	if (frame.RTR == 0) //data frame
	{
		for (unsigned int di = 0; di < frame.datalen; ++di)
		{
			frame.data[di] = 0;
			for (unsigned int k = 0; k < 8; ++k)
				frame.data[di] = frame.data[di]*2
					+ bitbuf.getbitBS();
		}
	}
		

	//% CRC field
	frame.calcCRC = bitbuf.getCRC();
	frame.recCRC = 0;
	for (unsigned int k = 0; k < 15; ++k)
		frame.recCRC = frame.recCRC*2 + bitbuf.getbitBS();

	if (bitbuf.getbit() != 1)
		std::cout << "CRC delimeter error" << std::endl;// TODO CRC delimeter error

	//% ACK field
	frame.ACK = (bitbuf.getbit() == 0);

	if (bitbuf.getbit() != 1)
		std::cout << "ACK delimeter error" << std::endl;// TODO ACK delimeter error

	if (frame.calcCRC != frame.recCRC)	// TODO && some flag
	{
		//std::cout << "calc: " << frame.calcCRC
		//	<< " rec: " << frame.recCRC << std::endl;
		throw("CRC error");	// TODO
	}
}







// =================================================================
// ========================= CanrecDecfile =========================
// =================================================================


CanrecDecfile::CanrecDecfile(const char* filename, bool seektoend, bool intermessageDelay)
  : data(filename), intermessageDelay(intermessageDelay)
{ 
	if(data.fail()) throw CanFileReadError(); // could not open file..
	if (seektoend)
	{
		data.seekg(0, std::ios_base::end);
	
		//sync; find syncbyte and discard rest of message
		unsigned char syncbyte1 = 0;
		unsigned char syncbyte2 = 0;
		while ((syncbyte1 != 250) && (syncbyte2 != 111))
		{
			syncbyte1 = syncbyte2;
			syncbyte2 = waitForByte();
		}
		const unsigned int msgsize = Canframe::getSizeof() - 2;
		for (unsigned int i = 0; i < msgsize; ++i)
			waitForByte();
	}
}



const Canframe& CanrecDecfile::getNextFrame(bool wait)
{	// if not available yet? throw special exception 
	// TODO: non-wait read (may keep buffer in object and continue filling)

	if (intermessageDelay) usleep(1000);

	const unsigned int framesize = Canframe::getSizeof();
	unsigned int bytesread = 0;
	unsigned char buffer[framesize];
	char* bufferptr = reinterpret_cast<char*>(buffer);
	while (bytesread < framesize)
	{
		data.read(bufferptr+bytesread, 1);
		if (data.bad())
			throw CanFileReadError(); // Bad bit!
		if (!data.eof())
			++bytesread;
		else
		{
			data.clear();
			usleep(4000);
		}
	}

	frame = buffer;

	return frame;
}



unsigned char CanrecDecfile::waitForByte()
{
	unsigned char byte;
	char* byteptr = reinterpret_cast<char*>(&byte);
	unsigned int bytesread = 0;
	data.clear();
	while (bytesread < 1)
	{
		data.read(byteptr, 1);
		if (data.bad())
			throw CanFileReadError(); // Bad bit!
		if (!data.eof())
			++bytesread;
		else
		{
			data.clear();
			usleep(4000);
		}
	}
	return byte;
}





// =================================================================
// ========================= CanrecFile ============================
// =================================================================


CanrecFile::CanrecFile(const char* filename)
	: infile(filename)
{ };

unsigned char CanrecFile::importByte()
{
	unsigned char newb;
	infile.read((char*)&newb, 1);
	if (!infile)
		throw(CanFileReadError()); // TODO throw something else
	return newb;
}

void CanrecFile::importBytes(std::vector<unsigned char>& bytes, unsigned int count)
{
	bytes.resize(count);
	infile.read((char*) &bytes[0], count);
	return;
}
	


