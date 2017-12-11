#include "CudaSoftBody.cuh"

#define SQRT2 1.41421356237

inline float3 vec_to_float(Vector3 vec) {
	float3 out;
	out.x = vec.x;
	out.y = vec.y;
	out.z = vec.z;
	return out;
}

inline Vector3 float_to_vec(float3 flo) {
	return Vector3(flo.x, flo.y, flo.z);
}


__device__ void resolveSpring(Node * nodes, Spring s,float dt) {

}


__global__ void UpdateSpringSet(Node * nodes, SpringSet set, int w, int h, float h_rest, float d_rest, float dt) {
	
	uint index = (blockIdx.x*blockDim.x) + threadIdx.x;

	int i = 0;
	int j = 0;

	while (index >= w) {
		index -= w;
		i += 1;
	}
	j = index;

	
	int p1_idx = 0;
	int p2_idx = 0;
	float rest_length = 0.0f;

	switch (set) {
	// ...
	// o--o  o--o  o--o ...
	// o--o  o--o  o--o ...
	case HOR1:
		if (j % 2 == 0 && j != w - 1) {
			rest_length = h_rest;
			p1_idx = (i*w) + j;
			p2_idx = (i*w) + j + 1;
		}
		else { return; }
		break;
	// o  o--o  o--o  o ...
	// o  o--o  o--o  o ...
	// ...
	case HOR2:
		if (j % 2 != 0 && j != w - 1) {
			rest_length = h_rest;
			p1_idx = (i*w) + j;
			p2_idx = (i*w) + j + 1;
		}
		else { return; }
		break;
	// o  o  o  o  o  o
	// |  |  |  |  |  |
	// o  o  o  o  o  o
	// 
	// o  o  o  o  o  o
	// |  |  |  |  |  |
	// o  o  o  o  o  o
	case VERT1:
		if (i % 2 == 0 && i != h - 1) {
			rest_length = h_rest;
			p1_idx = (i*w) + j;
			p2_idx = ((i+1)*w) + j;
		}
		else { return; }
		break;
	// o  o  o  o  o  o
	// 
	// o  o  o  o  o  o
	// |  |  |  |  |  |
	// o  o  o  o  o  o
	//
	// o  o  o  o  o  o
	case VERT2:
		if (i % 2 != 0 && i != h - 1) {
			rest_length = h_rest;
			p1_idx = (i*w) + j;
			p2_idx = ((i+1)*w) + j;
		}
		else { return; }
		break;
	// o  o  o  o  o  o
	//  \  \  \  \  \
	// o  o  o  o  o  o
	// 
	// o  o  o  o  o  o
	//  \  \  \  \  \
	// o  o  o  o  o  o
	case DIAG_L1:
		if (i % 2 == 0 && i != h - 1 && j != 0) {
			rest_length = d_rest;
			p1_idx = (i*w) + j;
			p2_idx = ((i+1)*w) + j - 1;
		}
		else { return; }
		break;
	// o  o  o  o  o  o
	// 
	// o  o  o  o  o  o
	//  \  \  \  \  \ 
	// o  o  o  o  o  o
	//
	// o  o  o  o  o  o
	case DIAG_L2:
		if (i % 2 != 0 && i != h - 1 && j != 0) {
			rest_length = d_rest;
			p1_idx = (i*w) + j;
			p2_idx = ((i+1)*w) + j - 1;
		}
		else { return; }
		break;
	// o  o  o  o  o  o
	//  /  /  /  /  /
	// o  o  o  o  o  o
	// 
	// o  o  o  o  o  o
	//   /  /  /  /  /
	// o  o  o  o  o  o
	case DIAG_R1:
		if (i % 2 == 0 && i != h - 1 && j != w-1) {
			rest_length = d_rest;
			p1_idx = (i*w) + j;
			p2_idx = ((i+1)*w) + j + 1;
		}
		else { return; }
		break;
	// o  o  o  o  o  o
	//  
	// o  o  o  o  o  o
	//	/  /  /  /  /
	// o  o  o  o  o  o
	//
	// o  o  o  o  o  o
	case DIAG_R2:
		if (i % 2 != 0 && i != h - 1 && j != w - 1) {
			rest_length = d_rest;
			p1_idx = (i*w) + j;
			p2_idx = ((i+1)*w) + j + 1;
		}
		else { return; }
		break;
	}

	if (p1_idx != p2_idx) {
		float3 ab = nodes[p2_idx].pos - nodes[p1_idx].pos;
		float3 abn = normalize(ab);

		float abnVel = dot(nodes[p1_idx].vel - nodes[p2_idx].vel, abn);

		float constraintMass = nodes[p1_idx].invmass + nodes[p2_idx].invmass;

		if (constraintMass > 0.0f) {
			float b = 0.0f;

			float distance_offset = length(ab) - rest_length;
			float baumgarte_scalar = 0.1f;
			b = -(baumgarte_scalar
				/ dt)
				* distance_offset;

			float jn = ((distance_offset * 0.1f) / (constraintMass * dt)) - (0.01f * (abnVel));
			//s.p1.vel -= abn * jn * s.p1.invmass;
			//s.p2.vel += abn * jn * s.p2.invmass;

			nodes[p1_idx].vel += abn * jn * nodes[p1_idx].invmass;
			nodes[p2_idx].vel -= abn * jn * nodes[p2_idx].invmass;
		}
	}


}




