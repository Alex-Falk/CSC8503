#pragma once

#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "ClientFunctions.h"

#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#define SERVER_PORT 1234
#define UPDATE_TIMESTEP 1.0f//(1.0f / 30.0f) //send 30 position updates per second
#define OUT_OF_RANGE -1

void Win32_PrintAllAdapterIPAddresses();

class Server
{
protected:
	NetworkBase * server;
	GameTimer timer;
	float accum_time = 0.0f;
	float rotation = 0.0f;

	// Maze & Pathfinding
	MazeGenerator * generator;

	SearchAStar* search_as;
	int astar_preset_idx;
	std::string astar_preset_text;
	vector<std::list<const GraphNode*>> paths;

	// Client Avatar position indeces
	vector<int> avatars;

public:
	Server() {
		server = new NetworkBase();
		printf("Server Initiated\n");

		Win32_PrintAllAdapterIPAddresses();

		timer.GetTimedMS();
		generator = new MazeGenerator();
		search_as = new SearchAStar();
		generator->Generate(10, 0.7f);
	};
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

	// Can either broadcast or send individually
	void SendWalls			(int i = OUT_OF_RANGE);
	void SendStartPositions	(int i = OUT_OF_RANGE);
	void SendEndPositions	(int i = OUT_OF_RANGE);
	void SendAvatarPositions(int i = OUT_OF_RANGE);

	// Sent individually
	void FollowPath			(int i);
	void SendPath			(int i);
	void UpdateAStarPreset	(int i);

	// Broadcasts
	void SendNumberClients	();
	void NewUser			(int i);



};

