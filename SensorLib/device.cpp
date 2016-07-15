#include "device.h"
#include "Utils.h"
#include <arduino.h>			//still required for the 'delay' function. use #ifdef for other platforms.

//create the object
Device::Device(LoRaModem* modem, Stream* monitor) :
		_maxRetries(SEND_MAX_RETRY), _minTimeBetweenSend(MIN_TIME_BETWEEN_SEND) {
	_modem = modem;
	_monitor = monitor;
	_lastTimeSent = 0;
}

void Device::processQueue() {
	_modem->processQueue();
}

boolean Device::performChecks() {
	return _modem->performChecks();
}

//connect with the to the lora gateway
bool Device::connect(unsigned char* devAddress, unsigned char* appKey,
		unsigned char* nwksKey, bool adr) {
	if (!_modem->stop()) {					//stop any previously running modems
//		PRINTF("can't communicate with modem");
		PRINTLNF(": HW issues?");
		return false;
	}

	if (!_modem->setLoRaWan(adr)) {	//switch to LoRaWan mode instead of peer to peer
//		PRINTF("can't switch modem to LoRaWAN mode");
		PRINTLNF(": HW issues?");
		return false;
	}
	if (!_modem->setDevAddress(devAddress)) {
//		PRINTF("can't assign device address to modem");
		PRINTLNF(": HW issues?");
		return false;
	}
	if (!_modem->setAppKey(appKey)) {
//		PRINTF("can't assign app session key to modem");
		PRINTLNF(": HW issues?");
		return false;
	}
	if (!_modem->setNWKSKey(nwksKey)) {
//		PRINTF("can't assign network session key to modem");
		PRINTLNF(": HW issues?");
		return false;
	}
	bool result = _modem->start();						//start the modem up
	if (result == true) {
		PRINTLNF("modem initialized!");
	} else {
		PRINTLNF(
				"Modem is responding, but failed to communicate with base station. Possibly out of reach or invalid credentials.");
	}
	return result;
}
bool Device::sendSafe(Sensor& sensorValue) {
	return _modem->sendSafe(sensorValue);
}

bool Device::send(Sensor& sensorValue, bool ack) {
	return _modem->send(sensorValue, ack);
}

unsigned long Device::getBackOffTimeMs() {
	return _modem->getBackOffTimeMs();
}

bool Device::sendQueueIsFull() {
	return _modem->sendQueueIsFull();
}
bool Device::sendQueueIsEmpty() {
	return _modem->sendQueueIsEmpty();
}

byte Device::sendQueueSize(){
	return _modem->sendQueueSize();
}
bool Device::canSendImmediately(){
	return _modem->canSendImmediately();
}
