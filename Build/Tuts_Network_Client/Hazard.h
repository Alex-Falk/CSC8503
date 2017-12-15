#pragma once
#include "MazeGenerator.h"
#include "SearchAStar.h"
class State;

// Class that handles hazards, which is controlled by an FSM.
// There are two different states. Patrol or pursue. Pursue has two different substates, these are:
// chase and find. The first of this makes the hazard chase an avatar if he's been seen and the second
// moves to the avatars last known position and check if the hazard can see it again.
// If it does it it again it moves back to chase. If it doesnt it will start patrolling the maze.

class Hazard {
public:
	bool _isActive = true;

	//Targeting
	float lerp_factor = 0.0f;
	int target = OUT_OF_RANGE;

	//Avatar Information
	vector<int> avatar_idcs;

	//Currently active state
	State * current_state;

	const MazeGenerator * maze;

	int current_idx;

	//Physicsnode that handles movement of the hazard
	PhysicsNode * pnode;

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

	//Switch between different states in the FSM
	virtual void SwitchState(State_enum s);

	// Callback function that handles what the Hazard should do if it collides with another object
	// The way it works right now is that if it collides with an avatar it will start patrolling again
	// the avatar will handle its own callback. Currently, if the hazard collides with another hazard,
	// nothing will happen.
	bool CollisionCallback(PhysicsNode * self, PhysicsNode * collidingObject);

	// Pass in updated avatar information
	virtual void UpdateAvatars(vector<int> avatars) {
		avatar_idcs = avatars;
	}

	// Picks a random node in the maze and returns it's index
	int PickRandomNode() {
		int num_nodes = maze->size * maze->size;
		int randomNode = rand() % num_nodes;
		return randomNode;
	}

	// Set the starting node to a given index, or if the input is -1 then it will choose a random node
	void SetStartNode(int i) {
		if (i == OUT_OF_RANGE)
			current_idx = PickRandomNode();
		else
			current_idx = i;
		pnode->SetPosition(maze->GetNode(current_idx)->_pos);
	}

};