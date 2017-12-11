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
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second

void Win32_PrintAllAdapterIPAddresses();

class Server
{
protected:
	NetworkBase * server;
	GameTimer timer;
	float accum_time = 0.0f;
	float rotation = 0.0f;

	MazeGenerator * generator;

	SearchAStar* search_as;
	int astar_preset_idx;
	std::string astar_preset_text;

public:
	Server() {
		server = new NetworkBase();
		printf("Server Initiated\n");

		Win32_PrintAllAdapterIPAddresses();

		timer.GetTimedMS();
		generator = new MazeGenerator();
		search_as = new SearchAStar();
		generator->Generate(10, 0.7f);
		UpdateAStarPreset();
	};
	~Server()
	{
		server->Release();
		system("pause");
		exit(0);
	};

	int ServerLoop();

	NetworkBase * getBase() { return server; }

	void SendWalls();

	void SendPath();

	void SendStartPosition();

	void SendEndPosition();

	void UpdateAStarPreset();
};

