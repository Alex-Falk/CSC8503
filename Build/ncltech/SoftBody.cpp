#include "SoftBody.h"

SoftBody::SoftBody(int w, int h, float s, Vector3 pos,GLuint tex)
{

	for (int i = 0; i < w; ++i) {
		for (int j = 0; j < h; ++j) {			
			PhysicsNode * pnode = new PhysicsNode();
			pnode->SetPosition(Vector3(pos.x + (s*i), pos.y + (s*j), 0));
			
			pnode->SetInverseMass(1.0f);
			
			if ((i == 0 ) && j == h-1) {
				pnode->SetInverseMass(0.0f);
			}
			
			CollisionShape* pColshape = new SphereCollisionShape(0.5f*s);
			pnode->SetCollisionShape(pColshape);
			physicsnodes.push_back(pnode);
		}
	}

	RenderNode* rnode = new RenderNode();


	RenderNode* dummy = new RenderNode(CommonMeshes::Sphere(), Vector4(1, 0, 0, 1));
	dummy->SetTransform(Matrix4::Scale(Vector3(0.1,0.1,0.1)));
	rnode->AddChild(dummy);

	rnode->SetTransform(Matrix4::Translation(Vector3(pos.x, pos.y, 0)));
	rnode->SetBoundingRadius(10);

	mgo = new MultiGameObject("softBody", rnode, physicsnodes);

	ScreenPicker::Instance()->RegisterNodeForMouseCallback(
		dummy, //Dummy is the rendernode that actually contains the drawable mesh, and the one we can to 'drag'
		std::bind(&CommonUtils::DragableObjectCallback, mgo, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	);

	// Adding Constraints to Each of the physicsnodes
	// TODO: Create spring constraint and use this instead of DistanceConstraint
	for (int i = 0; i < w; ++i) {
		for (int j = 0; j < h; ++j) {
		
			//Right Edge (apart from Top Right)
			if (i == 0 && j != h - 1)				
			{
				//Self->Left
				DistanceConstraint* constraint1 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[((i+1)*w) + j],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[((i + 1)*w) + j]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint1);

				//Self->Up
				DistanceConstraint* constraint2 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[(i*w) + j + 1],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[(i*w) + j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint2);

				//Self->Top_Left
				DistanceConstraint* constraint3 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[((i + 1)*w) + j + 1],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[((i + 1)*w) + j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint3);

			}
			//Right Edge (apart from Top Left)
			else if (i == w - 1 && j != h - 1)
			{	
				//Self->Up
				DistanceConstraint* constraint1 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[(i*w) + j + 1],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[(i*w) + j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint1);

				//Self->Top_Right
				DistanceConstraint* constraint2 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[((i - 1)*w) + j + 1],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[((i - 1)*w) + j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint2);

			}
			//Center (apart from top line)
			else if (j != h - 1) 
			{
				//Self->Left
				DistanceConstraint* constraint1 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[((i + 1)*w) + j],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[((i + 1)*w) + j]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint1);

				//Self->Up
				DistanceConstraint* constraint2 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[(i*w) + j + 1],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[(i*w) + j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint2);

				//Self->Up_left
				DistanceConstraint* constraint3 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[((i + 1)*w) + j + 1],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[((i + 1)*w) + j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint3);

				//Self->Up_right
				DistanceConstraint* constraint4 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[((i - 1)*w) + j + 1],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[((i - 1)*w) + j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint4);

			}
			//Top Edge (apart from top Left)
			else if (i != w - 1 && j == h - 1)
			{
				//Self->Left
				DistanceConstraint* constraint1 = new DistanceConstraint(
					physicsnodes[(i*w) + j], physicsnodes[((i + 1)*w) + j],
					physicsnodes[(i*w) + j]->GetPosition(), physicsnodes[((i + 1)*w) + j]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint1);

			}
			//Bottom Right corner has no more attachments needed
		}
	}
}

SoftBody::~SoftBody()
{
}

