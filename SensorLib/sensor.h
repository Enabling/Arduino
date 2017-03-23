#ifndef SENSOR_H_
#define SENSOR_H_

#include <arduino.h>
#include "DataPacket.h"
#include "EnCoPacket.h"

class Sensor {
protected:
	DataPacket _data;
	int _streamId = 0;
	unsigned long _timestamp = 0;
	void updateTimestamp();
	Sensor(int streamId);
public:
  virtual LoRaPacket* getAsBinary() = 0;
  virtual int getAsJson(char* into) = 0;
	virtual ~Sensor();
};

//////////////////////////////  EnCo sensor types (simple values)
class EnCoSensor: public Sensor {
protected:
	EnCoPacket _data;
public:
	EnCoSensor();
	LoRaPacket* getAsBinary();
};
//////////////////////////////  EnCo sensor types (simple values)

//////////////////////////////  Base sensor types (simple values)
class BinarySensor: public Sensor {
protected:
	BinarySensor(int id);bool _sensorValue = false;

public:
	BinarySensor();
	BinarySensor(bool sensorValue);
	LoRaPacket* getAsBinary();
  int getAsJson(char* into);
	bool value();
	void value(bool sensorValue);
};

class IntegerSensor: public Sensor {
protected:
	int _sensorValue = 0;

public:
	IntegerSensor();
	IntegerSensor(int sensorValue);
	LoRaPacket* getAsBinary();
  int getAsJson(char* into);
	int value();
	void value(int sensorValue);
};

class FloatSensor: public Sensor {
protected:
	FloatSensor(int id);
	float _sensorValue = 0;

public:
	FloatSensor();
	FloatSensor(float sensorValue);
	LoRaPacket* getAsBinary();
  int getAsJson(char* into);
	float value();
	void value(float sensorValue);
};

//////////////////////////////////////////////////////////////////////////////////
//////////////////BinaryTiltSensor/////////////////////
class BinaryTiltSensor: public BinarySensor {
public:
	BinaryTiltSensor();
	BinaryTiltSensor(bool sensorValue);
};
//////////////////PushButton/////////////////////
class PushButton: public BinarySensor {
public:
	PushButton();
	PushButton(bool sensorValue);
};
/////////////////DoorSensor////////////////////// 
class DoorSensor: public BinarySensor {
public:
	DoorSensor();
	DoorSensor(bool sensorValue);
};
/////////////////TemperatureSensor//////////////////////
class TemperatureSensor: public FloatSensor {
public:
	TemperatureSensor();
	TemperatureSensor(float sensorValue);
};
/////////////////LightSensor//////////////////////
class LightSensor: public FloatSensor {
public:
	LightSensor();
	LightSensor(float sensorValue);
};
//////////////////MotionSensor/////////////////////
class MotionSensor: public BinarySensor {
public:
	MotionSensor();
	MotionSensor(bool sensorValue);
};
//////////////////Accelerometer/////////////////////
class Accelerometer: public Sensor {
private:
	float _accelX = 0;
	float _accelY = 0;
	float _accelZ = 0;

public:
	Accelerometer();
	Accelerometer(float accelX, float accelY, float accelZ);
	virtual LoRaPacket* getAsBinary();
  int getAsJson(char* into);
	void setX(float accelX);
	float getX();
	void setY(float accelY);
	float getY();
	void setZ(float accelZ);
	float getZ();
};
/////////////////GPSSensor//////////////////////
class GPSSensor: public Sensor {
private:
	float _longitude = 0;
	float _latitude = 0;
	float _altitude = 0;
	float _timestamp = 0;

public:
	GPSSensor();
	GPSSensor(float longitude, float latitude, float alt, float timestamp);
	virtual LoRaPacket* getAsBinary();
  int getAsJson(char* into);
	void setLongitude(float longitude);
	float getLongitude();
	void setLatitude(float latitude);
	float getLatitude();
	void setAltitude(float alt);
	float getAltitude();
	void setTimestamp(float timestamp);
	float getTimestamp();
};
/////////////////PressureSensor//////////////////////
class PressureSensor: public FloatSensor {
public:
	PressureSensor();
	PressureSensor(float sensorValue);
};
/////////////////HumiditySensor//////////////////////
class HumiditySensor: public FloatSensor {
public:
	HumiditySensor();
	HumiditySensor(float sensorValue);
};
/////////////////LoudnessSensor//////////////////////
class LoudnessSensor: public FloatSensor {
public:
	LoudnessSensor();
	LoudnessSensor(float sensorValue);
};
/////////////////AirQualitySensor//////////////////////
class AirQualitySensor: public IntegerSensor {
public:
	AirQualitySensor();
	AirQualitySensor(int sensorValue);
};
/////////////////BatteryLevel//////////////////////
class BatteryLevel: public IntegerSensor {
public:
	BatteryLevel();
	BatteryLevel(int sensorValue);
};
/////////////////BinaryPayload//////////////////////
class BinaryPayload: public Sensor {
protected:
	char* _sensorValue = NULL;
	uint8_t _dataLength = 0;

public:
	~BinaryPayload();
	BinaryPayload();
	BinaryPayload(const uint8_t* binary,const uint8_t dataLen);
	virtual LoRaPacket* getAsBinary();
  int getAsJson(char* into);
	void value(const uint8_t* binary, const uint8_t dataLen);
};

#endif  /* SENSOR_H_ */
