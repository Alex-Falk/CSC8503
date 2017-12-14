#include "Chase.h"
#include "Hazard.h"

void Chase::Update() {
	h->last_known_pos = h->avatar_idcs[h->target];

	bool los = Check_Los(h->avatar_idcs[h->target]);
	if (!los) {
		_isActive = false;
		h->SwitchState(PURSUE_FIND);
		return;
	}

	if (h->path.size() > 0) {
		auto it = h->path.end();
		--it;
		if ((*it)->_idx == h->current_idx) {
			h->last_known_pos = h->avatar_idcs[h->target];
			UpdateAStarPreset(h->current_idx, h->last_known_pos);
		}
	}
	else if (h->last_known_pos != h->avatar_idcs[h->target] || h->path.size() == 0)
	{
		h->last_known_pos = h->avatar_idcs[h->target];
		UpdateAStarPreset(h->current_idx, h->last_known_pos);
	}

	FollowPath();
}