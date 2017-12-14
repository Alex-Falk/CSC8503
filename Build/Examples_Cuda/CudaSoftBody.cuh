#pragma once
#include <thrust\device_ptr.h>
#include <thrust\binary_search.h>
#include <thrust\iterator\counting_iterator.h>
#include <thrust\sort.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>

#include "CudaCommon.cuh"
#include <nclgl\common.h>
#include "../nclgl/RenderNode.h"
#include <ncltech\CommonMeshes.h>
#include <ncltech\GameObject.h>
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

struct CPUParticle {
	float3 pos;
	float radius;
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

	RenderNode * GetRenderNode() { return obj; };
	void UpdateMesh();
	void UpdateSoftBody(float dt, vector<PhysicsNode *> cpuparticles);

protected:
	//std::vector<PhysicsNode *> physicsnodes;
	//std::vector<Node *> nodes;
	//PhysicsNode *** physicsnodes;
	RenderNode * obj;
	Vector3 * pos;
	Mesh * m;
	int w;
	int h;
	int s;

	Node * nodes;
	float straight_rest_length;
	float diagonal_rest_length;

	Node * cuda_nodes;

	CPUParticle * cpu_particles;
	CPUParticle * cuda_cpu_particles;
};

