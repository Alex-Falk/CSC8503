#pragma once
#include "State.h"

class Chase;
class Find;

// Pursue state in the FSM that does not do anything but itself but handles the Chase and Find states.

class Pursue : public State {
public:
	Pursue(Hazard * h) : State(h) {};
	~Pursue() {};

	virtual void On_Initialize();

	virtual void Update() {};

};