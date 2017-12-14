#include "State.h"
#include "Hazard.h"

void State::On_Initialize () 
{
	_isActive = true;
}

bool State::AtLastNode() {

	if (h->path.size() > 0) {
		auto it = h->path.end();
		--it;
		if (h->current_idx == (*it)->_idx) {
			return true;
		}
		else { return false; }
	}
	return true;
}

void State::UpdateAStarPreset(int s, int e)
{
	//Example presets taken from:
	// http://movingai.com/astar-var.html
	float weightingG, weightingH;
	switch (h->astar_preset_idx)
	{
	default:
	case 0:
		//Only distance from the start node matters - fans out from start node
		weightingG = 1.0f;
		weightingH = 0.0f;
		h->astar_preset_text = "Dijkstra - None heuristic search";
		break;
	case 1:
		//Only distance to the end node matters
		weightingG = 0.0f;
		weightingH = 1.0f;
		h->astar_preset_text = "Pure Hueristic search";
		break;
	case 2:
		//Equal weighting
		weightingG = 1.0f;
		weightingH = 1.0f;
		h->astar_preset_text = "Traditional A-Star";
		break;
	case 3:
		//Greedily goes towards the goal node where possible, but still cares about distance travelled a little bit
		weightingG = 1.0f;
		weightingH = 2.0f;
		h->astar_preset_text = "Weighted Greedy A-Star";
		break;
	}
	h->search_as->SetWeightings(weightingG, weightingH);

	GraphNode* start =	h->maze->GetNode(s);
	GraphNode* end =	h->maze->GetNode(e);

	if (start && end) {
		h->search_as->FindBestPath(start, end);
		h->path = h->search_as->GetFinalPath();
	}

}

void State::FollowPath() {

	if (h->path.size() > 0) {
		for (std::list<const GraphNode*>::iterator it = h->path.begin(); it != h->path.end();) {
			if ((*it)->_idx == h->current_idx) {
				Vector3 posA = ((*it)->_pos);
				int idxA = (*it)->_idx;
				++it;
				if (it != h->path.end()) {
					Vector3 posB = ((*it)->_pos);
					int idxB = (*it)->_idx;

					if (it != h->path.end()) {
						h->lerp_factor += 0.05f;
						h->current_pos = InterpolatePositionLinear(posA, posB, h->lerp_factor);
					}

					if (h->lerp_factor >= 1.0f) {
						h->current_idx = idxB;
						h->lerp_factor = 0.0f;
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

bool State::Check_Los(int avatar_loc) {

	int j_self = h->current_idx;
	int j_avatar = avatar_loc;
	int i_self = 0;
	int i_avatar = 0;
	int s = h->maze->size;
	int base_offset = s * (s - 1);

	while (j_self > s - 1) {
		j_self -= s;
		++i_self;
	}

	while (j_avatar > s - 1) {
		j_avatar -= s;
		++i_avatar;
	}

	// Same column
	if (j_self == j_avatar) {
		if (i_self > i_avatar) {
			for (int i = i_self; i > i_avatar; --i) {
				if (h->maze->allEdges[base_offset + (j_self * (s - 1)) + i]._iswall) { return false; };
			}
		}
		else if (i_self < i_avatar) {
			for (int i = i_self; i < i_avatar; ++i) {
				if (h->maze->allEdges[base_offset + (j_self * (s - 1)) + i]._iswall) { return false; };
			}
		}
		else { return true; }
	}
	// Same Row
	else if (i_self == i_avatar) {
		if (j_self > j_avatar) {
			for (int j = j_self; j > j_avatar; --j) {
				if (h->maze->allEdges[(i_self * (s - 1) + j)]._iswall) { return false; };
			}
		}
		else if (j_self < j_avatar) {
			for (int j = j_self; j < j_avatar; ++j) {
				if (h->maze->allEdges[(i_self * (s - 1) + j)]._iswall) { return false; };
			}
		}
		else { return true; }
	}
	else { return false; }

}

