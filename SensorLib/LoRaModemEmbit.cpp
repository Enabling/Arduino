/*
 EmbitLoRaModem.cpp - SmartLiving.io LoRa Arduino library 
 */

#include <arduino.h>  			//required for the millis() function

#include "LoRaModemEmbit.h"
#include "Utils.h"

#define PORT 0x01
#define SERIAL_BAUD 9600

#define PACKET_TIME_OUT 45000

unsigned char CMD_STOP[1] = { 0x30 };
unsigned char CMD_LORA_PRVNET[2] = { 0x25, 0xA0 };
unsigned char CMD_LORA_PRVNET_NO_ADR[2] = { 0x25, 0x80 };
unsigned char CMD_DEV_ADDR[1] = { 0x21 };
unsigned char CMD_APPSKEY[2] = { 0x26, 0x11 };
unsigned char CMD_NWKSKEY[2] = { 0x26, 0x10 };
unsigned char CMD_START[1] = { 0x31 };
unsigned char CMD_SEND_PREFIX[4] = { 0x50, 0x0C, 0x00, PORT };
unsigned char CMD_SEND_PREFIX_NO_ACK[4] = { 0x50, 0x08, 0x00, PORT };

unsigned char sendBuffer[52];

LoRaModemEmbit::LoRaModemEmbit(Stream* stream, Stream* monitor) {
	_stream = stream;
	_monitor = monitor;
}

unsigned long LoRaModemEmbit::getDefaultBaudRate() {
	return SERIAL_BAUD;
}


bool LoRaModemEmbit::stop() {
#ifdef FULLDEBUG
	PRINTLNF("Sending the network stop command");
#endif
	sendPacket(CMD_STOP, sizeof(CMD_STOP));
	return readPacket();
}

bool LoRaModemEmbit::setLoRaWan(bool adr) {
#ifdef FULLDEBUG
	PRINTLNF("Setting the network preferences to LoRaWAN private network");
#endif
	if (adr == true)
		sendPacket(CMD_LORA_PRVNET, sizeof(CMD_LORA_PRVNET));
	else
		sendPacket(CMD_LORA_PRVNET_NO_ADR, sizeof(CMD_LORA_PRVNET_NO_ADR));
	return readPacket();
}

bool LoRaModemEmbit::setDevAddress(unsigned char* devAddress) {
#ifdef FULLDEBUG
	PRINTLNF("Setting the DevAddr");
#endif
	sendPacket(CMD_DEV_ADDR, sizeof(CMD_DEV_ADDR), devAddress, 4);
	return readPacket();
}

bool LoRaModemEmbit::setAppKey(unsigned char* appKey) {
#ifdef FULLDEBUG
	PRINTLNF("Setting the AppSKey");
#endif
	sendPacket(CMD_APPSKEY, sizeof(CMD_APPSKEY), appKey, 16);
	return readPacket();
}

bool LoRaModemEmbit::setNWKSKey(unsigned char* nwksKey) {
#ifdef FULLDEBUG
	PRINTLNF("Setting the NwkSKey");
#endif
	sendPacket(CMD_NWKSKEY, sizeof(CMD_NWKSKEY), nwksKey, 16);
	return readPacket();
}

bool LoRaModemEmbit::start() {
#ifdef FULLDEBUG
	PRINTLNF("Sending the netowrk start command");
#endif
	sendPacket(CMD_START, sizeof(CMD_START));
	readPacket();

	//TODO: check result of readPacket and return actual success or not.
	return true;
}

void LoRaModemEmbit::sendPacket(unsigned char* data, uint16_t length) {
#ifdef FULLDEBUG
	PRINTF("Sending: ");
#endif
	uint16_t packetLength = length + 3;
	unsigned char* len = (unsigned char*) &packetLength;
	unsigned char CRC = len[1] + len[0];

	//Little Endian vs big endian
	sendByte(len[1]);
	sendByte(len[0]);

	for (size_t i = 0; i < length; i++) {
		CRC += data[i];
		sendByte(data[i]);
	}

	sendByte(CRC);
	PRINTLN();
}

void LoRaModemEmbit::sendPacket(unsigned char* data, uint16_t length,
		unsigned char* data2, uint16_t length2) {
#ifdef FULLDEBUG
	PRINTF("Sending: ");
#endif
	uint16_t packetLength = length + length2 + 3;
	unsigned char* len = (unsigned char*) &packetLength;
	unsigned char CRC = len[1] + len[0];

	//Little Endian vs big endian
	sendByte(len[1]);
	sendByte(len[0]);

	for (size_t i = 0; i < length; i++) {
		CRC += data[i];
		sendByte(data[i]);
	}

	for (size_t i = 0; i < length2; i++) {
		CRC += data2[i];
		sendByte(data2[i]);
	}

	sendByte(CRC);
#ifdef FULLDEBUG
	PRINTLN();
#endif
}

void LoRaModemEmbit::sendByte(unsigned char data) {
	_stream->write(data);
#ifdef FULLDEBUG
	printHex(&data,1,false);
#endif
}

bool LoRaModemEmbit::readPacket() {
	uint32_t maxTS = millis() + PACKET_TIME_OUT;
	uint16_t length = 4;
	unsigned char firstByte = 0;

#ifdef FULLDEBUG
	PRINTF("Receiving: ");
#endif
	size_t i = 0;
	while ((maxTS > millis()) && (i < length)) {
		while ((!_stream->available()) && (maxTS > millis()))
			;

		if (_stream->available()) {
			unsigned char value = _stream->read();
			if (i == 0) {
				firstByte = value;
			} else if (i == 1) {
				length = firstByte * 256 + value;
			}
			printHex(&value,1,false);
			i++;
		}
	}

	if (i < length) {
#ifdef FULLDEBUG
		PRINTF("Timeout");
#endif
		return false;
	}
#ifdef FULLDEBUG
	PRINTLN();
#endif
	return true;
}

unsigned char LoRaModemEmbit::readPacket(unsigned char index) {
	uint32_t maxTS = millis() + PACKET_TIME_OUT;
	uint16_t length = 4;
	unsigned char firstByte = 0;
	unsigned char result = 0;

#ifdef FULLDEBUG
	PRINTF("Receiving: ");
#endif

	size_t i = 0;
	while ((maxTS > millis()) && (i < length)) {
		while ((!_stream->available()) && (maxTS > millis()))
			;

		if (_stream->available()) {
			unsigned char value = _stream->read();
			if (i == 0)
				firstByte = value;
			else if (i == 1)
				length = firstByte * 256 + value;
			else if (i == index)
				result = value;
			printHex(&value,1,false);
			i++;
		}
	}

#ifdef FULLDEBUG
	if (i < length) {
		PRINTF("Timeout");
	}
	PRINTLN();
#endif
	return result;
}

//process any incoming packets from the modem
void LoRaModemEmbit::processIncoming() {
	readPacket();
}

//extract the specified instrumentation parameter from the modem and return the value
int LoRaModemEmbit::getParam(instrumentationParam param) {
	// TODO : implement
	PRINTLNF("to be implemented: GetParam for embit modems");
	return -1;
}
//returns the id number of the modem type. See the container definition for the instrumentation container to see more details.
int LoRaModemEmbit::getModemId() {
	return 2;
}
