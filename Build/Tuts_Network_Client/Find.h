#pragma once
#include "Pursue.h"


// A state in the Hiercharchical FSM which handles the Hazard moving to the last known position fo the avatar after it lost sight of it

class Find : public Pursue {
public:
	Find(Hazard * h) : Pursue(h) {};

	~Find() {};

	virtual void On_Initialize() {
		_isActive = true;
		UpdateAStarPreset(h->current_idx, h->last_known_pos);
	}

	virtual void Update();

};