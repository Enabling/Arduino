#ifndef LORAMODEMMDOT_H_
#define LORAMODEMMDOT_H_

/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * The mDot serial communication is problematic on the Arduino (nano/uno/....)
 * using the SoftSerial implementation. Baud rate should be set at 9600 to have a workable communication line.
 * It is the RX side of things that's causing problems (missing/weird bytes) possibly due to the bad clock.
 * Going through the HW serial where available (mbili Serial1) gives a better/more stable experience.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#include "LoRaModem.h"
#include "LoRaPacket.h"
#include "instrumentationParamEnum.h"

#define DEFAULT_INPUT_BUFFER_SIZE 96
#define DEFAULT_TIMEOUT 120
#define RECEIVE_TIMEOUT 60000
#define MAX_SEND_RETRIES 10

class LoRaModemMDot: public LoRaModem {
private:
	static const unsigned long BAUD_RATE = 9600;// device is DEFAULT set at 115200, but the Arduino (software) serial library can't manage this speed without errors // 115200 <-> 9600
	unsigned long _sendBackOffMs = 0;
	unsigned long _lastSendMs = 0;
	unsigned char* nwSkey = NULL;
	unsigned char* appkey = NULL;
	unsigned char* devAddr = NULL;bool _adr = false;
protected:
	bool send(Packet* packet, bool ack = true);

public:
	//create the object
	LoRaModemMDot(Stream* stream, Stream* monitor = NULL);

	virtual ~LoRaModemMDot() {
	}

	// Returns the required baudrate for the device
	// !!!!! Configure devices on MUCH slower speed  !!!!
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
	//send a data packet to the server
	//bool send(LoRaPacket* packet, bool dt_safe = true, bool pl_safe = true, bool ack = true);
	//process any incoming packets from the modem
	void processIncoming();
	//extract the specified instrumentation parameter from the modem and return the value
	int getParam(instrumentationParam param);
	//returns the id number of the modem type. See the container definition for the instrumentation container to see more details.
	int getModemId();

	bool storeLastSendMs(unsigned long timeToSend);

	unsigned long getLastSendMs();

	bool storeSendBackOffMs(unsigned long timeToSend);

	unsigned long getSendBackOffMs();

	// Combine in the instrumentation object ?!
//	char* getSpreadingFactor();
//	char* getAdaptiveDataRateStatus();
//	char* getDataRate();
//	char* getGatewayCount();
//	char* getFWVersion();

private:
	char inputBuffer[DEFAULT_INPUT_BUFFER_SIZE + 1];
	unsigned short readLn(char* buf, unsigned short bufferSize,
			unsigned short start = 0);

	unsigned short readLn() {
		return readLn(this->inputBuffer, DEFAULT_INPUT_BUFFER_SIZE);
	}

	// sends the AT command to the modem to retrieve some information
	// returns true if after the command has been sent there is a result from the modem.
	bool queryModem(const char* str);

	bool expectOK(const unsigned int timeoutMs = DEFAULT_TIMEOUT);

	bool expectString(const char* str, unsigned short timeout = DEFAULT_TIMEOUT);

	void sendAT(const char* str);

	bool sendNWKSKey();
	
	bool sendAppKey();
	
	bool sendDevAddress();
	
	bool sendLoRaWan();
};

#endif /* LORAMODEMMDOT_H_ */
