/*
 AllThingsTalk - SmartLiving.io Communicate with Embit lora modems through binary AT commands
 Released into the public domain.

 Original author: Jan Bogaerts (2015)
 */

#ifndef LORAMODEMEMBIT_H_
#define LORAMODEMEMBIT_H_

#include "LoRaModem.h"
#include "LoRaPacket.h"
#include "instrumentationParamEnum.h"

//this class represents the ATT cloud platform.
class LoRaModemEmbit: public LoRaModem {
public:
	//create the object
	LoRaModemEmbit(Stream* stream, Stream* monitor = NULL);

	virtual ~LoRaModemEmbit() {
	}

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
	//send a data packet to the server
	//bool send(LoRaPacket* packet, bool dt_safe = true, bool pl_safe = true, bool ack = true);
	//process any incoming packets from the modem
	void processIncoming();
	//extract the specified instrumentation parameter from the modem and return the value
	int getParam(instrumentationParam param);
	//returns the id number of the modem type. See the container definition for the instrumentation container to see more details.
	int getModemId();
private:
	void sendByte(unsigned char data);
	void sendPacket(unsigned char* data, uint16_t length);
	void sendPacket(unsigned char* data, uint16_t length, unsigned char* data2, uint16_t length2);

	bool readPacket();
	//reads a packet from the modem and returns the value of the byte at the specified index position
	unsigned char readPacket(unsigned char index);

};

#endif /* LORAMODEMEMBIT_H_ */
