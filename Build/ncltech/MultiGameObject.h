#pragma once
#include "GameObject.h"

class MultiGameObject : public GameObject
{
	friend class Scene;			//Can set the private variables scene 
	friend class PhysicsEngine;

public:
	MultiGameObject(const std::string& name = "") :
		physicsNodes(NULL) {
		renderNode = NULL;
		friendlyName = name;
	}

	MultiGameObject(const std::string& name, RenderNode* renderNde, vector<PhysicsNode*> physicsNde)
		: physicsNodes(physicsNde) {
		renderNode = renderNde;
		friendlyName = name;

		physicsNode = physicsNde[0];

		RegisterPhysicsToRenderTransformCallback();

		for (vector<PhysicsNode*>::iterator it = physicsNde.begin(); it != physicsNde.end(); ++it)
		{
			(*it)->SetParent(this);
		}
		
	}

	virtual ~MultiGameObject()
	{
		if (renderNode)  GraphicsPipeline::Instance()->RemoveRenderNode(renderNode);
		
		for (int i = 0; i < physicsNodes.size(); ++i) {
			PhysicsEngine::Instance()->RemovePhysicsObject(physicsNodes[i]);
		}
		

		SAFE_DELETE(renderNode);
		SAFE_DELETE(physicsNode);
	}

	inline bool  HasPhysics() const { return (physicsNode != NULL); }
	inline const PhysicsNode*	Physics() const { return physicsNodes[0]; }
	inline		 PhysicsNode*	Physics() { return physicsNodes[0]; }
	
	virtual inline void RegisterPhysicsToRenderTransformCallback()
	{
		if (physicsNodes[0] && renderNode)
		{
			physicsNodes[0]->SetOnUpdateCallback(
				std::bind(
					&RenderNode::SetTransform,	// Function to call
					renderNode,					// Constant parameter (in this case, as a member function, we need a 'this' parameter to know which class it is)
					std::placeholders::_1)		// Variable parameter(s) that will be set by the callback function
			);
		}
	}

	inline void UnregisterPhysicsToRenderTransformCallback()
	{
		if (physicsNodes[0])
		{
			physicsNodes[0]->SetOnUpdateCallback([](const Matrix4&) {});
		}
	}

protected:
	std::vector<PhysicsNode*> physicsNodes;
	std::vector<RenderNode*> renderNodes;
};