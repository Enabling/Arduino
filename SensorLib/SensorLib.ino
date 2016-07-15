/**
 * Todo's / Improvements
 *
 *  - (I) : After a send, update SF and other vars so no need to get from modem on subsequent method invocations (TOA calc, ...)
 *  - (I) : Store payload in Q instead of Sensor.
 *  - (I) : Start/Stop IRQ when needed (-> Queue MT -> Stop, restart on 1st )
 *  - (I) : Power modem over output pin -> On/Off
 *  - ---------------------------------------------------------
 *  - (T) : Handle Queue full state. -> Error ? ...
 *  - (T) : Correct naming (casing)
 *
 *  - (?) : 30s/24h = written in stone ? (or TTN)  GET RULES ON ACTILITY NW ?!
 */

#include "keys.h"
#include "device.h"
#include "LowPower.h"

#include <avr/wdt.h>
#include <avr/sleep.h>

#include "sensor.h"

// Console
#define SERIAL_BAUD 57600
#define debugSerial Serial

// Button to send msg on which pin ??  2 OR 3
// Pin change interrupt possible on other pins if needed ....
#define BTN_SEND_PIN 2
// PIN 2 => IRQ0 // 3 => IRQ1
#define IRQ 0

// Arduino's
#if defined (__AVR_ATmega328P__)
#include <SoftwareSerial.h>
// Serial setup to connect Modem
#define PIN_TX_RN2483 8
#define PIN_RX_RN2483 9
#define PIN_PWR_RN2483 12

#define PIN_Q_MT PD3
#define PIN_Q_FULL PD5
#define PIN_Q_IN_BETWEEN PD4
SoftwareSerial modemSerial(PIN_RX_RN2483, PIN_TX_RN2483);  // ARDUINO
#define MODEM_SERIAL modemSerial
#elif defined (__AVR_ATmega1284P__)
// SODAQ Mbili
#define PIN_TX_RN2483 3
#define PIN_RX_RN2483 NULL
#define PIN_PWR_RN2483 23

#define PIN_Q_MT 4
#define PIN_Q_FULL 20
#define PIN_Q_IN_BETWEEN 21
#define MODEM_SERIAL Serial1
#endif

// Which modem ??
//#include "LoRaModemMicrochip.h"
//LoRaModemMicrochip modem(&MODEM_SERIAL, &debugSerial);

#include "LoRaModemMDot.h"
LoRaModemMDot modem(&MODEM_SERIAL, &debugSerial);

Device libTest(&modem, &debugSerial);
GPSSensor gpsSensor;
EnCoSensor enCoSensor;

volatile boolean canSendFromQueue = false;
volatile unsigned long lastMsgSent = millis();
volatile boolean sendGPS = true;
volatile byte delayCnt = 0;
volatile byte sensorSelect = 0;

bool connection = false;

void wakeUP_RN2483() {
#ifdef LORAMODEMMICROCHIP_H_
	// Still not 100% guaranteed !!
	// Power modem from OUTPUT pin so we can toggle power !!
	// Also needed to power down !!!!
	debugSerial.println("Attempting RN2483 Wakeup");
	MODEM_SERIAL.end();
	// Send a break to the RN2483
	pinMode(PIN_TX_RN2483, OUTPUT);
	digitalWrite(PIN_TX_RN2483, LOW);
	delay(5);
	digitalWrite(PIN_TX_RN2483, HIGH);
	MODEM_SERIAL.begin(modem.getDefaultBaudRate());
	// Send magic character for autobaud.
	MODEM_SERIAL.write(0x55);
	debugSerial.println("END RN2483 Wakeup");
#endif
}

void attemptModemConnection() {
	byte cntWakeUp = 0;
	while (!connection) {
		wakeUP_RN2483(); // RN2483 specific -> see if we can move it into modem class itself
		cntWakeUp++;
		if (libTest.connect(DEV_ADDR, APPSKEY, NWKSKEY, true)) {
			debugSerial.println(F("Connection to the network was successful."));
			connection = true;
			modem.storeTimeOnAirBudget(30000);
			pinMode(PIN_Q_FULL, OUTPUT);
			digitalWrite(PIN_Q_FULL, LOW);
			pinMode(PIN_Q_MT, OUTPUT);
			digitalWrite(PIN_Q_MT, HIGH);
			pinMode(PIN_Q_IN_BETWEEN, OUTPUT);
			digitalWrite(PIN_Q_IN_BETWEEN, LOW);
			break;
		} else {
#ifdef PIN_PWR_RN2483
			if (cntWakeUp > 3) {
				debugSerial.println(F("Attempting power cycle"));
				digitalWrite(PIN_PWR_RN2483, LOW);
				delay(2500);
				digitalWrite(PIN_PWR_RN2483, HIGH);
				delay(2500);
			} else {
#endif
				debugSerial.println(
						F(
								"Connection to the network failed!\n--== MoDeM init retry shortly ==--"));
				delay(5000);
#ifdef PIN_PWR_RN2483
			}
#endif
		}
	}
}

