#include "SHT3X.h"

SHT3X::SHT3X(uint8_t address)
{
	Wire.begin();
	_address=address;
}

byte SHT3X::get()
{
	unsigned int data[6];
	Wire.beginTransmission(_address);         // Start I2C Transmission
	Wire.write(0x2C); Wire.write(0x06);       // Send measurement command
	if(Wire.endTransmission()!=0) {return 1;} // Stop I2C transmission if no ACK from SHT
	delay(500);
	Wire.requestFrom(_address, 6);            // Request 6 bytes of data
	for(int i=0;i<6;i++) { data[i]=Wire.read(); };  // Read 6 bytes of data// cTemp msb, cTemp lsb, cTemp crc, humidity msb, humidity lsb, humidity crc
	delay(50);
	if(Wire.available()!=0) {return 2;}
	cTemp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;// Convert the data
	fTemp = (cTemp * 1.8) + 32;
	humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);
	return 0;
}
