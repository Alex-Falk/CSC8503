#pragma once
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "Server.h"

//Parent class for all the States in the FSM

class Hazard;

class State {
protected:
	
	bool _isActive = false;
public:
	Hazard * h;

	State(Hazard * parent)
	{
		h = parent;
	};

	~State() {}

	virtual void On_Initialize();

	virtual void Update() {};

	//If path has reached it's end returns true
	bool AtLastNode();

	//Update pathfinding with given start and end nodes (or ratiher indeces of nodes)
	void UpdateAStarPreset(int s, int e);

	//Follow the current path by checking when to change velocity
	void FollowPath();

	void StartFollowing();

	//Checks if any of hte avatars are in its line of sight (it checks for vertical and horizontal match
	bool Check_Los(int avatar_loc);

	Vector3 InterpolatePositionLinear(Vector3 posA, Vector3 posB, float factor)
	{
		return posA * (1.0f - factor) + posB * factor;
	}



};
