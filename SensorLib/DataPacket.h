/*
 AllThingsTalk - SmartLiving.io Arduino library 
 Released into the public domain.

 Original author: Jan Bogaerts (2015-2016)
 */

#ifndef DATAPACKET_H_
#define DATAPACKET_H_

#include <string.h>
#include <Stream.h>
#include "LoRaPacket.h"

//this class represents the ATT cloud platform.
class DataPacket: public LoRaPacket {
public:
	DataPacket();
	~DataPacket();
  
	//writes the packet content to the specified byte array.
	//This must be at least 51 bytes long.
	//returns: the n� of bytes actually written to the array.
	virtual unsigned char write(unsigned char* result);

	//loads a boolean data value into the data packet that is being prepared to send to the
	//cloud server.
	bool Add(bool value);

	//loads an integer data value into the data packet that is being prepared to send to the
	//cloud server.
	bool Add(int value);

	//loads a string data value into the data packet that is being prepared to send to the
	//cloud server.
	bool Add(String value);

	//loads a float data value into the data packet that is being prepared to send to the
	//cloud server.
	bool Add(float value);

	bool Add(uint8_t* value, uint8_t dataLen);

	//resets the content of the packet back to 0 ->> all data will be removed
	void reset();

	unsigned char getDataSize();

private:
	//define the stores for all the values for this packet
	char stringValues[48];
	int intValues[16];
	float floatValues[16];
	uint8_t* binaryPayload = NULL;

	unsigned char boolValues = 0;
	unsigned char stringPos = 0;
	unsigned char nrInts = 0;
	unsigned char nrFloats = 0;
	unsigned char nrBools = 0;
	uint8_t binaryPayloadLength = 0;
};

#endif /* DATAPACKET_H_ */
