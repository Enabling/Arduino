/*
 AllThingsTalk - SmartLiving.io Arduino library 
 Released into the public domain.

 Original author: Jan Bogaerts (2015-2016)
 */

#ifndef LORAPACKET_H_
#define LORAPACKET_H_

#include <Stream.h>

//this class represents the ATT cloud platform.
class LoRaPacket {
public:
	//create the object
	LoRaPacket();

	virtual ~LoRaPacket() {
	}
	//writes the packet content to the specified byte array. This must be at least 51 bytes long.
	//returns: the nr of bytes actually written to the array.
	virtual unsigned char write(unsigned char* result);

	//assigns the asset/container id to the packet
	void SetId(unsigned char id);

	//resets the content of the packet back to 0 ->> all data will be removed
	virtual void reset() = 0;

	virtual unsigned char getDataSize() = 0;


protected:
	//returns the frame type number for this lora packet. The default value is 0x40.
	//Inheritors that render other packet types can overwrite this.
	virtual unsigned char getFrameType();

	//calculate the checksum of the packet and return it.
	unsigned char calculateCheckSum(unsigned char* toSend, short len);
private:
	unsigned char contId = 0;
};

#endif  /* LORAPACKET_H_ */
