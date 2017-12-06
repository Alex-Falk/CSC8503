#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\SoftBody.h>

#define NUM_BALLS 300
#define RAND (rand() % 101) / 100.0f

//Fully striped back scene to use as a template for new scenes.
class BallPoolScene : public Scene
{
public:
	BallPoolScene(const std::string& friendly_name)
		: Scene(friendly_name)
	{

	}

	virtual ~BallPoolScene()
	{
	}

	virtual void OnInitializeScene() override
	{
		Scene::OnInitializeScene();

		// Floor
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, 0.0f, 0.0f),
			Vector3(10.0f, 1.0f, 10.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		// Wall 1
		GameObject * wall1 = CommonUtils::BuildCuboidObject(
			"Wall1",
			Vector3(10.0f, 3.0f-1.5f, 0.0f),
			Vector3(1.0f, 4.0f, 10.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		wall1->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), -45.0f));

		this->AddGameObject(wall1);

		// Wall 2
		GameObject * wall2 = CommonUtils::BuildCuboidObject(
			"Wall2",
			Vector3(-10.0f, 3.0f - 1.5f, 0.0f),
			Vector3(1.0f, 4.0f, 10.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		wall2->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), 45.0f));

		this->AddGameObject(wall2);

		// Wall 3
		GameObject * wall3 = CommonUtils::BuildCuboidObject(
			"Wall3",
			Vector3(0.0f, 3.0f - 1.5f, 10.0f),
			Vector3(10.0f, 4.0f, 1.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		wall3->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(1.0f, 0.0f, 0.0f), 45.0f));

		this->AddGameObject(wall3);

		// Wall 4
		GameObject * wall4 = CommonUtils::BuildCuboidObject(
			"Wall4",
			Vector3(0.0f, 3.0f - 1.5f, -10.0f),
			Vector3(10.0f, 4.0f, 1.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		wall4->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(1.0f, 0.0f, 0.0f), -45.0f));

		this->AddGameObject(wall4);

		for (int i = 0; i < NUM_BALLS; ++i) {
			this->AddGameObject(CommonUtils::BuildSphereObject("spawned_sphere",
				Vector3(RAND*20.0f-10.0f,RAND*2.0f+3.0f,RAND*20.0f-10.0f),
				0.75f,									//Radius
				true,									//Has Physics Object
				1.0f / 4.0f,							//Inverse Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColor(RAND, 0.8f)));	//Color

		}
		Scene::OnInitializeScene();
		PhysicsEngine::Instance()->SetOctreeMinSize(1.0f);
		PhysicsEngine::Instance()->SetLimits(Vector3(-20, -5, -20), Vector3(20, 15, 20));
	}

private:
	GLuint					tex;

};