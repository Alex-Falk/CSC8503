#pragma once
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "Server.h"

enum State_enum {PATROL,PURSUE,PURSUE_CHASE,PURSUE_FIND};

class State {
protected:

	State * current_state;
	State * patrol_state;
	State * pursue_state;

	const MazeGenerator * maze;

	// Pathfinding
	int astar_preset_idx;
	std::string astar_preset_text;
	std::list<const GraphNode*> path;

	// Positioning/Moving


	bool _isActive = false;
public:
	float lerp_factor = 0.0f;
	int target;
	vector<int> avatar_idcs;
	int current_idx;
	Vector3 current_pos;

	State * parent;
	SearchAStar* search_as;

	State(State * parent = nullptr) {
		this->parent = parent;
		current_state = nullptr;
	};
	State(const MazeGenerator * maze, State * parent = nullptr) {
		this->parent = parent;
		current_state = nullptr;
		this->maze = maze;
	}

	~State() {
		SAFE_DELETE(patrol_state);
		SAFE_DELETE(pursue_state);
		SAFE_DELETE(search_as);
	}

	void SetMaze(MazeGenerator * maze) { this->maze = maze; };

	virtual void On_Initialize();

	virtual void Update() {
		current_state->Update();
	};

	virtual void SwitchState(State_enum s);

	virtual void PushToParent() {
		if (parent) {
			parent->target = target;
			parent->avatar_idcs = avatar_idcs;
			parent->current_idx = current_idx;
			parent->current_pos = current_pos;
			parent->lerp_factor = lerp_factor;
		}
	}

	virtual void UpdateAvatars(vector<int> avatars) {
		avatar_idcs = avatars;
		if (current_state) {
			current_state->UpdateAvatars(avatars);
		}
	}

	bool AtLastNode() {
		
		if (path.size() > 0) {
			auto it = path.end();
			--it;
			if (current_idx == (*it)->_idx) {
				return true;
			}
			else {
				return false;
			}
		}
		return true;
	}

	void UpdateAStarPreset(int s, int e)
	{
		//Example presets taken from:
		// http://movingai.com/astar-var.html
		float weightingG, weightingH;
		switch (astar_preset_idx)
		{
		default:
		case 0:
			//Only distance from the start node matters - fans out from start node
			weightingG = 1.0f;
			weightingH = 0.0f;
			astar_preset_text = "Dijkstra - None heuristic search";
			break;
		case 1:
			//Only distance to the end node matters
			weightingG = 0.0f;
			weightingH = 1.0f;
			astar_preset_text = "Pure Hueristic search";
			break;
		case 2:
			//Equal weighting
			weightingG = 1.0f;
			weightingH = 1.0f;
			astar_preset_text = "Traditional A-Star";
			break;
		case 3:
			//Greedily goes towards the goal node where possible, but still cares about distance travelled a little bit
			weightingG = 1.0f;
			weightingH = 2.0f;
			astar_preset_text = "Weighted Greedy A-Star";
			break;
		}
		search_as->SetWeightings(weightingG, weightingH);

		GraphNode* start = maze->GetNode(s);
		GraphNode* end = maze->GetNode(e);

		if (start && end) {
			search_as->FindBestPath(start, end);
			path = search_as->GetFinalPath();
		}
		
	}

	void FollowPath() {
		if (path.size() > 0) {
			for (std::list<const GraphNode*>::iterator it = path.begin(); it != path.end();) {
				if ((*it)->_idx == current_idx) {
					Vector3 posA = ((*it)->_pos);
					int idxA = (*it)->_idx;
					++it;
					if (it != path.end()) {
						Vector3 posB = ((*it)->_pos);
						int idxB = (*it)->_idx;

						if (it != path.end()) {
							lerp_factor += 0.05f;
							current_pos = InterpolatePositionLinear(posA, posB, lerp_factor);
						}

						if (lerp_factor >= 1.0f) {
							current_idx = idxB;
							lerp_factor = 0.0f;
						}
					}
					break;
				}
				else {
					++it;
				}

			}
		}	
	}

	bool Check_Los(int avatar_loc) {

		int j_self = current_idx;
		int j_avatar = avatar_loc;
		int i_self = 0;
		int i_avatar = 0;
		int s = maze->size;
		int base_offset = s * (s - 1);

		while (j_self > s-1) {
			j_self -= s;
			++i_self;
		}

		while (j_avatar > s-1) {
			j_avatar -= s;
			++i_avatar;
		}

		// Same column
		if (j_self == j_avatar) {
			if (i_self > i_avatar) {
				for (int i = i_self; i > i_avatar; --i) {
					if (maze->allEdges[base_offset + (j_self * (s - 1)) + i]._iswall) { return false; };
				}
			}
			else if (i_self < i_avatar) {
				for (int i = i_self; i < i_avatar; ++i) {
					if (maze->allEdges[base_offset + (j_self * (s - 1)) + i]._iswall) { return false; };
				}
			}
			else { return true; }
		}
		// Same Row
		else if (i_self == i_avatar) {
			if (j_self > j_avatar) {
				for (int j = j_self; j > j_avatar; --j) {
					if (maze->allEdges[(i_self * (s - 1) + j)]._iswall) { return false; };
				}
			}
			else if (j_self < j_avatar) {
				for (int j = j_self; j < j_avatar; ++j) {
					if (maze->allEdges[(i_self * (s - 1) + j)]._iswall) { return false; };
				}
			}
			else { return true; }
		}
		else { return false; }

	}

	int  PickRandomNode()
	{
		int num_nodes = maze->size * maze->size;
		int randomNode = rand() % num_nodes;
		return randomNode;
	}

	void SetStartNode() {
		current_idx = PickRandomNode();
		current_pos = maze->GetNode(current_idx)->_pos;
	}

	Vector3 InterpolatePositionLinear(Vector3 posA, Vector3 posB, float factor)
	{
		//With factor between 0-1, this is defined as:
		// LerpA-B(factor) = (1 - factor) * A + factor * B

		return posA * (1.0f - factor) + posB * factor;
	}



};
