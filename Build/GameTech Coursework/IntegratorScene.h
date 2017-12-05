
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\GameObject.h>
#include <ncltech\CommonMeshes.h>
#include <ncltech\CommonUtils.h>

class IntegratorScene : public Scene
{
public:
	IntegratorScene(const std::string& friendly_name)
		: Scene(friendly_name)
	{
		tex = SOIL_load_OGL_texture(
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

		//SetWorldRadius(10.0f);
	}

	~IntegratorScene()
	{
		//SAFE_DELETE(m_TargetMesh);
	}

	virtual void OnInitializeScene() override
	{
		Scene::OnInitializeScene();
		m_TrajectoryPoints.clear();
		
		//Create Ground
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",								//Friendly ID/Name
			Vector3(-1.25f, -0.2f, 0.0f),			//Centre Position
			Vector3(10.0f, 0.1f, 2.f),				//Scale
			true,									//Physics
			0.0f,									//Inverse Mass 
			true,									//Collidable
			false,									//Draggable
			Vector4(0.2f, 1.0f, 0.5f, 1.0f)));		//Color

		GameObject * m_target = CommonUtils::BuildCuboidObject(
			"Target",								//Friendly ID/Name
			Vector3(0.1f + 5.f, 2.0f, 0.0f),		//Centre Position
			Vector3(0.1f, 2.0f, 2.f),				//Scale
			true,									//Physics
			0.0f,									//Inverse Mass 
			true,									//Collidable
			false,									//Draggable								
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),		//Color
			tex);

		this->AddGameObject(m_target);


		//Create a projectile SYMPLETIC
		{
			m_Sphere = CommonUtils::BuildSphereObject("sphere",
				Vector3(-7.5f, 2.0f, -1.5f),
				0.5f,									//Radius
				true,									//Has Physics Object
				1.0f,
				true,									//Has Collision Shape
				false,									//Dragable by the user
				Vector4(1.0f, 0.0f, 0.0f, 1.0f));		//Color

			m_Sphere->Physics()->SetIntegrator(SYMPLETIC);
			//Position, vel and acceleration all set in "ResetScene()"
			this->AddGameObject(m_Sphere);

			//Override the default PhysicsNode update callback (so we can see and store the trajectory over time)
			m_Sphere->Physics()->SetOnUpdateCallback([&](const Matrix4& transform)
			{
				m_Sphere->Render()->SetTransform(transform); //Default callback for any object that has a render and physics nodes
				UpdateTrajectory(transform.GetPositionVector(), &m_TrajectoryPoints); //Our cheeky injection to store physics engine position updates
			});
		}

		//Create a projectile RK2
		{
			m_SphereRK2 = CommonUtils::BuildSphereObject("sphereRK2",
				Vector3(-7.5f, 2.0f, 0.f),
				0.5f,									//Radius
				true,									//Has Physics Object
				1.0f,
				true,									//Has Collision Shape
				false,									//Dragable by the user
				Vector4(0.0f, 1.0f, 0.0f, 1.0f));		//Color

			m_SphereRK2->Physics()->SetIntegrator(RK2);
			//Position, vel and acceleration all set in "ResetScene()"
			this->AddGameObject(m_SphereRK2);

			//Override the default PhysicsNode update callback (so we can see and store the trajectory over time)
			m_SphereRK2->Physics()->SetOnUpdateCallback([&](const Matrix4& transform)
			{
				m_SphereRK2->Render()->SetTransform(transform); //Default callback for any object that has a render and physics nodes
				UpdateTrajectory(transform.GetPositionVector(), &m_TrajectoryPointsRK2); //Our cheeky injection to store physics engine position updates
			});
		}

		//Create a projectile RK4
		{
			m_SphereRK4 = CommonUtils::BuildSphereObject("sphereRK4",
				Vector3(-7.5f, 2.0f, 1.5f),
				0.5f,									//Radius
				true,									//Has Physics Object
				1.0f,
				true,									//Has Collision Shape
				false,									//Dragable by the user
				Vector4(0.0f, 0.0f, 1.0f, 1.0f));		//Color

			m_SphereRK4->Physics()->SetIntegrator(RK4);
			//Position, vel and acceleration all set in "ResetScene()"
			this->AddGameObject(m_SphereRK4);

			//Override the default PhysicsNode update callback (so we can see and store the trajectory over time)
			m_SphereRK4->Physics()->SetOnUpdateCallback([&](const Matrix4& transform)
			{
				m_SphereRK4->Render()->SetTransform(transform); //Default callback for any object that has a render and physics nodes
				UpdateTrajectory(transform.GetPositionVector(), &m_TrajectoryPointsRK4); //Our cheeky injection to store physics engine position updates
			});
		}

		//Setup starting values
		ResetScene(PhysicsEngine::Instance()->GetUpdateTimestep());

		Scene::OnInitializeScene();

		PhysicsEngine::Instance()->SetLimits(Vector3(-10, -5, -10), Vector3(10, 25, 10));
	}

	void ResetScene(float timestep)
	{
		PhysicsEngine::Instance()->SetUpdateTimestep(timestep);
		PhysicsEngine::Instance()->SetPaused(false);

		m_TrajectoryPoints.clear();
		m_TrajectoryPointsRK2.clear();
		m_TrajectoryPointsRK4.clear();

		m_Sphere->Physics()->SetPosition(Vector3(-7.5f, 2.0f, -1.5f));
		m_Sphere->Physics()->SetLinearVelocity(Vector3(3.0f, 2.0f, 0.0f));
		m_Sphere->Physics()->SetOrientation(Quaternion());
		m_Sphere->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));

		m_SphereRK2->Physics()->SetPosition(Vector3(-7.5f, 2.0f, 0.f));
		m_SphereRK2->Physics()->SetLinearVelocity(Vector3(3.0f, 2.0f, 0.0f));
		m_SphereRK2->Physics()->SetOrientation(Quaternion());
		m_SphereRK2->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));

		m_SphereRK4->Physics()->SetPosition(Vector3(-7.5f, 2.0f, 1.5f));
		m_SphereRK4->Physics()->SetLinearVelocity(Vector3(3.0f, 2.0f, 0.0f));
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
	GameObject*				m_Sphere;
	GameObject*				m_SphereRK2;
	GameObject*				m_SphereRK4;
	std::vector<Vector3>	m_TrajectoryPoints;
	std::vector<Vector3>	m_TrajectoryPointsRK2;
	std::vector<Vector3>	m_TrajectoryPointsRK4;
	GLuint					tex;
};