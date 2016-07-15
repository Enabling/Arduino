#ifndef LORAMODEMMICROCHIP_H_
#define LORAMODEMMICROCHIP_H_

#include "LoRaModem.h"
#include "LoRaPacket.h"
#include "instrumentationParamEnum.h"
#include "Utils.h"
#include "StringLiterals.h"

//52 <-> 222 !!
#define DEFAULT_PAYLOAD_SIZE 255
#define PORT 1
#define DEFAULT_INPUT_BUFFER_SIZE 96
#define DEFAULT_RECEIVED_PAYLOAD_BUFFER_SIZE 64
#define DEFAULT_TIMEOUT 120
#define RECEIVE_TIMEOUT 60000
#define MAX_SEND_RETRIES 4

enum MacTransmitErrorCodes {
	NoError = 0, NoResponse = 1, Timeout = 2, TransmissionFailure = 3
};

class LoRaModemMicrochip: public LoRaModem {
public:
	//create the object
	LoRaModemMicrochip(Stream* stream, Stream* monitor = NULL);
	// Returns the required baudrate for the device
	unsigned long getDefaultBaudRate();
	//stop the modem.
	bool stop();
	//set the modem in LoRaWan mode (vs private networks)
	//adr = adaptive data rate. true= use, false = none adaptive data rate
	bool setLoRaWan(bool adr = true);
	//assign a device address to the modem
	//devAddress must be 4 bytes long
	bool setDevAddress(unsigned char* devAddress);
	//set the app session key for the modem communication
	//app session key must be 16 bytes long
	bool setAppKey(unsigned char* appKey);
	//set the network session key
	//network session key must be 16 bytes long
	bool setNWKSKey(unsigned char* nwksKey);
	//start the modem , returns true if successful
	bool start();

	//process any incoming packets from the modem
	void processIncoming();
	//extract the specified instrumentation parameter from the modem and return the value
	int getParam(instrumentationParam param);
	//returns the id number of the modem type. See the container definition for the instrumentation container to see more details.
	int getModemId();
	//prints all configuration params (radio and mac) to the monitor
	void printModemConfig();

protected:
	//send a data packet to the server
	//bool send(LoRaPacket* packet, bool ack = true);

	bool send(Packet* packet, bool ack = true);

private:
	unsigned long _sendBackOffMs;
	unsigned long _lastSendMs;

	char inputBuffer[DEFAULT_INPUT_BUFFER_SIZE + 1];
	char receivedPayloadBuffer[DEFAULT_RECEIVED_PAYLOAD_BUFFER_SIZE + 1];
	unsigned char lookupMacTransmitError(const char* error);
	unsigned char onMacRX();
	unsigned short readLn(char* buf, unsigned short bufferSize,
			unsigned short start = 0);
	unsigned short readLn() {
		return readLn(this->inputBuffer, DEFAULT_INPUT_BUFFER_SIZE);
	}
	bool expectOK();

	bool expectString(const char* str,
			unsigned short timeout = DEFAULT_TIMEOUT);

	bool setMacParam(const char* paramName, const unsigned char* paramValue,
			unsigned short size);

	bool setMacParam(const char* paramName,
			unsigned char paramValue);

	bool setMacParam(const char* paramName,
			const char* paramValue);

	unsigned char macTransmit(const char* type, const unsigned char* payload,
			unsigned char size);

	//convert the text value for spreading factor into a number between 0 and 6
	int sfToIndex(char* value);

	char* getParam(const char* baseName, const char* paramName,
			unsigned short timeout = DEFAULT_TIMEOUT);
	char* getParam(const char* baseName, const char* paramName,
			const char* paramValue, unsigned short timeout = DEFAULT_TIMEOUT);bool sendSysCmd(
			const char* paramName);

protected:

public:
	bool storeLastSendMs(unsigned long timeToSend);

	unsigned long getLastSendMs();

	bool storeSendBackOffMs(unsigned long timeToSend);

	unsigned long getSendBackOffMs();

	// dangerous methods in their current form !!
	char* getSpreadingFactor() {// return numerical value ..
		return getParam("radio", "sf");
	}
	char* getAdaptiveDataRateStatus() {// boolean
		return getParam("mac", "adr");
	}
	char* getDataRate() {// numerical
		return getParam("mac", "dr");
	}
	char* getGatewayCount() {// numerical
		return getParam("mac", "gwnb");
	}
	char* getFWVersion() {// dump it or leave it ...
		return getParam("sys", "ver");
	}
	bool setModemLedOn() {
		return sendSysCmd("sys set pindig GPIO0 1");
	}
	bool setModemLedOff() {
		return sendSysCmd("sys set pindig GPIO0 0");
	}

	template<class T> int write_EEPROM(int address, const T& value) {
		const byte* p = (const byte*) (const void*) &value;
		int i;
		for (i = 0; i < sizeof(value); i++, p++) {
			_stream->print("sys set nvm ");
			_stream->print(address + i);   // Make proper HEX !!!
			_stream->print(" ");
			if (*p < 0x10) {
				_stream->print(
						static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(*p))));
			} else {
				_stream->print(
						static_cast<char>(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(*p))));
				_stream->print(
						static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(*p))));
			}
			_stream->print(CRLF);
			if (!expectOK()) {
				return false;
			}
		}
		return i;
	}

	template<class T> int read_EEPROM(int address, T& value) {
		byte* p = (byte*) (void*) &value;
		int i;
		int numread;
		for (i = 0; i < sizeof(value); i++, p++) { // perhaps also take some sort of time-out into account
			_stream->print("sys get nvm ");
			_stream->print(address + i);
			_stream->print(CRLF);

			while ((numread = readLn()) == 0) {
			}
			if (numread <= 2) {
				*p = HEX_PAIR_TO_BYTE('0', this->inputBuffer[0]);
			} else {
				*p = HEX_PAIR_TO_BYTE(this->inputBuffer[0],
						this->inputBuffer[1]);
			}
		}
		return i;
	}

};

#endif /*LORAMODEMMICROCHIP_H_*/
