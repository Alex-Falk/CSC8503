#include "Find.h"
#include "Hazard.h"

void Find::Update() {
	bool los = Check_Los(h->avatar_idcs[h->target]);


	if (h->avatar_idcs[h->target] == OUT_OF_RANGE) {
		h->SwitchState(PATROL);
		return;
	}

	if (los) {
		_isActive = false;
		h->SwitchState(PURSUE_CHASE);
		return;
	}

	if (h->last_known_pos != h->current_idx) {
		UpdateAStarPreset(h->current_idx, h->last_known_pos);
		FollowPath();
	}
	else {
		_isActive = false;
		h->SwitchState(PATROL);
		return;
	}
}