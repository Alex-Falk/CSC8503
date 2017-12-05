#pragma once
#include "SpringConstraint.h"
#include "SphereCollisionShape.h"
#include "../nclgl/RenderNode.h"
#include "CommonMeshes.h"
#include "GameObject.h"
#include "ScreenPicker.h"
#include "CommonUtils.h"

class SoftBody
{
public:
	SoftBody(int width = 10, int height = 10, float separation = 0.1f, Vector3 position = Vector3(0,0,0));
	~SoftBody();

	std::vector<GameObject *> * GetGameObjects() { return gameobjects; };

protected:
	std::vector<GameObject *> * gameobjects;
	PhysicsNode *** physicsnodes;
};

