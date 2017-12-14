#include "Hazard.h"
#include "Patrol.h"
#include "Pursue.h"
#include "Chase.h"
#include "State.h"
#include "Find.h"

Hazard::Hazard(MazeGenerator * maze) {
	this->maze = maze;

	patrol_state		= new Patrol(this);
	pursue_state		= new Pursue(this);
	pursue_chase_state	= new Chase(this);
	pursue_find_state	= new Find(this);

	search_as			= new SearchAStar();

	current_state		= patrol_state;
}

Hazard::~Hazard() {
	SAFE_DELETE(patrol_state);
	SAFE_DELETE(pursue_state);
	SAFE_DELETE(pursue_chase_state);
	SAFE_DELETE(pursue_find_state);
}

void Hazard::Update() {
	if (_isActive)
		current_state->Update();
}

void Hazard::SwitchState(State_enum s) {
	switch (s) {
	case PATROL:
		current_state = patrol_state;
		cout << "Now Patrolling\n";
		break;
	case PURSUE:
		current_state = pursue_state;
		cout << "Now Pursuing\n";
		break;
	case PURSUE_CHASE:
		current_state = pursue_chase_state;
		cout << "Now Chasing\n";
		break;
	case PURSUE_FIND:
		current_state = pursue_find_state;
		cout << "Now Finding\n";
		break;
	}

	current_state->On_Initialize();
}

