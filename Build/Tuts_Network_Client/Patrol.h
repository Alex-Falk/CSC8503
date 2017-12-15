#pragma once
#include "State.h"

// State in the Hiercharichal FSM which controls Patrolling. The Hazard will create a path to a random node,
// following this path and then find a new random pass unless it sees an avatar on its way 

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
		bool los = false;	// has Line of sight

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

		if (AtLastNode()) {		// At the end of current path
			UpdateAStarPreset(h->current_idx, h->PickRandomNode());
		}

		FollowPath();
	}
};