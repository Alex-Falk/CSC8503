#include "SoftBody.h"

SoftBody::SoftBody(int w, int h, float s, Vector3 pos)
{
	physicsnodes = new PhysicsNode**[w];
	gameobjects = new vector<GameObject *>;
	for (int i = 0; i < w; ++i) {
		physicsnodes[i] = new PhysicsNode*[h];
		for (int j = 0; j < h; ++j) {
			RenderNode* rnode = new RenderNode();

			RenderNode* dummy = new RenderNode(CommonMeshes::Sphere(), Vector4(1,0,0,1));
			dummy->SetTransform(Matrix4::Scale(Vector3(s,s,s)*0.1f));
			rnode->AddChild(dummy);

			rnode->SetTransform(Matrix4::Translation(Vector3(pos.x + (s*i), pos.y + (s*j), 0)));
			rnode->SetBoundingRadius(s);
			
			PhysicsNode * pnode = new PhysicsNode();
			pnode->SetPosition(Vector3(pos.x + (s*i), pos.y + (s*j), 0));
			if (j != h-1) {
				pnode->SetInverseMass(1.0f);
			}
			
			CollisionShape* pColshape = new SphereCollisionShape(0.5f*s);
			pnode->SetCollisionShape(pColshape);
			physicsnodes[i][j] = pnode;

			GameObject* obj = new GameObject("SoftBody", rnode, pnode);

			gameobjects->push_back(obj);

			ScreenPicker::Instance()->RegisterNodeForMouseCallback(
				dummy, //Dummy is the rendernode that actually contains the drawable mesh, and the one we can to 'drag'
				std::bind(&CommonUtils::DragableObjectCallback, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
			);

		}
	}

	// Adding Constraints to Each of the physicsnodes
	// TODO: Create spring constraint and use this instead of distanceConstraint
	for (int i = 0; i < w; ++i) {
		for (int j = 0; j < h; ++j) {
		
			//Left Edge (apart from Bottom Left)
			if (i == 0 && j != h - 1)				
			{
				//Self->Right
				SpringConstraint* constraint1 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i + 1][j],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i + 1][j]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint1);

				//Self->Down
				SpringConstraint* constraint2 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i][j + 1],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i][j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint2);

				//Self->Down_Right
				SpringConstraint* constraint3 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i + 1][j + 1],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i + 1][j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint3);

			}
			//Right Edge (apart from bottom right)
			else if (i == w - 1 && j != h - 1)
			{	
				//Self->Down
				SpringConstraint* constraint1 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i][j + 1],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i][j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint1);

			}
			//Center (apart from bottom line)
			else if (j != h - 1) 
			{
				//Self->Right
				SpringConstraint* constraint1 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i + 1][j],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i + 1][j]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint1);

				//Self->Down
				SpringConstraint* constraint2 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i][j + 1],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i][j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint2);

				//Self->Down_Right
				SpringConstraint* constraint3 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i + 1][j + 1],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i + 1][j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint3);

				//Self->Down_left
				SpringConstraint* constraint4 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i - 1][j + 1],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i - 1][j + 1]->GetPosition()
				);
				PhysicsEngine::Instance()->AddConstraint(constraint4);

			}
			//Bottom Edge (apart from bottom right)
			else if (i != w - 1 && j == h - 1)
			{
				//Self->Right
				SpringConstraint* constraint1 = new SpringConstraint(
					physicsnodes[i][j], physicsnodes[i + 1][j],
					physicsnodes[i][j]->GetPosition(), physicsnodes[i + 1][j]->GetPosition()
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

