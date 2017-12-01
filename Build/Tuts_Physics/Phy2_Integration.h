
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\GameObject.h>
#include <ncltech\CommonMeshes.h>
#include <ncltech\CommonUtils.h>

class Phy2_Integration : public Scene
{
public:
	Phy2_Integration(const std::string& friendly_name)
		: Scene(friendly_name)
	{
		GLuint tex = SOIL_load_OGL_texture(
			TEXTUREDIR"target.tga",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		Mesh* cube = CommonMeshes::Cube();
		m_TargetMesh = new Mesh(*cube);
		m_TargetMesh->SetTexture(tex);
		//SetWorldRadius(10.0f);
	}

	~Phy2_Integration()
	{
		SAFE_DELETE(m_TargetMesh);
	}

	virtual void OnInitializeScene() override
	{
		Scene::OnInitializeScene();
		m_TrajectoryPoints.clear();

	//Set Defaults
		PhysicsEngine::Instance()->SetGravity(Vector3(0.0f, 0.0f, 0.0f));		//No Gravity!
		PhysicsEngine::Instance()->SetDampingFactor(1.0f);						//No Damping!



	//Create Ground
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",								//Friendly ID/Name
			Vector3(-1.25f, -0.2f, 0.0f),			//Centre Position
			Vector3(10.0f, 0.1f, 2.f),				//Scale
			false,									//No Physics Yet
			0.0f,									//No Physical Mass Yet
			false,									//No Collision Shape 
			false,									//Not Dragable By the user
			Vector4(0.2f, 1.0f, 0.5f, 1.0f)));		//Color


	//Create Target (Purely graphical)
		RenderNode* target = new RenderNode();
		target->SetMesh(m_TargetMesh);
		target->SetTransform(Matrix4::Translation(Vector3(0.1f + 5.f, 2.0f, 0.0f)) * Matrix4::Scale(Vector3(0.1f, 2.0f, 2.f)));
		target->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		target->SetBoundingRadius(4.0f);
		this->AddGameObject(new GameObject("Target", target, NULL));


