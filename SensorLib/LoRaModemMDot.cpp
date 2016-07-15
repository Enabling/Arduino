#include "LoRaModemMDot.h"

#include <arduino.h>
#include "Utils.h"
#include "StringLiterals.h"

//#define FULLDEBUG

LoRaModemMDot::LoRaModemMDot(Stream* stream, Stream* monitor) {
	_stream = stream;
	_monitor = monitor;
}

unsigned long LoRaModemMDot::getDefaultBaudRate() {
	return BAUD_RATE;
}

bool LoRaModemMDot::stop() {
#ifdef FULLDEBUG
	PRINTLNF("Sending the network stop command");
#endif
//	sendAT("SLEEP=1");
//	return expectOK(); // RETURNING ????
	return true;
}
bool LoRaModemMDot::setLoRaWan(bool adr) {
	_adr = adr;
	return true;
}
bool LoRaModemMDot::sendLoRaWan() {
	//lorawan should be on by default (no private supported)
	//sendAT("AT+PN=1"); // Join PUBLIC NW !!
	boolean joinresult = true;	// = expectOK(10000);
	if (joinresult) {
		sendAT("AT+NJM=0"); // network join mode MANUAL !!
		joinresult = expectOK();
	}
	if (joinresult) {
		sendAT("ADR=1"); // switch adaptive data rate ON
		joinresult = expectOK();
	}
	return joinresult;
}
bool LoRaModemMDot::setDevAddress(unsigned char* devAddress) {
	devAddr = devAddress;
	return true;
}
bool LoRaModemMDot::sendDevAddress() {
#ifdef FULLDEBUG
	PRINTLNF("Setting the DevAddr");
#endif
	const char* command = "NA=..:..:..:..";
	writeHex(devAddr, 4, ((unsigned char*) command) + 3, ':');
	sendAT(command);
	return expectString("Set Network Address") && expectOK(); // RETURNING ????
}
bool LoRaModemMDot::setAppKey(unsigned char* appKey) {
	appkey = appKey;
	return true;
}
bool LoRaModemMDot::sendAppKey() {
#ifdef FULLDEBUG
	PRINTLNF("Setting the AppSKey");
#endif
	const char* command = "DSK=XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX";
	writeHex(appkey, 16, ((unsigned char*) command) + 4, ':');
	sendAT(command);
	return expectString("Set Data Session Key") && expectOK(); // RETURNING ????
}
bool LoRaModemMDot::setNWKSKey(unsigned char* nwksKey) {
	nwSkey = nwksKey;
	return true;
}
bool LoRaModemMDot::sendNWKSKey() {
#ifdef FULLDEBUG
	PRINTLNF("Setting the NwkSKey");
#endif
	const char* command = "NSK=XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX";
	writeHex(nwSkey, 16, ((unsigned char*) command) + 4, ':');
	sendAT(command);
	return expectString("Set Network Session Key") && expectOK(); // RETURNING ????
}

bool LoRaModemMDot::start() {
#ifdef FULLDEBUG
	PRINTLNF("Sending the network start commands");
#endif
	//sendAT("ATZ");
	//int numread = readLn();
	PRINTLNF("-- RESET ---");
	bool joinresult = true;	//expectOK(1500);
	// allow some time for the modem to get into correct state ....
	delay(250);

	if (joinresult) {
		joinresult = sendDevAddress();
	}
	if (joinresult) {
		joinresult = sendAppKey();
	}
	if (joinresult) {
		joinresult = sendNWKSKey();
	}
	// Setting more configuration ....    ACK / PORT / ....
	if (joinresult & _adr) {
		joinresult = sendLoRaWan();
	}

	// SAVE modem config !!
	if (joinresult) {
		sendAT("AT&W");
		joinresult = expectOK();
	}

//	if (joinresult) { // No need in manual mode !! ??
//		sendAT("JOIN");
//		joinresult = expectOK(15000);
//	}
	// reset time keeping ...
	storeSendBackOffMs(0);
	return joinresult;
}

/**
 * Helper methods
 */

void LoRaModemMDot::sendAT(const char* str) {
	if ((str[0] == 'A' || str[0] == 'a') && (str[1] == 'T' || str[1] == 't')) {
		_stream->write(str);
		_stream->write('\n');
#ifdef FULLDEBUG
		PRINTF("debug:");
		PRINTLN(str);
#endif
	} else {
		_stream->write("AT+");
		_stream->write(str);
		_stream->write('\n');
#ifdef FULLDEBUG
		PRINTF("debug:AT+");
		PRINTLN(str);
#endif
	}
}

bool LoRaModemMDot::queryModem(const char* str) {
	sendAT(str);
	readLn();	// clear 1 st blank line returned !?
	return (readLn() > 0);
}

// Check modem response after sending commands ...
bool LoRaModemMDot::expectString(const char* str, unsigned short timeout) {
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
			PRINTLNF(")");
#endif

			// TODO store possible answer for param req?
			if (strstr(this->inputBuffer, str) != NULL) {
#ifdef FULLDEBUG
				PRINTLNF(" found a match!");
#endif
				return true;
			} else if (strstr(this->inputBuffer, "ERROR") != NULL) {
				return false;
			} else {

			}
			// return false;
		}
	}

	return false;
}

bool LoRaModemMDot::expectOK(
		const unsigned int timeoutMs /*= DEFAULT_TIMEOUT*/) {
	return expectString("OK", timeoutMs);
}

