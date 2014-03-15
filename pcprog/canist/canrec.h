#ifndef CANREC_H
#define CANREC_H

#include "canframe.h"

#include <vector>
#include <fstream>
	
//TODO: one message read exception (expected to read next message ok)
// one more terminal exception

class CanException
{};

class CanTempOutOfData : public CanException
{};

class CanFileReadError : public CanException
{};


class Canrec
{
public:
	virtual const Canframe& getNextFrame(); //implemented, uses importByte and importBytes


	protected:
		virtual unsigned char importByte() = 0;
		virtual void importBytes(std::vector<unsigned char>& bytes, unsigned int count) = 0;


	Canframe frame; ///< last frame, valid until next get frame call

private:


	class BitBuffer
	{
	public:
		BitBuffer(const std::vector<bool>& framebits, unsigned int index = 0);
		bool getbitBS();
		bool getbit();
		unsigned int getCRC() {return crcreg; }; // uint16
	private:
		const 		std::vector<bool>& framebits;
		unsigned int 	index;
		unsigned int 	eqbits;
		bool 		lastbit;
		unsigned int	crcreg; // uint16
	};

	std::vector<unsigned char> lastMessage;
	std::vector<bool> framebits;

	/// decode frame in framebits vector
	void decodeFrame(const std::vector<bool>& framebits);
};






class CanrecDecfile : public Canrec
{
public:	 // TODO: implement nowait.
	CanrecDecfile(const char* filename, bool seektoend = true, bool intermessageDelay = false);
	inline virtual const Canframe& getNextFrame() {return getNextFrame(true);};
	virtual const Canframe& getNextFrame(bool wait);

private:
	virtual unsigned char importByte() {return 0; };
	virtual void importBytes(std::vector<unsigned char>& bytes, unsigned int count)
		{return; };

	bool intermessageDelay; // add sleep between messages (normally for playback of prerecorded files)

	std::ifstream data; // throw CanTempOutOfData when eof and nowait..
	unsigned char waitForByte(); // wait for and read byte
};






class CanrecFile : public Canrec // reading from raw dump file
{
public:
	CanrecFile(const char* filename);

protected: 
	virtual unsigned char importByte();
	virtual void importBytes(std::vector<unsigned char>& bytes, unsigned int count);

private:
	std::ifstream infile;	
};




#endif
