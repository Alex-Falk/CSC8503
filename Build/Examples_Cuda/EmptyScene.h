#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\SoftBody.h>
#include "CudaSoftBody.cuh"

//Fully striped back scene to use as a template for new scenes.
class EmptyScene : public Scene
{
public:
	EmptyScene(const std::string& friendly_name) 
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
	}

	virtual ~EmptyScene()
	{
	}

	virtual void OnInitializeScene() override
	{
		Scene::OnInitializeScene();
		PhysicsEngine::Instance()->SetGravity(Vector3(0,-1.0f,0));
		PhysicsEngine::Instance()->SetOctreeMinSize(1);

		//Who doesn't love finding some common ground?
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, 0.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		this->AddGameObject(CommonUtils::BuildSphereObject("Floating_sphere",
			Vector3(0,10,0),
			2.0f,									//Radius
			true,									//Has Physics Object
			0.0f,									//Inverse Mass
			true,									//Has Collision Shape
			true,									//Dragable by the user
			CommonUtils::GenColor(0.5, 0.8f)));	//Color
	
		cloth = new CudaSoftBody(25,25,.25,Vector3(0, 10, -10), tex);

		this->AddMultiGameObject(cloth->GetGameObject());

		Scene::OnInitializeScene();

		PhysicsEngine::Instance()->SetLimits(Vector3(-10, -5, -10), Vector3(10, 25, 10));
	}

	virtual void OnUpdateScene(float dt) override
	{
		cloth->UpdateSoftBody(PhysicsEngine::Instance()->GetDeltaTime());
	}

private:
	GLuint					tex;
	CudaSoftBody *			cloth;

};