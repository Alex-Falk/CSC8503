#pragma once
#include "State.h"

class Patrol : public State {
public:
	Patrol(Hazard * h) : State(h) {};
	~Patrol() {};

	virtual void On_Initialize() {
		_isActive = true;
		UpdateAStarPreset(h->current_idx, h->PickRandomNode());
	}

	virtual void Update() {
		const vector<int> avatars = h->avatar_idcs;
		bool los = false;

		for (int i = 0; i < avatars.size(); ++i) {
			los = Check_Los(avatars[i]);
			h->target = i;
			break;
		}

		if (los) {
			_isActive = false;
			h->SwitchState(PURSUE);
			return;
		}

		if (AtLastNode()) {
			UpdateAStarPreset(h->current_idx, h->PickRandomNode());
		}

		FollowPath();
	}
};