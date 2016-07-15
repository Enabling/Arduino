/*
 MicrochipLoRaModem.cpp - SmartLiving.io LoRa Arduino library
 */

#include "LoRaModemMicrochip.h"

#include "StringLiterals.h"
#include "Utils.h"
#include <Arduino.h>

//#define FULLDEBUG

#define PACKET_TIME_OUT 45000

// start of EEPROM memory
#define EEPROM_ADDR_LAST_SEND 300
#define EEPROM_ADDR_TOA_BUDGET 304
#define EEPROM_ADDR_BACKOFF_SEND 308

typedef struct StringEnumPair {
	const char* stringValue;
	uint8_t enumValue;
} StringEnumPair;

unsigned char microchipSendBuffer[DEFAULT_PAYLOAD_SIZE];

LoRaModemMicrochip::LoRaModemMicrochip(Stream* stream, Stream *monitor) {
	_stream = stream;
	_monitor = monitor;
	//storeLastSendMs(0);
	//storeSendBackOffMs(0);
}

bool LoRaModemMicrochip::stop() {
#ifdef FULLDEBUG
	PRINTLNF("[resetDevice]");
#endif

	_stream->print(STR_CMD_RESET);
	_stream->print(CRLF);

	return expectString(STR_DEVICE_TYPE);
}

bool LoRaModemMicrochip::setLoRaWan(bool adr) {
	//lorawan should be on by default (no private supported)
	return setMacParam(STR_ADR, BOOL_TO_ONOFF(adr)); //set to adaptive variable rate transmission
}

unsigned long LoRaModemMicrochip::getDefaultBaudRate() {
	return 57600;
}

bool LoRaModemMicrochip::setDevAddress(unsigned char* devAddress) {
#ifdef FULLDEBUG
	PRINTLNF("Setting the DevAddr");
#endif	
	return setMacParam(STR_DEV_ADDR, devAddress, 4);
}

bool LoRaModemMicrochip::setAppKey(unsigned char* appKey) {
#ifdef FULLDEBUG
	PRINTLNF("Setting the AppSKey");
#endif
	return setMacParam(STR_APP_SESSION_KEY, appKey, 16);
}

bool LoRaModemMicrochip::setNWKSKey(unsigned char* nwksKey) {
#ifdef FULLDEBUG
	PRINTLNF("Setting the NwkSKey");
#endif
	return setMacParam(STR_NETWORK_SESSION_KEY, nwksKey, 16);
}

bool LoRaModemMicrochip::start() {
#ifdef FULLDEBUG
	PRINTLNF("Sending the network start commands");
#endif
	_stream->print(STR_CMD_JOIN);
	_stream->print(STR_ABP);
	_stream->print(CRLF);

#ifdef FULLDEBUG
	PRINT(STR_CMD_JOIN);
	PRINT(STR_ABP);
	PRINT(CRLF);
#endif
	bool joinresult = expectOK() && expectString(STR_ACCEPTED);

	storeSendBackOffMs(0);

	return joinresult;
}

bool LoRaModemMicrochip::send(Packet* packet, bool ack) {
	unsigned char length = packet->sizeOfData;

//#ifdef FULLDEBUG
	PRINTF("Sending (");
  PRINT(length);
  PRINTF(") : ");
	printHex(packet->data,length,true);
//#endif

	unsigned char result;
	if (ack == true) {
		if (!setMacParam(STR_RETRIES, MAX_SEND_RETRIES)) { // not a fatal error -just show a debug message
#ifdef FULLDEBUG
				PRINTLNF("[send] Non-fatal : setting number of retries failed.");
#endif
		}
		result = macTransmit(STR_CONFIRMED, packet->data, length) == NoError;
	} else {
		result = macTransmit(STR_UNCONFIRMED, packet->data, length) == NoError;
	}
#ifdef FULLDEBUG
	if(result) {
		PRINTLNF("Packet SENT!");
	}
	else {
		PRINTLNF("Packet NOT sent!");
	}
#endif

	return result;
}

unsigned char LoRaModemMicrochip::macTransmit(const char* type,
		const unsigned char* payload, unsigned char size) {
	_stream->print(STR_CMD_MAC_TX);
	_stream->print(type);
	_stream->print(_messagePort);
	_stream->print(" ");

#ifdef FULLDEBUG
	PRINT(STR_CMD_MAC_TX);
	PRINT(type);
	PRINT(_messagePort);
	PRINTF(" ");
#endif

	for (int i = 0; i < size; ++i) {
		_stream->print(
				static_cast<char>(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(payload[i]))));
		_stream->print(
				static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(payload[i]))));
