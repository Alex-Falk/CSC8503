#include "Hazard.h"
#include "Patrol.h"
#include "Pursue.h"
#include "Chase.h"
#include "State.h"
#include "Find.h"
#include <ncltech\CuboidCollisionShape.h>

Hazard::Hazard(MazeGenerator * maze) {
	this->maze = maze;

	patrol_state		= new Patrol(this);
	pursue_state		= new Pursue(this);
	pursue_chase_state	= new Chase(this);
	pursue_find_state	= new Find(this);

	search_as			= new SearchAStar();
	
	pnode				= new PhysicsNode();
	pnode->SetName("Hazard");
	CollisionShape * pColshape = new CuboidCollisionShape(Vector3(0.33f, 0.33f, 0.33f));
	pnode->SetCollisionShape(pColshape);
	pnode->SetInverseMass(1.0f / 10.0f);
	pnode->SetBoundingRadius(2.0f);
	pnode->SetOnCollisionCallback(
		std::bind(
			&Hazard::CollisionCallback,
			this,
			std::placeholders::_1,
			std::placeholders::_2));

	PhysicsEngine::Instance()->AddPhysicsObject(pnode);

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
		break;
	case PURSUE:
		current_state = pursue_state;
		break;
	case PURSUE_CHASE:
		current_state = pursue_chase_state;
		break;
	case PURSUE_FIND:
		current_state = pursue_find_state;
		break;
	}

	current_state->On_Initialize();
}


bool Hazard::CollisionCallback(PhysicsNode * self, PhysicsNode * collidingObject) {
	if (collidingObject->getName() == "Hazard") {
		//current_state->UpdateAStarPreset(current_idx, PickRandomNode());
	}
	if (collidingObject->getName() == "Avatar") {
		SwitchState(PATROL);
		current_state->UpdateAStarPreset(current_idx, PickRandomNode());
	}
	return false;
}