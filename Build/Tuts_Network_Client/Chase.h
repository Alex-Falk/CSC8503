#pragma once
#include "Pursue.h"

class Chase : public Pursue {
public:
	Chase(const MazeGenerator * maze, State * parent, Pursue * pursue_parent, int target, int lkp) : Pursue(maze, parent, target)
	{
		this->target = target;
		last_known_pos = lkp;
		this->pursue_parent = pursue_parent;
	};
	~Chase() {};


	virtual void On_Initialize() {
		_isActive = true;
		target = parent->target;
		avatar_idcs = parent->avatar_idcs;
		current_idx = parent->current_idx;
		if (avatar_idcs.size() > 0)
			last_known_pos = avatar_idcs[target];
		else
			last_known_pos = 0;
		search_as = parent->search_as;
		lerp_factor = parent->lerp_factor;
		current_pos = parent->current_pos;
	}

	virtual void Update() {
		bool los = Check_Los(avatar_idcs[target]);
		if (!los) {
			_isActive = false;
			PushToParent();
			parent->SwitchState(PURSUE_FIND);
			return;
		}

		if (path.size() > 0) {
			auto it = path.end();
			--it;
			if ((*it)->_idx == current_idx) {
				last_known_pos = avatar_idcs[target];
				UpdateAStarPreset(current_idx, last_known_pos);
			}
		} else if (last_known_pos != avatar_idcs[target] || path.size() == 0)
		{
			last_known_pos = avatar_idcs[target];
			UpdateAStarPreset(current_idx, last_known_pos);
		}

		FollowPath();
		PushToParent();
	}
};