#ifdef FULLDEBUG
		PRINT(static_cast<char>(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(payload[i]))));
		PRINT(static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(payload[i]))));
#endif
	}
	_stream->print(CRLF);

#ifdef FULLDEBUG
	PRINTF(CRLF);
#endif

	// TODO lookup error
	if (!expectOK())
		return TransmissionFailure;

#ifdef FULLDEBUG
	PRINTLNF("Waiting for server response");
#endif
	unsigned long timeout = millis() + RECEIVE_TIMEOUT;
	while (millis() < timeout) {
#ifdef FULLDEBUG
		PRINTF(".");
#endif
		if (readLn() > 0) {
#ifdef FULLDEBUG
			PRINTF(".(");
			PRINT(this->inputBuffer);
			PRINTF(")");
#endif

			if (strstr(this->inputBuffer, " ") != NULL) // to avoid double delimiter search 
			{
				// there is a splittable line -only case known is mac_rx
#ifdef FULLDEBUG
				PRINTLNF("Splittable response found");
#endif
				onMacRX();
				return NoError; // TODO remove
			} else if (strstr(this->inputBuffer, STR_RESULT_MAC_TX_OK)) {
				// done
#ifdef FULLDEBUG
				PRINTLNF("Received mac_tx_ok");
#endif
				return NoError;
			} else {
#ifdef FULLDEBUG
				PRINTLNF("Some other string received (error)");
#endif
				return lookupMacTransmitError(this->inputBuffer);
			}
		}
	}
#ifdef FULLDEBUG
	PRINTLNF("Timed-out on response!");
#endif
	return Timeout;
}

uint8_t LoRaModemMicrochip::lookupMacTransmitError(const char* error) {
#ifdef FULLDEBUG
	PRINTF("[lookupMacTransmitError]: ");
	PRINTLN(error);
#endif
	if (error[0] == 0) {
#ifdef FULLDEBUG
		PRINTLNF("[lookupMacTransmitError]: The string is empty!");
		return NoResponse;
#endif
	}

	StringEnumPair errorTable[] = { { STR_RESULT_OK, NoError }, {
	STR_RESULT_INVALID_PARAM, TransmissionFailure }, {
	STR_RESULT_NOT_JOINED, TransmissionFailure }, {
	STR_RESULT_NO_FREE_CHANNEL, TransmissionFailure }, {
	STR_RESULT_SILENT, TransmissionFailure }, {
	STR_RESULT_FRAME_COUNTER_ERROR, TransmissionFailure }, {
	STR_RESULT_BUSY, TransmissionFailure }, { STR_RESULT_MAC_PAUSED,
			TransmissionFailure }, { STR_RESULT_INVALID_DATA_LEN,
			TransmissionFailure },
			{ STR_RESULT_MAC_ERROR, TransmissionFailure }, {
			STR_RESULT_MAC_TX_OK, NoError } };

	for (StringEnumPair* p = errorTable; p->stringValue != NULL; ++p) {
		if (strcmp(p->stringValue, error) == 0) {
#ifdef FULLDEBUG
			PRINTF("[lookupMacTransmitError]: found ");
			PRINTLN(p->enumValue);
#endif
			return p->enumValue;
		}
	}

#ifdef FULLDEBUG
	PRINTLNF("[lookupMacTransmitError]: not found!");
#endif
	return NoResponse;
}

// waits for string, if str is found returns ok, if other string is found returns false, if timeout returns false
bool LoRaModemMicrochip::expectString(const char* str, unsigned short timeout) {
#ifdef FULLDEBUG
	PRINTF("[expectString] expecting ");
	PRINTLN(str);
#endif

	unsigned long start = millis();
	while (millis() < start + timeout) {
#ifdef FULLDEBUG
		PRINTF(".");
#endif

		if (readLn() > 0) {
#ifdef FULLDEBUG
			PRINTF("(");
			PRINT(this->inputBuffer);
			PRINTF(")");
#endif

			// TODO make more strict?
			if (strstr(this->inputBuffer, str) != NULL) {
#ifdef FULLDEBUG
				PRINTLNF(" found a match!");
#endif
				return true;
			}
			return false;
		}
	}

	return false;
}

