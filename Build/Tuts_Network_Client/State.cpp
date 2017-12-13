#include "State.h"
#include "Patrol.h"
#include "Pursue.h"

void State::On_Initialize () 
{
	_isActive = true;
	target = 0;
	search_as = new SearchAStar();
	patrol_state = new Patrol(maze, this);
	patrol_state->On_Initialize();
	pursue_state = new Pursue(maze, this, target);
	pursue_state->On_Initialize();


	SwitchState(PATROL);
}

void State::SwitchState(State_enum s){
	switch (s) {
	case PATROL:
		if (!patrol_state) { 
			patrol_state = new Patrol(maze, this); 
		}
		current_state = patrol_state;
		break;
	case PURSUE:
		if (!pursue_state) { 
			pursue_state = new Pursue(maze, this, target);
		}
		current_state = pursue_state;
		break;
	}

	current_state->On_Initialize();
}