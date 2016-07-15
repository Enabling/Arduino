/*
 AllThingsTalk - SmartLiving.io Abstract class for LoRa modems
 Released into the public domain.

 Original author: Jan Bogaerts (2015)
 */

#ifndef LORAMODEM_H_
#define LORAMODEM_H_

#include "LoRaPacket.h"
#include "instrumentationParamEnum.h"
#include "Sensor.h"
#include "SimpleFIFO.h"
#include "Utils.h"

// How big is our message Queue
#define MAX_FIFO_SIZE 5

//this class represents the ATT cloud platform.
class LoRaModem {
protected:

	// define a leaner structure to store in Queue
	struct Packet {
		byte sizeOfData;
		byte* data;
		Packet(LoRaPacket* dp) {
			sizeOfData = dp->getDataSize();
			data = new byte[sizeOfData];
			dp->write(data);
		}
		~Packet() {
			delete[] data;
		}
	};
	SimpleFIFO<Packet*, MAX_FIFO_SIZE> packets;

	const int LORA_HEADER_SIZE = 13;
	const bool EXPLICIT_HEADER = true;
	const int PREAMBLE_SYMBOLS = 8; // ! "radio get prlen" ?

	Stream* _monitor; // debugging purposes
	Stream* _stream; //the stream to communicate with the lora modem.

	float _toaBudget; // keep track of the available time-on-air budget.

	// PORT n° where messages get sent. Previously defined as const, but user might want to target different port
	uint8_t _messagePort = 1;


	// outputs the data in HEX format on the debug Stream (_monitor).
	void printHex(uint8_t *data, uint8_t length, bool newLine = true);

private:
	volatile boolean processingQueue = false;

public:
	static const byte MAX_QUEUE = MAX_FIFO_SIZE;

	void blinkLed(); //testing
	// LoRaModem();

	virtual ~LoRaModem() {
	}
	// Returns the required baudrate for the device
	virtual unsigned long getDefaultBaudRate() = 0;
	//stop the modem.
	virtual bool stop() = 0;
	//set the modem in LoRaWan mode (vs private networks)
	virtual bool setLoRaWan(bool adr = true) = 0;
	//assign a device address to the modem
	//devAddress must be 4 bytes long
	virtual bool setDevAddress(unsigned char* devAddress) = 0;
	//set the app session key for the modem communication
	//app session key must be 16 bytes long
	virtual bool setAppKey(unsigned char* appKey) = 0;
	//set the network session key
	//network session key must be 16 bytes long
	virtual bool setNWKSKey(unsigned char* nwksKey) = 0;
	//start the modem: returns true if successful
	virtual bool start() = 0;

	//process any incoming packets from the modem
	virtual void processIncoming() = 0;
	//extract the specified instrumentation parameter from the modem and return the value
	virtual int getParam(instrumentationParam param) = 0;
	//returns the id number of the modem type. See the container definition for the instrumentation container to see more details.
	virtual int getModemId() = 0;

	// in the final library, should split packets if too big, queue them to adhere to duty cycle limitations
	bool sendSafe(Sensor& sensorValue);

	// Ack should be used sparingly
	bool send(Sensor& sensorValue, bool ack = false);

	unsigned long getBackOffTimeMs();

	bool sendQueueIsFull();

	bool sendQueueIsEmpty();

	byte sendQueueSize();

	bool canSendImmediately();

	void setDestinationPort(uint8_t portn);

	uint8_t getDestinationPort();

protected:
	//The actual transmission onto the network
	//virtual bool send(LoRaPacket* packet, bool ack = false) = 0;
	virtual bool send(Packet* packet, bool ack = false) = 0;

	float updateTimeOnAirBudget(float milliSeconds);

	virtual bool storeLastSendMs(unsigned long timeToSend) = 0;
	virtual unsigned long getLastSendMs() = 0;
	virtual bool storeSendBackOffMs(unsigned long timeToSend) = 0;
	virtual unsigned long getSendBackOffMs() = 0;

public:
	virtual bool storeTimeOnAirBudget(float budget);
	virtual float getTimeOnAirBudget();


	// TODO : better defined visibility for release
	float calculateSymbolTime(short spreading_factor = -1, short bandwidth = -1);

	int calculateSymbolsInPayload(unsigned char appPayloadSize, short spreading_factor = -1);

	float calculateTimeOnAir(unsigned char appPayloadSize, short spreading_factor = -1);

	int maxPayloadForSF(short spreading_factor = -1);

	void processQueue();
	boolean performChecks();
};

#endif /* LORAMODEM_H_ */