unsigned short LoRaModemMicrochip::readLn(char* buffer, unsigned short size,
		unsigned short start) {
	int len = _stream->readBytesUntil('\n', buffer + start, size);
	if (len > 0)
		this->inputBuffer[start + len - 1] = 0; // bytes until \n always end with \r, so get rid of it (-1)
	else
		this->inputBuffer[start] = 0;

	return len;
}

bool LoRaModemMicrochip::expectOK() {
	return expectString(STR_RESULT_OK);
}

// paramName should include the trailing space
bool LoRaModemMicrochip::setMacParam(const char* paramName,
		const unsigned char* paramValue, unsigned short size) {
#ifdef FULLDEBUG
	PRINTF("[setMacParam] ");
	PRINT(paramName);
	PRINTF("= [array]");
#endif

	_stream->print(STR_CMD_SET);
	_stream->print(paramName);
#ifdef FULLDEBUG
	PRINT(STR_CMD_SET);
	PRINT(paramName);
#endif

	for (unsigned short i = 0; i < size; ++i) {
		_stream->print(
				static_cast<char>(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(paramValue[i]))));
		_stream->print(
				static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(paramValue[i]))));
#ifdef FULLDEBUG
		PRINT(static_cast<char>(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(paramValue[i]))));
		PRINT(static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(paramValue[i]))));
#endif
	}

	_stream->print(CRLF);
#ifdef FULLDEBUG
	PRINT(CRLF);
#endif

	return expectOK();
}

// paramName should include the trailing space
bool LoRaModemMicrochip::setMacParam(const char* paramName,
		uint8_t paramValue) {
#ifdef FULLDEBUG
	PRINTF("[setMacParam] ");
	PRINT(paramName);
	PRINT("= ");
	PRINTLN(paramValue);
#endif

	_stream->print(STR_CMD_SET);
	_stream->print(paramName);
	_stream->print(paramValue);
	_stream->print(CRLF);

#ifdef FULLDEBUG
	PRINT(STR_CMD_SET);
	PRINT(paramName);
	PRINT(paramValue);
	PRINT(CRLF);
#endif

	return expectOK();
}

// paramName should include the trailing space
bool LoRaModemMicrochip::setMacParam(const char* paramName,
		const char* paramValue) {
	_stream->print(STR_CMD_SET);
	_stream->print(paramName);
	_stream->print(paramValue);
	_stream->print(CRLF);

#ifdef FULLDEBUG
	PRINT(STR_CMD_SET);
	PRINT(paramName);
	PRINT(paramValue);
	PRINT(CRLF);
#endif

	return expectOK();
}

//process any incoming packets from the modem
void LoRaModemMicrochip::processIncoming() {
	readLn();
}

unsigned char LoRaModemMicrochip::onMacRX() {

	// parse inputbuffer, put payload into packet buffer
	char* token = strtok(this->inputBuffer, " ");

	// sanity check
	if (strcmp(token, STR_RESULT_MAC_RX) != 0) {
#ifdef FULLDEBUG
		PRINTLNF("no mac_rx found in result");
#endif
		return NoResponse; // TODO create more appropriate error codes
	}

	// port
	token = strtok(NULL, " ");

	// payload
	token = strtok(NULL, " "); // until end of string
	memcpy(this->receivedPayloadBuffer, token, strlen(token) + 1); // TODO check for buffer limit

	return NoError;
}

