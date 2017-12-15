#pragma once

#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "ClientFunctions.h"
#include <ncltech\CommonUtils.h>
#include "Hazard.h"

#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

// Class which handles everything on the serverside

#define HAZARD_NUM 3			//number of hazards
#define SERVER_PORT 1234
#define UPDATE_TIMESTEP 1.0f/30.0f//(1.0f / 30.0f) //send 30 position updates per second

void Win32_PrintAllAdapterIPAddresses();

class Server
{
protected:
	NetworkBase * server;
	GameTimer timer;
	float accum_time = 0.0f;
	float rotation = 0.0f;
	vector<float> lerp_factor;

	// Maze & Pathfinding
	MazeGenerator * generator;

	SearchAStar* search_as;
	int astar_preset_idx;
	std::string astar_preset_text;
	vector<std::list<const GraphNode*>> paths;

	// Client Avatar position indeces
	vector<int> avatars;
	vector<PhysicsNode *> avatar_obj;

	vector<Hazard*> hazards;
public:
	Server();
	~Server()
	{
		server->Release();
		system("pause");
		exit(0);
	};

	int ServerLoop();

	NetworkBase * getBase() { return server; }

	//--------------------------------------------------------------------------------------------//
	// Sending / Broadcasting methods
	//--------------------------------------------------------------------------------------------//

	void InitializeArrayElements(int id);

	void UpdateHazards();

	void ResetHazards();

	Vector3 InterpolatePositionLinear(Vector3 posA, Vector3 posB, float factor);

	//Callback that handles collissions between avatars (or an avatar and another object)
	bool ColissionCallback(PhysicsNode * self, PhysicsNode * other, int self_idx);

	// Can either broadcast or send individually
	void SendWalls			(int i = OUT_OF_RANGE);
	void SendAvatarPositions(int i = OUT_OF_RANGE);
	void SendHazardPosition	(int i = OUT_OF_RANGE);

	// Sent individually
	void FollowPath				(int i);
	void StartFollowing(int i);
	void SendPath				(int i);
	void UpdateAStarPreset		(int i);
	std::list<const GraphNode*> StringPulling(int i);

	bool Check_los(int start, int end);

	// Broadcasts
	void NewUser			(int i);



};

