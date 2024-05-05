#include "CurrentSensor.h"

CurrentSensor::CurrentSensor(uint8_t iSensorPin) {
	sensorPin = iSensorPin;
	iValue = -1;
  pinMode(sensorPin, INPUT);
}

byte CurrentSensor::GetCurrentByte() {
	MeasureCurrent();
	return (long) iValue * 255 / 380.0;
}

float CurrentSensor::GetCurrentFloat() {
	MeasureCurrent();
	return (float)iValue *2500000.0 / 1024.0 / 185.0;
}

CurrentSensorAC::CurrentSensorAC(uint8_t iSensorPin) : CurrentSensor(iSensorPin){
	minValue = 1024;
	maxValue = 0;
	nbValue = 0;
}

void CurrentSensorAC::MeasureCurrent() {
	int readValue = 0;
	
	//This allow to always get a value to return;
	do {
		readValue = analogRead(sensorPin);

		// see if you have a new maxValue
		if (readValue > maxValue)
		{
			/*record the maximum sensor value*/
			maxValue = readValue;
		}
		if (readValue < minValue)
		{
			/*record the minimum sensor value*/
			minValue = readValue;
		}
		nbValue++;

		//We publish a value every second (processor running at 8MHz + delay to read = half)
		if (nbValue >= 4000)
		{
			iValue = maxValue - minValue;
			minValue = 1024;
			maxValue = 0;
			nbValue = 0;
		}
	} while (iValue == -1);
}

CurrentSensorDC::CurrentSensorDC(uint8_t iSensorPin) : CurrentSensor(iSensorPin) {

}

void CurrentSensorDC::MeasureCurrent() {
	iValue = analogRead(sensorPin);
}
