#include "EnCoPacket.h"

EnCoPacket::EnCoPacket() {
}

unsigned char EnCoPacket::write(unsigned char* result) {
	memcpy(result, _dataArray, _dataPointer);
	return _dataPointer;
}

unsigned char EnCoPacket::getDataSize() {
	return _dataPointer;
}

void EnCoPacket::reset() {
	_dataPointer = 0;
}

bool EnCoPacket::Add(uint8_t varIdx, bool value) {
	if (_dataPointer >= MAX_DATA_SIZE - 1 /* sizeof(bool) */)
		return false;

	_dataArray[_dataPointer++] = varIdx;
	_dataArray[_dataPointer++] = value ? 0x01 : 0x00;
	return true;
}

bool EnCoPacket::Add(uint8_t varIdx, char value) {
	if (_dataPointer >= MAX_DATA_SIZE - 1 /*sizeof(char)*/)
		return false;

	_dataArray[_dataPointer++] = varIdx;
	_dataArray[_dataPointer++] = value;
	return true;
}

bool EnCoPacket::Add(uint8_t varIdx, short value) {
	if (_dataPointer >= MAX_DATA_SIZE - 2 /*sizeof(short)*/)
		return false;

	_dataArray[_dataPointer++] = varIdx;
	memcpy(_dataArray + _dataPointer, &value, 2);
	_dataPointer += 2;
	return true;
}

bool EnCoPacket::Add(uint8_t varIdx, int value) {
	if (_dataPointer >= MAX_DATA_SIZE - 4 /*sizeof(int)*/)
		return false;

	_dataArray[_dataPointer++] = varIdx;
	// on Arduino int is 2 bytes
	memcpy(_dataArray + _dataPointer, &value, 2);
	_dataPointer += 2;
	// LSB -> 2 x '0x00' after to fill 4 bytes
	_dataArray[_dataPointer++] = 0;
	_dataArray[_dataPointer++] = 0;
	return true;
}

bool EnCoPacket::Add(uint8_t varIdx, String value) {
	return Add(varIdx, value.c_str(), value.length());
}

bool EnCoPacket::Add(uint8_t varIdx, const char* value,
		const uint8_t len) {
	if (_dataPointer >= MAX_DATA_SIZE - len)
		return false;

	_dataArray[_dataPointer++] = varIdx;
	_dataArray[_dataPointer++] = len;// Add the length of the text to the data stream
	memcpy(_dataArray + _dataPointer, value, len);
	_dataPointer += len;
	return true;
}

bool EnCoPacket::Add(uint8_t varIdx, float value) {
	if (_dataPointer >= MAX_DATA_SIZE - 4 /*sizeof(float)*/)
		return false;

	_dataArray[_dataPointer++] = varIdx;
	memcpy(_dataArray + _dataPointer, &value, 4);
	_dataPointer += 4;
	return true;
}
