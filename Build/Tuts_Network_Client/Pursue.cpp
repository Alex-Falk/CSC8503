#include "Pursue.h"
#include "Find.h"
#include "Chase.h"

void Pursue::On_Initialize() {
	_isActive = true;
	target = parent->target;
	avatar_idcs = parent->avatar_idcs;
	current_idx = parent->current_idx;
	search_as = parent->search_as;
	lerp_factor = parent->lerp_factor;
	current_pos = parent->current_pos;
	if (avatar_idcs.size() > 0)
		last_known_pos = avatar_idcs[target];
	else
		last_known_pos = 0;
	chase_state = new Chase(maze, this, this, target, last_known_pos);
	find_state = new Find(maze, this, this, target, last_known_pos);

	chase_state->On_Initialize();
	find_state->On_Initialize();

	current_pursue_state = chase_state;
}

void Pursue::SwitchState(State_enum s) {
	switch (s) {
	case PURSUE_CHASE:
		if (!chase_state) { chase_state = new Chase(maze, this, this, target, last_known_pos); }
		current_pursue_state = chase_state;
		PushToParent();
		break;
	case PURSUE_FIND:
		if (!find_state) { find_state = new Find(maze, this, this, target, last_known_pos); }
		current_pursue_state = find_state;
		PushToParent();
		break;
	case PATROL:
		_isActive = false;
		PushToParent();
		parent->SwitchState(s);
		return;
		break;
	}

	current_pursue_state->On_Initialize();
}