# Arduino
Arduino and LoRa related stuff

Adaptation of the ATT LoRa kit libraries.

- Created classes for the different Sensor types for easier manipulation.
- Added features to help a developer adhere better to the LoRa specifications (Sending speed, packet size, ...)
- Added support for mDot modem



Basic usage :
(.ino file has a lot of samples in there, but ...)

All available modem implementations rely on serial communications, so let's start with that.

(for the ATT kits we can use the Serial1 on an Arduino nano we can opt to use SoftSerial)
```
#define MODEM_SERIAL Serial1
#define debugSerial Serial
// Let's also define a serial port on which the modem may put debug information
// I tried to make the code as generic as possible ans as such make some use of these #define 
// and #if defined compiler directives for conditional compilation
```

With the serial definition, select a modem implementation and create an object.
```
#include "LoRaModemMDot.h"
LoRaModemMDot modem(&MODEM_SERIAL, &debugSerial);
```
With these, we can create an instance of 'Device' representing our sensing device :

```
#include "device.h"
Device mySensingDevice(&modem, &debugSerial);
```

Now we'll make a function to read out the physical sensor value and send it onto the LoRa network:
```
#include "sensor.h"

float readTemperatureValue(){
    // MOCK ...  gonna leave it as an exercise to read out a real sensor
    return 18.5;
}

void readAndSend(){
    TemperatureSensor fSens(readTemperatureValue());
    mySensingDevice.sendSafe(fSens);
}
```

The 'sendSafe' convenience method will help in adhering to LoRaWAN specifications with regards to
the delay between sends, the size of messages which all depend on the quality of the connection.


Please feel free to comment, improve, suggest .......

The EnCo team.