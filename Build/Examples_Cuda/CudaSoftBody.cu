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

__global__ void CollideParticles(float baumgarte_factor, uint num_particles, Node* particles, CPUParticle * cpu_particles, float separation, int num_CPU_obj)
{
	uint index = blockIdx.x*blockDim.x + threadIdx.x;
	if (index >= num_particles)
		return;

	Node p = particles[index];

	if (p.invmass != 0.0f)
	{
		for (int i = 0; i < num_CPU_obj; ++i) {
			CPUParticle cpu_p = cpu_particles[i];
			//float3 newpos = make_float3(cpu_p.pos.x, cpu_p.pos.y, cpu_p.pos.z);

			//Do a quick sphere-sphere test
			float3 ab = cpu_p.pos - p.pos;
			float lengthSq = dot(ab, ab);

			const float diameterSq = ((separation + cpu_p.radius + 1.0f) * (separation + cpu_p.radius + 1.0f));
			if (lengthSq < diameterSq)
			{
				//We have a collision!
				float len = sqrtf(lengthSq);
				float3 abn = ab / len;

				//Direct normal collision (no friction/shear)
				float abnVel = dot(cpu_p.vel - cpu_p.vel, abn);
				float jn = -(abnVel * (1.f + 0.00f));

				//Extra energy to overcome overlap error
				float overlap = cpu_p.radius - len;
				float b = overlap * baumgarte_factor;


				jn += b;

				jn = max(jn, 0.0f);
				p.vel -= abn * (jn * 0.5f);
			}
		}
	}
	particles[index] = p;
	
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

			float jn = (distance_offset * 0.6f) / (constraintMass * dt) - (0.01f * (abnVel));
			//s.p1.vel -= abn * jn * s.p1.invmass;
			//s.p2.vel += abn * jn * s.p2.invmass;

			nodes[p1_idx].vel += abn * jn * nodes[p1_idx].invmass;
			nodes[p2_idx].vel -= abn * jn * nodes[p2_idx].invmass;
		}
	}


}

__host__
struct UpdatePositions
{
	UpdatePositions(float dt, float3 gravity)
		: _dt(dt)
		, _gravity(gravity)
	{
	}

	float _dt;
	float3 _gravity;

	__host__ __device__
		void operator()(Node& p)
	{
		//Time integration
		if(p.invmass != 0)
			p.vel += _gravity;
		p.vel *= 0.999f;

		p.pos += p.vel * _dt;
	}
};

CudaSoftBody::CudaSoftBody(int w, int h, float s, Vector3 pos, GLuint tex)
{
	this->w = w;
	this->h = h;
	this->s = s;
	this->pos = new Vector3(pos);

	nodes = new Node[w*h];

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			Node pnode;
			pnode.pos = vec_to_float((Vector3(pos.x + (s*j), pos.y, pos.z + (s*i))));
			pnode.vel = make_float3(0.0f, 0.0f, 0.0f);
			pnode.invmass = (60.0f);
			if (i == 0)
				pnode.invmass = 0.f;
			
			nodes[(i*w) + j] = pnode;

			
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

	obj = rnode;

	gpuErrchk(cudaMalloc(&cuda_nodes,		w * h *	sizeof(Node)));
	
}

CudaSoftBody::~CudaSoftBody()
{
}

void CudaSoftBody::UpdateMesh() {
	int k = 0;
	for (int i = 0; i < h - 1; ++i) {
		for (int j = 0; j < w - 1; ++j) {
			m->vertices[k] = float_to_vec(nodes[(i * w) + j].pos) - *pos;
			m->vertices[k + 1] = float_to_vec(nodes[(i * w) + j + 1].pos) - *pos;
			m->vertices[k + 2] = float_to_vec(nodes[((i + 1) * w) + j].pos) - *pos;

			m->vertices[k + 3] = float_to_vec(nodes[(i * w) + j + 1].pos) - *pos;
			m->vertices[k + 4] = float_to_vec(nodes[((i + 1) * w) + j + 1].pos) - *pos;
			m->vertices[k + 5] = float_to_vec(nodes[((i + 1) * w) + j].pos) - *pos;

			k += 6;
		}
	}
	m->GenerateNormals();
	m->GenerateTangents();
	m->ClearBuffers();
	m->BufferData();
}

void CudaSoftBody::UpdateSoftBody(float dt, vector<PhysicsNode *> cpuparticles)
{
	const float fixed_timestep = 1.0f / 60.0f;
	float baumgarte_factor = 0.05f / fixed_timestep;
	const float3 gravity = make_float3(0, -0.01f, 0);

	cuda_cpu_particles = new CPUParticle[cpuparticles.size()];
	cpu_particles = new CPUParticle[cpuparticles.size()];

	for (int i = 0; i < cpuparticles.size(); ++i) {
		cpu_particles[i].invmass	= cpuparticles[i]->GetInverseMass();
		cpu_particles[i].radius		= cpuparticles[i]->GetBoundingRadius();
		cpu_particles[i].pos		= vec_to_float(cpuparticles[i]->GetPosition());
		cpu_particles[i].vel		= vec_to_float(cpuparticles[i]->GetLinearVelocity());
	}


	gpuErrchk(cudaMalloc(&cuda_cpu_particles, cpuparticles.size() * sizeof(CPUParticle)));

	gpuErrchk(cudaMemcpy(cuda_cpu_particles, cpu_particles, cpuparticles.size() * sizeof(CPUParticle), cudaMemcpyHostToDevice));
	gpuErrchk(cudaMemcpy(cuda_nodes, nodes, w * h * sizeof(Node), cudaMemcpyHostToDevice));

	thrust::for_each(
		thrust::device_ptr<Node>(cuda_nodes),
		thrust::device_ptr<Node>(cuda_nodes + (w*h)),
		UpdatePositions(fixed_timestep, gravity));



	

	for (int i = 0; i < 10; ++i) {
		CollideParticles << < w*h, 1 >> > (baumgarte_factor, w*h, cuda_nodes, cuda_cpu_particles, s, cpuparticles.size());

		UpdateSpringSet << < w*h, 1 >> > (cuda_nodes, HOR1, w, h, straight_rest_length, diagonal_rest_length, dt);

		UpdateSpringSet << < w*h, 1 >> > (cuda_nodes, HOR2, w, h, straight_rest_length, diagonal_rest_length, dt);

		UpdateSpringSet << < w*h, 1 >> > (cuda_nodes, VERT1, w, h, straight_rest_length, diagonal_rest_length, dt);

		UpdateSpringSet << < w*h, 1 >> > (cuda_nodes, VERT2, w, h, straight_rest_length, diagonal_rest_length, dt);

		UpdateSpringSet << < w, h >> > (cuda_nodes, DIAG_R1, w, h, straight_rest_length, diagonal_rest_length, dt);

		UpdateSpringSet << < w, h >> > (cuda_nodes, DIAG_R2, w, h, straight_rest_length, diagonal_rest_length, dt);

		UpdateSpringSet << < w, h >> > (cuda_nodes, DIAG_L1, w, h, straight_rest_length, diagonal_rest_length, dt);

		UpdateSpringSet << < w, h >> > (cuda_nodes, DIAG_L2, w, h, straight_rest_length, diagonal_rest_length, dt);

	}

	gpuErrchk(cudaMemcpy(nodes, cuda_nodes, w * h * sizeof(Node), cudaMemcpyDeviceToHost));
	gpuErrchk(cudaFree(cuda_cpu_particles));
	cuda_cpu_particles = NULL;
	UpdateMesh();
}

