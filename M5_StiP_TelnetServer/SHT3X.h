#ifndef __SHT3X_H__
#define __SHT3X_H__

#include "Arduino.h"
#include "Wire.h"

class SHT3X
{
	public:
		SHT3X(uint8_t address=0x44);
		byte get(void);
		float cTemp=0;
		float fTemp=0;
		float humidity=0;

	private:
		uint8_t _address;
};

#endif