//extract the specified instrumentation parameter from the modem and return the value
int LoRaModemMDot::getParam(instrumentationParam param) {
	int result = -1;

	switch (param) {
	case MODEM: // WHAT DOES IT RETURN ?? getModemId() ???
		return getModemId();
	case FREQUENCYBAND: {
		if (queryModem("AT+FREQ?")) {
			if (strstr(this->inputBuffer, "868")) {
				result = 1;
			} else {
				result = 0;
			}
		}
		break;
	}
	case SP_FACTOR: { //DR0 - SF12BW125// simplify ??
		if (queryModem("AT+TXDR?")) {
			char* loc = strstr(this->inputBuffer, "SF");
			if (loc) {
				char* l2 = strstr(this->inputBuffer, "BW");
				char sf[3];
				strncpy(sf, loc + 2, (l2 - loc) - 2);
				sf[2] = 0;
				result = atoi(sf) - 6; // TODO : get rid of this '-6' !!  Check other modem implementations !!
			}
		}
		break;
	}
	case ADR: {
		if (queryModem("AT+ADR?")) {
			result = atoi(this->inputBuffer);
			//strstr(this->inputBuffer, "1") == 0 ? 0 : 1;
		}
		break;
	}
	case POWER_INDEX: {
		if (queryModem("AT+TXP?")) {
			result = atoi(this->inputBuffer);
		}
		break;
	}
	case BANDWIDTH: {
		if (queryModem("AT+TXDR?")) {
			char* loc = strstr(this->inputBuffer, "BW");
			if (loc) {
				if (strstr(this->inputBuffer, "500")) {
					result = 3;
				} else if (strstr(this->inputBuffer, "250")) {
					result = 2;
				} else if (strstr(this->inputBuffer, "125")) {
					result = 1;
				} else {
					result = 0;
				}
			}
		}
		break;
	}
	case CODING_RATE: {
		if (queryModem("AT+FEC?")) {
			result = atoi(this->inputBuffer) - 1;
		}
		break;
	}
	case DUTY_CYCLE: { // Can't find it in the AT command reference.
		return -1;
	}
	case SNR: {
		// AT+SNR: Display signal to noise ratio of received packets: last, min, max, avg
		if (queryModem("AT+SNR?")) {
			// gives us 4 values : we'll take the last (= average)
			char* pch;
			char* avg = NULL;
			pch = strtok(this->inputBuffer, ",");
			while (pch != NULL) {
				avg = pch;
				pch = strtok(NULL, ",");
			}
			if (avg != NULL) {
				result = atoi(avg);
			}
		}
		break;
	}
	case GATEWAY_COUNT: { //
		// AT+NLC: Network Link Check
		// 1st param in result is dBm level above the demodulation floor.
		// 2nd param is the number of gateways in the end device's range.
		if (queryModem("AT+NLC?")) {
			char* pch;
			char* gwcount = NULL;
			pch = strtok(this->inputBuffer, ",");
			while (pch != NULL) {
				gwcount = pch;
				pch = strtok(NULL, ",");
			}
			if (gwcount != NULL) {
				result = atoi(gwcount);
			}
		}
		break;
	}
	case RETRANSMISSION_COUNT: {
		if (queryModem("AT+ACK?")) {
			result = atoi(this->inputBuffer);
		}
		break;
	}
	case DATA_RATE: {
		if (queryModem("AT+TXDR?")) {
			char dr[3];
			strncpy(dr, this->inputBuffer + 2, 3);
			result = atoi(dr);
		}
		break;
	}
	default:
		break;
	}
	if (result >= 0)	// only if result parsed ??
		expectOK();		// clear up rest of incoming data !

	return result;
}

//returns the id number of the modem type. See the container definition for the instrumentation container to see more details.
int LoRaModemMDot::getModemId() {
	return 4; /// Microchip + 1 !?
}
bool LoRaModemMDot::storeLastSendMs(unsigned long lastSend) {
	_lastSendMs = lastSend;
	return true;
}

unsigned long LoRaModemMDot::getLastSendMs() {
	return _lastSendMs;
}

bool LoRaModemMDot::storeSendBackOffMs(unsigned long lastSend) {
	_sendBackOffMs = lastSend;
	return true;
}

unsigned long LoRaModemMDot::getSendBackOffMs() {
	return _sendBackOffMs;
}

/*
 *	Setting PORT / ACK / ....    BEFORE sending actual data !!
 */
bool LoRaModemMDot::send(Packet* packet, bool ack) {
	unsigned char length = packet->sizeOfData;

	char* str = new char[(length * 2) + 10];
	strcpy(str, "AT+SENDB=");
	str[(length * 2) + 9] = 0;

	writeHex(packet->data, packet->sizeOfData, ((unsigned char*) str) + 9);

#ifdef FULLDEBUG
	PRINTF("Sending : ");
	PRINTLN(str);
#endif

	sendAT(str);
	delete[] str;
	// was there a response from the server in the downstream communication ??
//	if(readLn() > 0){
	// copy from buffer into less temporary var ....
//	}
	return expectOK(3000); // RETURNING ????
}

unsigned short LoRaModemMDot::readLn(char* buffer, unsigned short size,
		unsigned short start) {
	int len = _stream->readBytesUntil('\n', buffer + start, size);
	if (len > 0) { // or > 2 .. 3  ??
		this->inputBuffer[start + len - 1] = 0; // bytes until \n always end with \r, so get rid of it (-1)
		len--; // effectively shorten the length by 1
#ifdef FULLDEBUG
		PRINTLN(this->inputBuffer);
#endif
	} else {
		this->inputBuffer[start] = 0;
		len = 0; // just in case check for > 2 or 3
	}
	return len;
}
//process any incoming packets from the modem
void LoRaModemMDot::processIncoming() {
	readLn();
}


unsigned long timeUbntilNext(){
	// AT+TXN?
}
