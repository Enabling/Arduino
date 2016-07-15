#ifndef ENCOPACKET_H_
#define ENCOPACKET_H_

/**
 * Sending sensor data onto the EnCo platform using LoRa.
 *
 * A byte array is created in the following form :
 * - 1 byte containing the index of the targeted stream variable
 * - (on text data an optional length byte is added)
 * - n bytes containing the actual value (depending on the data type)
 *
 *
 * TODO : Checksum / Packet ID / ....?
 */

#include "LoRaPacket.h"

// can we find a less memory hungry implementation std::vector ??
// possible solution is to redefine on real USE-CASE scenario's to acceptable level. (what is your biggest payload?!)
#define MAX_DATA_SIZE 64

class EnCoPacket: public LoRaPacket {

private:
	uint8_t _dataArray[MAX_DATA_SIZE];
	uint8_t _dataPointer = 0;

public:
	EnCoPacket();
	virtual ~EnCoPacket() {
	}

	virtual unsigned char write(unsigned char* result);

	unsigned char getDataSize();

	void reset();

	//Adding a boolean value to the data packet for the given stream related variable index.
	bool Add(uint8_t varIdx, bool value); // using 1 byte for the data field

	//Adding a char value to the data packet for the given stream related variable index.
	bool Add(uint8_t varIdx, char value); // using 1 byte for the data field

	//Adding a short value to the data packet for the given stream related variable index.
	bool Add(uint8_t varIdx, short value); // using 2 bytes for the data field

	//Adding an integer value to the data packet for the given stream related variable index.
	bool Add(uint8_t varIdx, int value); // using 4 bytes for the data field

	//Adding a String value to the data packet for the given stream related variable index.
	bool Add(uint8_t varIdx, String value); // using value.Length() bytes for the data field

	//Adding a String value to the data packet for the given stream related variable index.
	bool Add(uint8_t varIdx, const char* value, const uint8_t len); // using value.Length() bytes for the data field

	//Adding a float value to the data packet for the given stream related variable index.
	bool Add(uint8_t varIdx, float value); // using 4 bytes for the data field

};
#endif /* ENCOPACKET_H_ */