//extract the specified instrumentation parameter from the modem and return the value
int LoRaModemMicrochip::getParam(instrumentationParam param) {
	switch (param) {
	case MODEM:// WHAT DOES IT RETURN ?? getModemId() ???
		return 0;
	case FREQUENCYBAND: {
		if (strstr(getParam("mac", "band"), "868"))
			return 1;
		return 0;
	}
	case SP_FACTOR: {
		char* val = getParam("radio", "sf");
		return sfToIndex(val);
	}
	case ADR: {
		if (strstr(getParam("mac", "adr"), "on"))
			return 1;
		return 0;
	}
	case POWER_INDEX: {
		char* val = getParam("mac", "pwridx");
		return atoi(val);
	}
	case BANDWIDTH: {
		char* val = getParam("radio", "bw");
		if (strstr(val, "500"))
			return 3;
		if (strstr(val, "250"))
			return 2;
		if (strstr(val, "125"))
			return 1;
		return 0;
	}
	case CODING_RATE: {
		char* val = getParam("radio", "cr");
		if (strstr(val, "4/8"))
			return 3;
		if (strstr(val, "4/7"))
			return 2;
		if (strstr(val, "4/6"))
			return 1;
		return 0;// 4/5
	}
	case DUTY_CYCLE: {
		char* val = getParam("mac", "dcycleps");
		return atoi(val);
	}
	case SNR: {
		char* val = getParam("radio", "snr");
		return atoi(val);
	}
	case GATEWAY_COUNT: {
		char* val = getParam("mac", "gwnb");
		return atoi(val);
	}
	case RETRANSMISSION_COUNT: {
		char* val = getParam("mac", "retx");
		return atoi(val);
	}
	case DATA_RATE: {
		char* val = getParam("mac", "dr");
		return atoi(val);
	}
	default:
		return false;
	}
}
bool LoRaModemMicrochip::sendSysCmd(const char* paramName) {
#ifdef FULLDEBUG
	PRINTF("[sending CMD]:");
	PRINTLN(paramName);
#endif

	_stream->print(paramName);
	_stream->print(CRLF);

	return expectOK();
}

char* LoRaModemMicrochip::getParam(const char* baseName, const char* paramName,
		unsigned short timeout) {
	return getParam(baseName, paramName, NULL, timeout);
}

char* LoRaModemMicrochip::getParam(const char* baseName, const char* paramName,
		const char* paramValue, unsigned short timeout) {
#ifdef FULLDEBUG
	PRINTF("[getMacParam] ");
	PRINT(paramName);
	if(paramValue) {
		PRINT(paramValue);
	}
#endif

	_stream->print(baseName);
	_stream->print(" get ");
	_stream->print(paramName);
	if (paramValue) {
		_stream->print(paramValue);
	}
	_stream->print(CRLF);

	unsigned long start = millis();
	while (millis() < start + timeout) {
		if (readLn() > 0) {
#ifdef FULLDEBUG
			PRINT(" = ");
			PRINTLN(this->inputBuffer);
#endif
			return this->inputBuffer;
		}
	}
	return NULL;
}

