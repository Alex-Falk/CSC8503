
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SpringConstraint.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\NCLDebug.h>

class Phy3_Constraints : public Scene
{
public:
	Phy3_Constraints(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	virtual void OnInitializeScene() override
	{
		//Create Ground (..why not?)
		GameObject* ground = CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, 0.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));

		this->AddGameObject(ground);

		GameObject *handle, *ball;

		//Create Hanging Ball
			handle = CommonUtils::BuildSphereObject("handle",
				Vector3(-7.f, 7.f, -5.0f),				//Position
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				false,									//No Collision Shape Yet
				true,									//Dragable by the user
				CommonUtils::GenColor(0.45f, 0.5f));	//Color

			ball = CommonUtils::BuildSphereObject("ball",
				Vector3(-7.f, 4.f, -5.0f),				//Position
				0.5f,									//Radius
				true,									//Has Physics Object
				1.0f,									// Inverse Mass = 1 / 1kg mass
				false,									//No Collision Shape Yet
				true,									//Dragable by the user
				CommonUtils::GenColor(0.5f, 1.0f));		//Color

			this->AddGameObject(handle);
			this->AddGameObject(ball);

			//Add distance constraint between the two objects
			SpringConstraint* constraint = new SpringConstraint(
				handle->Physics(),					//Physics Object A
				ball->Physics(),					//Physics Object B
				handle->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
				ball->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
			PhysicsEngine::Instance()->AddConstraint(constraint);
		




		//Create Hanging Cube (Attached by corner)
			handle = CommonUtils::BuildSphereObject("handle2",
				Vector3(4.f, 7.f, -5.0f),				//Position
				1.0f,									//Radius
				true,									//Has Physics Object
				0.1f,									//Inverse Mass = 1 / 10 kg mass (For creating rotational inertia tensor)
				false,									//No Collision Shape Yet
				true,									//Dragable by the user
				CommonUtils::GenColor(0.55f, 0.5f));	//Color

			//Set linear mass to be infinite, so it can rotate still but not move
			handle->Physics()->SetInverseMass(0.0f);

			ball = CommonUtils::BuildCuboidObject("cube",
				Vector3(7.f, 7.f, -5.0f),				//Position
				Vector3(0.5f, 0.5f, 0.5f),				//Half Dimensions
				true,									//Has Physics Object
				1.0f,									//Inverse Mass = 1 / 1kg mass
				false,									//No Collision Shape Yet
				true,									//Dragable by the user
				CommonUtils::GenColor(0.6f, 1.0f));		//Color

			this->AddGameObject(handle);
			this->AddGameObject(ball);

			PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(
				handle->Physics(),													//Physics Object A
				ball->Physics(),													//Physics Object B
				handle->Physics()->GetPosition() + Vector3(1.0f, 0.0f, 0.0f),		//Attachment Position on Object A	-> Currently the far right edge
				ball->Physics()->GetPosition() + Vector3(-0.5f, -0.5f, -0.5f)));	//Attachment Position on Object B	-> Currently the far left edge 
		
			Scene::OnInitializeScene();

	}

	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD1))	PhysicsEngine::Instance()->SetIntegrator(SYMPLETIC);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD2))	PhysicsEngine::Instance()->SetIntegrator(RK2);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_NUMPAD3))	PhysicsEngine::Instance()->SetIntegrator(RK4);

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
		{
			//Create a projectile
			RenderNode* sphereRenderSpawn = new RenderNode();
			sphereRenderSpawn->SetMesh(CommonMeshes::Sphere());
			sphereRenderSpawn->SetTransform(Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f))); //No position! That is now all handled in PhysicsNode
			sphereRenderSpawn->SetColor(Vector4(0.5f, 0.2f, 0.5f, 1.0f));
			sphereRenderSpawn->SetBoundingRadius(1.0f);

			GameObject* m_SphereSpawn = new GameObject("Sphere2");
			m_SphereSpawn->SetRender(new RenderNode());
			m_SphereSpawn->Render()->AddChild(sphereRenderSpawn);
			m_SphereSpawn->SetPhysics(new PhysicsNode());
			m_SphereSpawn->Physics()->SetInverseMass(1.f);
			//Position, vel and acceleration all set in "ResetScene()"
			this->AddGameObject(m_SphereSpawn);

			m_SphereSpawn->Physics()->SetPosition(GraphicsPipeline::Instance()->GetCamera()->GetPosition() + GraphicsPipeline::Instance()->GetCamera()->GetViewDirection().Normalise()*20.0f);
			m_SphereSpawn->Physics()->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
			//m_SphereSpawn->Physics()->SetForce(GraphicsPipeline::Instance()->GetCamera()->GetViewDirection().Normalise()*100.0f);

			//Cause we can.. we will also spin the ball 1 revolution per second (5 full spins before hitting target)
			// - Rotation is in radians (so 2PI is 360 degrees), richard has provided a DegToRad() function in <nclgl\common.h> if you want as well.
			m_SphereSpawn->Physics()->SetOrientation(Quaternion());
			m_SphereSpawn->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));

			PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(
				this->FindGameObject("ball")->Physics(),							//Physics Object A
				m_SphereSpawn->Physics(),											//Physics Object B
				this->FindGameObject("ball")->Physics()->GetPosition(),				//Attachment Position on Object A	-> Currently the far right edge
				m_SphereSpawn->Physics()->GetPosition()));							//Attachment Position on Object B	-> Currently the far left edge 
		}
	}

};