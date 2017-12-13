#pragma once
#include "State.h"

class Chase;
class Find;

class Pursue : public State {
public:
	Pursue(const MazeGenerator * maze, State * parent, int target) : State(maze, parent) {
		this->target = target;
		pursue_parent = nullptr;
	};
	~Pursue() {};

	Pursue * current_pursue_state;
	Pursue * pursue_parent;

	Pursue * chase_state;
	Pursue * find_state;

	int last_known_pos = -1;

	virtual void UpdateAvatars(vector<int> avatars) {
		avatar_idcs = avatars;
		if (current_pursue_state) {
			current_pursue_state->UpdateAvatars(avatars);
		}
	}

	virtual void PushToParent() {
		if (parent) {
			parent->target = target;
			parent->avatar_idcs = avatar_idcs;
			parent->current_idx = current_idx;
			parent->current_pos = current_pos;
			parent->lerp_factor = lerp_factor;
		}
		if (pursue_parent) {
			pursue_parent->last_known_pos = last_known_pos;
		}
	}

	virtual void SwitchState(State_enum s);

	virtual void On_Initialize();

	virtual void Update() {
		current_pursue_state->Update();
		last_known_pos = current_pursue_state->last_known_pos;
		PushToParent();
	}

};