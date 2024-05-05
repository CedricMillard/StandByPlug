// Author: Cedric Millard
// This library manage a latching relay type OMRON G5RL-U-E
// Relay is triggered through a H bridge piloted from 2 inputs

#ifndef __LATCHING_RELAY__
#define __LATCHING_RELAY__

#include <Arduino.h>

class LatchingRelay {

public:
	//Initiatlize the latching relay
	//@param iPinBridgeH1 - The 1st pin number controlling the H bridge
	//@param iPinBridgeH2 - The 2nd pin number controlling the H bridge
	//@param ibIsOn - Current state of the latching relay (true = On ; false = Off)
	LatchingRelay(uint8_t iPinBridgeH1, uint8_t iPinBridgeH2, bool ibIsOn);
	
	//Switch the relay state (from On to Off or Off to On)
	void Switch();
	
	//Turn On the relay (even if already On)
	void TurnOn();
	
	//Turn Off the relay (even if already Off)
	void TurnOff();
	
	//Return the status of the latching relay
	//@return - true = relay is On ; false = relay is Off
	bool GetStatus();
	
	//Set Status of the Relay
	//@param ibIsOn - true = relay is On, false = relay is Off
	void SetStatus(bool ibIsOn);

private:
	bool bIsOn;
	uint8_t pinBridgeH1;
	uint8_t pinBridgeH2;
};

#endif
