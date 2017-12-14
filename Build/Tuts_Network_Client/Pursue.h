#pragma once
#include "State.h"

class Chase;
class Find;

class Pursue : public State {
public:
	Pursue(Hazard * h) : State(h) {};
	~Pursue() {};

	virtual void On_Initialize();

	virtual void Update() {};

};