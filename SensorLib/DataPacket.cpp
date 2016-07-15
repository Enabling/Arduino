#include "DataPacket.h"

//create the object
DataPacket::DataPacket() {
}

DataPacket::~DataPacket() {
  if (binaryPayload != NULL) {
    free(binaryPayload);
  }
}

unsigned char DataPacket::write(unsigned char* result) {
	unsigned char curPos = LoRaPacket::write(result);
	// BINARY PAYLOAD ??
	if (binaryPayloadLength > 0) {
		memcpy(result + curPos, binaryPayload, binaryPayloadLength);
		curPos += binaryPayloadLength;
	} else {
		result[curPos] = boolValues;
		curPos++;
		if (nrInts > 0) {
			result[curPos++] = nrInts;
			for (int i = 0; i < nrInts; i++) {
				short len = sizeof intValues[i];
				memcpy(result + curPos, &intValues[i], len);
				curPos += len;
			}
		} else if (nrFloats > 0 || stringPos > 0) {
			result[curPos++] = 0;
		}
		if (nrFloats > 0) {
			result[curPos++] = nrFloats;
			for (int i = 0; i < nrFloats; i++) {
				short len = sizeof floatValues[i];
				memcpy(result + curPos, &floatValues[i], len);
				curPos += len;
			}
		} else if (stringPos > 0) {
			result[curPos++] = 0;
		}
		if (stringPos > 0) {
			memcpy(result + curPos, stringValues, stringPos);
			curPos += stringPos;
		}
	}
	short packetLen = curPos - 3;				//write the packet length
	memcpy(result + 1,
			static_cast<const char*>(static_cast<const void*>(&packetLen)),
			sizeof(short));

	result[curPos] = calculateCheckSum(result + 3, curPos - 3);	//add the checksum
	curPos++;

	return curPos;
}

bool DataPacket::Add(bool value) {
	if (nrBools >= 8 || binaryPayloadLength > 0)
		return false;

	unsigned char val = value == true ? 1 : 0;
	val = val << nrBools;
	nrBools++;
	boolValues |= val;
	return true;
}

bool DataPacket::Add(int value) {
	if (nrInts >= 16 || binaryPayloadLength > 0)
		return false;

	intValues[nrInts++] = value;
	return true;
}

bool DataPacket::Add(String value) {
	int len = value.length();
	if (stringPos + len >= 48 || binaryPayloadLength > 0)
		return false;

	value.toCharArray(stringValues + stringPos, len);
	stringPos += len;
	return true;
}

bool DataPacket::Add(float value) {
	if (nrFloats >= 16 || binaryPayloadLength > 0)
		return false;

	floatValues[nrFloats++] = value;
	return true;
}

bool DataPacket::Add(uint8_t* value, uint8_t dataLen) {
	if (stringPos + nrFloats + nrInts + nrBools + boolValues > 0) {
		return false;
	}
	if (binaryPayload != NULL) {
		free(binaryPayload);
		binaryPayloadLength = 0;
	}
	binaryPayload = (uint8_t*) malloc(dataLen);
	if (binaryPayload) {
		memcpy(binaryPayload, value, dataLen);
		binaryPayloadLength = dataLen;
		return true;
	} // else ERROR !!!!
	return false;
}

void DataPacket::reset() {
	stringPos = 0;
	nrFloats = 0;
	nrInts = 0;
	nrBools = 0;
	boolValues = 0;
	if (binaryPayload != NULL) {
		free(binaryPayload);
		binaryPayloadLength = 0;
	}
}

unsigned char DataPacket::getDataSize() {
	unsigned char cnt = 5;	// LoRaPacket header size;
	if (binaryPayloadLength > 0)
		return cnt + binaryPayloadLength + 1;

	cnt += 1;	// bool value always written (1 byte)
	if (nrInts > 0) {
		cnt += 1;
		cnt += nrInts * sizeof intValues[0];
	} else if (nrFloats > 0 || stringPos > 0) {
		cnt += 1;
	}
	if (nrFloats > 0) {
		cnt += 1;
		cnt += nrFloats * sizeof floatValues[0];
	} else if (stringPos > 0) {
		cnt += 1;
	}
	cnt += stringPos;
	cnt += 1; // crc
	return cnt;
}
