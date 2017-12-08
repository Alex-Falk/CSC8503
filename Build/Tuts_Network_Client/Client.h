#pragma once
#include <nclgl\Window.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\SceneManager.h>
#include <nclgl\NCLDebug.h>
#include <nclgl\PerfTimer.h>
#include <ncltech\NetworkBase.h>


class Client
{
public:
	Client() {};
	~Client();

	int ClientLoop();

	void PrintStatusEntries()
	{
		const Vector4 status_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		//Print Current Scene Name
		NCLDebug::AddStatusEntry(status_color, "[%d/%d]: %s ([T]/[Y] to cycle or [R] to reload)",
			SceneManager::Instance()->GetCurrentSceneIndex() + 1,
			SceneManager::Instance()->SceneCount(),
			SceneManager::Instance()->GetCurrentScene()->GetSceneName().c_str()
		);
	}

	void HandleKeyboardInputs()
	{
		uint sceneIdx = SceneManager::Instance()->GetCurrentSceneIndex();
		uint sceneMax = SceneManager::Instance()->SceneCount();
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Y))
			SceneManager::Instance()->JumpToScene((sceneIdx + 1) % sceneMax);

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T))
			SceneManager::Instance()->JumpToScene((sceneIdx == 0 ? sceneMax : sceneIdx) - 1);

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R))
			SceneManager::Instance()->JumpToScene(sceneIdx);
	}

	void ProcessNetworkEvent(const ENetEvent& evnt);
};