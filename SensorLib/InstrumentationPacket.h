/*
 AllThingsTalk - SmartLiving.io Arduino library 
 Released into the public domain.

 Original author: Jan Bogaerts (2015-2016)
 */

#ifndef INSTRUMENTATIONPACKET_H_
#define INSTRUMENTATIONPACKET_H_

#include <string.h>
#include <Stream.h>

#include "instrumentationParamEnum.h"
#include "LoRaPacket.h"

#define INST_DATA_SiZE 8

//this class represents the ATT cloud platform.
class InstrumentationPacket: public LoRaPacket {
public:
	//create the object
	InstrumentationPacket();

	virtual ~InstrumentationPacket() {
	}

	//writes the packet content to the specified byte array. This must be at least 51 bytes long.
	//returns: the nr of bytes actually written to the array.
	virtual unsigned char write(unsigned char* result);

	//resets the content of the packet back to 0 ->> all data will be removed
	void reset();

	bool setParam(instrumentationParam param, int value);
protected:
	//returns the frame type number for this lora packet. The default value is 0x40. Inheritors that render other packet types can overwrite this.
	unsigned char getFrameType();
private:
	unsigned char _data[INST_DATA_SiZE];
};

#endif /* INSTRUMENTATIONPACKET_H_ */
