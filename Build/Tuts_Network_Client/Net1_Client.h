
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "ClientFunctions.h"
#include "MazeRenderer.h"
#include <nclgl\OBJMesh.h>

//Basic Network Example

class Net1_Client : public Scene
{
public:
	Net1_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;
	virtual bool foundServer() { return (serverConnection != NULL); }

	void ProcessNetworkEvent(const ENetEvent& evnt);

	void ApplyMaze(MazeStruct m);

	void SendStartPosition();
	void SendEndPosition();
	void RequestNewMaze();
	ENetPeer* getPeer() { return serverConnection; }

protected:
	GameObject* box;

	NetworkBase network;
	ENetPeer*	serverConnection;
	MazeGenerator* generator;
	MazeRenderer* maze;

	Mesh* wallmesh;
	int size;
	std::list<const GraphNode*> path;
};