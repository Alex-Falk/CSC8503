#pragma once
#include "Pursue.h"

class Find : public Pursue {
public:
	Find(const MazeGenerator * maze, State * parent,Pursue * pursue_parent, int target, int lkp) : Pursue(maze, parent, target) {
		this->target = target;
		last_known_pos = lkp;
		this->pursue_parent = pursue_parent;
	};
	~Find() {};

	virtual void On_Initialize() {
		_isActive = true;
		target = parent->target;
		avatar_idcs = parent->avatar_idcs;
		current_idx = parent->current_idx;
		search_as = parent->search_as;
		UpdateAStarPreset(current_idx, last_known_pos);
		lerp_factor = parent->lerp_factor;
		current_pos = parent->current_pos;
		last_known_pos = pursue_parent->last_known_pos;
		PushToParent();
	}

	virtual void Update() {
		bool los = Check_Los(avatar_idcs[target]);
		if (los) {
			_isActive = false;
			PushToParent();
			parent->SwitchState(PURSUE_CHASE);
			return;
		}

		if (last_known_pos != current_idx) {
			UpdateAStarPreset(current_idx, last_known_pos);
			FollowPath();
		}
		else {
			_isActive = false;
			PushToParent();
			parent->SwitchState(PATROL);
			return;
		}

		PushToParent();
	}

};