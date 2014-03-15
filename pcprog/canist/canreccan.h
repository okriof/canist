#ifndef CANRECCAN_H
#define CANRECCAN_H

#include "canrec.h"
#include "ftd2xx.h"


class CanrecCan : public Canrec
{
	public:
		CanrecCan();
		~CanrecCan();

	protected: 
		virtual unsigned char importByte();
		virtual void importBytes(std::vector<unsigned char>& bytes, unsigned int count);
	
	private:
	FT_HANDLE ftHandle;

};





#endif
