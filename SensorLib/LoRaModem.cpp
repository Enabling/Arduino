#include "LoRaModem.h"
#include <Arduino.h>
#include "Utils.h"
#include "SimpleFIFO.h"

//#define developing

#if defined developing
#define TIME_CHECK 15000
#else
#define TIME_CHECK getSendBackOffMs()
#endif

void LoRaModem::blinkLed() {
//	digitalWrite(LED1, !digitalRead(LED1));
//	digitalWrite(LED2, !digitalRead(LED2));
}

float LoRaModem::calculateSymbolTime(short spreading_factor, short bandwidth) {
	if (spreading_factor < 0)
		spreading_factor = getParam(SP_FACTOR) + 6;
	if (bandwidth < 0)
		bandwidth = (pow(2, getParam(BANDWIDTH)) - 1) * 125;
	return (pow(2, (float) spreading_factor)) / ((float) bandwidth * 1000.0)
			* 1000.0;
}

float LoRaModem::calculateTimeOnAir(unsigned char appPayloadSize,
		short spreading_factor) {
	float symbTime = calculateSymbolTime(spreading_factor);
	return (symbTime
			* (float) calculateSymbolsInPayload(appPayloadSize,
					spreading_factor)) + (symbTime * (4.25 + PREAMBLE_SYMBOLS));
}

int LoRaModem::calculateSymbolsInPayload(unsigned char appPayloadSize,
		short spreading_factor) {
	if (spreading_factor < 0)
		spreading_factor = getParam(SP_FACTOR) + 6;
	int bandwidth = (pow(2, getParam(BANDWIDTH)) - 1) * 125;
	int coding_rate = getParam(CODING_RATE) + 5;
	int low_dr_corr =
			((spreading_factor == 11 || spreading_factor == 12)
					&& bandwidth == 125) ? 2 : 0;
	return 8
			+ max(
					ceil(
							(((float )(8 * (appPayloadSize + LORA_HEADER_SIZE))
									- (4 * spreading_factor) + 28 + 16
									- (EXPLICIT_HEADER ? 0 : 20))
									/ (float ) (4
											* (spreading_factor - low_dr_corr))))
							* coding_rate, 0);

}

int LoRaModem::maxPayloadForSF(short spreading_factor) {
	if (spreading_factor < 0) {
		spreading_factor = getParam(SP_FACTOR) + 6;
	}
	if (spreading_factor >= 10) {
		return 51;
	} else if (spreading_factor < 9) {
		return 222;
	} else {
		return 115;
	}
}


// prints 8-bit data in hex
void LoRaModem::printHex(uint8_t *data, uint8_t length, bool newLine) {
	char tmp[length * 3 + 1];
	writeHex(data, length, (uint8_t *) tmp, ' ');
	if (newLine) {
		PRINTLN(tmp);
	} else {
		PRINT(tmp);
	}
}

float LoRaModem::updateTimeOnAirBudget(float milliSeconds) {
	float newBudget = getTimeOnAirBudget() + milliSeconds;
	storeTimeOnAirBudget(newBudget);
	return newBudget;
}

bool LoRaModem::sendSafe(Sensor& sensorValue) {
	bool result = true;
	// check if can be sent otherwise add to queue
	LoRaPacket* thePacket = sensorValue.getAsBinary();
	byte packSize = thePacket->getDataSize();
	if (packSize > maxPayloadForSF()) {
		PRINTLNF("Data size exceeds limitations for current spreading factor.")
		result = false;
	}
	if (result) {
		unsigned long elapsed = (unsigned long) (millis() - getLastSendMs());
		if (packets.count() > 0 || elapsed < getSendBackOffMs()) {
			// Can we Queue ??
			if (packets.count() < MAX_FIFO_SIZE) {
				packets.enqueue(new Packet(thePacket));	// Delete after use !!!!!
				PRINTLNF("Msg on Q")
//				PRINT(packets.count())
//				PRINTF("/")
//				PRINTLN(MAX_FIFO_SIZE)
				result = false; // false or true ??
			} else { // Exception ?
				PRINTLNF("Msg dropped")
//				PRINTLN((getSendBackOffMs() - elapsed) / 1000)
				result = false;
			}
		}
	}
	if (result) {
		// calculate BEFORE or AFTER send ??  (-> sf might change ... before would be the actual value used in send)
		float toa = calculateTimeOnAir(packSize); // calculate for current settings, so BEFORE send !!
		Packet p(thePacket);
		result = send(&p, false);
		updateTimeOnAirBudget(-toa); // lower the budget with the calculated toa
		storeLastSendMs(millis());
		storeSendBackOffMs(ceil(toa * 100));
	}
	return result;
}

bool LoRaModem::send(Sensor& sensorValue, bool ack) {
	// what if still in Queueueueueue ????
	// need to update the TOA usage even if not checking here ....
	// do we need to add if ack and result == false ??
	DataPacket* theDataPacket = (DataPacket*) (sensorValue.getAsBinary());
	updateTimeOnAirBudget(-calculateTimeOnAir(theDataPacket->getDataSize())); // lower the budget with the calculated toa
	storeLastSendMs(millis());
	storeSendBackOffMs(0); // set it or not ??
	Packet p(theDataPacket);
	return send(&p, ack);
}
void LoRaModem::processQueue() {
	processingQueue = true;
	// anything in the Queue ?
	if (packets.count() > 0) {
		unsigned long elapsed = (unsigned long) (millis() - getLastSendMs());
		if (elapsed > TIME_CHECK) {
			Serial.println(F("Q >"));
			Packet* theDataPacket = packets.peek(); // use 1st sensor value from queue, leaving it on in case send fails ...
			float toa = calculateTimeOnAir(theDataPacket->sizeOfData);
			updateTimeOnAirBudget(-toa); // lower the budget with the calculated toa
			storeLastSendMs(millis());
			storeSendBackOffMs(ceil(toa * 100));
			if (send(theDataPacket, false)) {
				Serial.println(F("OK"));
				// all OK , delete from Queueu
				delete (packets.dequeue());
			} else {
				Serial.println(F("ERR"));
			}
		}
	}
	processingQueue = false;
}
boolean LoRaModem::performChecks() {
//	blinkLed();
	if (processingQueue)
		return false;
	// Process Queueueueueue??
	if (packets.count() > 0) {
		unsigned long elapsed = (unsigned long) (millis() - getLastSendMs());
		return elapsed > TIME_CHECK; // millis() STOPS on sleep !!! SCREWED !! need EXTERNAL clock for timed operations !!
	}
	return false;
}

unsigned long LoRaModem::getBackOffTimeMs() {
	unsigned long elapsed = (unsigned long) (millis() - getLastSendMs());
	if (elapsed >= getSendBackOffMs()) {
		return 0;
	} else {
		return getSendBackOffMs() - elapsed;
	}
}
bool LoRaModem::sendQueueIsFull() {
	return packets.count() == MAX_FIFO_SIZE;
}
bool LoRaModem::sendQueueIsEmpty() {
	return packets.count() == 0;
}

byte LoRaModem::sendQueueSize() {
	return packets.count();
}
bool LoRaModem::canSendImmediately() {
	if (!sendQueueIsEmpty()) {
		return false;
	}
	return getBackOffTimeMs() == 0;
}

void LoRaModem::setDestinationPort(uint8_t portn) {
	_messagePort = portn;
}
uint8_t LoRaModem::getDestinationPort() {
	return _messagePort;
}
bool LoRaModem::storeTimeOnAirBudget(float budget) {
	_toaBudget = budget;
	return true;
}
float LoRaModem::getTimeOnAirBudget() {
	return _toaBudget;
}
