#pragma once
#include "CudaCommon.cuh"
#include <nclgl\common.h>
#include <ncltech\SpringConstraint.h>
#include <ncltech\SphereCollisionShape.h>
#include "../nclgl/RenderNode.h"
#include <ncltech\CommonMeshes.h>
#include <ncltech\GameObject.h>
#include <ncltech\MultiGameObject.h>
#include <ncltech\ScreenPicker.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\Mesh.h>
#include <nclgl\Matrix4.h>

enum SpringSet {HOR1,HOR2,VERT1,VERT2,DIAG_R1,DIAG_R2, DIAG_L1, DIAG_L2};


struct Node {
	float3 pos;
	float pad1;
	float3 vel;
	float invmass;
};

struct Spring{
	Node p1;
	Node p2;
	float rest_length;
	int p1_idx;
	int p2_idx;
};

class CudaSoftBody
{
public:
	CudaSoftBody(int width = 10, int height = 10, float separation = 0.1f, Vector3 position = Vector3(0, 0, 0), GLuint tex = NULL);
	~CudaSoftBody();

	MultiGameObject * GetGameObject() { return mgo; };
	void UpdateMesh(const Matrix4 &matrix);
	void UpdateSoftBody(float dt);
	GameObject* test;

protected:
	std::vector<PhysicsNode *> physicsnodes;
	//PhysicsNode *** physicsnodes;
	MultiGameObject* mgo;
	Vector3 * pos;
	Mesh * m;
	int w;
	int h;

	Node * nodes;
	float straight_rest_length;
	float diagonal_rest_length;

	Node * cuda_nodes;
};