void setup() {
#ifdef PIN_PWR_RN2483
	pinMode(PIN_PWR_RN2483, OUTPUT);
	digitalWrite(PIN_PWR_RN2483, HIGH);
#endif
	debugSerial.begin(SERIAL_BAUD);
	debugSerial.println("Starting .....");
	MODEM_SERIAL.begin(modem.getDefaultBaudRate());

	attemptModemConnection();

	// INT0
	pinMode(2, INPUT_PULLUP);
}

// found at learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
int freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void updateQueueStatus() {
	return;
//	digitalWrite(PIN_Q_MT, LOW);
//	digitalWrite(PIN_Q_IN_BETWEEN, LOW);
//	digitalWrite(PIN_Q_FULL, LOW);
	if (libTest.sendQueueIsEmpty()) {
		digitalWrite(PIN_Q_MT, HIGH);
		digitalWrite(PIN_Q_IN_BETWEEN, LOW);
		digitalWrite(PIN_Q_FULL, LOW);
	} else if (libTest.sendQueueIsFull()) {
		digitalWrite(PIN_Q_FULL, HIGH);
		digitalWrite(PIN_Q_MT, LOW);
		digitalWrite(PIN_Q_IN_BETWEEN, LOW);
	} else {
//		digitalWrite(PIN_Q_MT, HIGH);
//		digitalWrite(PIN_Q_FULL, HIGH);
		digitalWrite(PIN_Q_IN_BETWEEN, HIGH);
		digitalWrite(PIN_Q_MT, LOW);
		digitalWrite(PIN_Q_FULL, LOW);
	}
}

void sendGPSData() {
	bool sendResult = false;
	static float altitude = 8.15;
//	debugSerial.print(F("Spreading factor : "));
//	debugSerial.println(modem.getSpreadingFactor());
	gpsSensor.setLongitude(4.35844212770462);
	gpsSensor.setLatitude(50.86034364457187);
	gpsSensor.setAltitude(altitude);
	altitude += 3.75;
	debugSerial.print(F("Sending GPS data : [lon : "));
	debugSerial.print(gpsSensor.getLongitude());
	debugSerial.print(F(" , lat : "));
	debugSerial.print(gpsSensor.getLatitude());
	debugSerial.print(F(" , alt : "));
	debugSerial.print(gpsSensor.getAltitude());
	debugSerial.println(F("]"));
	sendResult = libTest.sendSafe(gpsSensor);
	debugSerial.print(F("Send succeeded : "));
	debugSerial.println(sendResult ? "YES" : "NO");
	debugSerial.println(F("--------------------------------------------"));
}

//void sendEnCoSensor(){
//	debugSerial.println("Sending EnCo ....");
//	bool sendResult = libTest.sendSafe(enCoSensor);
//	debugSerial.print(F("Send succeeded : "));
//	debugSerial.println(sendResult ? "YES" : "NO");
//	debugSerial.println(F("--------------------------------------------"));
//}

void sendSensor() {
	switch (sensorSelect) {
	case 0:
		sendBoolSensor();
		break;
	case 1:
		sendIntSensor();
		break;
	case 2:
		sendFloatSensor();
		break;
	case 3:
		sendBinaryDataSensor();
		break;
	case 4:
		sendGPSSensor();
		break;
	case 5:
		sendAccelSensor();
		break;
	}
	sensorSelect++;
	sensorSelect %= 6;

}

void sendBoolSensor() {
	debugSerial.println("Sending BOOLEAN data ....");
	BinarySensor bSens(false);
	bool sendResult = libTest.sendSafe(bSens);
	debugSerial.print(F("Send succeeded : "));
	debugSerial.println(sendResult ? "YES" : "NO");
	debugSerial.println(F("--------------------------------------------"));
}
void sendIntSensor() {
	debugSerial.println("Sending INTEGER data ....");
	IntegerSensor iSens(666);
	bool sendResult = libTest.sendSafe(iSens);
	debugSerial.print(F("Send succeeded : "));
	debugSerial.println(sendResult ? "YES" : "NO");
	debugSerial.println(F("--------------------------------------------"));
}
void sendFloatSensor() {
	debugSerial.println("Sending FLOAT data ....");
	FloatSensor fSens((float) -58.6);
	bool sendResult = libTest.sendSafe(fSens);
	debugSerial.print(F("Send succeeded : "));
	debugSerial.println(sendResult ? "YES" : "NO");
	debugSerial.println(F("--------------------------------------------"));
}
void sendBinaryDataSensor() {
	debugSerial.println("Sending RAW data ....");
	BinaryPayload rawSens(
			(uint8_t*) "Greetings from the ATT - Kit with the mDot modem.", 49);
	bool sendResult = libTest.sendSafe(rawSens);
	debugSerial.print(F("Send succeeded : "));
	debugSerial.println(sendResult ? "YES" : "NO");
	debugSerial.println(F("--------------------------------------------"));
}
void sendGPSSensor() {
	debugSerial.println("Sending GPS data ....");
	GPSSensor gpsSensor(4.3, 51.222, 15.5, 0);
	bool sendResult = libTest.sendSafe(gpsSensor);
	debugSerial.print(F("Send succeeded : "));
	debugSerial.println(sendResult ? "YES" : "NO");
	debugSerial.println(F("--------------------------------------------"));
}
void sendAccelSensor() {
	debugSerial.println("Sending ACCEL data ....");
	Accelerometer accSensor(12.2, 2.1, 0);
	bool sendResult = libTest.sendSafe(accSensor);
	debugSerial.print(F("Send succeeded : "));
	debugSerial.println(sendResult ? "YES" : "NO");
	debugSerial.println(F("--------------------------------------------"));
}
void sloop() {
//	debugSerial.println("Looping ....");

	if (!canSendFromQueue) {
		canSendFromQueue = libTest.performChecks();  /// ...
		updateQueueStatus();
	}
	if (canSendFromQueue) {
		debugSerial.println("Sending GPS from Queue ....");
		libTest.processQueue();
		canSendFromQueue = false;
		updateQueueStatus();
		return;
//	} else {
//		debugSerial.println("Nothing to send from Q ....");
	}
	if (sendGPS) {
		sendGPS = false;
		//sendGPSData();
//		sendEnCoSensor();
		sendSensor();
		updateQueueStatus();
	} else {
		if (++delayCnt >= 120) {
			delayCnt = 0;
			sendGPS = true;
		}
//		debugSerial.println("Not sending new GPS data ....");
	}

}

