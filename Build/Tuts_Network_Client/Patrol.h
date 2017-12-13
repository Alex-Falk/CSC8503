#pragma once
#include "State.h"

class Patrol : public State {
public:
	Patrol(const MazeGenerator * maze, State * parent) : State(maze, parent) {};
	~Patrol() {};

	virtual void On_Initialize() {
		_isActive = true;
		target = parent->target;
		avatar_idcs = parent->avatar_idcs;
		current_idx = parent->current_idx;
		search_as = parent->search_as;
		lerp_factor = parent->lerp_factor;
		current_pos = parent->current_pos;
		UpdateAStarPreset(current_idx, PickRandomNode());
	}

	virtual void Update() {
		const vector<int> avatars = avatar_idcs;
		bool los = false;

		for (int i = 0; i < avatars.size(); ++i) {
			los = Check_Los(avatars[i]);
			target = i;
			break;
		}

		if (los) {
			_isActive = false;
			PushToParent();
			parent->SwitchState(PURSUE);
			return;
		}

		if (AtLastNode()) {
			UpdateAStarPreset(current_idx, PickRandomNode());
		}

		FollowPath();
		PushToParent();
	}
};