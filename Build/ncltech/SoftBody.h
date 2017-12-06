#pragma once
#include "SpringConstraint.h"
#include "DistanceConstraint.h"
#include "SphereCollisionShape.h"
#include "../nclgl/RenderNode.h"
#include "CommonMeshes.h"
#include "GameObject.h"
#include "MultiGameObject.h"
#include "ScreenPicker.h"
#include "CommonUtils.h"
#include <nclgl\Mesh.h>

class SoftBody
{
public:
	SoftBody(int width = 10, int height = 10, float separation = 0.1f, Vector3 position = Vector3(0,0,0), GLuint tex = NULL);
	~SoftBody();

	MultiGameObject * GetGameObject() { return mgo; };
	GameObject* test;

protected:
	std::vector<PhysicsNode *> physicsnodes;
	//PhysicsNode *** physicsnodes;
	MultiGameObject* mgo;
	
};

