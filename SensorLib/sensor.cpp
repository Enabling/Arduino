#include "sensor.h"

void Sensor::updateTimestamp() {
	_timestamp = millis();
}
Sensor::Sensor(int streamId) {
	_streamId = streamId;
}
Sensor::~Sensor() {
}

EnCoSensor::EnCoSensor() :
		Sensor(0) {
}
// //////////
BinarySensor::BinarySensor() :
		Sensor(1) {
}
BinarySensor::BinarySensor(int id) :
		Sensor(id) {
}
BinarySensor::BinarySensor(bool sensorValue) :
		Sensor(1) {
	_sensorValue = sensorValue;
}
bool BinarySensor::value() {
	return _sensorValue;
}
void BinarySensor::value(bool sensorValue) {
	_sensorValue = sensorValue;
}
LoRaPacket* BinarySensor::getAsBinary() {
	_data.reset();
	_data.SetId(_streamId);
	_data.Add(_sensorValue);
	return &_data;
}
// //////////
IntegerSensor::IntegerSensor() :
		Sensor(15) {
}
IntegerSensor::IntegerSensor(int sensorValue) :
		Sensor(15) {
	_sensorValue = sensorValue;
}
int IntegerSensor::value() {
	return _sensorValue;
}
void IntegerSensor::value(int sensorValue) {
	_sensorValue = sensorValue;
}
LoRaPacket* IntegerSensor::getAsBinary() {
	_data.reset();
	_data.SetId(_streamId);
	_data.Add(_sensorValue);
	return &_data;
}
// //////////
FloatSensor::FloatSensor(int id) :
		Sensor(id) {
}
FloatSensor::FloatSensor() :
		Sensor(16) {
}
FloatSensor::FloatSensor(float sensorValue) :
		Sensor(16) {
	_sensorValue = sensorValue;
}
float FloatSensor::value() {
	return _sensorValue;
}
void FloatSensor::value(float sensorValue) {
	_sensorValue = sensorValue;
}
LoRaPacket* FloatSensor::getAsBinary() {
	_data.reset();
	_data.SetId(_streamId);
	_data.Add(_sensorValue);
	return &_data;
}
// //////////
BinaryTiltSensor::BinaryTiltSensor() :
		BinarySensor(2) {
}
BinaryTiltSensor::BinaryTiltSensor(bool sensorValue) :
		BinarySensor(2) {
	_sensorValue = sensorValue;
}
// //////////
PushButton::PushButton() :
		BinarySensor(3) {
}
PushButton::PushButton(bool sensorValue) :
		BinarySensor(3) {
	_sensorValue = sensorValue;
}
// //////////
DoorSensor::DoorSensor() :
		BinarySensor(4) {
}
DoorSensor::DoorSensor(bool sensorValue) :
		BinarySensor(4) {
	_sensorValue = sensorValue;
}
// //////////
TemperatureSensor::TemperatureSensor() :
		FloatSensor(5) {
}
TemperatureSensor::TemperatureSensor(float sensorValue) :
		FloatSensor(5) {
	_sensorValue = sensorValue;
}
// //////////
LightSensor::LightSensor() :
		FloatSensor(6) {
}
LightSensor::LightSensor(float sensorValue) :
		FloatSensor(6) {
	_sensorValue = sensorValue;
}
// //////////
MotionSensor::MotionSensor() :
		BinarySensor(7) {
}
MotionSensor::MotionSensor(bool sensorValue) :
		BinarySensor(7) {
	_sensorValue = sensorValue;
}
// //////////
Accelerometer::Accelerometer() :
		Sensor(8) {
}
Accelerometer::Accelerometer(float accelX, float accelY, float accelZ) :
		Sensor(8) {
	_accelX = accelX;
	_accelY = accelY;
	_accelZ = accelZ;
}
void Accelerometer::setX(float accelX) {
	_accelX = accelX;
}
float Accelerometer::getX() {
	return _accelX;
}
void Accelerometer::setY(float accelY) {
	_accelY = accelY;
}
float Accelerometer::getY() {
	return _accelY;
}
void Accelerometer::setZ(float accelZ) {
	_accelZ = accelZ;
}
float Accelerometer::getZ() {
	return _accelZ;
}
LoRaPacket* Accelerometer::getAsBinary() {
	_data.reset();
	_data.SetId(_streamId);
	_data.Add(_accelX);
	_data.Add(_accelY);
	_data.Add(_accelZ);
	return &_data;
}
// //////////
GPSSensor::GPSSensor() :
		Sensor(9) {
}
GPSSensor::GPSSensor(float longitude, float latitude, float alt,
		float timestamp) :
		Sensor(9) {
	_longitude = longitude;
	_latitude = latitude;
	_altitude = alt;
	_timestamp = timestamp;
}
void GPSSensor::setLongitude(float longitude) {
	_longitude = longitude;
}
float GPSSensor::getLongitude() {
	return _longitude;
}
void GPSSensor::setLatitude(float latitude) {
	_latitude = latitude;
}
float GPSSensor::getLatitude() {
	return _latitude;
}
void GPSSensor::setAltitude(float alt) {
	_altitude = alt;
}
float GPSSensor::getAltitude() {
	return _altitude;
}
void GPSSensor::setTimestamp(float timestamp) {
	_timestamp = timestamp;
}
float GPSSensor::getTimestamp() {
	return _timestamp;
}
LoRaPacket* GPSSensor::getAsBinary() {
	_data.reset();
	_data.SetId(_streamId);
	_data.Add(_latitude);
	_data.Add(_longitude);
	_data.Add(_altitude);
	_data.Add(_timestamp);
	return &_data;
}
// //////////
PressureSensor::PressureSensor() :
		FloatSensor(10) {
}
PressureSensor::PressureSensor(float sensorValue) :
		FloatSensor(10) {
	_sensorValue = sensorValue;
}
// //////////
HumiditySensor::HumiditySensor() :
		FloatSensor(11) {
}
HumiditySensor::HumiditySensor(float sensorValue) :
		FloatSensor(11) {
	_sensorValue = sensorValue;
}
// //////////
LoudnessSensor::LoudnessSensor() :
		FloatSensor(12) {
}
LoudnessSensor::LoudnessSensor(float sensorValue) :
		FloatSensor(12) {
	_sensorValue = sensorValue;
}
// //////////
AirQualitySensor::AirQualitySensor() :
		IntegerSensor() {
	_streamId = 13;
}
AirQualitySensor::AirQualitySensor(int sensorValue) :
		IntegerSensor() {
	_streamId = 13;
	_sensorValue = sensorValue;
}
// //////////
BatteryLevel::BatteryLevel() :
		IntegerSensor() {
	_streamId = 14;
}
BatteryLevel::BatteryLevel(int sensorValue) :
		IntegerSensor() {
	_streamId = 14;
	_sensorValue = sensorValue;
}
// //////////
BinaryPayload::~BinaryPayload() {
	if (_sensorValue != NULL) {
		free(_sensorValue);
	}
}
BinaryPayload::BinaryPayload() :
		Sensor(17) {
}
BinaryPayload::BinaryPayload(const uint8_t* binary, const uint8_t dataLen) :
		Sensor(17) {
	value(binary, dataLen);
}
LoRaPacket* BinaryPayload::getAsBinary() {
	_data.reset();
	_data.SetId(_streamId);
	_data.Add(_sensorValue, _dataLength);
	return &_data;
}
void BinaryPayload::value(const uint8_t* binary, const uint8_t dataLen) {
	if (_sensorValue != NULL) {
		free(_sensorValue);
		_dataLength = 0;
	}
	if (binary != NULL && dataLen > 0) {
		_sensorValue = (uint8_t*) malloc(dataLen);
		if (_sensorValue) {
			memcpy(_sensorValue, binary, dataLen);
			_dataLength = dataLen;
		} // else ERROR !!!!
	}
}
// TODO : Use as base for better defined sensor
LoRaPacket* EnCoSensor::getAsBinary() {
	_data.reset();
	_data.Add(1, "13CharsRandom", 13);
	_data.Add(2, (short) 255);
	_data.Add(3, (String) "0123456789ABCdeF");
	return &_data;
}
