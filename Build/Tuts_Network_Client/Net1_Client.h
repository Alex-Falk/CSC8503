
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "ClientFunctions.h"
#include "MazeRenderer.h"
#include <nclgl\OBJMesh.h>


#define ID serverConnection->outgoingPeerID
#define OUT_OF_RANGE -1
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
	void SendAvatarLocation();
	Vector3 Pos_To_Maze(Vector3 pos);
	Vector3 Maze_Scale();
	void ClickableLocationCallback(int idx);
	ENetPeer* getPeer() { return serverConnection; }

protected:
	GameObject* box;

	NetworkBase network;
	ENetPeer*	serverConnection;
	MazeGenerator* generator;
	MazeRenderer* maze;

	Mesh* wallmesh;
	int size = 10;
	float density = 0.5f;
	bool drawPath = false;
	bool drawMesh = false;
	int clients;
	std::list<const GraphNode*> path;

	vector<int> avatars;
};