#ifdef FULLDEBUG
//prints all configuration params (radio and mac) to the monitor
void LoRaModemMicrochip::printModemConfig() {
	PRINTLNF("----- SYSTEM -----")
	char* res = getParam("sys", "ver");
	PRINTF("sysver = ")
	PRINTLN(res);
	PRINTLNF("----- RADIO -----")
	res = getParam("radio", "bt");
	PRINTF("bt = ")
	PRINTLN(res);
	res = getParam("radio", "mod");
	PRINTF("mod = ")
	PRINTLN(res);
	res = getParam("radio", "freq");
	PRINTF("freq = ")
	PRINTLN(res);
	res = getParam("radio", "sf");
	PRINTF("sf = ")
	PRINTLN(res);
	res = getParam("radio", "afcbw");
	PRINTF("afcbw = ")
	PRINTLN(res);
	res = getParam("radio", "rxbw");
	PRINTF("rxbw = ")
	PRINTLN(res);
	res = getParam("radio", "bitrate");
	PRINTF("bitrate = ")
	PRINTLN(res);
	res = getParam("radio", "fdev");
	PRINTF("fdev = ")
	PRINTLN(res);
	res = getParam("radio", "prlen");
	PRINTF("prlen = ")
	PRINTLN(res);
	res = getParam("radio", "crc");
	PRINTF("crc = ")
	PRINTLN(res);
	res = getParam("radio", "iqi");
	PRINTF("iqi = ")
	PRINTLN(res);
	res = getParam("radio", "cr");
	PRINTF("cr = ")
	PRINTLN(res);
	res = getParam("radio", "wdt");
	PRINTF("wdt = ")
	PRINTLN(res);
	res = getParam("radio", "bw");
	PRINTF("bw = ")
	PRINTLN(res);
	res = getParam("radio", "snr");
	PRINTF("snr = ")
	PRINTLN(res);
	PRINTLNF("----- MAC -----")
	res = getParam("mac", "devaddr");
	PRINTF("devaddr = ")
	PRINTLN(res);
	res = getParam("mac", "deveui");
	PRINTF("deveui = ")
	PRINTLN(res);
	res = getParam("mac", "appeui");
	PRINTF("appeui = ")
	PRINTLN(res);
	res = getParam("mac", "dr");
	PRINTF("dr = ")
	PRINTLN(res);
	res = getParam("mac", "band");
	PRINTF("band = ")
	PRINTLN(res);
	res = getParam("mac", "pwridx");
	PRINTF("pwridx = ")
	PRINTLN(res);
	res = getParam("mac", "adr");
	PRINTF("adr = ")
	PRINTLN(res);
	res = getParam("mac", "retx");
	PRINTF("retx = ")
	PRINTLN(res);
	res = getParam("mac", "rxdelay1");
	PRINTF("rxdelay1 = ")
	PRINTLN(res);
	res = getParam("mac", "rxdelay2");
	PRINTF("rxdelay2 = ")
	PRINTLN(res);
	res = getParam("mac", "ar");
	PRINTF("ar = ")
	PRINTLN(res);
	res = getParam("mac", "rx2 868");
	PRINTF("rx2 = ")
	PRINTLN(res);
	res = getParam("mac", "dcycleps");
	PRINTF("dcycleps = ")
	PRINTLN(res);
	res = getParam("mac", "mrgn");
	PRINTF("mrgn = ")
	PRINTLN(res);
	res = getParam("mac", "gwnb");
	PRINTF("gwnb = ")
	PRINTLN(res);
	res = getParam("mac", "status");
	PRINTF("status = ")
	PRINTLN(res);

	for (int ch = 0; ch <= 15; ch++) {
		res = getParam("mac", "ch freq ", int2str(ch));
		PRINTF("ch freq ")
		PRINT(ch)
		PRINTF(" = ")
		PRINTLN(res);
	}

	for (int ch = 0; ch <= 15; ch++) {
		res = getParam("mac", "ch dcycle ", int2str(ch));
		PRINTF("ch dcycle ")
		PRINT(ch)
		PRINTF(" = ")
		PRINTLN(res);
	}

	for (int ch = 0; ch <= 15; ch++) {
		res = getParam("mac", "ch drrange ", int2str(ch));
		PRINTF("ch drrange ")
		PRINT(ch)
		PRINTF(" = ")
		PRINTLN(res);
	}

	for (int ch = 0; ch <= 15; ch++) {
		res = getParam("mac", "ch status ", int2str(ch));
		PRINTF("ch status ")
		PRINT(ch)
		PRINTF(" = ")
		PRINTLN(res);
	}
}
#endif

//convert the text value for spreading factor into a number between 0 and 6
int LoRaModemMicrochip::sfToIndex(char* value) {
	int len = strlen(value);
	if (len == 3) {
		int res = value[2] - 54; //48 = ascii 0,  ascii 55 = 7 -> transaltes to index 0x01
		if (res >= 1 && res < 4)
			return res;	//small sanity check, make certain that it is within the expected range
	} else if (len == 4) {
		int res = value[3] - 47 + 3;
		if (res >= 4 && res < 7)
			return res;	//small sanity check, make certain that it is within the expected range
	}
	return 6;   //unknown spreading factor -> return for sf 12
}

//returns the id number of the modem type. See the container definition for the instrumentation container to see more details.
int LoRaModemMicrochip::getModemId() {
	return 3;
}

bool LoRaModemMicrochip::storeLastSendMs(unsigned long lastSend) {
	_lastSendMs = lastSend;
	return true;
//	return write_EEPROM(EEPROM_ADDR_LAST_SEND, lastSend) == 4;
}

unsigned long LoRaModemMicrochip::getLastSendMs() {
//	unsigned long result = 0;
//	read_EEPROM(EEPROM_ADDR_LAST_SEND, result);
//	return result;
	return _lastSendMs;
}

bool LoRaModemMicrochip::storeSendBackOffMs(unsigned long lastSend) {
	_sendBackOffMs = lastSend;
	return true;
	//return write_EEPROM(EEPROM_ADDR_BACKOFF_SEND, lastSend) == 4;
}

unsigned long LoRaModemMicrochip::getSendBackOffMs() {
	//  unsigned long result = 0;
	//  read_EEPROM(EEPROM_ADDR_BACKOFF_SEND, result);
	//  return result;
	return _sendBackOffMs;
}
