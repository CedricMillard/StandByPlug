#include "LatchingRelay.h"

LatchingRelay::LatchingRelay(uint8_t iPinBridgeH1, uint8_t iPinBridgeH2, bool ibIsOn) {
	pinBridgeH1 = iPinBridgeH1;
	pinBridgeH2 = iPinBridgeH2;
	bIsOn = ibIsOn;
  pinMode(pinBridgeH1, OUTPUT);
  pinMode(pinBridgeH2, OUTPUT);
  digitalWrite(pinBridgeH1, LOW);
  digitalWrite(pinBridgeH2, LOW);
}

void LatchingRelay::Switch() {
	if (bIsOn)
	{
		digitalWrite(pinBridgeH1, HIGH);
		delay(100);
		digitalWrite(pinBridgeH1, LOW);
	}
	else
	{
		digitalWrite(pinBridgeH2, HIGH);
		delay(100);
		digitalWrite(pinBridgeH2, LOW);
	}
  bIsOn = !bIsOn;
}

void LatchingRelay::TurnOn() {
	bIsOn = true;
	digitalWrite(pinBridgeH2, HIGH);
	delay(100);
	digitalWrite(pinBridgeH2, LOW);
}

void LatchingRelay::TurnOff() {
	bIsOn = false;
	digitalWrite(pinBridgeH1, HIGH);
	delay(100);
	digitalWrite(pinBridgeH1, LOW);
}

bool LatchingRelay::GetStatus(){
	return bIsOn;
}

void LatchingRelay::SetStatus(bool ibIsOn) {
	bIsOn = ibIsOn;
}
