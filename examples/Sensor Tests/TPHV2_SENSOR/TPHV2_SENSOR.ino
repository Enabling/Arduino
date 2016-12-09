#include <Sodaq_DS3231.h>


#define debugSerial Serial

#include "keys.h"
#include "device.h"
#include "sensor.h"
#include "LoRaModemMDot.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SERIAL_BAUD 57600

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

#include "LoRaModemMicrochip.h"
LoRaModemMicrochip modem(&MODEM_SERIAL, &debugSerial);
Device mySensingDevice(&modem, &debugSerial);

Adafruit_BME280 bme; // I2C

bool connection = false;
volatile boolean canSendFromQueue = false;

void wakeUP_RN2483() {
#ifdef LORAMODEMMICROCHIP_H_
  debugSerial.println("Attempting RN2483 Wakeup");
  MODEM_SERIAL.end();
  pinMode(PIN_TX_RN2483, OUTPUT);
  digitalWrite(PIN_TX_RN2483, LOW);
  delay(5);
  digitalWrite(PIN_TX_RN2483, HIGH);
  MODEM_SERIAL.begin(modem.getDefaultBaudRate());
  MODEM_SERIAL.write(0x55);
  debugSerial.println("END RN2483 Wakeup");
#endif
}

void attemptModemConnection() {
  byte cntWakeUp = 0;
  while (!connection) {
    wakeUP_RN2483(); // RN2483 specific -> see if we can move it into modem class itself
    cntWakeUp++;
    if (mySensingDevice.connect(DEV_ADDR, APPSKEY, NWKSKEY, true)) {
      debugSerial.println(F("Connection to the network was successful."));
      connection = true;
      modem.storeTimeOnAirBudget(30000);
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
        debugSerial.println(F("Connection to the network failed!\n--== MoDeM init retry shortly ==--"));
        delay(5000);
#ifdef PIN_PWR_RN2483
      }
#endif
    }
  }
}

void setup() 
{
#ifdef PIN_PWR_RN2483
  pinMode(PIN_PWR_RN2483, OUTPUT);
  digitalWrite(PIN_PWR_RN2483, HIGH);
#endif
  debugSerial.begin(SERIAL_BAUD);
  debugSerial.println("Starting .....");
  MODEM_SERIAL.begin(modem.getDefaultBaudRate());

  attemptModemConnection();

  // INT0
  //pinMode(2, INPUT_PULLUP);
  
  if (!bme.begin()) {
    debugSerial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  debugSerial.println("Ready to send data");    
  debugSerial.println(F("--------------------------------------------"));

}

void sendTemperatureSensor(){
  float temp = bme.readTemperature();
  debugSerial.print("send value:");
  debugSerial.println(temp);
  TemperatureSensor fSens(temp);
  bool sendResult = mySensingDevice.send(fSens);
  debugSerial.print(F("Send succeeded : "));
  debugSerial.println(sendResult ? "YES" : "NO");
}

void loop() 
{
  if (connection) {
    sendTemperatureSensor();
  }
  
  delay(30000);
}


