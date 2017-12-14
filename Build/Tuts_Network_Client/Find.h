#pragma once
#include "Pursue.h"

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