#pragma once
#include "MazeGenerator.h"
#include "SearchAStar.h"
class State;

class Hazard {
public:
	bool _isActive = true;

	float lerp_factor = 0.0f;
	int target = OUT_OF_RANGE;
	vector<int> avatar_idcs;

	State * current_state;
	const MazeGenerator * maze;

	int current_idx;
	Vector3 current_pos;

	// Pathfinding
	int last_known_pos = OUT_OF_RANGE;
	int astar_preset_idx;
	std::string astar_preset_text;
	std::list<const GraphNode*> path;
	SearchAStar * search_as;

	// States
	State * patrol_state		= nullptr;
	State * pursue_state		= nullptr;
	// Substates of Pursue (Hierarchical)
	State * pursue_chase_state	= nullptr;
	State * pursue_find_state	= nullptr;


	Hazard(MazeGenerator * maze);
	~Hazard();

	void SetMaze(MazeGenerator * maze) { this->maze = maze; };

	virtual void Update();

	virtual void SwitchState(State_enum s);

	virtual void UpdateAvatars(vector<int> avatars) {
		avatar_idcs = avatars;
	}

	int PickRandomNode() {
		int num_nodes = maze->size * maze->size;
		int randomNode = rand() % num_nodes;
		return randomNode;
	}

	void SetStartNode(int i) {
		if (i == OUT_OF_RANGE)
			current_idx = PickRandomNode();
		else
			current_idx = i;
		current_pos = maze->GetNode(current_idx)->_pos;
	}

};