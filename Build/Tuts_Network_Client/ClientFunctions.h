#pragma once
#include <ncltech\PhysicsEngine.h>
#include <ncltech\SceneManager.h>
#include <nclgl\NCLDebug.h>
#include <nclgl\PerfTimer.h>
#include <ncltech\NetworkBase.h>

// Functions used mainly by the client but that could be used by the server

class Net1_Client;

struct MazeStruct {
	int size;
	float density;
	vector<int> walls;
};

// Struct holding the position and related maze index
struct PosStruct {
	int idx;
	Vector3 pos;
};

void PrintStatusEntries();
void HandleKeyboardInputs(Net1_Client * c);
int ClientLoop(Net1_Client * c);

// Functions that handle incoming packets from the server
MazeStruct Recieve_maze(const ENetEvent& evnt);
PosStruct Recieve_pos(const ENetEvent& evnt);
vector<PosStruct> Recieve_positions(const ENetEvent& evnt);
vector<int> Recieve_path(const ENetEvent& evnt);

// Helper functions to interpret string into either a vector of strings, ints or floats
vector<string> split_string(string s, char d);
vector<int> split_string_toInt(string s, char d);
vector<float> split_string_toFloat(string s, char d);
bool isNumber(char c);