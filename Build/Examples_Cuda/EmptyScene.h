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
			TEXTUREDIR"tileable-fabric-textures-1.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		earth = SOIL_load_OGL_texture(
			TEXTUREDIR"BeachBallColor.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		glBindTexture(GL_TEXTURE_2D, earth);
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

		GameObject* sphere;
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 5; ++j) {
				sphere = (CommonUtils::BuildSphereObject("Floating_sphere",
					Vector3((i-2.5f), 10+2*(rand() % 11 / 10.0f), (j-2.5f)),
					0.5f,									//Radius
					true,									//Has Physics Object
					0.0f,									//Inverse Mass
					true,									//Has Collision Shape
					true,									//Dragable by the user
					Vector4(1, 1, 1, 1)));	//Color

				sphere->Physics()->SetBoundingRadius(0.55f);
				sphere->Physics()->SetOrientation(Quaternion(Vector3(rand() % 11 / 10.0f, rand() % 11 / 10.0f, rand() % 11 / 10.0f), 1));
				sphere->Render()->GetChild()->GetMesh()->SetTexture(earth);

				this->AddGameObject(sphere);
			}
		}
		


		cloth = new CudaSoftBody(120,120,.05,Vector3(-3,15,-3), tex);
		
		GameObject * go = new GameObject("CudaCloth", cloth->GetRenderNode());
		this->AddGameObject(go);

		Scene::OnInitializeScene();

		PhysicsEngine::Instance()->SetLimits(Vector3(-10, -5, -10), Vector3(10, 25, 10));
	}

	virtual void OnUpdateScene(float dt) override
	{
		physicsnodes.clear();
		for (std::vector<GameObject*>::iterator it = m_vpObjects.begin(); it != m_vpObjects.end(); ++it) {
			if ((*it)->HasPhysics()) {
				physicsnodes.push_back((*it)->Physics());
			}
		}
		cloth->UpdateSoftBody(PhysicsEngine::Instance()->GetDeltaTime(),physicsnodes);
	}

private:
	GLuint					tex;
	GLuint					earth;
	CudaSoftBody *			cloth;
	vector<PhysicsNode *>	physicsnodes;

};