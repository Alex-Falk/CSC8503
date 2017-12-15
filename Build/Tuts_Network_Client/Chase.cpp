#include "Chase.h"
#include "Hazard.h"

void Chase::Update() {
	h->last_known_pos = h->avatar_idcs[h->target];

	// If avatar no longer exists (because of disconnection or "killed" by other hazard)
	if (h->avatar_idcs[h->target] == OUT_OF_RANGE) {
		h->SwitchState(PATROL);
		return;
	}

	// If looses sight of the target avatar then switch to find mode
	bool los = Check_Los(h->avatar_idcs[h->target]);
	if (!los) {
		_isActive = false;
		h->SwitchState(PURSUE_FIND);
		return;
	}

	// If the current path actually has length Update Astar to go to the avatars current position (aka 'last_known_position')
	if (h->path.size() > 0) {
		auto it = h->path.end();
		--it;
		if ((*it)->_idx == h->current_idx) {
			h->last_known_pos = h->avatar_idcs[h->target];
			UpdateAStarPreset(h->current_idx, h->last_known_pos);
		}
	}
	// if the last known position is not the same as the avatars current position and the hazard has reached the end of its path,
	// then update the pathfinding with the last known position of the target avatar
	else if (h->last_known_pos != h->avatar_idcs[h->target] || h->path.size() == 0)
	{
		h->last_known_pos = h->avatar_idcs[h->target];
		UpdateAStarPreset(h->current_idx, h->last_known_pos);
	}

	// follow the path generated by A*
	FollowPath();
}