	//Create a projectile
		RenderNode* sphereRender = new RenderNode();
		sphereRender->SetMesh(CommonMeshes::Sphere());
		sphereRender->SetTransform(Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f))); //No position! That is now all handled in PhysicsNode
		sphereRender->SetBaseColor(Vector4(1.0f, 0.2f, 0.5f, 1.0f));
		sphereRender->SetBoundingRadius(1.0f);

		m_Sphere = new GameObject("Sphere");
		m_Sphere->SetRender(new RenderNode());
		m_Sphere->Render()->AddChild(sphereRender);
		m_Sphere->SetPhysics(new PhysicsNode(SYMPLETIC));
		m_Sphere->Physics()->SetInverseMass(1.f);
		//Position, vel and acceleration all set in "ResetScene()"
		this->AddGameObject(m_Sphere);

		//Override the default PhysicsNode update callback (so we can see and store the trajectory over time)
		m_Sphere->Physics()->SetOnUpdateCallback([&](const Matrix4& transform)
		{
			m_Sphere->Render()->SetTransform(transform); //Default callback for any object that has a render and physics nodes
			UpdateTrajectory(transform.GetPositionVector(), &m_TrajectoryPoints); //Our cheeky injection to store physics engine position updates
		});

		//Create a projectile
		RenderNode* sphereRenderRK2 = new RenderNode();
		sphereRenderRK2->SetMesh(CommonMeshes::Sphere());
		sphereRenderRK2->SetTransform(Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f))); //No position! That is now all handled in PhysicsNode
		sphereRenderRK2->SetBaseColor(Vector4(0.2f, 1.0f, 0.5f, 1.0f));
		sphereRenderRK2->SetBoundingRadius(1.0f);

		m_SphereRK2 = new GameObject("SphereRK2");
		m_SphereRK2->SetRender(new RenderNode());
		m_SphereRK2->Render()->AddChild(sphereRenderRK2);
		m_SphereRK2->SetPhysics(new PhysicsNode(RK2));
		m_SphereRK2->Physics()->SetInverseMass(1.f);
		//Position, vel and acceleration all set in "ResetScene()"
		this->AddGameObject(m_SphereRK2);

		//Override the default PhysicsNode update callback (so we can see and store the trajectory over time)
		m_SphereRK2->Physics()->SetOnUpdateCallback([&](const Matrix4& transform)
		{
			m_SphereRK2->Render()->SetTransform(transform); //Default callback for any object that has a render and physics nodes
			UpdateTrajectory(transform.GetPositionVector(), &m_TrajectoryPointsRK2); //Our cheeky injection to store physics engine position updates
		});

		//Create a projectile
		RenderNode* sphereRenderRK4 = new RenderNode();
		sphereRenderRK4->SetMesh(CommonMeshes::Sphere());
		sphereRenderRK4->SetTransform(Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f))); //No position! That is now all handled in PhysicsNode
		sphereRenderRK4->SetBaseColor(Vector4(0.5f, 0.2f, 1.0f, 1.0f));
		sphereRenderRK4->SetBoundingRadius(1.0f);

		m_SphereRK4 = new GameObject("SphereRK4");
		m_SphereRK4->SetRender(new RenderNode());
		m_SphereRK4->Render()->AddChild(sphereRenderRK4);
		m_SphereRK4->SetPhysics(new PhysicsNode(RK4));
		m_SphereRK4->Physics()->SetInverseMass(1.f);
		//Position, vel and acceleration all set in "ResetScene()"
		this->AddGameObject(m_SphereRK4);

		//Override the default PhysicsNode update callback (so we can see and store the trajectory over time)
		m_SphereRK4->Physics()->SetOnUpdateCallback([&](const Matrix4& transform)
		{
			m_SphereRK4->Render()->SetTransform(transform); //Default callback for any object that has a render and physics nodes
			UpdateTrajectory(transform.GetPositionVector(), &m_TrajectoryPointsRK4); //Our cheeky injection to store physics engine position updates
		});

	//Setup starting values
		ResetScene(PhysicsEngine::Instance()->GetUpdateTimestep());
	}

	void ResetScene(float timestep)
	{
		PhysicsEngine::Instance()->SetUpdateTimestep(timestep);
		PhysicsEngine::Instance()->SetPaused(false);
		
		m_TrajectoryPoints.clear();
		m_TrajectoryPointsRK2.clear();
		m_TrajectoryPointsRK4.clear();

		//These values were worked out analytically by
		// doing the integration over time. The ball (if everything works)
		// should take 5 seconds to arc into the centre of the target.
		m_Sphere->Physics()->SetPosition(Vector3(-7.5f, 2.0f, -1.5f));
		m_Sphere->Physics()->SetLinearVelocity(Vector3(3.0f, 2.0f, 0.0f));
		m_Sphere->Physics()->SetForce(Vector3(0.0f, -1.0f, 0.0f));

		//Cause we can.. we will also spin the ball 1 revolution per second (5 full spins before hitting target)
		// - Rotation is in radians (so 2PI is 360 degrees), richard has provided a DegToRad() function in <nclgl\common.h> if you want as well.
		m_Sphere->Physics()->SetOrientation(Quaternion());
		m_Sphere->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));

		m_SphereRK2->Physics()->SetPosition(Vector3(-7.5f, 2.0f, 0.f));
		m_SphereRK2->Physics()->SetLinearVelocity(Vector3(3.0f, 2.0f, 0.0f));
		m_SphereRK2->Physics()->SetForce(Vector3(0.0f, -1.0f, 0.0f));
		m_SphereRK2->Physics()->SetOrientation(Quaternion());
		m_SphereRK2->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));

		m_SphereRK4->Physics()->SetPosition(Vector3(-7.5f, 2.0f, 1.5f));
		m_SphereRK4->Physics()->SetLinearVelocity(Vector3(3.0f, 2.0f, 0.0f));
		m_SphereRK4->Physics()->SetForce(Vector3(0.0f, -1.0f, 0.0f));
		m_SphereRK4->Physics()->SetOrientation(Quaternion());
		m_SphereRK4->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));

	}

	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

	//Status Output:-
		NCLDebug::AddStatusEntry(
			Vector4(1.0f, 0.9f, 0.8f, 1.0f),
			"Physics Timestep: %5.2fms [%5.2ffps]",
			PhysicsEngine::Instance()->GetUpdateTimestep() * 1000.0f,
			1.0f / PhysicsEngine::Instance()->GetUpdateTimestep()
		);

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Select Integration Timestep:");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    [1] 5fps");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    [2] 15fps");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    [3] 30fps");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    [4] 60fps");

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))	ResetScene(1.0f / 5.0f);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))	ResetScene(1.0f / 15.0f);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3))	ResetScene(1.0f / 30.0f);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4))	ResetScene(1.0f / 60.0f);
	
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

			m_SphereSpawn->Physics()->SetPosition(GraphicsPipeline::Instance()->GetCamera()->GetPosition());
			m_SphereSpawn->Physics()->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
			m_SphereSpawn->Physics()->SetForce(GraphicsPipeline::Instance()->GetCamera()->GetViewDirection().Normalise());

			//Cause we can.. we will also spin the ball 1 revolution per second (5 full spins before hitting target)
			// - Rotation is in radians (so 2PI is 360 degrees), richard has provided a DegToRad() function in <nclgl\common.h> if you want as well.
			m_SphereSpawn->Physics()->SetOrientation(Quaternion());
			m_SphereSpawn->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));
		}

	//Draw the trajectory:-	
		const Vector4 cols[2] = {
			Vector4(1.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f)
		};
		for (size_t i = 1; i < m_TrajectoryPoints.size(); i++)
		{
			NCLDebug::DrawThickLine(
				m_TrajectoryPoints[i - 1],
				m_TrajectoryPoints[i],
				0.01f,
				cols[i % 2]
			);
		}
		for (size_t i = 1; i < m_TrajectoryPointsRK2.size(); i++)
		{
			NCLDebug::DrawThickLine(
				m_TrajectoryPointsRK2[i - 1],
				m_TrajectoryPointsRK2[i],
				0.01f,
				cols[i % 2]
			);
		}
		for (size_t i = 1; i < m_TrajectoryPointsRK4.size(); i++)
		{
			NCLDebug::DrawThickLine(
				m_TrajectoryPointsRK4[i - 1],
				m_TrajectoryPointsRK4[i],
				0.01f,
				cols[i % 2]
			);
		}

	}

	void UpdateTrajectory(const Vector3& pos, vector<Vector3> * trajPoints)
	{
		trajPoints->push_back(pos);
	}

private:
	Mesh*					m_TargetMesh;
	GameObject*				m_Sphere;
	GameObject*				m_SphereRK2;
	GameObject*				m_SphereRK4;
	std::vector<Vector3>	m_TrajectoryPoints;
	std::vector<Vector3>	m_TrajectoryPointsRK2;
	std::vector<Vector3>	m_TrajectoryPointsRK4;
};