#pragma once
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "Server.h"

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

	bool AtLastNode();

	void UpdateAStarPreset(int s, int e);

	void FollowPath();

	bool Check_Los(int avatar_loc);

	Vector3 InterpolatePositionLinear(Vector3 posA, Vector3 posB, float factor)
	{
		return posA * (1.0f - factor) + posB * factor;
	}



};
