/*
 AllThingsTalk - SmartLiving.io LoRa Arduino library 
 Released into the public domain.

 Original author: Jan Bogaerts (2015)
 */

#ifndef DEVICE_H_
#define DEVICE_H_

//#include "Arduino.h"
#include <string.h>
#include <Stream.h>

#include "LoRaModem.h"
#include "DataPacket.h"
#include "instrumentationParamEnum.h"
#include "InstrumentationPacket.h"
#include "sensor.h"

/////////////////////////////////////////////////////////////
//	Configuration
/////////////////////////////////////////////////////////////
#define SEND_MAX_RETRY 5			//the default max nr of times that 'send' functions will retry to send the same value.
#define MIN_TIME_BETWEEN_SEND 15000 //the minimum time between 2 consecutive calls to Send data.
/////////////////////////////////////////////////////////////

class Device {
protected:
	volatile int seconds = 0;

public:
	//create the object
	//modem: the object that represents the modem that should be used.
	//monitor: the stream used to write log lines to.
	Device(LoRaModem* modem, Stream* monitor = NULL);
	//Device();

	void processQueue();
	boolean performChecks();
	/*
	 * connect with the LoRaWAN network (call this first !!!)
	 * returns: true when subscribe was successful, otherwise false.
	 */
	bool connect(unsigned char* devAddress, unsigned char* appKey, unsigned char* nwksKey, bool adr = true);

	//collects all the instrumentation data from the modem (RSSI, ADR, datarate,..) and sends it over
	//if ack = true -> request acknowledge, otherwise no acknowledge is waited for.
	// bool sendInstrumentation(bool ack = true);

	/*
	 * THIS IS THE ONE !!!!!!
	 *
	 * What IF (ack == TRUE) AND safety == TRUE AND message gets QUEUED ?? -> re-think this flow !
	 * 	separate Send (opt ACK) / SendSafe (NO ACK)???
	 * 	=> bool sendSafe(Sensor& sensorValue);
	 * 	=> bool send(Sensor& sensorValue, bool ack=false);
	 */
	bool sendSafe(Sensor& sensorValue);

	bool send(Sensor& sensorValue, bool ack = false);

	unsigned long getBackOffTimeMs();

	bool sendQueueIsFull();

	bool sendQueueIsEmpty();

	byte sendQueueSize();

	bool canSendImmediately();

	//set the max nr of times that the 'Send' functions will try to resend a message when previously not successful.
	//default value = #define SEND_MAX_RETRY ^^^^
	//set to -1 for continuous until success.
	void setMaxSendRetry(short maxRetries) {
		_maxRetries = maxRetries;
	};

	//set the minimum amount of time between 2 consecutive messages that are sent to the cloud.
	//default value: 15 seconds.
	//minTimeBetweenSend: the nr of milli seconds that should be between 2 data packets.
	void setMinTimeBetweenSend(short minTimeBetweenSend) {
		_minTimeBetweenSend = minTimeBetweenSend;
	};

	bool isDownlinkDataAvailable();
  byte getDownlinkDataSize();
	byte* getDownlinkData();
	
	
private:
	Stream *_monitor;

	LoRaModem* _modem;
	short _maxRetries;//the max nr of times that a send function will try to resend a message.
	short _minTimeBetweenSend;
	unsigned long _lastTimeSent;//the last time that a message was sent, so we can block sending if user calls send to quickly

	//store the param in the  data packet, and print to serial.
	//void setInstrumentationParam(InstrumentationPacket* data, instrumentationParam param, const char* name, int value);

};

#endif  /* DEVICE_H_ */
