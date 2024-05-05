// Author: Cedric Millard
// This library manage a current sensor
// DO NOT USE the mother class "CurrentSensor" but use either CurrentSensorDC or CurrentSensorAC

#ifndef __CURRENT_SENSOR__
#define __CURRENT_SENSOR__

#include <Arduino.h>

class CurrentSensor {

public:
	CurrentSensor(uint8_t iSensorPin);
	byte GetCurrentByte();
	float GetCurrentFloat();
	
protected:
	int iValue;
	uint8_t sensorPin;
	virtual void MeasureCurrent() = 0;
};

class CurrentSensorAC : public CurrentSensor {

public:
	byte GetLastCurrentByte();
	float GetLastCurrentFloat();
	CurrentSensorAC(uint8_t iSensorPin);
	
protected:	
	void MeasureCurrent();

private:
	uint32_t start_time;
	int maxValue;
	int minValue;
	int nbValue;
};

class CurrentSensorDC : public CurrentSensor {

public:
	CurrentSensorDC(uint8_t iSensorPin);

protected:
	void MeasureCurrent();
};

#endif __CURRENT_SENSOR__