CudaSoftBody::CudaSoftBody(int w, int h, float s, Vector3 pos, GLuint tex)
{
	this->w = w;
	this->h = h;
	this->pos = new Vector3(pos);
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			PhysicsNode * pnode = new PhysicsNode();
			pnode->SetPosition(Vector3(pos.x + (s*j), pos.y + (s*i), pos.z));

			pnode->SetInverseMass(60.0f);

			//if (i == h - 1) {//&& j == 0 || (i == h - 1 && j == w - 1)) {
			//	pnode->SetInverseMass(0.0f);
			//}

			CollisionShape* pColshape = new SphereCollisionShape(0.7f*s);
			pnode->SetCollisionShape(pColshape);
			physicsnodes.push_back(pnode);

		}
	}

	straight_rest_length = s;
	diagonal_rest_length = SQRT2*s;

	RenderNode* rnode = new RenderNode();

	m = Mesh::GenerateMesh(w - 1, h - 1, s);
	m->SetTexture(tex);
	RenderNode* dummy = new RenderNode(m, Vector4(1, 0, 0, 1));
	rnode->AddChild(dummy);
	dummy->SetCulling(false);

	rnode->SetTransform(Matrix4::Translation(Vector3(pos.x, pos.y, pos.z)));
	rnode->SetBoundingRadius(10);

	mgo = new MultiGameObject("softBody", rnode, physicsnodes, this->pos);
	mgo->SetSiblingsCollide(false);
	mgo->RotateObject(Vector3(1, 0, 0), -90);
	mgo->UpdatePosition(Vector3(2.5, 15, -2.5));

	ScreenPicker::Instance()->RegisterNodeForMouseCallback(
		dummy, //Dummy is the rendernode that actually contains the drawable mesh, and the one we can to 'drag'
		std::bind(&CommonUtils::DragableObjectCallback, mgo, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	);

	physicsnodes[0]->SetOnUpdateCallback(
		std::bind(
			&CudaSoftBody::UpdateMesh,
			this,
			std::placeholders::_1)
	);

	nodes = new Node[physicsnodes.size()];

	for (int i = 0; i < physicsnodes.size(); ++i) {
		nodes[i].invmass = physicsnodes[i]->GetInverseMass();
		nodes[i].pos = vec_to_float(physicsnodes[i]->GetPosition());
		nodes[i].vel = vec_to_float(physicsnodes[i]->GetLinearVelocity());
	}

	gpuErrchk(cudaMalloc(&cuda_nodes,		w * h *	sizeof(Node)));
	
}

CudaSoftBody::~CudaSoftBody()
{
}

void CudaSoftBody::UpdateMesh(const Matrix4 &matrix) {
	int k = 0;
	for (int i = 0; i < h - 1; ++i) {
		for (int j = 0; j < w - 1; ++j) {
			m->vertices[k] = physicsnodes[(i * w) + j]->GetPosition() - *pos;
			m->vertices[k + 1] = physicsnodes[(i * w) + j + 1]->GetPosition() - *pos;
			m->vertices[k + 2] = physicsnodes[((i + 1) * w) + j]->GetPosition() - *pos;

			m->vertices[k + 3] = physicsnodes[(i * w) + j + 1]->GetPosition() - *pos;
			m->vertices[k + 4] = physicsnodes[((i + 1) * w) + j + 1]->GetPosition() - *pos;
			m->vertices[k + 5] = physicsnodes[((i + 1) * w) + j]->GetPosition() - *pos;

			k += 6;
		}
	}
	m->GenerateNormals();
	m->GenerateTangents();
	m->ClearBuffers();
	m->BufferData();
}

void CudaSoftBody::UpdateSoftBody(float dt)
{

	for (int i = 0; i < physicsnodes.size(); ++i) {
		nodes[i].invmass = physicsnodes[i]->GetInverseMass();
		nodes[i].pos = vec_to_float(physicsnodes[i]->GetPosition());
		nodes[i].vel = vec_to_float(physicsnodes[i]->GetLinearVelocity());
	}

	gpuErrchk(cudaMemcpy(cuda_nodes, nodes, w * h * sizeof(Node), cudaMemcpyHostToDevice));

	UpdateSpringSet << < w*h, 1 >> > (cuda_nodes, HOR1, w, h, straight_rest_length, diagonal_rest_length, dt);

	UpdateSpringSet << < w*h, 1 >> > (cuda_nodes, HOR2, w, h, straight_rest_length, diagonal_rest_length, dt);

	UpdateSpringSet << < w*h, 1 >> > (cuda_nodes, VERT1, w, h, straight_rest_length, diagonal_rest_length, dt);

	UpdateSpringSet << < w*h, 1 >> > (cuda_nodes, VERT2, w, h, straight_rest_length, diagonal_rest_length, dt);

	UpdateSpringSet << < w, h >> > (cuda_nodes, DIAG_R1, w, h,	straight_rest_length, diagonal_rest_length, dt);

	UpdateSpringSet << < w, h >> > (cuda_nodes, DIAG_R2, w, h,	straight_rest_length, diagonal_rest_length, dt);
	
	UpdateSpringSet << < w, h >> > (cuda_nodes, DIAG_L1, w, h,	straight_rest_length, diagonal_rest_length, dt);

	UpdateSpringSet << < w, h >> > (cuda_nodes, DIAG_L2, w, h,	straight_rest_length, diagonal_rest_length, dt);

	gpuErrchk(cudaMemcpy(nodes, cuda_nodes, w * h * sizeof(Node), cudaMemcpyDeviceToHost));

	for (int i = 0; i < physicsnodes.size(); ++i) {
		physicsnodes[i]->SetLinearVelocity(float_to_vec(nodes[i].vel));
	}
}