/**
 * Button pressed interrupt method !!
 * (de-bounce ??)
 */
void wakeUp() {
	// Disable external pin interrupt on wake up pin.
	detachInterrupt(IRQ);
	sendGPS = true;
}

/**
 * Going into deep sleep STOPS the clock !!
 * Correct the millis() so the rest of our code keeps working as expected (elapsed time !!)
 * call with the correct amount to adjust after coming out of SLEEP
 * A better solution would be to use an external time triggered interrupt (RTC)
 */
extern volatile unsigned long timer0_millis;
unsigned long new_value = 0;
void addMillis(unsigned long inc_millis) {
	uint8_t oldSREG = SREG;
	cli();
	timer0_millis += inc_millis;
	SREG = oldSREG;
}

// sleep for 8s using watchdog
void loop() {
	if (connection) {
		sloop();

		debugSerial.print(F("FREEMem : "));
		debugSerial.print(freeRam());
		debugSerial.print(F(" bytes"));
		debugSerial.print(F("\t QueueSize : "));
		debugSerial.print(libTest.sendQueueSize());
		debugSerial.print(F("/"));
		debugSerial.print(LoRaModem::MAX_QUEUE);
		debugSerial.print(F("\t Can send immediately : "));
		debugSerial.println(libTest.canSendImmediately() ? "YES" : "NO");

		// Interrupt stuff
		{
			// Enter power down state with ADC and BOD module disabled.
			// Wake up when wake up pin is low.
			debugSerial.flush();
			debugSerial.end();
			// attachInterrupt(IRQ, wakeUp, FALLING);
			// could decide on deeper sleep if Queue is empty, but complete loss of time keeping !!!!

			LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
			// BUTTON !! :: coming from external interrupt will add too (even if perhaps not fully elapsed !!)
			addMillis(LowPower.delayValue(SLEEP_1S)); // correct elapsed time !!?? KEEP IN SYNC WITH SLEEP PERIOD !!!
			//detachInterrupt(IRQ);
			debugSerial.begin(SERIAL_BAUD);
		}
	} else {
		debugSerial.print(F("MODEM (4) : "));
		debugSerial.println(modem.getParam(MODEM));
		debugSerial.print(F("FREQ (1) : "));
		debugSerial.println(modem.getParam(FREQUENCYBAND));
		debugSerial.print(F("SF (6) : "));
		debugSerial.println(modem.getParam(SP_FACTOR));
		debugSerial.print(F("ADR (0) : "));
		debugSerial.println(modem.getParam(ADR));
		debugSerial.print(F("POW (11) : "));
		debugSerial.println(modem.getParam(POWER_INDEX));
		debugSerial.print(F("BW (1) : "));
		debugSerial.println(modem.getParam(BANDWIDTH));
		debugSerial.print(F("CR (0) : "));
		debugSerial.println(modem.getParam(CODING_RATE));
		debugSerial.print(F("DC (-1 / NA) : "));
		debugSerial.println(modem.getParam(DUTY_CYCLE));
		debugSerial.print(F("SNR (0) : "));
		debugSerial.println(modem.getParam(SNR));
		debugSerial.print(F("#GW (-1 / NA) : "));
		debugSerial.println(modem.getParam(GATEWAY_COUNT));
		debugSerial.print(F("#RETR (0) : "));
		debugSerial.println(modem.getParam(RETRANSMISSION_COUNT));
		debugSerial.print(F("DR (0) : "));
		debugSerial.println(modem.getParam(DATA_RATE));
		delay(120000);
	}
}
