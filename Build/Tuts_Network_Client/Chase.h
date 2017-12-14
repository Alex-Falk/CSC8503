#pragma once
#include "Pursue.h"

class Hazard;

class Chase : public Pursue {
public:
	Chase(Hazard * h) : Pursue(h) {}
	~Chase() {};


	virtual void On_Initialize() {
		_isActive = true;
		UpdateAStarPreset(h->current_idx, h->avatar_idcs[h->target]);
	}

	virtual void Update();
};